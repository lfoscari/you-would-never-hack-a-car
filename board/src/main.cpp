#include <Arduino.h>
#include <SPIFFS.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>
#include <Wire.h>

#define CONFIG_ASYNC_TCP_RUNNING_CORE 1
#include <ESPAsyncWebServer.h>

// UTIL
#define SIZE(x) sizeof(x) / sizeof(x[0])

// DEBUG
#define DEBUG_PORT Serial
#define _ENABLE_OBD true
#define _ENABLE_GYRO true
#define _ENABLE_WIFI_SAP true
#define _ENABLE_WEB_SERVER true

// FUNCTIONS
void setup_obd();
void send_obd_data();
void read_obd(struct engine_parameter ep);

void setup_gyro();
void send_gyro_data();

void handle_client(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// SOFT ACCESS POINT SSID & Password
#define SSID "Offroad"
#define PASSWORD "31415926"

// WEB SERVER
AsyncWebServer server(80);
AsyncWebSocket ws("/engine");
AsyncWebSocketClient *target = NULL; // Handle just one client (to handle more trasform this into a list)


/********************************+
 * Gyroscope
 */

#define MPU_addr 0x68
#define minVal 265
#define maxVal 402

bool gyro_ready = false;
int16_t AcX, AcY, AcZ, xAng, yAng, zAng, x, y;

void send_gyro_data() {
    
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();

  xAng = map(AcX, minVal, maxVal, -90, 90);
  yAng = map(AcY, minVal, maxVal, -90, 90);
  zAng = map(AcZ, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);

  target->printf("%s:%d", "xTilt", x > 180 ? 360 - x : -1 * x);
  delay(100);

  target->printf("%s:%d", "yTilt", y > 180 ? 360 - y : -1 * y);
  delay(100);
}

void setup_gyro() {
  Serial.print("Connecting to and calibrating MPU6050... ");

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  gyro_ready = true;
  Serial.println("done.");
}


/********************************+
 * OBD
 */

#define ELM_PORT SerialBT

bool obd_ready = false;
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
  { VEHICLE_SPEED,                (char *) "vehicleSpeed",              [] (float value) { return value; },          -1.0f },
  { ENGINE_RPM,                   (char *) "engineRPM",                 [] (float value) { return value / 4.0; },    -1.0f },
  { INTAKE_MANIFOLD_ABS_PRESSURE, (char *) "intakePressure",            [] (float value) { return value; },          -1.0f },

  { INTAKE_AIR_TEMP,              (char *) "intakeAirTemperature",      [] (float value) { return value; },          -1.0f },
  { AMBIENT_AIR_TEMP,             (char *) "ambientAirTemperature",     [] (float value) { return value - 40.0; },   -1.0f },
  { ENGINE_COOLANT_TEMP,          (char *) "coolantTemperature",        [] (float value) { return value; },          -1.0f },

  { ENGINE_LOAD,                  (char *) "engineLoad",                [] (float value) { return value / 2.55; },   -1.0f },
  { RELATIVE_THROTTLE_POSITION,   (char *) "relativeThrottlePosition",  [] (float value) { return value / 2.55; },   -1.0f },
};

void read_obd(struct engine_parameter ep) {
  engine.queryPID(SERVICE_01, ep.pid);
  float value;

  switch (engine.status) {
    case ELM_SUCCESS:
      // Everything went fine
      value = ep.map(engine.findResponse());

      if (value != ep.old) { // Send only if new (to reduce traffic)
        target->printf("%s:%d", ep.par_name, (int) value);
        ep.old = value;

        delay(10);
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

      DEBUG_PORT.print("ELM error ");
      DEBUG_PORT.println(engine.status);

      delay(10);
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

  // Serial.print("1-20: ");
  // Serial.println(engine.supportedPIDs_1_20(), BIN);

  // Serial.print("21-40: ");
  // Serial.println(engine.supportedPIDs_21_40(), BIN);

  // Serial.print("41-60: ");
  // Serial.println(engine.supportedPIDs_41_60(), BIN);

  // Serial.print("61-80: ");
  // Serial.println(engine.supportedPIDs_61_80(), BIN);

  obd_ready = true;
  Serial.println("done.");
}


/********************************+
 * Socket
 */

void handle_client(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
  
    Serial.println("New client");
    target = client;

  } else if (type == WS_EVT_DISCONNECT) {

    Serial.println("Client disconnected");
    free(target);
    target = NULL;

  }
}


/********************************+
 * Setup
 */

void setup() {
  Serial.begin(115200);

  #if _ENABLE_WIFI_SAP

    /* Create soft access point */

    Serial.print("Creating access point... ");
    WiFi.softAP(SSID, PASSWORD);

    Serial.print("done [");
    Serial.print(WiFi.softAPIP());
    Serial.println("].");

  #else

    /* Connect to existing Wi-Fi */

    Serial.print("Connecting to Wifi... ");

    WiFi.begin("", "");

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

    server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html");
    });

    // WebSocket
    ws.onEvent(handle_client);
    server.addHandler(&ws);

    server.begin();
  
    Serial.println("done.");

  #else
    Serial.println("Skipping file system and web server...");
  #endif

  Serial.println("Setup complete.");
}

void loop() {
  if (target != NULL) {

    if (_ENABLE_GYRO) {
      if (!gyro_ready) {
        setup_gyro();
      }

      send_gyro_data();
    }

    if (_ENABLE_OBD) {
      if (!obd_ready) {
        setup_obd();
      }

      for (int i = 0; i < SIZE(elm_data); i++) {
        read_obd(elm_data[i]);
      }
    }

  }
}