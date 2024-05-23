#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <FastLED_NeoPixel.h>
#include "airports50PWMTAC.h"
#include <WiFiManager.h>

//using namespace std;


String response="";
int DEBUG = 1; //Debug levels: 0-None, 1-Wx and LED color, 2-Incl WiFi connectivity, 3- Incl API call response, 4-Verbose;
unsigned long retryClock; 
unsigned long lastCall; 
String windDirection = "";
String windSpeed = "";
String windGusts = "";
String category = "";
String color = "";

 // Which pin on the Arduino is connected to the LEDs?
#define DATA_PIN 5
// LED brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 20

#define WIND_LIMIT 20 // Sets the maximimum wind to show VFR airport as green, else it will show yellow
#define RETRY_SPEED 50000 //Sets the minimum time between successive calls to the aviation weather API. 1000 is one second

//FastLED_NeoPixel<NUM_AIRPORTS, DATA_PIN, NEO_GRB> strip;      // <- FastLED NeoPixel version
Adafruit_NeoPixel strip(NUM_AIRPORTS, DATA_PIN, NEO_GRB);  // <- Adafruit NeoPixel version


// API
const char* host = "aviationweather.gov";
const char* url = "/api/data/dataserver?requestType=retrieve&dataSource=metars&hoursBeforeNow=2&mostRecent=true&format=xml&stationString=";
const int httpsPort = 443;

void setup() {
  //Get connected
  Serial.begin(115200);

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //wm.resetSettings();
  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("LEDWeatherMap"); // anonymous ap
  //res = wm.autoConnect("WeatherMap","Muppet"); // password protected ap
  if(!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
  //The following bit prolly doesn't do anything now, since I added wifimanager (but I am too lazy to test and remove it)
  Serial.println("");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Initialize LEDs
	strip.begin();  // initialize strip (required!)
	strip.setBrightness(BRIGHTNESS);
  for (int i=0; i<NUM_AIRPORTS; i++) {
    strip.setPixelColor(i,strip.Color(0,0,0));
  }
  //Indicate status with LEDs
  strip.setPixelColor(0, strip.Color(180,180,180)); //set first pixel to white to indicate loading
  strip.show();

  //Debug print out wifi connection
  if (DEBUG>1) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  lastCall = millis();
}

void assignKeyColor(int indexID) {
  //Doing this the hard way with two arrays so that we can output the settings to serial for debug

  if      (airports[indexID] == "VFR")  color = ("GREEN");
  else if (airports[indexID] == "MVFR") color = ("BLUE");
  else if (airports[indexID] == "IFR")  color = ("RED");
  else if (airports[indexID] == "LIFR") color = ("MAGENTA");
  else if (airports[indexID] == "WVFR") color = ("YELLOW");
  else if (airports[indexID] == "NULL") color = ("BLACK");   

  if      (airports[indexID] == "VFR")  strip.setPixelColor(indexID, strip.Color(255,0,0));  //GREEN
  else if (airports[indexID] == "MVFR") strip.setPixelColor(indexID, strip.Color(0,0,255));  //BLUE
  else if (airports[indexID] == "IFR")  strip.setPixelColor(indexID, strip.Color(0,255,0));  //RED
  else if (airports[indexID] == "LIFR") strip.setPixelColor(indexID, strip.Color(0,255,255));//MAGENTA
  else if (airports[indexID] == "WVFR") strip.setPixelColor(indexID, strip.Color(255,255,0));//YELLOW
  else if (airports[indexID] == "NULL") strip.setPixelColor(indexID, strip.Color(0,0,0));    //BLACK
  strip.show();
}

void setLedColor(int index, String conditions, int gusts) {

  if      (conditions == "MVFR") color = ("BLUE");
  else if (conditions == "IFR")  color = ("RED");
  else if (conditions == "LIFR") color = ("MAGENTA");
  else if (conditions == "VFR" && gusts > WIND_LIMIT) color = ("YELLOW");
  else if (conditions == "VFR" && gusts < WIND_LIMIT) color = ("GREEN");   

  if      (conditions == "MVFR")                          strip.setPixelColor(index, strip.Color(0,0,255));  //BLUE
  else if (conditions == "IFR")                           strip.setPixelColor(index, strip.Color(0,255,0));  //RED
  else if (conditions == "LIFR")                          strip.setPixelColor(index, strip.Color(0,255,255));//MAGENTA
  else if ((conditions == "VFR") && (gusts > WIND_LIMIT)) strip.setPixelColor(index, strip.Color(255,255,0));//YELLOW
  else if ((conditions == "VFR") && (gusts < WIND_LIMIT)) strip.setPixelColor(index, strip.Color(255,0,0));  //GREEN

  strip.show();
}

void getWX(int indexVal) {

  response = "";
  windDirection = "";
  windSpeed = "";
  windGusts = "";
  category = "";
  color = "";

  //Connect to WIFI
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;
    String query = String(host) + String(url) + airports[indexVal];
    String queryString = "https://" + query;
    if (https.begin(client, queryString)) {
      int httpCode = https.GET();
      response = https.getString();

      if (DEBUG >2) {
        Serial.println();
        Serial.println("==================================");
        Serial.print("GET: ");
        Serial.println(query);
        Serial.print("Response code: ");
        Serial.println(String(httpCode));
        if (DEBUG>3) {
          Serial.print("Received: "); 
          Serial.println(response);
        }
      }

      https.end();
    } 
  }
  
  if (response.indexOf("<METAR>")>0) {

    // Find the station_id tag
    int stationStart = response.indexOf("<station_id>");
    int stationEnd   = response.indexOf("</station_id>", stationStart);
    String station   = response.substring(stationStart + 12, stationEnd);

    // Find the wind_dir_degrees tag
    int winddirStart = response.indexOf("<wind_dir_degrees>");
    int winddirEnd   = response.indexOf("</wind_dir_degrees>", winddirStart);
    windDirection    = response.substring(winddirStart + 18, winddirEnd);

    // Find the wind_speed_kt tag
    int windStart = response.indexOf("<wind_speed_kt>");
    int windEnd   = response.indexOf("</wind_speed_kt>", windStart);
    windSpeed     = response.substring(windStart + 15, windEnd);

    // Find the wind_gust_kt tag
    int gustStart = response.indexOf("<wind_gust_kt>");
    int gustEnd   = response.indexOf("</wind_gust_kt>", gustStart);
    windGusts     = response.substring(gustStart + 14, gustEnd);

    // Find the flight_category tag
    int categoryStart = response.indexOf("<flight_category>");
    int categoryEnd   = response.indexOf("</flight_category>", categoryStart);
    category          = response.substring(categoryStart + 17, categoryEnd);

    //Debug print stuff to serial
    if (DEBUG>3) {
      //Debug section
      Serial.print("[");
      Serial.print(indexVal);
      Serial.print("] ");
      Serial.print("Station: ");
      Serial.print(station);
      Serial.print(" wind: ");
      Serial.print(windDirection);
      if (windSpeed.toInt()<10) Serial.print("0");
      Serial.print(windSpeed);
      if (windGusts.toInt() >0) {
        Serial.print("G");
        Serial.print(windGusts);
      }
      Serial.print(" conditions: ");
      Serial.println(category);
    }
    
    setLedColor(indexVal,category,windGusts.toInt());

  } else {
    if (DEBUG>3) {
      Serial.print("Station: ");
      Serial.print(airports[indexVal]);
      Serial.println(" - Station METAR not found");
      Serial.println();
    }
    strip.setPixelColor(indexVal, strip.Color(0,0,0));
    color = "BLACK";
  }
  
}

void loop() {

  //Iterate through the array of airports, query the API for each, parse the XML and set the resulting value
  for (int i=0; i<NUM_AIRPORTS; i++) {
    delay(1000);
    if (airports[i] == "NULL" || airports[i] == "VFR" || airports[i] == "MVFR" || airports[i] == "IFR" || airports[i] == "LIFR" || airports[i] == "WVFR") {
      assignKeyColor(i);
    } else {
      getWX(i);
    }
    lastCall = millis(); 
    
    if (DEBUG>0) {
      Serial.print("[");
      Serial.print(i);
      Serial.print("] ");
      Serial.print(airports[i]);
      Serial.print(" Wind: ");
      if (windDirection.toInt()<1) {
        Serial.print("00");
      }
      Serial.print(windDirection);
      if (windSpeed.toInt()<10) {
        Serial.print("0");
      }
      Serial.print(windSpeed);
      if (windGusts.toInt() >0) {
        Serial.print("G");
        Serial.print(windGusts);
      }
      Serial.print(" conditions: ");
      Serial.print(category);
      Serial.print(" ");
      Serial.println(color);
    }
  }
  while (retryClock < (lastCall + RETRY_SPEED)) { 
    Serial.print(".");
    retryClock = millis();
    delay(100);
  } 
}
