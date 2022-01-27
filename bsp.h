#if !defined(_BSP_H_)
#define _BSP_H_

#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <Preferences.h>

#define TASK(T) for(static unsigned long var = 0; var + (T) < millis(); var = millis())

// Pins
static const uint8_t KEY1 = 25, KEY2 = 27;
static const uint8_t LED1_PIN = LED_BUILTIN;
static const uint8_t RELAY_PIN = 33;
static const uint8_t BEEPER_PIN = 23;
static const uint8_t ADC_PIN = 34;

static const uint8_t BMP280_SCL_PIN = 22,
                     BMP280_SDA_PIN = 21;

static const uint8_t OLED_CLK_PIN = 14,
                     OLED_MOSI_PIN = 13,
                     OLED_RESET_PIN = 5,
                     OLED_DC_PIN = 4,
                     OLED_CS_PIN = 15;

static const uint8_t INFRARED_OUT_PIN = 26,
                     INFRARED_IN_PIN = 19;

static const uint8_t MOTOR_CTL_A_PIN = 12,
                     MOTOR_CTL_B_PIN = 18;

static const uint8_t PWM_OUT_PIN = 32;
static const uint8_t MOTOR_PWM_CHANNEL = 1;

#define KEY_PRESSED(KEY_PIN) (!digitalRead(KEY_PIN))
#define RELAY_SET(STA) digitalWrite(RELAY_PIN, (STA) ? HIGH : LOW)
#define BEEPER_SET(STA) digitalWrite(BEEPER_PIN, (STA) ? HIGH : LOW)

// Peripherals
extern Adafruit_SSD1306 OLED;
extern Adafruit_BMP280 BMP280;

typedef enum {
    KEY_RELEASE,
    KEY_PRESS_NOT_STABLE,
    KEY_PRESS,
    KEY_LONG_PRESS,
} key_status_t;

void KEY_Setup();
void key_scan();
bool key_is(int id, key_status_t sta);
bool key_is_not(int id, key_status_t sta);
int key_get_key();
key_status_t key_get_status();

void LED_Setup();
void LED_Set(uint8_t val);
void LED_Set(bool sta);
uint8_t LED_Get();
void LED_Flip();

void Relay_Setup();
void Relay_Set(bool);
bool Relay_Get();
void Relay_Flip();

void Beeper_Setup();
void Beeper_Set(bool);
bool Beeper_Get();
void Beeper_Flip();

void MotorControl_Setup();
void MotorControl_SetSpeed(int val);
int MotorControl_GetSpeed();

void OLED_Setup();
void BMP280_Setup();
uint16_t Photoresistor_GetRaw();
float Photoresistor_GetVoltage();

void Preferences_Init();
Preferences& Preferences_Get();

bool WIFI_Setup();
bool WIFI_IsConnected();
void WiFi_RestoreSettings(Preferences &pref);
bool WiFi_UpdateSetting(String setting, Preferences &pref);
bool WiFi_UpdateSetting(const char *_ssid, const char *_password, Preferences &pref);

void TimeStamp_Restore(Preferences &pref);
void TimeStamp_Update(long t, Preferences &pref);
void TimeStamp_Update(Preferences &pref);

void NTP_Setup();
int setUnixtime(time_t unixtime);
time_t getUnixTime();
String LocalTime_GetString();

#endif // _BSP_H_
