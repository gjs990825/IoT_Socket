#if !defined(_INFRARED_H_)
#define _INFRARED_H_

#include <stdint.h>
#include <Preferences.h>

#define INFRARED_CAPTURE_TIMEOUT 3000

typedef struct {
    uint8_t code[100];
    uint8_t len;
} infrared_code_t;

bool Infrared_IsCapturing();
// void Infrared_StartCapture();
bool Infrared_StartCapture(int n = 0);
bool Infrared_EndCapture(int n);
bool Infrared_EndCapture();
void Infrared_CheckCapture();
bool Infrared_SendPreset(int n);
void Infrared_RestorePreset(Preferences &pref);
void Infrared_StorePreset();
void Infrared_StorePreset(Preferences &pref);
bool Infrared_StorePreset(int n, Preferences &pref);
bool Infrared_RemovePreset(int n, Preferences &pref);

#endif // _INFRARED_H_
