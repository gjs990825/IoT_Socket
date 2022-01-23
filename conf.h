#if !defined(_CONF_H_)
#define _CONF_H_

#include <Arduino.h>
#include <IPAddress.h>

#define PREFERENCES_DEBUG 0

static const String DEVICE_NAME = "IoT_Socket";

// WIFI
static String ssid = "";
static String password = "";

// Server address
static const IPAddress SERVER_IP_ADDRESS = IPAddress(120, 24, 84, 40);
static const String SERVER_IP_ADDRESS_STRING = "120.24.84.40";
// static const IPAddress SERVER_IP_ADDRESS = IPAddress(192, 168, 123, 100);
// static const String SERVER_IP_ADDRESS_STRING = "192.168.123.100";

// HTTP
static const int SERVER_HTTP_PORT = 8080;
static const String HTTP_BASE_ADDRESS = "http://" + SERVER_IP_ADDRESS_STRING + ":" + SERVER_HTTP_PORT + "/IOT_Socket";
static const String HTTP_DATA_HANDLER = HTTP_BASE_ADDRESS + "/ESP32UploadData";
static const String HTTP_GET_COMMAND = HTTP_BASE_ADDRESS + "/ESP32GetCommand";

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
#define INFRARED_CAPTURE_TIMEOUT 2000
#define INFRARED_CODE_LENGTH 100

// Json helper
#define JSON_BUFFER_SIZE 512
#define ARDUINOJSON_BUFFER_SIZE 1024

#endif // _CONF_H_
