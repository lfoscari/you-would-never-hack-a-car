#include <WiFi.h>
#include <FS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>

#include <Wire.h>
#include <MPU6050_light.h>

#define SIZE(array) (sizeof(array) / sizeof(*(array)))

#define DEBUG true

#define DEBUG_PORT Serial
#define ELM_PORT SerialBT

// SSID & Password
const char *ssid = "Offroad";
const char *password = "31415926";

// IP Address details
// IPAddress local_ip(192, 168, 1, 1);
// IPAddress gateway(192, 168, 1, 1);
// IPAddress subnet(255, 255, 255, 0);

// Gyro
MPU6050 mpu(Wire);

// Web server
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Bluetooth
BluetoothSerial ELM_PORT; // SerialBT
StaticJsonDocument<JSON_OBJECT_SIZE(10)> vehicle_data;

// ELM327
ELM327 Engine;

// ELM data recovery informations
struct engine_parameter {
  uint8_t pid;
  char *par_name;
  std::function<float (float)> map;
};

engine_parameter elm_data[] = {
  { VEHICLE_SPEED,               "vehicleSpeed",              [] (float value) { return value; } },
  { ENGINE_RPM,                  "engineRPM",                 [] (float value) { return value / 4.0; } },
  { FUEL_TANK_LEVEL_INPUT,       "fuelLevel",                 [] (float value) { return value / 2.55; } },

  { AMBIENT_AIR_TEMP,            "ambientAirTemperature",     [] (float value) { return value - 40; } },
  { ENGINE_OIL_TEMP,             "oilTemperature",            [] (float value) { return value - 40; } },
  { ENGINE_COOLANT_TEMP,         "coolantTemperature",        [] (float value) { return value; } },
  { INTAKE_AIR_TEMP,             "intakeAirTemperature",      [] (float value) { return value; } },

  { ENGINE_LOAD,                 "engineLoad",                [] (float value) { return value / 2.55; } },
  { RELATIVE_THROTTLE_POSITION,  "relativeThrottlePosition",  [] (float value) { return value / 2.55; } },
  { ACTUAL_ENGINE_TORQUE,        "actualTorque",              [] (float value) { return value - 125; } }
};

#define par_count 12

// Utility
char vehicle_data_string[par_count * 30];

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

  #ifdef DEBUG
  for(File file = SPIFFS.open("/"); file; file = file.openNextFile())
    Serial.println(file.name());
  #endif

  Serial.println("File system mounted.");

  /* ELM327 */

  /* ELM_PORT.begin("ESP32", true);

  if (!ELM_PORT.connect("OBDII")) {
    for (;;) {
      DEBUG_PORT.println("Couldn't connect to OBD scanner.");
      delay(1000);
    }
  }

  Engine.begin(ELM_PORT);
  Serial.println("Connected to ELM327."); */

  /* Gyro */

  Wire.begin();
  mpu.begin();

  // Base values
  mpu.calcGyroOffsets();

  Serial.println("Connected to MPU6050.");

  /* Web server */

  server.serveStatic("/", SPIFFS, "/");
  // server.serveStatic("/", SPIFFS, "/js/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });

  server.begin();
  server.addHandler(&events);

  Serial.println("Web server started.");
}

void loop()
{
  /* update_data();
  serializeJson(vehicle_data, vehicle_data_string);

  events.send((const char *)vehicle_data_string, "dataupdate", millis());
  delay(5); */
}

void update_data()
{
  for(int i = 0; i < SIZE(elm_data); i++)
    read_obd(elm_data[i]);

  read_gyro();

  /* Or if you want to detect errors */
  // int i = 0;
  // while(i < elm_data_len && read_obd(elm_data[i++]));

  #ifdef DEBUG
    serializeJsonPretty(vehicle_data, Serial);
    Serial.println();
  #endif
}

bool read_obd(struct engine_parameter ep)
{
  Engine.queryPID(SERVICE_01, ep.pid);

  switch(Engine.status) {
    case ELM_SUCCESS:
      // Everything went fine
      vehicle_data[ep.par_name] = ep.map(Engine.findResponse());
      return true;
    case ELM_NO_RESPONSE:
    case ELM_GARBAGE:
      // The ECU doesn't provide this datum, the page will handle the error
      vehicle_data[ep.par_name] = NULL;

      DEBUG_PORT.print(ep.par_name);
      DEBUG_PORT.println(" is not available, skipping...");
      return true;
    default:
      // There is a problem with the ELM sensor
      // Check https://github.com/PowerBroker2/ELMduino/blob/master/src/ELMduino.h#L264
      vehicle_data["ERROR"] = Engine.status;

      DEBUG_PORT.print("ELM not responding with error ");
      DEBUG_PORT.println(Engine.status);
      return false;
  }
}

bool read_gyro() {
  // float angle[3] = { mpu.getAngleX(), mpu.getAngleY(), mpu.getAngleZ() };
  mpu.update();
  
  // Pitching (Y)
  vehicle_data["xTilt"] = mpu.getAngleY();

  // Rolling (Z???)
  vehicle_data["yTilt"] = mpu.getAngleZ();

  return true;
}
