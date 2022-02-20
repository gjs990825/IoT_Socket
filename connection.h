#if !defined(_CONNECTION_H_)
#define _CONNECTION_H_

#include <WString.h>
#include "conf.h"
#include "json_helper.h"
#include "bsp.h"
#include <WiFiClient.h>
#include <MQTTClient.h>


class Connection {
protected:
    bool (*cmd_handler)(String cmd);
    int (*msg_code_getter)(void);

    virtual void sendRaw(String topic, const char *payload) = 0;

    void ack(bool status, int msg_code = 0) {
        sendRaw(TOPIC_ACK, json_helper_parse_ack(status, msg_code));
    }

public:
    const String TOPIC_COMMAND = DEVICE_NAME + "/Command";
    const String TOPIC_STATE = DEVICE_NAME + "/State";
    const String TOPIC_ACK = DEVICE_NAME + "/Ack";

    Connection(bool (*command_handler)(String), int (*message_code_getter)(void))
        : cmd_handler(command_handler), msg_code_getter(message_code_getter) {}

    virtual bool isConnected() = 0;

    void onCommandReceived(String cmd_string) {
        if (cmd_handler == NULL)
            return;

        bool status = cmd_handler(cmd_string);

        if (msg_code_getter != NULL) {
            ack(status, msg_code_getter());
        } else {
            ack(status);
        }

        if (status) {
            report();
        } else {
            log_e("cmd:\"%s\" execute failed", cmd_string.c_str());
        }
    }

    void report() { 
        sendRaw(TOPIC_STATE, json_helper_parse_report()); 
    }
};

class MqttConnection: public Connection
{
private:
    WiFiClient net;
    MQTTClient client = MQTTClient(MQTT_CLIENT_BUFFER_SIZE);

    void sendRaw(String topic, const char *payload) {
        log_d("MQTT msg:%s", payload);
        if (!client.publish(topic, payload, false, MQTT_QOS_AT_LEAST_ONCE)) {
            log_e("MQTT msg send failed");
        }
    }

public:
    enum {
        MQTT_QOS_AT_MOST_ONCE = 0,
        MQTT_QOS_AT_LEAST_ONCE = 1,
        MQTT_QOS_EXACTLY_ONCE = 2,
    };


    MqttConnection(bool (*command_handler)(String), int (*message_code_getter)(void))
        : Connection(command_handler, msg_code_getter) {
        client.begin(SERVER_IP_ADDRESS_STRING.c_str(), net);
        MqttConnection* connection = this;
        client.onMessage([connection](String &topic, String &payload) -> void {
            if (topic.equals(connection->TOPIC_COMMAND)) {
                connection->onCommandReceived(payload);
            } else {
                log_w("unknown topic:%s", topic.c_str());
            }
        });
    }

    bool connect() {
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

    bool isConnected() { return client.connected(); }

    void check() {
        if (!client.loop()) {
            connect();
        }
    }
};



#endif // _CONNECTION_H_
