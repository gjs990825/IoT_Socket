#include "mqtt_connection.h"
#include <WiFiClient.h>
#include <MQTTClient.h>
#include "bsp.h"
#include "conf.h"
#include "json_helper.h"
#include "command.h"

WiFiClient net;
MQTTClient client(MQTT_CLIENT_BUFFER_SIZE);

bool (*mqtt_command_handler)(String cmd) = NULL;
int (*mqtt_message_code_getter)(void) = NULL;

void MQTT_SetCommandTools(bool (*handler)(String), int (*msg_code_getter)(void)) {
    mqtt_command_handler = handler;
    mqtt_message_code_getter = msg_code_getter;
}

bool MQTT_Connect() {
    if (!WIFI_IsConnected()) {
        log_e("no connection, mqtt connect failed");
        return false;
    }
    if (!client.connect(MQTT_CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
        log_e("disconnected");
        return false;
    }
    log_i("connected");
    client.subscribe(MQTT_TOPIC_COMMAND, MQTT_QOS_EXACTLY_ONCE);
    return true;
}

void mqtt_message_received(String &topic, String &payload) {
    if (topic == MQTT_TOPIC_COMMAND) {
        if (mqtt_command_handler != NULL) {
            log_i("command received:%s", payload.c_str());
            bool status = mqtt_command_handler(payload);
            
            if (mqtt_message_code_getter != NULL) {
                MQTT_Ack(status, mqtt_message_code_getter());
            } else {
                MQTT_Ack(status);
            }
            
            MQTT_Send();
            if (!status) {
                log_e("cmd:\"%s\" execute failed", payload.c_str());
            }
        }
    } else {
        log_w("unknown topic:%s, msg:%s", topic.c_str(), payload.c_str());
    }
}

void MQTT_Setup() {
    client.begin(SERVER_IP_ADDRESS_STRING.c_str(), net);
    client.onMessage(mqtt_message_received);
    MQTT_Connect();
}

bool MQTT_IsConnected() {
    return client.connected();
}

void MQTT_Check() {
    if (!client.loop()) {
        MQTT_Connect();
    }
}

unsigned long mqtt_last_send = 0;

unsigned long MQTT_GetLastSend() { return mqtt_last_send; }

void MQTT_Send() {
    MQTT_Send(json_helper_parse_send());
}

void MQTT_Send(const char *payload) {
    mqtt_last_send = millis();
    log_d("MQTT msg:%s", payload);
    if (!client.publish(MQTT_TOPIC_STATE, payload, false, MQTT_QOS_AT_LEAST_ONCE)) {
        log_e("MQTT msg send failed");
    }
}

void MQTT_Ack(bool status, int msg_code) {
    char *json = json_helper_parse_ack(status, msg_code);
    log_i("MQTT ack:%d with msg code:%d", status, msg_code);
    if (!client.publish(MQTT_TOPIC_ACK, json, false, MQTT_QOS_AT_LEAST_ONCE)) {
        log_e("MQTT ack failed");
    }
}
