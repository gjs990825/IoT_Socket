#if !defined(_INFRARED_H_)
#define _INFRARED_H_

#include <stdint.h>
#include <Preferences.h>
#include "conf.h"

bool Infrared_IsCapturing();
bool Infrared_Capture(int n = 0);
bool Infrared_SendPreset(int n);
void Infrared_RestorePreset(Preferences &pref);
void Infrared_StorePreset();
void Infrared_StorePreset(Preferences &pref);
bool Infrared_StorePreset(int n, Preferences &pref);
bool Infrared_RemovePreset(int n, Preferences &pref);

#endif // _INFRARED_H_
