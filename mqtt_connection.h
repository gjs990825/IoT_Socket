#if !defined(_MQTT_CONNECTION_H_)
#define _MQTT_CONNECTION_H_

#include <WString.h>

void MQTT_Setup();
void MQTT_SetCommandTools(bool (*handler)(String), int (*msg_code_getter)(void));
void MQTT_Check();
void MQTT_Send();
void MQTT_Send(const char *payload);
unsigned long MQTT_GetLastSend();
void MQTT_Ack(bool status, int msg_code = 0);

#endif // _MQTT_CONNECTION_H_
