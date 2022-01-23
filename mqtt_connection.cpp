#include "mqtt_connection.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <MQTTClient.h>
#include "bsp.h"
#include "sensors.h"
#include "json_helper.h"

WiFiClient net;
MQTTClient client(MQTT_CLIENT_BUFFER_SIZE);

bool (*mqtt_command_handler)(String cmd) = NULL;

void MQTT_SetCommandHandler(bool (*handler)(String)) {
    mqtt_command_handler = handler;
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
        if (mqtt_command_handler != NULL) {
            log_i("command received:%s", payload.c_str());
            if (!mqtt_command_handler(payload)) {
                log_e("cmd:\"%s\" execute failed", payload.c_str());
            }
        }
    } else {
        log_i("topic received :%s, msg:%s", topic.c_str(), payload.c_str());
    }
}

void MQTT_Setup() {
    client.begin(SERVER_IP_ADDRESS_STRING.c_str(), net);
    client.onMessage(mqtt_message_received);
    MQTT_Connect();
}

void MQTT_Check() {
    client.loop();
    if (!client.connected()) {
        MQTT_Connect();
    }
}

void MQTT_Send(const char *payload) {
    log_d("MQTT msg:%s", payload);
    if (!client.publish(MQTT_TOPIC_STATE, payload)) {
        log_e("MQTT msg send failed");
    }
}
