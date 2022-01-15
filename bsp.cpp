#include "bsp.h"
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Preferences.h>
#include <BluetoothSerial.h>
#include "infrared.h"
#include <MQTT.h>
#include <ArduinoJson.h>
#include "sensors.h"

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
    pinMode(LED1, OUTPUT);
    ledcSetup(LED1_CHANNEL, 1000, 8);
    ledcAttachPin(LED1, LED1_CHANNEL);
    LED_Set(false);
}

void KEY_Setup() {
    pinMode(K1, INPUT_PULLUP);
    pinMode(K2, INPUT_PULLUP);
}

const int keys[] = {K1, K2};
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
    pinMode(RELAY, OUTPUT);
    Relay_Set(false);
}

bool beep_status = false;
bool Beep_Get() {
    return beep_status;
}

void Beep_Set(bool sta) {
    beep_status = sta;
    BEEP_SET(sta);
}

void Beep_Setup() {
    pinMode(BEEP, OUTPUT);
    Beep_Set(false);
}

void Beep_Flip() {
    Beep_Set(!Beep_Get());
}

void MotorControl_Setup() {
    pinMode(PWM_OUT_PIN, OUTPUT);
    pinMode(M_CTL_A_PIN, OUTPUT);
    pinMode(M_CTL_B_PIN, OUTPUT);
    ledcSetup(MOTOR_PWM_CHANNEL, 15000, 10); // 15KHz, 10bits
    ledcAttachPin(PWM_OUT_PIN, MOTOR_PWM_CHANNEL);
}

int motor_speed = 0;

void MotorControl_SetSpeed(int val) {
    motor_speed = val;
    val = constrain(val, -100, 100);
    bool is_positive = val >= 0;
    digitalWrite(M_CTL_A_PIN, is_positive);
    digitalWrite(M_CTL_B_PIN, !is_positive);
    uint32_t duty = (uint32_t)map(abs(val), 0, 100, 0, 0x3FF);
    ledcWrite(MOTOR_PWM_CHANNEL, duty);
}

int MotorControl_GetSpeed() {
    return motor_speed;
}

Adafruit_SSD1306 OLED(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

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
    Wire.begin(BMP280_SDA, BMP280_SCL);
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

BluetoothSerial SerialBT;
Preferences preferences;

void Preferences_Init() {
    preferences.begin("wifi_access", false);

#if defined(PREFERENCES_DEBUG)
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
    while (WiFi.status() != WL_CONNECTED) {
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

void blueToothCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    if (event == ESP_SPP_DATA_IND_EVT)
    {
        String btString = SerialBT.readString();
        log_i("BT:%s", btString.c_str());
        SerialBT.printf(":\"%s\"\n", btString.c_str());
        Preferences_UpdateWIFISetting(btString);
    }
    if (event == ESP_SPP_CL_INIT_EVT) {
        log_i("BT Connected");
    }
}


WiFiClient net;
MQTTClient client(512);

bool (*command_handler)(String cmd) = NULL;

void MQTT_SetCommandHandler(bool (*handler)(String)) {
    command_handler = handler;
}

bool MQTT_Connect() {
    if (WiFi.status() != WL_CONNECTED) {
        log_e("no connection, mqtt connect failed");
        return false;
    }
    if (!client.connect("IoT_Socket", "esp32", "password")) {
        log_e("disconnected");
        return false;
    }
    log_i("connected");
    client.subscribe(MQTT_TOPIC_COMMAND);
    return true;
}

void mqtt_message_received(String &topic, String &payload) {
    if (topic == MQTT_TOPIC_COMMAND) {
        if (command_handler != NULL) {
            if (!command_handler(payload)) {
                log_e("cmd:\"%s\" execute failed", payload.c_str());
            }
        }
    } else {
        log_i("topic received :%s, msg:%s", topic.c_str(), payload.c_str());
    }
}

void MQTT_Setup() {
    client.begin(serverIPAdressString.c_str(), net);
    client.onMessage(mqtt_message_received);
    MQTT_Connect();
}

void MQTT_Check() {
    client.loop();
    if (!client.connected()) {
        MQTT_Connect();
    }
}

#define JSON_BUFFER_SIZE 512
DynamicJsonDocument doc(1024);
char json_buffer[JSON_BUFFER_SIZE];

void MQTT_Upload() {
    doc["sensor"]["temperature"] = Sensors::getTemperature();
    doc["sensor"]["pressture"] = Sensors::getPressure();
    doc["sensor"]["light"] = Sensors::getLight();
    doc["peripheral"]["relay"] = Relay_Get();
    doc["peripheral"]["led"] = LED_Get();
    doc["peripheral"]["beeper"] = Beep_Get();
    doc["peripheral"]["motor"] = MotorControl_GetSpeed();
    doc["system"]["time"] = getUnixTime();
    doc["system"]["temperature"] = temperatureRead();

    serializeJson(doc, json_buffer, JSON_BUFFER_SIZE);
    log_d("MQTT msg:%s", json_buffer);
    if (!client.publish(MQTT_TOPIC_STATE, json_buffer)) {
        log_e("MQTT msg send failed");
    }
}

void BlueTooth_Setup() {
    SerialBT.register_callback(blueToothCallback);
    SerialBT.begin(blueToothName);
}

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
    configTime(gmtOffset, daylightOffset, ntpServer);
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
