#include <Arduino.h>
#include <SPIFFS.h>

#include <ESPAsyncWebServer.h>

#include <BluetoothSerial.h>
#include <ELMduino.h>

#include <Adafruit_MPU6050.h>

// Task handlers
TaskHandle_t obd_task = NULL;
TaskHandle_t gyro_task = NULL;

int t = 4;

// Functions
void setup_obd();
void send_obd_data(void *parameters);
void read_obd_datum(ELM327 engine, struct engine_parameter ep);

void setup_gyro();
void send_gyro_data(void *parameters);

void handle_client(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// DEBUG
#define DEBUG_PORT Serial
#define _ENABLE_OBD false
#define _ENABLE_GYRO true
#define _ENABLE_WIFI_AP false
#define _ENABLE_WEB_SERVER true

// SOFT ACCESS POINT SSID & Password
#define SSID "Offroad"
#define PASSWORD "31415926"

// Web server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *target = NULL; // Handle just one client (to handle more trasform this into a list)


/********************************+
 * Gyroscope
 */

Adafruit_MPU6050 mpu;

void send_gyro_data(void *parameters) {
  setup_gyro();

  for (;;) {
    sensors_event_t g;

    // Update values
    mpu.getGyroSensor()->getEvent(&g);
    double now = millis() / 1000;

    target->printf("%s:%d", "xTilt", (int) (g.gyro.x * now));
    target->printf("%s:%d", "yTilt", (int) (g.gyro.y * now));

    delay(200);
  }

  vTaskDelete(NULL);
}

void setup_gyro() {
  Serial.print("Connecting to and calibrating MPU6050... ");

  if (!mpu.begin()) {
    DEBUG_PORT.println("failed!\nCouldn't connect to MPU6050. Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("done.");
}


/********************************+
 * OBD
 */

// OBDII PARAMETERS 
#define PARAMETERS_COUNT 12
#define SIZE(x) sizeof(x) / sizeof(x[0])

// CONNECTION TO OBD
#define ELM_PORT SerialBT

ELM327 engine;
BluetoothSerial ELM_PORT;

// ELM data recovery informations
struct engine_parameter {
  uint8_t pid;
  char *par_name;
  std::function<float (float)> map;
  float old;
};

const engine_parameter elm_data[] = {
  { VEHICLE_SPEED,               (char *) "vehicleSpeed",              [] (float value) { return value; },          0.0 },
  { ENGINE_RPM,                  (char *) "engineRPM",                 [] (float value) { return value / 4.0; },    0.0 },
  { FUEL_TANK_LEVEL_INPUT,       (char *) "fuelLevel",                 [] (float value) { return value / 2.55; },   0.0 },

  { AMBIENT_AIR_TEMP,            (char *) "ambientAirTemperature",     [] (float value) { return value - 40; },     0.0 },
  { ENGINE_OIL_TEMP,             (char *) "oilTemperature",            [] (float value) { return value - 40; },     0.0 },
  { ENGINE_COOLANT_TEMP,         (char *) "coolantTemperature",        [] (float value) { return value; },          0.0 },
  { INTAKE_AIR_TEMP,             (char *) "intakeAirTemperature",      [] (float value) { return value; },          0.0 },

  { ENGINE_LOAD,                 (char *) "engineLoad",                [] (float value) { return value / 2.55; },   0.0 },
  { RELATIVE_THROTTLE_POSITION,  (char *) "relativeThrottlePosition",  [] (float value) { return value / 2.55; },   0.0 },
  { ACTUAL_ENGINE_TORQUE,        (char *) "actualTorque",              [] (float value) { return value - 125; },    0.0 }
};

void send_obd_data(void *parameters) {
  if (!engine.connected) {
    setup_obd();
  }

  for (int i = 0;; i = (i + 1) % SIZE(elm_data)) {
    read_obd_datum(engine, elm_data[i]);
  }

  vTaskDelete(NULL);
}

void read_obd_datum(ELM327 engine, struct engine_parameter ep)
{
  engine.queryPID(SERVICE_01, ep.pid);
  float value;

  switch (engine.status) {
    case ELM_SUCCESS:
      // Everything went fine
      value = ep.map(engine.findResponse());

      if (value != ep.old) { // Send only if new (to reduce traffic)
        target->printf("%s:%d", ep.par_name, (int) value);
        ep.old = value;
        delay(100);
      }

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
      target->printf("%s:%d", "error", (int) engine.status);
      delay(100);

      DEBUG_PORT.print("ELM error ");
      DEBUG_PORT.println(engine.status);

      break;
  }
}

void setup_obd() {
  Serial.print("Connecting to OBDII... ");

  ELM_PORT.begin("ESP32", true);

  if (!ELM_PORT.connect("OBDII")) {
    DEBUG_PORT.println("failed!\nCouldn't connect to OBD scanner (phase 1). Restarting...");
    delay(3000);
    ESP.restart();
  }

  if (!engine.begin(ELM_PORT)) {
    DEBUG_PORT.println("failed!\nCouldn't connect to OBD scanner (phase 2). Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("done.");
}


/********************************+
 * Socket
 */

void handle_client(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  if (type == WS_EVT_CONNECT) {
  
    Serial.println("Websocket client connection received");
    target = client;

    if (_ENABLE_OBD && obd_task == NULL) {
      xTaskCreatePinnedToCore(send_obd_data, "OBD", 10000, NULL, 1, &obd_task, 1);
    }

    if (_ENABLE_GYRO && gyro_task == NULL) {
      xTaskCreatePinnedToCore(send_gyro_data, "GYRO", 10000, NULL, 1, &gyro_task, 1);
    }

  } else if (type == WS_EVT_DISCONNECT) {
    
    Serial.println("Client disconnected");

    vTaskDelete(obd_task);
    vTaskDelete(gyro_task);
    
    target = NULL;

  }
}


/********************************+
 * Setup
 */

void setup()
{
  Serial.begin(115200);


  #if _ENABLE_WIFI_AP

    /* Create soft access point */

    Serial.print("Creating access point... ");
    WiFi.softAP(SSID, PASSWORD);

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

    server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
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


  Serial.println("Setup complete.");
}

void loop() {}