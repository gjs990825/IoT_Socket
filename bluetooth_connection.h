#if !defined(_BLUETOOTH_CONNECTION_H_)
#define _BLUETOOTH_CONNECTION_H_

#include <WString.h>

void Bluetooth_Setup();
void Bluetooth_SetCommandTools(bool (*handler)(String), int (*msg_code_getter)(void));
void Bluetooth_Send(const char *payload);
void Bluetooth_Ack(bool status, int msg_code = 0);
bool Bluetooth_IsConnected();

#endif // _BLUETOOTH_CONNECTION_H_
