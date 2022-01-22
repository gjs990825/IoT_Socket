#include "json_helper.h"
#include "conf.h"
#include "ArduinoJson.h"
#include "bsp.h"
#include "sensors.h"

DynamicJsonDocument doc(1024);
char json_buffer[JSON_BUFFER_SIZE];

char *parse_json_buffer() {
    doc["sensor"]["temperature"] = Sensors::getTemperature();
    doc["sensor"]["pressture"] = Sensors::getPressure();
    doc["sensor"]["brightness"] = Sensors::getBrightness();
    doc["peripheral"]["relay"] = Relay_Get();
    doc["peripheral"]["led"] = LED_Get();
    doc["peripheral"]["beeper"] = Beeper_Get();
    doc["peripheral"]["motor"] = MotorControl_GetSpeed();
    doc["system"]["time"] = getUnixTime();
    doc["system"]["temperature"] = temperatureRead();

    serializeJson(doc, json_buffer, JSON_BUFFER_SIZE);
    return json_buffer;
}
