#if !defined(_BSP_H_)
#define _BSP_H_

#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include "conf.h"

#define TASK(T) for(static unsigned long var = 0; var + (T) < millis(); var = millis())

// Pins
static const int K1 = 25, K2 = 27;
static const int LED1 = LED_BUILTIN;
static const int RELAY = 33;
static const int BEEP = 23;
static const int ADC_PIN = 32;
static const int BMP280_SCL = 22, BMP280_SDA = 21;
static const int OLED_CLK = 14,
                 OLED_MOSI = 13,
                 OLED_RESET = 5,
                 OLED_DC = 4,
                 OLED_CS = 15;

// Peripherals
extern Adafruit_SSD1306 OLED;
extern Adafruit_BMP280 BMP280;

// Macros
#define KEY_PRESSED(ID) (!digitalRead(ID))
// #define LED_SET(STA) LED_Set((STA) ? 0xFF : 0x00)
#define RELAY_SET(STA) digitalWrite(RELAY, (STA) ? HIGH : LOW)
#define BEEP_SET(STA) digitalWrite(BEEP, (STA) ? HIGH : LOW)

typedef enum {
    KEY_RELEASE,
    KEY_PRESS_NOT_STABLE,
    KEY_PRESS,
    KEY_LONG_PRESS,
} key_status_t;


// Initialization and operation functions
void LED_Set(uint8_t val);
void LED_Set(bool sta);
uint8_t LED_Get();
void LED_Flip();
void LED_Setup();

void KEY_Setup();
void key_scan();
bool key_is(int id, key_status_t sta);
bool key_is_not(int id, key_status_t sta);
int key_get_key();
key_status_t key_get_status();

void Relay_Set(bool sta);
bool Relay_Get();
void Relay_Flip();
void Relay_Setup();
bool Beep_Get();
void Beep_Set(bool sta);
void Beep_Setup();
void Beep_Flip();

void OLED_Setup();
void BMP280_Setup();
uint16_t Photoresistor_GetRaw();
float Photoresistor_GetVoltage();

void Preferences_Init();
void updateTimePreference();
void BlueTooth_Setup();
bool WIFI_Setup();
void NTP_Setup();
int setUnixtime(time_t unixtime = timeStamp);
String LocalTime_GetString();

#endif // _BSP_H_
