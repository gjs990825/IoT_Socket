#include "bsp.h"
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Preferences.h>
#include "infrared.h"

const uint8_t LED1_CHANNEL = 0;

uint8_t led_status = 0;
void LED_Set(uint8_t val) {
    led_status = val;
    ledcWrite(LED1_CHANNEL, val);
}

void LED_Set(bool sta) {
    LED_Set((uint8_t)(sta ? 0xFF : 0x00));
}

uint8_t LED_Get() {
    return led_status;
}

void LED_Flip() {
    LED_Set(LED_Get() == 0);
}

void LED_Setup() {
    pinMode(LED1_PIN, OUTPUT);
    ledcSetup(LED1_CHANNEL, 1000, 8);
    ledcAttachPin(LED1_PIN, LED1_CHANNEL);
    LED_Set(false);
}

void KEY_Setup() {
    pinMode(KEY1, INPUT_PULLUP);
    pinMode(KEY2, INPUT_PULLUP);
}

const int keys[] = {KEY1, KEY2};
const int key_count = sizeof(keys) / sizeof(keys[0]);

int key_get(void) {
    for (int i = 0; i < key_count; i++)
        if (KEY_PRESSED(keys[i]))
            return keys[i];
    return -1;
}

int s_key = 0;
key_status_t s_status = KEY_RELEASE;

int key_get_key() { return s_key; }
key_status_t key_get_status() { return s_status; }

bool key_is(int id, key_status_t sta) { return key_get_key() == id && key_get_status() == sta; }
bool key_is_not(int id, key_status_t sta) { return !key_is(id, sta); }

void key_scan() {
	static long key_t;
	
	s_key = key_get();
	if (s_key == -1){
		s_status = KEY_RELEASE;
		return;
	}
	
	switch(s_status) {
		case KEY_RELEASE: s_status = KEY_PRESS_NOT_STABLE;
			break;
		case KEY_PRESS_NOT_STABLE: s_status = KEY_PRESS; key_t = millis();
			break;
		case KEY_PRESS: if(key_t + 1000 < millis()) s_status = KEY_LONG_PRESS;
			break;
		default:
			break;
	}
}

bool relay_status = false;
void Relay_Set(bool sta) {
    relay_status = sta;
    RELAY_SET(sta);
}

bool Relay_Get() {
    return relay_status;
}

void Relay_Flip() {
    Relay_Set(!Relay_Get());
}

void Relay_Setup() {
    pinMode(RELAY_PIN, OUTPUT);
    Relay_Set(false);
}

bool beeper_status = false;
bool Beeper_Get() {
    return beeper_status;
}

void Beeper_Set(bool sta) {
    beeper_status = sta;
    BEEPER_SET(sta);
}

void Beeper_Setup() {
    pinMode(BEEPER_PIN, OUTPUT);
    Beeper_Set(false);
}

void Beeper_Flip() {
    Beeper_Set(!Beeper_Get());
}

void MotorControl_Setup() {
    pinMode(PWM_OUT_PIN, OUTPUT);
    pinMode(MOTOR_CTL_A_PIN, OUTPUT);
    pinMode(MOTOR_CTL_B_PIN, OUTPUT);
    ledcSetup(MOTOR_PWM_CHANNEL, 15000, 10); // 15KHz, 10bits
    ledcAttachPin(PWM_OUT_PIN, MOTOR_PWM_CHANNEL);
}

int motor_speed = 0;

void MotorControl_SetSpeed(int val) {
    motor_speed = val;
    val = constrain(val, -100, 100);
    bool is_positive = val >= 0;
    digitalWrite(MOTOR_CTL_A_PIN, is_positive);
    digitalWrite(MOTOR_CTL_B_PIN, !is_positive);
    // 0 ~ 100 => 0 ~ 2^10
    uint32_t duty = (uint32_t)map(abs(val), 0, 100, 0, 0x3FF);
    ledcWrite(MOTOR_PWM_CHANNEL, duty);
}

int MotorControl_GetSpeed() {
    return motor_speed;
}

Adafruit_SSD1306 OLED(128, 64, OLED_MOSI_PIN, OLED_CLK_PIN, OLED_DC_PIN, OLED_RESET_PIN, OLED_CS_PIN);

void OLED_Setup() {
    OLED.begin(SSD1306_SWITCHCAPVCC);
    OLED.clearDisplay();
    OLED.setTextColor(WHITE);
    OLED.setCursor(0, 0);
    OLED.setTextSize(1);
    OLED.println("OLED OK");
    OLED.display();
}

Adafruit_BMP280 BMP280;

void BMP280_Setup() {
    Wire.begin(BMP280_SDA_PIN, BMP280_SCL_PIN);
    if (!BMP280.begin(BMP280_ADDRESS_ALT)) {
        log_e("BMP280 ERR!");
    } else {
        BMP280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                           Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                           Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                           Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                           Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    }
}

Preferences preferences;

void Preferences_Init() {
    preferences.begin("wifi_access", false);

#if (PREFERENCES_DEBUG)
    preferences.clear(); // clear config
    log_w("Preferences cleared");
#endif // PREFERENCES_DEBUG

    Preferences_RestoreWIFISetting();
    Preferences_RestoreTimeStamp();
    Infrared_RestorePreset(preferences);
}

Preferences& Preferences_Get() {
    return preferences;
}

void Preferences_RestoreWIFISetting() {
    ssid = preferences.getString("pref_ssid");
    password = preferences.getString("pref_pass");
    log_i("Wi-Fi setting restored: ssid:\"%s\" password:\"%s\"", 
        ssid.c_str(), password.c_str());
}

bool Preferences_UpdateWIFISetting(String setting) {
    char ssidBuf[20], passwordBuf[20];
    int res = sscanf(setting.c_str(), "SSID:%s PASS:%s", ssidBuf, passwordBuf);
    if (res >= 1) {
        if (res == 1) passwordBuf[0] = '\0';
        return Preferences_UpdateWIFISetting(ssidBuf, passwordBuf);
    }
    return false;
}

bool Preferences_UpdateWIFISetting(const char *_ssid, const char *_password) {
    ssid = _ssid; password = _password;
    if (WIFI_Setup()) {
        preferences.putString("pref_ssid", ssid);
        preferences.putString("pref_pass", password);
        log_i("Wi-Fi setting updated");
        return true;
    } else {
        log_e("Wi-Fi setting invalid");
        return false;
    }
}

void Preferences_RestoreTimeStamp() {
    timeStamp = preferences.getLong("pref_time_stamp");
    log_i("time stamp restored:%ld", timeStamp);
}

void Preferences_UpdateTimeStamp(long t) {
    if (t > 1577836800) { // 2020.01.01
        setUnixtime(t);
        preferences.putLong("pref_time_stamp", t);
        log_i("time preference updated");
    } else {
        log_e("time stamp error");
    }
}

bool WIFI_Setup() {
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    Serial.println("Connecting");
    int retry = 0;
    while (!WIFI_IsConnected()) {
        delay(500);
        Serial.print(".");
        if (++retry > 10) {
            Serial.println();
            log_e("Startup Failed!");
            OLED.println("WiFi Startup Failed!");
            return false;
        }
    }
    Serial.println();
    log_i("Address:%s", WiFi.localIP().toString().c_str());
    OLED.print("IP:");
    OLED.println(WiFi.localIP());
    OLED.display();

    return true;
}

bool WIFI_IsConnected() { return  WiFi.status() == WL_CONNECTED; }

int setUnixtime(time_t unixtime) {
    timeval epoch = {unixtime, 0};
    return settimeofday((const timeval*)&epoch, 0);
}

time_t getUnixTime() {
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return 0;
    }
    time(&now);
    return now;
}

void Preferences_UpdateTimeStamp() {
    timeval epoch;
    gettimeofday(&epoch, 0);
    Preferences_UpdateTimeStamp(epoch.tv_sec);
}

void NTP_Setup() {
    setUnixtime();
    configTime(GMT_OFFSET, DAYLIGHT_OFFSET, NTP_SERVER);
}

String LocalTime_GetString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 5))
        return String("--:--:--");
    char buf[30];
    strftime(buf, 30, "%T", &timeinfo);
    return String(buf);
}

uint16_t Photoresistor_GetRaw() {
    return analogRead(ADC_PIN);
}

float Photoresistor_GetVoltage() {
    return ((Photoresistor_GetRaw() * 3.3) / 4095);
}
