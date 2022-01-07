#if !defined(_INFRARED_H_)
#define _INFRARED_H_

#include <stdint.h>
#include <Preferences.h>

typedef struct {
    uint8_t code[100];
    uint8_t len;
} infrared_code_t;

void Infrared_StartCapture();
void Infrared_EndCapture(int n);
void Infrared_SendPreset(int n);
void Infrared_RestorePreset(Preferences &pref);
void Infrared_StorePreset(Preferences &pref);

#endif // _INFRARED_H_
