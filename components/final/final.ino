#include <WiFi.h>
#include <FS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>

// SSID & Password
const char *ssid = "ESP32";         // Enter your SSID here
const char *password = "123456789"; // Enter your Password here

// IP Address details
// IPAddress local_ip(192, 168, 1, 1);
// IPAddress gateway(192, 168, 1, 1);
// IPAddress subnet(255, 255, 255, 0);

// Web server
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Bluetooth
BluetoothSerial SerialBT;

// ELM327
ELM327 Engine;
StaticJsonDocument<JSON_OBJECT_SIZE(10)> engineData;

void setup()
{
  Serial.begin(115200);

  // Create soft access point
  WiFi.softAP(ssid, password);
  // WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // DEBUG: ls
  // for(File file = SPIFFS.open("/"); file; file = file.openNextFile())
  //  Serial.println(file.name());

  server.serveStatic("/", SPIFFS, "/");
  // ??? server.serveStatic("/", SPIFFS, "/js/");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });

  server.begin();

  // EventSource
  events.onConnect([](AsyncEventSourceClient *client){
    client->send("hello!", NULL, millis(), 1000);
  });
  
  server.addHandler(&events);
}

/* Test */

char myData[] = "{\"gear\": 5,\"lights\": false,\"throttle\": 15,\"oil_temp\": 55,\"critical_oil_temp\": 100,\"rpm\": 0,\"max_rpm\": 5000,\"air_temperature\": 25,\"fuel\": 25}";

// 9 -> Number of members in the JSON
StaticJsonDocument<JSON_OBJECT_SIZE(9)> engine;

int i = 0;

// Read data into engine
DeserializationError err = deserializeJson(engine, myData);

void loop()
{
  engine["rpm"] = i++;

  char jsonChar[300];
  serializeJson(engine, jsonChar);
  
  events.send((const char *) jsonChar, "dataupdate", millis());
  delay(100);
}
