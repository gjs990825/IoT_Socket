#if !defined(_BLUETOOTH_CONNECTION_H_)
#define _BLUETOOTH_CONNECTION_H_

#include <Arduino.h>

void Bluetooth_Setup();
void Bluetooth_SetCommandHandler(bool (*handler)(String));
void Bluetooth_Send(const char *payload);
bool Bluetooth_IsConnected();

#endif // _BLUETOOTH_CONNECTION_H_
