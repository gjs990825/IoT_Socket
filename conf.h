#if !defined(_CONF_H_)
#define _CONF_H_

#include <IPAddress.h>

// #define PREFERENCES_DEBUG

// BT
static const char *const blueToothName = "IoT_Socket";

// WIFI
static String ssid = "";
static String password = "";

// Server
// static const IPAddress serverIPAdress = IPAddress(192, 168, 123, 100);
// static const String serverIPAdressString = "192.168.123.100";
static const IPAddress serverIPAdress = IPAddress(120, 24, 84, 40);
static const String serverIPAdressString = "120.24.84.40";
static const int serverPort = 8080;
static const String serverBaseAddress = "http://" + serverIPAdressString + ":" + serverPort + "/IOT_Socket";
static const String serverDataHandler = serverBaseAddress + "/ESP32UploadData";
static const String serverGetCommand = serverBaseAddress + "/ESP32GetCommand";

static const String MQTT_TOPIC_COMMAND = "IoT_Socket/Command";
static const String MQTT_TOPIC_STATE = "IoT_Socket/State";


// Time
static const char *ntpServer = "ntp.aliyun.com";
static const int gmtOffset = 3600 * 8;
static const int daylightOffset = 0;

// Fri Jan 01 2021 00:00:00 GMT+0800 (China Standard Time)
static long timeStamp = 1609430400;

#endif // _CONF_H_
