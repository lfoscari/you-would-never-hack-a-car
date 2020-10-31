#include <WiFi.h>
#include <FS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>

#define DEBUG_PORT Serial
#define ELM_PORT SerialBT

// SSID & Password
const char *ssid = "Offroad";       // Enter your SSID here
const char *password = "31415926";  // Enter your Password here

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

// Utility
char engineDataString[300];

void setup()
{
  Serial.begin(115200);

  /* Create soft access point */

  WiFi.softAP(ssid, password);
  // WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.print("Wifi access point created.\nIP address: ");
  Serial.println(WiFi.softAPIP());

  /* Initialize SPIFFS */

  if (!SPIFFS.begin(true)) {
    for (;;) {
      DEBUG_PORT.println("Couldn't mount SPIFFS.");
      delay(1000);
    }
  }

  // DEBUG: ls
  // for(File file = SPIFFS.open("/"); file; file = file.openNextFile())
  //  Serial.println(file.name());

  Serial.println("File system mounted.");

  /* Web server */

  server.serveStatic("/", SPIFFS, "/");
  // server.serveStatic("/", SPIFFS, "/js/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });

  server.begin();
  server.addHandler(&events);

  Serial.println("Web server started.");

  /* ELM327 */

  ELM_PORT.begin("ESP32", true);

  if (!ELM_PORT.connect("OBDII")) {
    for (;;) {
      DEBUG_PORT.println("Couldn't connect to OBD scanner.");
      delay(1000);
    }
  }

  Engine.begin(ELM_PORT);
  Serial.println("Connected to ELM327.");
}

void loop()
{
  update_data();
  serializeJson(engineData, engineDataString);

  events.send((const char *)engineDataString, "dataupdate", millis());
  delay(5);
}

void update_data()
{ 
  engineData["VEHICLE_SPEED"] = read_obd(VEHICLE_SPEED);
  engineData["ENGINE_RPM"] = read_obd(ENGINE_RPM) / 4.0;
  engineData["FUEL_TANK_LEVEL_INPUT"] = read_obd(FUEL_TANK_LEVEL_INPUT) / 2.55;

  engineData["AMBIENT_AIR_TEMP"] = read_obd(AMBIENT_AIR_TEMP) - 40;
  engineData["ENGINE_OIL_TEMP"] = read_obd(ENGINE_OIL_TEMP) - 40;
  engineData["ENGINE_COOLANT_TEMP"] = read_obd(ENGINE_COOLANT_TEMP);
  engineData["INTAKE_AIR_TEMP"] = read_obd(INTAKE_AIR_TEMP);

  engineData["ENGINE_LOAD"] = read_obd(ENGINE_LOAD) / 2.55;
  engineData["RELATIVE_THROTTLE_POSITION"] = read_obd(RELATIVE_THROTTLE_POSITION) / 2.55;
  engineData["ACTUAL_ENGINE_TORQUE"] = read_obd(ACTUAL_ENGINE_TORQUE) - 125;

  // DEBUG
  serializeJsonPretty(engineData, Serial);
  Serial.println();
}

float read_obd(uint8_t pid)
{
  Engine.queryPID(SERVICE_01, pid);

  // Everything when fine
  if(Engine.status == ELM_SUCCESS)
    return Engine.findResponse();

  // Something went wrong, Engine.status contains the error code
  DEBUG.print("OBD error code ");
  DEBUG.println(error);

  // There is a problem with the ELM sensor
  if(Engine.status != ELM_NO_RESPONSE && Engine.status != ELM_GARBAGE)
    engineData["ERROR"] = error;

  // The ECU doesn't provide this datum, the page will handle the error
  return NULL;
}