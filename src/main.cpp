#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "time.h"
#include "WifiCredentials.h"

// -------- Wifi and NTP -------- //
const char *ssid = SSID;
const char *password = PASSWORD;

const char* ntpServer = "north-america.pool.ntp.org";
const long  gmtOffset_sec = -18000; // UTC-5.00 (New England)
const int   daylightOffset_sec = 3600;

// -------- OLED Display -------- //
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// TODO: 
//  - Get time from that time server thing and check accuracy every x hours/minutes/days
//    - Maybe use esp_sleep_get_wakeup_cause to see if button woke it up vs timer - only re-sync with wifi if timer wakeup?
//  - Make sure remote station can be reached at least an hour before alarm time
//    - If not, make sure alarm still goes off but display error and allow user to shut off alarm at the base
//  - Button to wake up screen to see current time, alarm time, and time left until alarm
//  - Generate code to shutoff alarm when a new alarm is set and store it until it needs to be sent to the remote station

tm timeThing() {
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(50);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  int retries = 0;
  const int maxRetries = 10;

  while (!getLocalTime(&timeinfo) && retries++ < maxRetries) {
    Serial.println("Waiting for NTP time...");
    delay(1000);
  }

  if (retries < maxRetries) {
    Serial.println("Time acquired!");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  } else {
    Serial.println("Failed to get time from NTP.");
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  return timeinfo;
}

void testThing() {
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Current time: "));
  String test = "10:43AM";
  display.println(test);

  display.print(F("Alarm: "));
  display.println("8:00AM");

  display.print("Time left: ");
  display.println("1hr 32m");

  display.display();
}

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

  testThing();
  struct tm timeNumbers = timeThing();
  Serial.println(&timeNumbers, "different place lolol %A, %B %d %Y %H:%M:%S");
}

// function for getting numpad inputs:
//  When first turning on, it sets the time and everything and then goes to deep sleep right away. Then look for a wakeup signal from any button press
//  from the keypad. You can then launch a menu system for setting the alarm. Use while loop to get all inputs to make the system wait for you to press any
//  buttons you need to (setting and confirming time)

void loop() {
  
}
