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
StaticJsonDocument<JSON_OBJECT_SIZE(10)> engine_data;

// ELM data recovery informations
struct engine_parameter {
  uint8_t pid;
  char *par_name;
  std::function<float (float)> map;
};

std::function<float (float)> id = [] (float value) { return value; };

engine_parameter elm_data[] = {
  { VEHICLE_SPEED,               "VEHICLE_SPEED",              id },
  { ENGINE_RPM,                  "ENGINE_RPM",                 [] (float value) { return value / 4.0; } },
  { FUEL_TANK_LEVEL_INPUT,       "FUEL_TANK_LEVEL_INPUT",      [] (float value) { return value / 2.55; } },
  
  { AMBIENT_AIR_TEMP,            "AMBIENT_AIR_TEMP",           [] (float value) { return value - 40; } },
  { ENGINE_OIL_TEMP,             "ENGINE_OIL_TEMP",            [] (float value) { return value - 40; } },
  { ENGINE_COOLANT_TEMP,         "ENGINE_COOLANT_TEMP",        id },
  { INTAKE_AIR_TEMP,             "INTAKE_AIR_TEMP",            id },

  { ENGINE_LOAD,                 "ENGINE_LOAD",                [] (float value) { return value / 2.55; } },
  { RELATIVE_THROTTLE_POSITION,  "RELATIVE_THROTTLE_POSITION", [] (float value) { return value / 2.55; } },
  { ACTUAL_ENGINE_TORQUE,        "ACTUAL_ENGINE_TORQUE",       [] (float value) { return value - 125; } }
};

#define elm_data_len 10

// Utility
char engine_data_string[elm_data_len * 30];

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
  serializeJson(engine_data, engine_data_string);

  events.send((const char *)engine_data_string, "dataupdate", millis());
  delay(5);
}

void update_data()
{
  for(int i = 0; i < elm_data_len; i++)
    read_obd(elm_data[i]);

  /* Or if you want to detect errors */
  // int i = 0;
  // while(i < elm_data_len && read_obd(elm_data[i++]));

  #ifdef DEBUG
    serializeJsonPretty(engine_data, Serial);  
    Serial.println();
  #endif
}

bool read_obd(struct engine_parameter ep)
{
  Engine.queryPID(SERVICE_01, ep.pid);

  switch(Engine.status) {
    case ELM_SUCCESS:
      // Everything when fine
      engine_data[ep.par_name] = ep.map(Engine.findResponse());
      return true;
    case ELM_NO_RESPONSE:
    case ELM_GARBAGE:
      // The ECU doesn't provide this datum, the page will handle the error
      engine_data[ep.par_name] = NULL;

      DEBUG_PORT.print(ep.par_name);
      DEBUG_PORT.println(" is not available, skipping...");
      return true;
    default:
      // There is a problem with the ELM sensor
      // Check https://github.com/PowerBroker2/ELMduino/blob/master/src/ELMduino.h#L264
      engine_data["ERROR"] = Engine.status;

      DEBUG_PORT.print("ELM not responding with error ");
      DEBUG_PORT.println(Engine.status);
      return false;
  }
}
