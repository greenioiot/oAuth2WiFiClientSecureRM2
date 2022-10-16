


#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */


#define WIFI_AP ""
#define WIFI_PASSWORD ""



#include <Adafruit_MLX90614.h>

#include <FS.h>
#include "SPIFFS.h"
#include <WiFi.h>

#include <DNSServer.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <TaskScheduler.h>
#include <PubSubClient.h>


#include <ArduinoOTA.h>
#include <WebServer.h>


#include "Alert.h" // Out of range alert icon
#include "wifi1.h" // Signal Streng WiFi
#include "wifi2.h" // Signal Streng WiFi
#include "wifi3.h" // Signal Streng WiFi
#include "wifi4.h" // Signal Streng WiFi

// # Add On
#include <TimeLib.h>

#include "time.h"

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include "Free_Fonts.h" // Include the header file attached to this sketch

#include "RTClib.h"
#include "Free_Fonts.h"
#include "EEPROM.h"

#define HOSTNAME "CPN"
#define PASSWORD "7650"
#include <Arduino.h>
#include <WiFi.h>

// Modbus server TCP
#include "ModbusServerTCPasync.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

 
const char* host = "thingcontrol.io"; //adresa webservera (doména) na ktorú sa odosielajú dáta
String _urlToken = "/api/auth/login"; //URL adresa - cesta pod domenou k cieľovemu .php súboru, ktorý realizuje zápis

const String data = "{\"username\":\"admin@ais.co.th\", \"password\":\"eddy_7650!\"}";
const String _param = "?keys=co,co2,hum,no2,o3,pm10,pm2.5,RSSI,so2,temp,tvoc,CH2O";
int rssi = 0;
String deviceToken = "";
uint64_t chipId = 0;
bool shouldSaveConfig = false;


WiFiManager wifiManager;
unsigned long time_s = 0;
String wifiName = "";
struct tm timeinfo;
int countInSec = 0;
int avgPM2_5 = 0;
struct Sensor
{
  String deviceId;

  String co;
  String co2;
  String hum;
  String lux;
  String no2;
  String o3;
  String so2;
  String pm10;
  String pm2_5;
  String rssi;
  String temp;
  String tvoc;
  String ch2o;
};
Sensor sensor[18];
const int _NUMofSENSOR = 14;

#define HEADER_WIDTH  130
#define HEADER_HEIGHT 30
#define WEIGHT_WIDTH 160
#define WEIGHT_HEIGHT 50
#define TIME_WIDTH  85
#define TIME_HEIGHT 35

#define WiFi_WIDTH  30
#define WiFi_HEIGHT 30

#define VER_WIDTH  35
#define VER_HEIGHT 30
#define countInSec_WIDTH 30
#define countInSec_HEIGHT 30



String _JSON = "";
String _bearer = "";
unsigned long ms;
WiFiClientSecure client;
// Create server
ModbusServerTCPasync MBserver;

WebServer server(80);

struct Settings
{
  char TOKEN[40] = "";
  char SERVER[40] = "mqttservice.smartfarmpro.com";
  int PORT = 1883;
  char MODE[60] = "Farm/Cloud/Device/Mode2";
  uint32_t ip;
} sett;


Adafruit_MLX90614 mlx = Adafruit_MLX90614();


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 7;

const int   daylightOffset_sec = 3600;



//unsigned long drawTime = 0;
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library with default width and height
TFT_eSprite pm2_5Sprite = TFT_eSprite(&tft); // Invoke custom library with default width and height

TFT_eSprite headerSprite = TFT_eSprite(&tft); // Invoke custom library with default width and height
TFT_eSprite wifiSprite = TFT_eSprite(&tft); // Invoke custom library with default width and height
TFT_eSprite timeSprite = TFT_eSprite(&tft); // Invoke custom library with default width and height
TFT_eSprite updateSprite = TFT_eSprite(&tft); // Invoke custom library with default width and height

TFT_eSprite ver = TFT_eSprite(&tft); // Invoke custom library with default width and height
TFT_eSprite countInSecSprite = TFT_eSprite(&tft); // Invoke custom library with default width and height

uint16_t memo[20];                     // Test server memory: 20 words


class IPAddressParameter : public WiFiManagerParameter
{
  public:
    IPAddressParameter(const char *id, const char *placeholder, IPAddress address)
      : WiFiManagerParameter("")
    {
      init(id, placeholder, address.toString().c_str(), 16, "", WFM_LABEL_BEFORE);
    }

    bool getValue(IPAddress &ip)
    {
      return ip.fromString(WiFiManagerParameter::getValue());
    }
};

class IntParameter : public WiFiManagerParameter
{
  public:
    IntParameter(const char *id, const char *placeholder, long value, const uint8_t length = 10)
      : WiFiManagerParameter("")
    {
      init(id, placeholder, String(value).c_str(), length, "", WFM_LABEL_BEFORE);
    }

    long getValue()
    {
      return String(WiFiManagerParameter::getValue()).toInt();
    }
};



char  char_to_byte(char c)
{
  if ((c >= '0') && (c <= '9'))
  {
    return (c - 0x30);
  }
  if ((c >= 'A') && (c <= 'F'))
  {
    return (c - 55);
  }
}

String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';

  return String(data);
}


void drawWiFi( )
{


  rssi = map(WiFi.RSSI(), -90, -30, 1, 4);
  Serial.print("rssi:");
  Serial.println(rssi);
  if (rssi <= 1)
    wifiSprite.pushImage(0 , 0, wifi1Width, wifi1Height, wifi1);
  if (rssi == 2)
    wifiSprite.pushImage(0 , 0, wifi1Width, wifi1Height, wifi2);
  if (rssi == 3)
    wifiSprite.pushImage(0 , 0, wifi1Width, wifi1Height, wifi3);
  if (rssi >= 4)
    wifiSprite.pushImage(0 , 0, wifi1Width, wifi1Height, wifi4);

  wifiSprite.pushSprite(140, 0);
}


void configModeCallback (WiFiManager * myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
  Serial.print("saveConfigCallback:");
  Serial.println(sett.TOKEN);
}


void getChipID() {
  chipId = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("ESP32ChipID=%04X", (uint16_t)(chipId >> 32)); //print High 2 bytes
  Serial.printf("%08X\n", (uint32_t)chipId); //print Low 4bytes.

}

void setupOTA()
{
  //Port defaults to 8266
  //ArduinoOTA.setPort(8266);

  //Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(uint64ToString(chipId).c_str());

  //No authentication by default
  ArduinoOTA.setPassword(PASSWORD);

  //Password can be set with it's md5 value as well
  //MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  //ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]()
  {
    Serial.println("Start Updating....");

    Serial.printf("Start Updating....Type:%s\n", (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem");
  });

  ArduinoOTA.onEnd([]()
  {

    Serial.println("Update Complete!");

    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    String pro = String(progress / (total / 100)) + "%";
    int progressbar = (progress / (total / 100));
    //int progressbar = (progress / 5) % 100;
    //int pro = progress / (total / 100);

    drawUpdate(progressbar, 110, 0);


    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error)
  {
    Serial.printf("Error[%u]: ", error);
    String info = "Error Info:";
    switch (error)
    {
      case OTA_AUTH_ERROR:
        info += "Auth Failed";
        Serial.println("Auth Failed");
        break;

      case OTA_BEGIN_ERROR:
        info += "Begin Failed";
        Serial.println("Begin Failed");
        break;

      case OTA_CONNECT_ERROR:
        info += "Connect Failed";
        Serial.println("Connect Failed");
        break;

      case OTA_RECEIVE_ERROR:
        info += "Receive Failed";
        Serial.println("Receive Failed");
        break;

      case OTA_END_ERROR:
        info += "End Failed";
        Serial.println("End Failed");
        break;
    }


    Serial.println(info);
    ESP.restart();
  });

  ArduinoOTA.begin();
}

String uint64ToString(uint64_t input) {
  String result = "";
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c += '0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}


void setupWIFI()
{
  WiFi.setHostname(uint64ToString(chipId).c_str());

  byte count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Serial.print(".");
  }


  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connecting...OK.");
  } else {
    Serial.println("Connecting...Failed");
    if (count >= 10)
      ESP.restart();
  }
  //  reconnectMqtt();
}


void writeString(char add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

void _writeEEPROM(String data) {
  //Serial.print("Writing Data:");
  //Serial.println(data);
  writeString(10, data);  //Address 10 and String type data
  delay(10);
}


void splash() {

  tft.init();
  // Swap the colour byte order when rendering
  tft.setSwapBytes(true);
  tft.setRotation(1);  // landscape

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(5, (tft.height() / 2) - 50, 1);

  tft.setTextColor(TFT_YELLOW);


  tft.setTextFont(4);
  tft.println("Cloud");
  tft.println("    2Modbus");
  tft.println("         WiFi");
  tft.println("            CPN");

  delay(10000);
  //  tft.setTextPadding(180);
  
  
  Serial.println(F("Start..."));
  
  sensor[0].deviceId = "e76d9d10-4401-11ed-a4d4-fbb3cfb14f76";
  sensor[1].deviceId = "8c53a9f0-4402-11ed-a4d4-fbb3cfb14f76";
  sensor[2].deviceId = "91d442e0-3e7b-11ed-a4d4-fbb3cfb14f76";
  sensor[3].deviceId = "f155a440-3e79-11ed-a4d4-fbb3cfb14f76";
  sensor[4].deviceId = "a6ea0310-3e60-11ed-a4d4-fbb3cfb14f76";
  sensor[5].deviceId = "20dadf20-3e64-11ed-a4d4-fbb3cfb14f76";
  sensor[6].deviceId = "965935d0-3fdb-11ed-a4d4-fbb3cfb14f76";
  sensor[7].deviceId = "7ae20900-41a0-11ed-a4d4-fbb3cfb14f76";
  sensor[8].deviceId = "f2133e00-3e70-11ed-a4d4-fbb3cfb14f76";
  sensor[9].deviceId = "7ce4e110-41a4-11ed-a4d4-fbb3cfb14f76";
  sensor[10].deviceId = "c34bcf80-3e73-11ed-a4d4-fbb3cfb14f76";
  sensor[11].deviceId = "26d9f020-3fde-11ed-a4d4-fbb3cfb14f76";
  sensor[12].deviceId = "863e8260-3e76-11ed-a4d4-fbb3cfb14f76";
  sensor[13].deviceId = "dff57f60-40c0-11ed-a4d4-fbb3cfb14f76";

  for ( int i = 0; i < 250; i++)
  {
    tft.drawString(".", 1 + 2 * i, 300, GFXFF);
    delay(10);
    
  }
  Serial.println("end");
}


void _initLCD() {
  tft.fillScreen(TFT_BLACK);
  // TFT
  splash();
  // MLX
  mlx.begin();
}



char baudrate[8];
char dpsbits[5];
char program[10];
char mode_select[3];
void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>Setting</title></head><body> <div style=\"text-align: center;\"> <h1>Setting</h1><a style=\"background-color: red;border: 0;padding: 10px 20px;color: white;font-weight: 600;border-radius: 5px;\" href=\"/setting\">Setting</a> </div></body></html>");
  server.client().stop(); // Stop is needed because we sent no content length
}
void handleSetting() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>Setting</title></head><body> <h1>Setting</h1> <form method=\"POST\" action=\"save_setting\"> <label for=\"token\">Device Token</label> <input type=\"text\" name=\"token\" ");
  if (strcmp(sett.TOKEN, "") != 0) {
    server.sendContent("value=\"" + String(sett.TOKEN) + "\"");
  }
  server.sendContent("><br><label for=\"server\">Server</label><input type=\"text\" name=\"server\" ");
  if (strcmp(sett.SERVER, "") != 0) {
    server.sendContent("value=\"" + String(sett.SERVER) + "\"");
  }
  server.sendContent("><br><label for=\"port\">Port</label><input type=\"text\" name=\"port\" ");
  if (sett.PORT != 0) {
    server.sendContent("value=\"" + String(sett.PORT) + "\"");
  }
  server.sendContent("><br><label for=\"baudrate\">Serial Port : Baudrate</label><input type=\"text\" name=\"baudrate\" ");
  if (strcmp(baudrate, "") != 0) {
    server.sendContent("value=\"" + String(baudrate) + "\"");
  }
  server.sendContent("><br><label for=\"dpsbits\">Serial Port : Data Bits, Parity Bits, Stop Bits</label><input type=\"text\" name=\"dpsbits\" ");
  if (strcmp(dpsbits, "") != 0) {
    server.sendContent("value=\"" + String(dpsbits) + "\"");
  }
  server.sendContent("><br><label for=\"program\">Program</label><input type=\"text\" name=\"program\" ");
  if (strcmp(program, "") != 0) {
    server.sendContent("value=\"" + String(program) + "\"");
  }
  server.sendContent("><br><label for=\"mode_select\">Mode (1-10)</label><input type=\"text\" name=\"mode_select\" ");
  if (strcmp(mode_select, "") != 0) {
    server.sendContent("value=\"" + String(mode_select) + "\"");
  }
  server.sendContent("><br><input type=\"submit\" value=\"Save\"></form></body></html>");
  server.client().stop();
}
void handleSettingSave() {
  Serial.println("setting save");
  server.arg("token").toCharArray(sett.TOKEN, sizeof(sett.TOKEN));
  server.arg("server").toCharArray(sett.SERVER, sizeof(sett.SERVER));
  sett.PORT = server.arg("port").toInt();
  server.arg("baudrate").toCharArray(baudrate, sizeof(baudrate));
  server.arg("dpsbits").toCharArray(dpsbits, sizeof(dpsbits));
  server.arg("program").toCharArray(program, sizeof(program));
  server.arg("mode_select").toCharArray(mode_select, sizeof(mode_select));
  server.sendHeader("Location", "setting", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  Serial.println("saving config");
  DynamicJsonDocument jsonBuffer(1024);
  jsonBuffer["token"] = sett.TOKEN;
  jsonBuffer["server"] = sett.SERVER;
  jsonBuffer["port"] = sett.PORT;
  jsonBuffer["baudrate"] = baudrate;
  jsonBuffer["dpsbits"] = dpsbits;
  jsonBuffer["program"] = program;
  jsonBuffer["mode_select"] = mode_select;
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  serializeJson(jsonBuffer, Serial);
  serializeJson(jsonBuffer, configFile);
  configFile.close();
  String topic = "Farm/Cloud/Device/Mode" + String(mode_select) + "/" + deviceToken;
  topic.toCharArray(sett.MODE, sizeof(sett.MODE));
}
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}
/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String("TATWeightCapturer") + ".local")) {
    Serial.print("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void handle_NotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.send(404, "text/plain", "Not found");
}


// Server function to handle FC 0x03 and 0x04
ModbusMessage FC003(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {

    response.add((uint16_t)sensor[0].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[0].no2.toInt());     // add value to holding register
  }
  return response;
}


// Server function to handle FC 0x03 and 0x04
ModbusMessage FC103(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[1].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[1].no2.toInt());     // add value to holding register
  }
  return response;
}

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC203(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[2].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[2].no2.toInt());     // add value to holding register
  }
  return response;
}


// Server function to handle FC 0x03 and 0x04
ModbusMessage FC303(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[3].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[3].no2.toInt());     // add value to holding register
  }
  return response;
}

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC403(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[4].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[4].no2.toInt());     // add value to holding register
  }
  return response;
}


// Server function to handle FC 0x03 and 0x04
ModbusMessage FC503(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[5].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[5].no2.toInt());     // add value to holding register
  }
  return response;
}


// Server function to handle FC 0x03 and 0x04
ModbusMessage FC603(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[6].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[6].no2.toInt());     // add value to holding register
  }
  return response;
}


// Server function to handle FC 0x03 and 0x04
ModbusMessage FC703(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[7].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[7].no2.toInt());     // add value to holding register
  }
  return response;
}

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC803(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[8].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[8].no2.toInt());     // add value to holding register
  }
  return response;
}

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC903(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[9].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[9].no2.toInt());     // add value to holding register
  }
  return response;
}

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC1003(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[10].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[10].no2.toInt());     // add value to holding register
  }
  return response;
}

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC1103(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[11].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[11].no2.toInt());     // add value to holding register
  }
  return response;
}


// Server function to handle FC 0x03 and 0x04
ModbusMessage FC1203(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[12].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[12].no2.toInt());     // add value to holding register
  }
  return response;
}

// Server function to handle FC 0x03 and 0x04
ModbusMessage FC1303(ModbusMessage request) {
  ModbusMessage response;      // The Modbus message we are going to give back
  uint16_t addr = 0;           // Start address
  uint16_t words = 0;          // # of words requested
  request.get(2, addr);        // read address from request
  request.get(4, words);       // read # of words from request

  // Address overflow?
  if ((addr + words) > 12) { // number of holding registers (parameters)
    // Yes - send respective error response
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  // Set up response
  response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
  // Request for FC 0x03?
  if (request.getFunctionCode() == READ_HOLD_REGISTER) {
    response.add((uint16_t)sensor[13].tvoc.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].temp.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].hum.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].co.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].co2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].ch2o.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].pm2_5.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].pm10.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].o3.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].so2.toInt());     // add value to holding register
    response.add((uint16_t)sensor[13].no2.toInt());     // add value to holding register
  }
  return response;
}

void setup() {
  Serial.begin(115200);
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument jsonBuffer(1024);
        deserializeJson(jsonBuffer, buf.get());
        serializeJson(jsonBuffer, Serial);
        if (!jsonBuffer.isNull()) {
          Serial.println("\nparsed json");
          //strcpy(output, json["output"]);
          if (jsonBuffer.containsKey("baudrate")) strcpy(baudrate, jsonBuffer["baudrate"]);
          if (jsonBuffer.containsKey("dpsbits")) strcpy(dpsbits, jsonBuffer["dpsbits"]);
          if (jsonBuffer.containsKey("program")) strcpy(program, jsonBuffer["program"]);
          if (jsonBuffer.containsKey("mode_select")) strcpy(mode_select, jsonBuffer["mode_select"]);
          if (jsonBuffer.containsKey("token")) strcpy(sett.TOKEN, jsonBuffer["token"]);
          if (jsonBuffer.containsKey("server")) strcpy(sett.SERVER, jsonBuffer["server"]);
          if (jsonBuffer.containsKey("port")) sett.PORT = jsonBuffer["port"];
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  Serial.begin(115200);
  // make the pins outputs:
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  delay(1000);
  Serial.println("Start");

  EEPROM.begin(512);
  _initLCD();
 
  getChipID();
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.drawString("WiFi Setting", 5, (tft.height() / 2) - 30, 4);
  tft.drawString("(120 Sec)", 40, (tft.height() / 2) + 5, 4);
  tft.setTextColor(TFT_BLUE);

  WiFiManagerParameter baudrate_param("baudrate", "Serial Port : Baudrate", baudrate, 8);
  WiFiManagerParameter dpsbits_param("databits", "Serial Port : Data Bits, Parity Bits, Stop Bits", dpsbits, 5);
  WiFiManagerParameter program_param("program", "Program", program, 10);
  WiFiManagerParameter mode_param("mode", "Mode (1-10)", mode_select, 3);
  wifiManager.setTimeout(120);

  wifiManager.setAPCallback(configModeCallback);
  std::vector<const char *> menu = {"wifi", "info", "sep", "restart", "exit"};
  wifiManager.setMenu(menu);
  wifiManager.setClass("invert");
  wifiManager.setConfigPortalTimeout(120); // auto close configportal after n seconds
  wifiManager.setAPClientCheck(true); // avoid timeout if client connected to softap
  wifiManager.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

  WiFiManagerParameter blnk_Text("<b>Device setup.</b> <br>");
  sett.TOKEN[39] = '\0';   //add null terminator at the end cause overflow
  sett.SERVER[39] = '\0';   //add null terminator at the end cause overflow
  WiFiManagerParameter blynk_Token( "blynktoken", "device Token",  sett.TOKEN, 40);
  WiFiManagerParameter blynk_Server( "blynkserver", "Server",  sett.SERVER, 40);
  IntParameter blynk_Port( "blynkport", "Port",  sett.PORT);

  wifiManager.addParameter( &blnk_Text );
  wifiManager.addParameter( &blynk_Token );
  wifiManager.addParameter( &blynk_Server );
  wifiManager.addParameter( &blynk_Port );
  wifiManager.addParameter(&baudrate_param);
  wifiManager.addParameter(&dpsbits_param);
  wifiManager.addParameter(&program_param);
  wifiManager.addParameter(&mode_param);


  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);




  wifiName.concat(uint64ToString(chipId));
  if (!wifiManager.autoConnect(wifiName.c_str())) {
    Serial.println("failed to connect and hit timeout");

    ESP.restart();
  }
  deviceToken = wifiName.c_str();
  if (baudrate_param.getValue() != "") strcpy(baudrate, baudrate_param.getValue());
  if (dpsbits_param.getValue() != "") strcpy(dpsbits, dpsbits_param.getValue());
  if (program_param.getValue() != "") strcpy(program, program_param.getValue());
  if (mode_param.getValue() != "") strcpy(mode_select, mode_param.getValue());
  if (blynk_Token.getValue() != "") strcpy(sett.TOKEN, blynk_Token.getValue());
  if (blynk_Server.getValue() != "") strcpy(sett.SERVER, blynk_Server.getValue());
  if (blynk_Port.getValue() != 0) sett.PORT =  blynk_Port.getValue();
  Serial.println("saving config");
  DynamicJsonDocument jsonBuffer(1024);
  jsonBuffer["baudrate"] = baudrate;
  jsonBuffer["dpsbits"] = dpsbits;
  jsonBuffer["program"] = program;
  jsonBuffer["mode_select"] = mode_select;
  jsonBuffer["token"] = sett.TOKEN;
  jsonBuffer["server"] = sett.SERVER;
  jsonBuffer["port"] = sett.PORT;
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  serializeJson(jsonBuffer, Serial);
  serializeJson(jsonBuffer, configFile);
  configFile.close();
  int dps_int;
  if (strcmp(dpsbits, "5N1") == 0) dps_int = 134217744;
  else if (strcmp(dpsbits, "6N1") == 0) dps_int = 134217748;
  else if (strcmp(dpsbits, "7N1") == 0) dps_int = 134217752;
  else if (strcmp(dpsbits, "8N1") == 0) dps_int = 134217756;
  else if (strcmp(dpsbits, "5N2") == 0) dps_int = 134217776;
  else if (strcmp(dpsbits, "6N2") == 0) dps_int = 134217780;
  else if (strcmp(dpsbits, "7N2") == 0) dps_int = 134217784;
  else if (strcmp(dpsbits, "8N2") == 0) dps_int = 134217788;
  else if (strcmp(dpsbits, "5E1") == 0) dps_int = 134217746;
  else if (strcmp(dpsbits, "6E1") == 0) dps_int = 134217750;
  else if (strcmp(dpsbits, "7E1") == 0) dps_int = 134217754;
  else if (strcmp(dpsbits, "8E1") == 0) dps_int = 134217758;
  else if (strcmp(dpsbits, "5E2") == 0) dps_int = 134217778;
  else if (strcmp(dpsbits, "6E2") == 0) dps_int = 134217782;
  else if (strcmp(dpsbits, "7E2") == 0) dps_int = 134217786;
  else if (strcmp(dpsbits, "8E2") == 0) dps_int = 134217790;
  else if (strcmp(dpsbits, "5O1") == 0) dps_int = 134217747;
  else if (strcmp(dpsbits, "6O1") == 0) dps_int = 134217751;
  else if (strcmp(dpsbits, "7O1") == 0) dps_int = 134217755;
  else if (strcmp(dpsbits, "8O1") == 0) dps_int = 134217759;
  else if (strcmp(dpsbits, "5O2") == 0) dps_int = 134217779;
  else if (strcmp(dpsbits, "6O2") == 0) dps_int = 134217783;
  else if (strcmp(dpsbits, "7O2") == 0) dps_int = 134217787;
  else if (strcmp(dpsbits, "6O2") == 0) dps_int = 134217791;

  String topic = "Farm/Cloud/Device/Mode" + String(mode_select) + "/" + deviceToken;
  topic.toCharArray(sett.MODE, sizeof(sett.MODE));
  configTime(gmtOffset_sec, 0, ntpServer);
  //  client.setServer( sett.SERVER, sett.PORT );


  setupWIFI();
  setupOTA();

  if (WiFi.status() != WL_CONNECTED) {
    /* Put IP Address details */
    IPAddress local_ip(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    WiFi.softAP("Cloud2Modbus", "7650");
    WiFi.softAPConfig(local_ip, gateway, subnet);
  }

  server.on("/", handleRoot);
  server.on("/setting", handleSetting);
  server.on("/save_setting", handleSettingSave);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.onNotFound ( handle_NotFound );
  server.begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());


  //  Serial.println("Initialized scheduler");
  tft.begin();

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  pm2_5Sprite.createSprite(WEIGHT_WIDTH, WEIGHT_HEIGHT);
  pm2_5Sprite.fillSprite(TFT_BLACK);

  headerSprite.createSprite(HEADER_WIDTH, HEADER_HEIGHT);
  headerSprite.fillSprite(TFT_BLACK);

  wifiSprite.createSprite(WiFi_WIDTH, WiFi_HEIGHT);
  wifiSprite.fillSprite(TFT_BLACK);

  timeSprite.createSprite(TIME_WIDTH, TIME_HEIGHT);
  timeSprite.fillSprite(TFT_BLACK);

  ver.createSprite(VER_WIDTH, VER_HEIGHT);
  ver.fillSprite(TFT_BLACK);
  drawWiFi();
  drawTime();
  drawVersion();
  String ip = "IP:";
  ip.concat(WiFi.localIP().toString().c_str());
  header(ip.c_str());
 
  Serial.println(F(""));
  Serial.println(F("Wifi connected with IP:"));
  Serial.println(WiFi.localIP());
  _bearer = oAuth();
  Serial.print("bearer:");
  Serial.println(_bearer);


  // Set up test memory
  for (uint16_t i = 0; i < 20; ++i) { // number of device 20

    memo[i] = (i * 2) << 8 | ((i * 2) + 1);
    Serial.println(memo[i]);
    // Define and start RTU server

  }

  MBserver.registerWorker(1, READ_HOLD_REGISTER, &FC003);      // FC=03 for serverID=1
  MBserver.registerWorker(2, READ_HOLD_REGISTER, &FC103);      // FC=03 for serverID=2
  MBserver.registerWorker(3, READ_HOLD_REGISTER, &FC203);      // FC=03 for serverID=3
  MBserver.registerWorker(4, READ_HOLD_REGISTER, &FC303);      // FC=03 for serverID=4
  MBserver.registerWorker(5, READ_HOLD_REGISTER, &FC403);      // FC=03 for serverID=5
  MBserver.registerWorker(6, READ_HOLD_REGISTER, &FC503);      // FC=03 for serverID=6
  MBserver.registerWorker(7, READ_HOLD_REGISTER, &FC603);      // FC=03 for serverID=7
  MBserver.registerWorker(8, READ_HOLD_REGISTER, &FC703);      // FC=03 for serverID=8
  MBserver.registerWorker(9, READ_HOLD_REGISTER, &FC803);      // FC=03 for serverID=9
  MBserver.registerWorker(10, READ_HOLD_REGISTER, &FC903);      // FC=03 for serverID=10
  MBserver.registerWorker(11, READ_HOLD_REGISTER, &FC1003);      // FC=03 for serverID=11
  MBserver.registerWorker(12, READ_HOLD_REGISTER, &FC1103);      // FC=03 for serverID=12
  MBserver.registerWorker(13, READ_HOLD_REGISTER, &FC1203);      // FC=03 for serverID=13
  MBserver.registerWorker(14, READ_HOLD_REGISTER, &FC1303);      // FC=03 for serverID=14

  Serial.println("Run Test Server Done");
  MBserver.start(502, 1, 20000);
  drawAVGPM2_5();
  getSensorValue();
  drawAVGPM2_5();
  Serial.printf("free heap: %d\n", ESP.getFreeHeap());

}
void getSensorValue() {
  getValue();
  Serial.printf("free heap: %d\n", ESP.getFreeHeap());
  for (int i = 0; i < _NUMofSENSOR; i++) {



    Serial.print(" tvoc:"); Serial.print(sensor[i].tvoc);
    Serial.print(" temp:"); Serial.print(sensor[i].temp);
    Serial.print(" hum:"); Serial.print(sensor[i].hum);
    Serial.print(" co:"); Serial.print(sensor[i].co);
    Serial.print(" co2:");  Serial.print(sensor[i].co2);
    Serial.print(" ch2o:");  Serial.print(sensor[i].ch2o);
    Serial.print(" pm2.5:");  Serial.print(sensor[i].pm2_5);
    Serial.print(" pm10:");  Serial.print(sensor[i].pm10);
    Serial.print(" o3:");  Serial.print(sensor[i].o3);
    Serial.print(" so2:");  Serial.print(sensor[i].so2);
    Serial.print(" no2:");  Serial.println(sensor[i].no2);
  }
}
void loop() {

  ms = millis();


  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    ms = millis();
    if (ms % 6000 == 0)
    {
      ESP.restart();
    }
  }



  if (ms % 60000  == 0) {
    drawTime();
    drawWiFi();
    Serial.println("Attach WiFi for，OTA "); Serial.println(WiFi.RSSI() );

    setupWIFI();
    setupOTA();

  }

  if (ms % 120000 == 0)
  {
    getSensorValue();
    drawAVGPM2_5();
  }
  server.handleClient();
  ArduinoOTA.handle();
}



void drawAVGPM2_5() {
  int offset = 0;
  pm2_5Sprite.fillScreen(TFT_BLACK);
  pm2_5Sprite.setTextColor(TFT_WHITE);



  for (int i = 0; i < _NUMofSENSOR; i++ ) {
    avgPM2_5 += sensor[i].pm2_5.toInt();
  }
  avgPM2_5 = avgPM2_5 / _NUMofSENSOR;

  Serial.print("PM2.5:");
  Serial.println(avgPM2_5);

  if (avgPM2_5 <= 9) {
    offset = 68;
  } else if ((avgPM2_5 >= 10) && (avgPM2_5 <= 99)) {
    offset = 48;
  } else {
    offset = 28;
  }
  pm2_5Sprite.drawString("PM2.5 ", 5, 14, 2); // Value in middle
  pm2_5Sprite.drawNumber(avgPM2_5, offset, 0, 7); // Value in middle
  pm2_5Sprite.pushSprite(0, 40);

}
String a0(int n) {
  return (n < 10) ? "0" + String(n) : String(n);
}

void drawTime() {

  String dateS = "";
  String timeS = "";
  //  _epoch = time(&now) + 25200; // GMT+7 Asia/BKK Timezone
  //  ts = _epoch;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return;
  }


  dateS = a0(timeinfo.tm_mday) + "/" + a0(timeinfo.tm_mon + 1) + "/" + String(timeinfo.tm_year + 1900);
  timeS = a0(timeinfo.tm_hour) + ":" + a0(timeinfo.tm_min);//  + ":" + a0(timeinfo.tm_sec);
  timeSprite.fillScreen(TFT_BLACK);

  timeSprite.setTextColor(TFT_YELLOW);
  timeSprite.drawString(dateS, 0, 0, 2); // Font 4 for fast drawing with background
  timeSprite.drawString(timeS, 0, 15, 2); // Font 4 for fast drawing with background

  timeSprite.pushSprite(0, 0);

}

void drawUpdate(int num, int x, int y)
{
  updateSprite.createSprite(25, 20);
  updateSprite.fillScreen(TFT_BLACK);
  //  updateSprite.setFreeFont(FSB9);
  updateSprite.setTextColor(TFT_YELLOW);
  updateSprite.setTextSize(1);
  updateSprite.drawNumber(num, 0, 4);
  updateSprite.drawString("%", 20, 4);
  updateSprite.pushSprite(x, y);
  updateSprite.deleteSprite();
}

void drawVersion() {
  ver.createSprite(35, 20);
  ver.setTextColor(TFT_ORANGE);
  ver.setTextSize(1);
  ver.drawString("v1.0", 0, 3);
  ver.pushSprite(134, 115);
}

void drawCountInSec() {


  countInSecSprite.createSprite(30, 20);
  countInSecSprite.fillScreen(TFT_BLACK);
  countInSecSprite.setTextColor(TFT_GOLD);
  countInSecSprite.setTextSize(2);
  countInSecSprite.drawNumber(countInSec, 0, 0);
  countInSecSprite.pushSprite(84, 0);

}
//====================================================================================
// This is the function to draw the icon stored as an array in program memory (FLASH)
//====================================================================================

// To speed up rendering we use a 64 pixel buffer
#define BUFF_SIZE 64

// Draw array "icon" of defined width and height at coordinate x,y
// Maximum icon size is 255x255 pixels to avoid integer overflow

// Print the header for a display screen
void header(const char *string)
{
  headerSprite.setTextSize(1);


  headerSprite.setTextColor(TFT_GREEN);
  headerSprite.drawString(string, 5, 2, 2); // Font 4 for fast drawing with background
  headerSprite.pushSprite(0, tft.height() - 15);


}

// Draw a + mark centred on x,y
void drawDatum(int x, int y)
{
  tft.drawLine(x - 5, y, x + 5, y, TFT_GREEN);
  tft.drawLine(x, y - 5, x, y + 5, TFT_GREEN);
}


String oAuth() {
  client.stop();
  if (client.connect(host, 443)) {
    //    Serial.println(F("Connected to server successfully"));
    client.println("POST " + _urlToken + " HTTP/1.1");
    client.println("Host: " + (String)host);
    client.println(F("Accept:  application/json"));
    client.println(F("Content-Type:  application/json"));
    client.print(F("Content-Length: "));
    client.println(data.length());
    //    Serial.println(data.length());
    //    Serial.println(data);
    client.println();
    client.println(data);
    Serial.println(F("Datas were sent to server successfully"));
    int count = 0;
    while (client.connected()) {

      _JSON = client.readStringUntil('\n');
      //      Serial.print(count); Serial.print(":"); Serial.println(_JSON);

      if (_JSON.length() > 0 ) {
        if (_JSON.indexOf("token") > 0) {

          //          Serial.println(_JSON);
          break;
        }
      }

    }

    client.stop();
    _JSON.trim();
    //    Serial.println(_JSON);
  } else {
    Serial.println(F("Connection to webserver was NOT successful"));
  }

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, _JSON);
  JsonObject obj = doc.as<JsonObject>();
  String _token = obj[String("token")];
  Serial.println(_token);
  return _token;

}

String parseValue(String _JSON, String v) {
  DynamicJsonDocument doc1(1024);
  DynamicJsonDocument doc2(128);
  deserializeJson(doc1, _JSON);
  JsonObject obj = doc1.as<JsonObject>();
  String _value = obj[String(v)];
  _value = _value.substring(1, _value.length() - 1);;
  deserializeJson(doc2, _value);
  JsonObject obj2 = doc2.as<JsonObject>();
  String value = obj2[String("value")];
  return value;

}
void getValue() {
  String _req = "";

  client.stop();


  if (client.connect(host, 443)) {
    for (int i = 0; i < _NUMofSENSOR; i++ ) {
      //      Serial.print("deviceId:"); Serial.println(sensor[i].deviceId);
      _req = "/api/plugins/telemetry/DEVICE/" + sensor[i].deviceId + "/values/timeseries";
      //      Serial.println(F("Connected to server successfully"));
      client.println("GET " + _req + _param + " HTTP/1.1");
      client.println("Host: " + (String)host);
      client.println("User-Agent: Cloud2Modbus");
      client.println("Content-Type: application/json");
      client.println("Connection: Keep-Alive");
      client.println("X-Authorization: Bearer " + _bearer);
      client.println();

      //          Serial.println("GET " + _req+_param + " HTTP/1.1");
      //          Serial.println("Host: " + (String)host);
      //          Serial.println("User-Agent: curl/7.47.0");
      //
      //          Serial.println("Content-Type: application/json");
      //          Serial.println("Connection: Keep-Alive");
      //          Serial.println("X-Authorization: Bearer " + _bearer);
      //          Serial.println();

      while (client.connected()) {

        _JSON = client.readStringUntil('\n');
        //        Serial.println(_JSON);

        if (_JSON.length() > 0 ) {
          if (_JSON.indexOf("ts") > 0) {
            //            Serial.println(_JSON);
            break;
          }
        }

      }


      sensor[i].pm2_5 = parseValue(_JSON, "pm2.5");
      sensor[i].pm10 = parseValue(_JSON, "pm10");
      sensor[i].co = parseValue(_JSON, "co");
      sensor[i].co2 = parseValue(_JSON, "co2");
      sensor[i].temp = parseValue(_JSON, "temp");
      sensor[i].hum = parseValue(_JSON, "hum");
      sensor[i].rssi = parseValue(_JSON, "RSSI");
      sensor[i].tvoc = parseValue(_JSON, "tvoc");
      sensor[i].ch2o = parseValue(_JSON, "CH2O");
      sensor[i].no2 = parseValue(_JSON, "no2");
      sensor[i].so2 = parseValue(_JSON, "so2");
      sensor[i].o3 = parseValue(_JSON, "o3");

    }
  } else {
    Serial.println(F("Connection to webserver was NOT successful"));

  }
  Serial.println(ESP.getFreeHeap());
}
