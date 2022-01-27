#include "json_helper.h"
#include "conf.h"
#include "bsp.h"
#include "sensors.h"

DynamicJsonDocument jsonDoc(ARDUINOJSON_BUFFER_SIZE);
char json_buffer[JSON_BUFFER_SIZE];

char *json_helper_serialize() { 
    serializeJson(jsonDoc, json_buffer, JSON_BUFFER_SIZE); 
    return json_buffer;
}

char *json_helper_parse_send() {
    jsonDoc["sensor"]["temperature"] = Sensors::getTemperature();
    jsonDoc["sensor"]["pressure"] = Sensors::getPressure();
    jsonDoc["sensor"]["brightness"] = Sensors::getBrightness();
    jsonDoc["peripheral"]["relay"] = Relay_Get();
    jsonDoc["peripheral"]["led"] = LED_Get();
    jsonDoc["peripheral"]["beeper"] = Beeper_Get();
    jsonDoc["peripheral"]["motor"] = MotorControl_GetSpeed();
    jsonDoc["system"]["time"] = getUnixTime();
    jsonDoc["system"]["temperature"] = temperatureRead();

    return json_helper_serialize();
}
