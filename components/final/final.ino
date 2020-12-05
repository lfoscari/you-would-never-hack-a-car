#include <WiFi.h>
// #include <FS.h>
#include <SPIFFS.h>

// Queste due cose sono le più pesanti,
// si potrebbe trovare un'alternativa pià leggera
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include <Wire.h>
#include <MPU6050_light.h>

#define DEBUG true
#define SIZE(x) sizeof(x) / sizeof(x[0])

#define DEBUG_PORT Serial
#define ELM_PORT SerialBT

#define PARAMETERS_COUNT 12

// SSID & Password
#define ssid "Offroad"
#define password "31415926"

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
BluetoothSerial ELM_PORT;

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

void setup()
{
  Serial.begin(115200);


  /* ELM327 */

  Serial.print("Connecting to OBDII... ");

  ELM_PORT.begin("ESP32", true);

  if (!ELM_PORT.connect("OBDII")) {
    DEBUG_PORT.println("\nCouldn't connect to OBD scanner (phase 1). Restarting...");
    delay(3000);
    ESP.restart();
  }

  if (!Engine.begin(ELM_PORT)) {
    DEBUG_PORT.println("\nCouldn't connect to OBD scanner (phase 2). Restarting...");
    delay(3000);
    ESP.restart();
  }
  
  Serial.println("done.");


  /* Gyro */

  Serial.print("Connecting to MPU6050... ");

  Wire.begin();
  mpu.begin();

  // Base values
  mpu.calcGyroOffsets();

  Serial.println("done.");


  /* Create soft access point */

  Serial.print("Creating access point... ");

  WiFi.softAP(ssid, password);
  // WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.print("done [");
  Serial.print(WiFi.softAPIP());
  Serial.println("].");


  /* Initialize SPIFFS */

  Serial.print("Initializing file system... ");

  if (!SPIFFS.begin(true)) {
    DEBUG_PORT.println("\nCouldn't mount SPIFFS. Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("done.");


  /* Web server */

  Serial.print("Starting web server... ");

  server.serveStatic("/", SPIFFS, "/");
  // server.serveStatic("/", SPIFFS, "/js/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });

  server.begin();
  server.addHandler(&events);

  Serial.println("done.");
}

void loop()
{
  for(int i = 0; i < SIZE(elm_data); i++)
    send_obd_data(elm_data[i]);

  send_gyro_data();
}

void send_obd_data(struct engine_parameter ep)
{
  Engine.queryPID(SERVICE_01, ep.pid);
  char value[6];

  switch(Engine.status) {
    case ELM_SUCCESS:
      // Everything went fine
      snprintf(value, sizeof(value), "%d", (int) ep.map(Engine.findResponse()));
      events.send(value, ep.par_name, millis());
      delay(100);

      break;

    case ELM_NO_RESPONSE:
    case ELM_GARBAGE:
    case ELM_NO_DATA:
      // The ECU doesn't provide this datum, the page will handle the error
      DEBUG_PORT.print(ep.par_name);
      DEBUG_PORT.println(" is not available, skipping...");

      break;

    default:
      // There is a problem with the ELM sensor
      // Check https://github.com/PowerBroker2/ELMduino/blob/master/src/ELMduino.h#L264
      snprintf(value, sizeof(value), "%d", Engine.status);
      events.send(value, "error", millis());
      delay(100);

      DEBUG_PORT.print("ELM error ");
      DEBUG_PORT.println(Engine.status);

      break;
  }
}

void send_gyro_data() {
  mpu.update();
  char value[6];

  snprintf(value, sizeof(value), "%f", mpu.getAngleY());
  events.send(value, "yTilt", millis());
  delay(100);

  snprintf(value, sizeof(value), "%f", mpu.getAngleZ());
  events.send(value, "zTilt", millis());
  delay(100);
}
