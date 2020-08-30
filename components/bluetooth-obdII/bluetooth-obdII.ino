/*
  Bluetooth test sketch
  for connecting the ESP32 to ELM327
*/

#define DEBUG_PORT Serial
#define ELM_PORT SerialBT

#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <ELMduino.h>

BluetoothSerial SerialBT;

ELM327 Engine;
StaticJsonDocument<JSON_OBJECT_SIZE(10)> engineData;

void setup()
{
  DEBUG_PORT.begin(115200);

  ELM_PORT.begin("ESP32", true);
  DEBUG_PORT.println("Attempting to connect to ELM327...");

  if (!ELM_PORT.connect("OBDII")) {
    DEBUG_PORT.println("Couldn't connect to OBD scanner");
    while (1);
  }

  Engine.begin(ELM_PORT);

  DEBUG_PORT.println("Connected to ELM327");
  DEBUG_PORT.println("Ensure your serial monitor line ending is set to 'Carriage Return'");
}

void loop()
{
  engineData["VEHICLE_SPEED"] = getValueFromOBD(VEHICLE_SPEED);
  engineData["ENGINE_RPM"] = getValueFromOBD(ENGINE_RPM) / 4.0;
  engineData["FUEL_TANK_LEVEL_INPUT"] = getValueFromOBD(FUEL_TANK_LEVEL_INPUT) / 2.55;

  engineData["AMBIENT_AIR_TEMP"] = getValueFromOBD(AMBIENT_AIR_TEMP) - 40;
  engineData["ENGINE_OIL_TEMP"] = getValueFromOBD(ENGINE_OIL_TEMP) - 40;
  engineData["ENGINE_COOLANT_TEMP"] = getValueFromOBD(ENGINE_COOLANT_TEMP);
  engineData["INTAKE_AIR_TEMP"] = getValueFromOBD(INTAKE_AIR_TEMP);

  engineData["ENGINE_LOAD"] = getValueFromOBD(ENGINE_LOAD) / 2.55;
  engineData["RELATIVE_THROTTLE_POSITION"] = getValueFromOBD(RELATIVE_THROTTLE_POSITION) / 2.55;
  engineData["ACTUAL_ENGINE_TORQUE"] = getValueFromOBD(ACTUAL_ENGINE_TORQUE) - 125;

  serializeJsonPretty(engineData, DEBUG_PORT);
  DEBUG_PORT.println();

  delay(100);

  // Wanna see me do it again?

  /* float tempRPM = myELM327.rpm();

  if (myELM327.status == ELM_SUCCESS) {
    rpm = (uint32_t)tempRPM;
    Serial.print("RPM: "); Serial.println(rpm);
  } else {
    Serial.print(F("\tERROR: "));
    Serial.println(myELM327.status);
    delay(100);
  } */
}


