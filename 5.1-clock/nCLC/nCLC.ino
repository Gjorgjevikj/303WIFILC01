// nCLC.ino - alternative firmware for 303WIFILC01: basic NTP clock
// board: Generic ESP8266 module
#define VERSION "1.1"

#include <coredecls.h> // Only needed for settimeofday_cb()
#include <core_version.h> // ARDUINO_ESP8266_RELEASE
#include <time.h>
#include "Cfg.h" // A library that lets a user configure an ESP8266 app (https://github.com/maarten-pennings/Cfg)
#include "led.h"
#include "but.h"
#include "disp.h"
#include "wifi.h"

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define OTA_PASSWORD "IoTOTA"
#define OTA_TIMEOUT 300000

NvmField cfg_fields[] = {
  {"Access points"   , ""                           ,  0, "The clock uses internet to get time. Supply credentials for one or more WiFi access points (APs). " },
  {"Ssid.1"          , "SSID for AP1"               , 32, "The ssid of the first wifi network the clock could connect to (mandatory)." },
  {"Password.1"      , "Password for AP1"           , 32, "The password of the first wifi network the clock could connect to (mandatory). "},
  {"Ssid.2"          , "SSID for AP2"               , 32, "The ssid of the second wifi network (optional, may be blank)." },
  {"Password.2"      , "Password for AP2"           , 32, "The password of the second wifi network (optional, may be blank). "},
  {"Ssid.3"          , "SSID for AP3"               , 32, "The ssid of the third wifi network (optional, may be blank)." },
  {"Password.3"      , "Password for AP3"           , 32, "The password of the third wifi network (optional, may be blank). "},
  
  {"Time management" , ""                           ,  0, "Time is obtained from so-called NTP servers. They provide UTC time, so also the time-zone must be entered. " },
  {"NTP.server.1"    , "pool.ntp.org"               , 32, "The hostname of the first NTP server." },
  {"NTP.server.2"    , "europe.pool.ntp.org"        , 32, "The hostname of a second NTP server." },
  {"NTP.server.3"    , "north-america.pool.ntp.org" , 32, "The hostname of a third NTP server. " },
  {"Timezone"        , "CET-1CEST,M3.5.0,M10.5.0/3" , 48, "The timezone string (including daylight saving), see <A href='https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html'>details</A>. " },

  {"Rendering"       , ""                           ,  0, "Determines how time and date is shown on the display. " },
  {"hours"           , "24"                         ,  3, "Use <b>24</b> or <b>12</b> for 24 or 12 hour clock; append <b>a</b> or <b>p</b> to use decimal point for am or pm." },
  {"dateorder"       , "d"                          ,  2, "Use <b>d</b> for day-month (europe) or <b>m</b> month-day (US) order." },
  {"monthnames"      , "JaFeMrApMYJnJlAuSeOcNoDe"   , 24, "Supply 12 pairs of letters for month names, otherwise month will be numbered." },

  {0                 , 0                            ,  0, 0},  
};


#define CFG_BUT_PIN 0 // Button to select Config mode (mapped to the SET button)
#define CFG_LED_PIN 2 // LED to indicate Config mode (mapped to the (only) LED on the board
Cfg cfg("nCLC", cfg_fields, CFG_SERIALLVL_USR, CFG_LED_PIN);

int render_hours_len; // 12 or 24
#define RENDER_FLAG_AM 0
#define RENDER_FLAG_PM 1
#define RENDER_FLAG_NO 2
int render_hours_flag; // RENDER_FLAG_AM, RENDER_FLAG_PM or RENDER_FLAG_NO
const char * render_hours_flag_names[3] = {"am","pm","no"};
int render_dayfirst; 
const char * render_months=" 1 2 3 4 5 6 7 8 9101112";
bool ota_on = true;
bool sync = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUT2_PIN, INPUT_PULLUP);

  //  OTA on by default for 300s, holding Buttone S2 on reset will turn it off
  if (digitalRead(BUT2_PIN) == LOW) 
  {
      //Serial.printf("To Go OTA...");
      ota_on = false;
  }

  Serial.printf("\n\n");
  Serial.printf("main: Welcome to nCLC, a basic NTP clock version %s\n",VERSION);

  Serial.printf("main: Nvm %s\n",NVM_VERSION);
  Serial.printf("main: Cfg %s\n",CFG_VERSION);
  Serial.printf("main: Arduino ESP8266 " ARDUINO_ESP8266_RELEASE "\n" );
  Serial.printf("main: compiler " __VERSION__ "\n" );
  Serial.printf("main: arduino %d\n",ARDUINO );
  Serial.printf("main: compiled " __DATE__ ", " __TIME__ "\n" );

  // Identify oursleves, regardless if we go into config mode or the real app
  disp_init();
  disp_power_set(1);
  disp_show("nClC");

  // On boot: check if config button is pressed
  cfg.check(100,CFG_BUT_PIN); // Wait 100 flashes (of 50ms) for a change on pin CFG_BUT_PIN
  // if in config mode, do config setup (when config completes, it restarts the device)
  if( cfg.cfgmode() ) { cfg.setup(); return; }
  Serial.printf("main: no configuration requested, starting clock\n\n");

  // LED on
  led_init();
  led_on(); 
  but_init();

  // Preprocess config for rendering
  disp_show("NtP");
  if( cfg.getval("hours")[0]=='1' && cfg.getval("hours")[1]=='2' ) render_hours_len=12; else render_hours_len=24;
  if( cfg.getval("hours")[2]=='a' ) render_hours_flag = RENDER_FLAG_AM;
  else if( cfg.getval("hours")[2]=='p' ) render_hours_flag = RENDER_FLAG_PM;
  else render_hours_flag = RENDER_FLAG_NO;
  Serial.printf("rend: hours %d, flag %s\n",render_hours_len, render_hours_flag_names[render_hours_flag]);
  render_dayfirst = cfg.getval("dateorder")[0]!='m';
  if( strlen(cfg.getval("monthnames"))==24 ) render_months=cfg.getval("monthnames");
  Serial.printf("rend: date %s, names '%s'\n",render_dayfirst?"day:month":"month:day",render_months);

  // WiFi and NTP
  wifi_init(cfg.getval("Ssid.1"),cfg.getval("Password.1"), cfg.getval("Ssid.2"),cfg.getval("Password.2"), cfg.getval("Ssid.3"),cfg.getval("Password.3"));
  configTime( cfg.getval("Timezone"), cfg.getval("NTP.server.1"), cfg.getval("NTP.server.2"), cfg.getval("NTP.server.3"));
  Serial.printf("clk : init: %s %s %s\n", cfg.getval("NTP.server.1"), cfg.getval("NTP.server.2"), cfg.getval("NTP.server.3"));
  Serial.printf("clk : timezone: %s\n", cfg.getval("Timezone") );
  settimeofday_cb( [](){Serial.printf("clk : NTP sync\n");} );  // Pass lambda function to print SET when time is set

  if (ota_on)
  {
      Serial.println("Starting OTA...");

      ArduinoOTA.onStart([]() {
          String type;
          if (ArduinoOTA.getCommand() == U_FLASH) {
              type = "sketch";
          }
          else { // U_SPIFFS
              type = "filesystem";
              // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
              Serial.println("Start updating " + type);
          }
          sync = false;
          disp_show("OtA");
          });
      ArduinoOTA.onEnd([]() {
          Serial.println("\nEnd");
          disp_show("----");
          });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
          static int prog = 0;
          static char disp_raw[6] = "    ";
          static int dots = 0;
          const int dots_mask = 0b10000;
          int newprog = progress*200 / total ;

          int val = (progress << 3) / total;

          if (val)
          {
              if (val % 2) //odd
                  disp_raw[val >> 1] = 'I'; // '`';
              else
                  disp_raw[(val >> 1) - 1] = 'X'; // '\"';
          }

          if (newprog < prog) // reset
          {
              prog = 0;
          }
          else if (newprog - prog > 1)
          {
              prog = newprog;
              if (!dots)
                  dots = dots_mask;
              dots >>= 1;
              Serial.printf("Progress: %3u%% %d:[%s]\n", newprog, val, disp_raw);
          }
          disp_show(disp_raw, dots&0b1101);
          });

      ArduinoOTA.onError([](ota_error_t error) {
          Serial.printf("Error[%u]: ", error);
          disp_show("Err");
          if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
          else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
          else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
          else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
          else if (error == OTA_END_ERROR) Serial.println("End Failed");
          });

      char otaHostName[12];
      sprintf(otaHostName, "nCLc-%04X", (ESP.getChipId() & 0xffff));
      ArduinoOTA.setHostname(otaHostName);
      ArduinoOTA.setPassword(OTA_PASSWORD);

      ArduinoOTA.begin();
      Serial.printf("OTA Ready\nhost: %s, password: %s ", otaHostName, OTA_PASSWORD);
      //Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

  }

  // App starts running
  Serial.printf("\n");
}


// Record last received seconds, to blink colon
int       colon_prev = -1;
uint32_t  colon_msecs;


// Showing time or date
int       show_date; 


void loop() 
{

    if (ota_on && millis() > OTA_TIMEOUT)
        ota_on = false;

    if (ota_on)
    {
        ArduinoOTA.handle();
        //Serial.print(".");
    }

  // if in config mode, do config loop (when config completes, it restarts the device)
  if( cfg.cfgmode() ) { cfg.loop(); return; }

  // In normal application mode
  led_set( !wifi_isconnected() );     // LED is on when not connected
  but_scan();

  if( but_wentdown(BUT3) ) disp_brightness_set( disp_brightness_get()%8 + 1 );
  if( but_wentdown(BUT2) ) show_date = !show_date;
  
  time_t      tnow= time(NULL);       // Returns seconds (and writes to the passed pointer, when not NULL) - note `time_t` is just a `long`.
  struct tm * snow= localtime(&tnow); // Returns a struct with time fields (https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm)
              sync= snow->tm_year>120;// We miss-use "old" time as indication of "time not yet set" (year is 1900 based)

  // If seconds changed: print
  if( snow->tm_sec != colon_prev ) {
    // In `snow` the `tm_year` field is 1900 based, `tm_month` is 0 based, rest is as expected
    Serial.printf("main: %d-%02d-%02d %02d:%02d:%02d (dst=%d) %s %s\n", snow->tm_year + 1900, snow->tm_mon + 1, snow->tm_mday, snow->tm_hour, snow->tm_min, snow->tm_sec, snow->tm_isdst, sync?"":"NO NTP", ota_on?"OTA":"");
    // Record that seconds changed, for colon
    colon_prev = snow->tm_sec; 
    colon_msecs = millis();
  }

  if (sync)
  {
      char bnow[5];
      int dots = DISP_DOTNO;

      if (show_date)
      {
          if (render_dayfirst)
              sprintf(bnow, "%2d%c%c", snow->tm_mday, render_months[snow->tm_mon * 2], render_months[snow->tm_mon * 2 + 1]);
          else
              sprintf(bnow, "%c%c%2d", render_months[snow->tm_mon * 2], render_months[snow->tm_mon * 2 + 1], snow->tm_mday);
      }
      else // if (sync && !show_date) 
      {
          bool pm = snow->tm_hour >= 12;
          int  hr = snow->tm_hour % render_hours_len;
          unsigned long in_sec = millis() - colon_msecs;
          sprintf(bnow, "%2d%02d", hr, snow->tm_min);
          dots = in_sec < 500 ? DISP_DOTNO : DISP_DOTCOLON;
          if (render_hours_flag == RENDER_FLAG_AM && !pm) dots |= DISP_DOT4;
          if (render_hours_flag == RENDER_FLAG_PM && pm) dots |= DISP_DOT4;
          if (ota_on && snow->tm_sec%5==0)
          {
              if ((in_sec < 50)
                  || ((in_sec > 100) && (in_sec < 150))
                  || ((in_sec > 300) && (in_sec < 350))
                  || ((in_sec > 400) && (in_sec < 450))
                  )
                  dots |= DISP_DOT1;
          }
      }
      disp_show(bnow, dots);
  }
  
}
