#include "bsp.h"
#include "conf.h"
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Preferences.h>

LED::LED() {
    pinMode(LED1_PIN, OUTPUT);
    ledcSetup(LED1_CHANNEL, 1000, 8);
    ledcAttachPin(LED1_PIN, LED1_CHANNEL);
    set(false);
}

void LED::set(int v) {
    output = constrain(v, 0, 0xFF);
    ledcWrite(LED1_CHANNEL, output);
}

void LED::set(bool v) { 
    set(v == false ? 0 : 255);
}

RELAY::RELAY() {
    pinMode(RELAY_PIN, OUTPUT);
    set(false);
};

void RELAY::set(int v) { set(!!v); }

void RELAY::set(bool v) { 
    output = v;
    RELAY_SET(output);
}

BEEPER::BEEPER() {
    pinMode(BEEPER_PIN, OUTPUT);
    set(false);
};

void BEEPER::set(int v) { set(!!v); }

void BEEPER::set(bool v) { 
    output = v;
    BEEPER_SET(output);
}

PWM::PWM() {
    pinMode(PWM_OUT_PIN, OUTPUT);
    pinMode(MOTOR_CTL_A_PIN, OUTPUT);
    pinMode(MOTOR_CTL_B_PIN, OUTPUT);
    ledcSetup(MOTOR_PWM_CHANNEL, 15000, 10); // 15KHz, 10bits
    ledcAttachPin(PWM_OUT_PIN, MOTOR_PWM_CHANNEL);
}

void PWM::set(int v) {
    output = constrain(v, -100, 100);
    bool is_positive = output >= 0;
    digitalWrite(MOTOR_CTL_A_PIN, is_positive);
    digitalWrite(MOTOR_CTL_B_PIN, !is_positive);
    // 0 ~ 100 => 0 ~ 2^10
    uint32_t duty = (uint32_t)map(abs(output), 0, 100, 0, 0x3FF);
    ledcWrite(MOTOR_PWM_CHANNEL, duty);
}

void PWM::set(bool v) { set(v ? 100 : 0); }

LED Led;
RELAY Relay;
BEEPER Beeper;
PWM Pwm;

const int keys[] = {KEY1, KEY2};

void KEY_Setup() {
    for (auto &&key : keys) { 
        pinMode(key, INPUT_PULLUP);
    }
}

int key_get(void) {
    for (auto &&key : keys) {
        if (KEY_PRESSED(key))
            return key;
    }
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

key_handler k1_press = NULL;
key_handler k2_press = NULL;
key_handler k1_long_press = NULL;
key_handler k2_long_press = NULL;

void key_set_handler(key_handler k1,
                    key_handler k1_long,
                    key_handler k2,
                    key_handler k2_long) {
    k1_press = k1;
    k1_long_press = k1_long;
    k2_press = k2;
    k2_long_press = k2_long;
}

#define CHECK_KEY(PIN, PRESS_HANDLER, LONG_PRESS_HANDLER) \
    do {                                                  \
        if (key_is(PIN, KEY_PRESS)) {                     \
            while (key_is(PIN, KEY_PRESS)) {              \
                key_scan();                               \
            }                                             \
            if (key_is(PIN, KEY_LONG_PRESS)) {            \
                if (LONG_PRESS_HANDLER) {                 \
                    LONG_PRESS_HANDLER();                 \
                }                                         \
                while (key_is(PIN, KEY_LONG_PRESS)) {     \
                    key_scan();                           \
                }                                         \
            } else {                                      \
                if (PRESS_HANDLER) {                      \
                    PRESS_HANDLER();                      \
                }                                         \
            }                                             \
        }                                                 \
    } while (0)

void key_check() {
    static uint8_t count;

    // every 10ms
    key_scan();
    if (++count == 5) {
        count = 0;

        // every 50ms
        CHECK_KEY(KEY1, k1_press, k1_long_press);
        CHECK_KEY(KEY2, k2_press, k2_long_press);
    }
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

Preferences preferences;

void Preferences_Init() {
    preferences.begin("wifi_access", false);

#if (PREFERENCES_DEBUG)
    preferences.clear(); // clear config
    log_w("Preferences cleared");
#endif // PREFERENCES_DEBUG
}

Preferences& Preferences_Get() {
    return preferences;
}

void WiFi_RestoreSettings(Preferences &pref) {
    ssid = pref.getString("pref_ssid");
    password = pref.getString("pref_pass");
    log_i("Wi-Fi setting restored: ssid:\"%s\" password:\"%s\"", 
        ssid.c_str(), password.c_str());
}

bool WiFi_UpdateSetting(String setting, Preferences &pref) {
    char ssidBuf[20], passwordBuf[20];
    int res = sscanf(setting.c_str(), "SSID:%s PASS:%s", ssidBuf, passwordBuf);
    if (res >= 1) {
        if (res == 1) passwordBuf[0] = '\0';
        return WiFi_UpdateSetting(ssidBuf, passwordBuf, pref);
    }
    return false;
}

bool WiFi_UpdateSetting(const char *_ssid, const char *_password, Preferences &pref) {
    ssid = _ssid; password = _password;
    if (WIFI_Setup()) {
        pref.putString("pref_ssid", ssid);
        pref.putString("pref_pass", password);
        log_i("Wi-Fi setting updated");
        return true;
    } else {
        log_e("Wi-Fi setting invalid");
        return false;
    }
}

bool WiFi_ForceUpdateSetting(const char *_ssid, const char *_password, Preferences &pref) {
    ssid = _ssid; password = _password;
    pref.putString("pref_ssid", ssid);
    pref.putString("pref_pass", password);
    WIFI_Setup();
    log_i("Wi-Fi setting updated");
    return true;
}

void TimeStamp_Restore(Preferences &pref) {
    timeStamp = pref.getLong("pref_time_stamp");
    log_i("time stamp restored:%ld", timeStamp);
}

bool TimeStamp_Update(long t, Preferences &pref) {
    if (t > 1577836800) { // 2020.01.01
        setUnixtime(t);
        pref.putLong("pref_time_stamp", t);
        log_i("time preference updated");
        return true;
    } else {
        log_e("time stamp error");
        return false;
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

bool TimeStamp_Update(Preferences &pref) {
    timeval epoch;
    gettimeofday(&epoch, 0);
    return TimeStamp_Update(epoch.tv_sec, pref);
}

void NTP_Setup() {
    setUnixtime(timeStamp);
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
