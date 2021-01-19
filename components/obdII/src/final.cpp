#include <Arduino.h>
#include <SPIFFS.h>

#include <ESPAsyncWebServer.h>

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include <Adafruit_MPU6050.h>

// Functions
void handle_client(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void send_obd_data(struct engine_parameter ep);
void send_gyro_data();

// DEBUG
#define _ENABLE_OBD false
#define _ENABLE_GYRO true
#define _ENABLE_WIFI_AP false
#define _ENABLE_WEB_SERVER true

#define DEBUG_PORT Serial

// CONNECTION TO OBD
#define ELM_PORT SerialBT

// OBDII PARAMETERS 
#define PARAMETERS_COUNT 12
#define SIZE(x) sizeof(x) / sizeof(x[0])

// SOFT ACCESS POINT SSID & Password
#define SSID "Offroad"
#define PASSWORD "31415926"

// Soft access point coordinates
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Gyro
Adafruit_MPU6050 mpu;
double x, y, now;

// Web server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *target = NULL; // handle one client

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
  { VEHICLE_SPEED,               (char *) "vehicleSpeed",              [] (float value) { return value; } },
  { ENGINE_RPM,                  (char *) "engineRPM",                 [] (float value) { return value / 4.0; } },
  { FUEL_TANK_LEVEL_INPUT,       (char *) "fuelLevel",                 [] (float value) { return value / 2.55; } },

  { AMBIENT_AIR_TEMP,            (char *) "ambientAirTemperature",     [] (float value) { return value - 40; } },
  { ENGINE_OIL_TEMP,             (char *) "oilTemperature",            [] (float value) { return value - 40; } },
  { ENGINE_COOLANT_TEMP,         (char *) "coolantTemperature",        [] (float value) { return value; } },
  { INTAKE_AIR_TEMP,             (char *) "intakeAirTemperature",      [] (float value) { return value; } },

  { ENGINE_LOAD,                 (char *) "engineLoad",                [] (float value) { return value / 2.55; } },
  { RELATIVE_THROTTLE_POSITION,  (char *) "relativeThrottlePosition",  [] (float value) { return value / 2.55; } },
  { ACTUAL_ENGINE_TORQUE,        (char *) "actualTorque",              [] (float value) { return value - 125; } }
};

void setup()
{
  Serial.begin(115200);


  #if _ENABLE_WIFI_AP

    /* Create soft access point */

    Serial.print("Creating access point... ");

    WiFi.softAP(SSID, PASSWORD);
    // WiFi.softAPConfig(local_ip, gateway, subnet);

    Serial.print("done [");
    Serial.print(WiFi.softAPIP());
    Serial.println("].");

  #else

    /* Connect to existing Wi-Fi */

    Serial.print("Connecting to Wifi... ");

    WiFi.begin("👾", "passworD");

    while (WiFi.status() != WL_CONNECTED)
      delay(500);

    Serial.print("done [");
    Serial.print(WiFi.localIP());
    Serial.println("].");

  #endif


  #if _ENABLE_WEB_SERVER

    /* Initialize SPIFFS */

    Serial.print("Initializing file system... ");

    if (!SPIFFS.begin(true)) {
      DEBUG_PORT.println("failed!\nCouldn't mount SPIFFS. Restarting...");
      delay(3000);
      ESP.restart();
    }

    Serial.println("done.");

    /* Start web server */

    Serial.print("Starting web server... ");

    server.serveStatic("/", SPIFFS, "/");
    // server.serveStatic("/", SPIFFS, "/js/");
    // server.serveStatic("/", SPIFFS, "/images/");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html");
    });

    // WebSocket
    ws.onEvent(handle_client);
    server.addHandler(&ws);

    server.begin();
    // server.addHandler(&events);
  
    Serial.println("done.");

  #else

    Serial.println("Skipping file system and web server...");

  #endif


  #if _ENABLE_OBD

    /* ELM327 */

    Serial.print("Connecting to OBDII... ");

    ELM_PORT.begin("ESP32", true);

    if (!ELM_PORT.connect("OBDII")) {
      DEBUG_PORT.println("failed!\nCouldn't connect to OBD scanner (phase 1). Restarting...");
      delay(3000);
      ESP.restart();
    }

    if (!Engine.begin(ELM_PORT)) {
      DEBUG_PORT.println("failed!\nCouldn't connect to OBD scanner (phase 2). Restarting...");
      delay(3000);
      ESP.restart();
    }

    Serial.println("done.");

  #else

    Serial.println("Skipping OBDII...");

  #endif


  #if _ENABLE_GYRO

    /* Gyro */

    Serial.print("Connecting to and calibrating MPU6050... ");

    if (!mpu.begin()) {
      DEBUG_PORT.println("failed!\nCouldn't connect to MPU6050. Restarting...");
      delay(3000);
      ESP.restart();
    }

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    /* The MPU6050 sensor provides a rad/s measurement, to obtain the absolute
    * delta of radiants from the base values we need to integrate over time.
    * The following are the base values */
    now = millis() / 1000;
    x = g.gyro.x * now;
    y = g.gyro.y * now;

    Serial.println("done.");

  #else

    Serial.println("Skipping gyroscope...");

  #endif

  Serial.println("Setup complete.");
}

void handle_client(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  if (type == WS_EVT_CONNECT) {
    Serial.println("Websocket client connection received");
    target = client;
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("Client disconnected");
    target = NULL;
  }
}

void send_obd_data(struct engine_parameter ep)
{
  Engine.queryPID(SERVICE_01, ep.pid);

  switch(Engine.status) {
    case ELM_SUCCESS:
      // Everything went fine
      target->printf("%s:%d", ep.par_name, (int) ep.map(Engine.findResponse()));
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
      target->printf("%s:%d", "error", (int) Engine.status);
      delay(100);

      DEBUG_PORT.print("ELM error ");
      DEBUG_PORT.println(Engine.status);

      break;
  }
}

void send_gyro_data() {
  sensors_event_t g;

  // Update values
  mpu.getGyroSensor()->getEvent(&g);

  now = millis() / 1000;
  x = g.gyro.x * now;
  y = g.gyro.y * now;

  target->printf("%s:%d", "xTilt", (int) x);
  target->printf("%s:%d", "yTilt", (int) y);

  delay(100);
}

void loop()
{
  if (target != NULL) {
    #if _ENABLE_OBD
      for(int i = 0; i < SIZE(elm_data); i++)
        send_obd_data(elm_data[i]);
    #endif

    #if _ENABLE_GYRO
      send_gyro_data();
    #endif
  }
}
