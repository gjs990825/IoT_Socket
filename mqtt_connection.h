#if !defined(_MQTT_CONNECTION_H_)
#define _MQTT_CONNECTION_H_

#include <Arduino.h>
#include "conf.h"

void MQTT_Setup();
void MQTT_SetCommandHandler(bool (*handler)(String));
void MQTT_Check();
void MQTT_Send();
void MQTT_Send(const char *payload);
unsigned long MQTT_GetLastSend();
void MQTT_Ack(bool status);

#endif // _MQTT_CONNECTION_H_
