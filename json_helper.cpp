#include "json_helper.h"
#include "conf.h"
#include "bsp.h"
#include "sensors.h"

DynamicJsonDocument jsonDocSend(ARDUINOJSON_SEND_BUFFER_SIZE);
DynamicJsonDocument jsonDocAck(ARDUINOJSON_ACK_BUFFER_SIZE);
char json_buffer[JSON_BUFFER_SIZE];

char *json_helper_serialize(DynamicJsonDocument &jsonDoc) { 
    serializeJson(jsonDoc, json_buffer, JSON_BUFFER_SIZE); 
    return json_buffer;
}

char *json_helper_parse_send() {
    jsonDocSend["sensor"]["temperature"] = Sensors::getTemperature();
    jsonDocSend["sensor"]["pressure"] = Sensors::getPressure();
    jsonDocSend["sensor"]["brightness"] = Sensors::getBrightness();
    jsonDocSend["peripheral"]["relay"] = Relay_Get();
    jsonDocSend["peripheral"]["led"] = LED_GetBool();
    jsonDocSend["peripheral"]["beeper"] = Beeper_Get();
    jsonDocSend["peripheral"]["motor"] = MotorControl_GetSpeed();
    jsonDocSend["system"]["time"] = getUnixTime();
    jsonDocSend["system"]["temperature"] = temperatureRead();

    return json_helper_serialize(jsonDocSend);
}

char *json_helper_parse_ack(bool status, int msg) {
    jsonDocAck.clear();
    jsonDocAck["acknowledgement"] = status ? ACK_OK : ACK_FAIL;
    jsonDocAck["message_code"] = msg;

    return json_helper_serialize(jsonDocAck);
}
