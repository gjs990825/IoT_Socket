#if !defined(_CONF_H_)
#define _CONF_H_

#include <WString.h>
#include <IPAddress.h>

#define PREFERENCES_DEBUG 0

static const String DEVICE_NAME = "IoT_Socket";

// WIFI
static String ssid = "";
static String password = "";

// Server address
static const IPAddress SERVER_IP_ADDRESS = IPAddress(120, 24, 84, 40);
static const String SERVER_IP_ADDRESS_STRING = "120.24.84.40";

// Acks
const char *const ACK_OK = "OK";
const char *const ACK_FAIL = "FAIL";

// MQTT
#define MQTT_CLIENT_BUFFER_SIZE 512

static const char *const MQTT_CLIENT_ID = DEVICE_NAME.c_str();
static const char *const MQTT_USER_NAME = "esp32";
static const char *const MQTT_PASSWORD = "password";

static const String MQTT_TOPIC_COMMAND = DEVICE_NAME + "/Command";
static const String MQTT_TOPIC_STATE = DEVICE_NAME + "/State";
static const String MQTT_TOPIC_ACK = DEVICE_NAME + "/Ack";

// BT
static const String BLUETOOTH_NAME = DEVICE_NAME;

// Time
static const char *const NTP_SERVER = "ntp.aliyun.com";
static const int GMT_OFFSET = 3600 * 8;
static const int DAYLIGHT_OFFSET = 0;

static long timeStamp = 1609430400; // Fri Jan 01 2021 00:00:00 GMT+0800 (China Standard Time)

// Infrared
#define INFRARED_CAPTURE_TIMEOUT 5000
#define INFRARED_MAX_CODE_LENGTH 150

// Json helper
#define JSON_BUFFER_SIZE 512
#define ARDUINOJSON_SEND_BUFFER_SIZE 512
#define ARDUINOJSON_ACK_BUFFER_SIZE 128

#endif // _CONF_H_
