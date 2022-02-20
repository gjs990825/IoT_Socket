#if !defined(_CONNECTION_H_)
#define _CONNECTION_H_

#include <WString.h>
#include "conf.h"
#include "json_helper.h"
#include "bsp.h"
#include <WiFiClient.h>
#include <MQTTClient.h>
#include <BluetoothSerial.h>


class Connection {
protected:
    bool (*cmd_handler)(String cmd);
    int (*msg_code_getter)(void);

    virtual void sendRaw(String topic, const char *payload) = 0;
    void ack(bool status, int msg_code = 0);

public:
    const String TOPIC_COMMAND = DEVICE_NAME + "/Command";
    const String TOPIC_STATE = DEVICE_NAME + "/State";
    const String TOPIC_ACK = DEVICE_NAME + "/Ack";

    Connection(bool (*command_handler)(String), int (*message_code_getter)(void));
    virtual bool isConnected() = 0;
    void onCommandReceived(String cmd_string);
    void report();
};

class MqttConnection: public Connection {
private:
    WiFiClient net;
    MQTTClient client = MQTTClient(MQTT_CLIENT_BUFFER_SIZE);

    void sendRaw(String topic, const char *payload);
    bool connect();

public:
    enum {
        QOS_AT_MOST_ONCE = 0,
        QOS_AT_LEAST_ONCE = 1,
        QOS_EXACTLY_ONCE = 2,
    };

    MqttConnection(bool (*command_handler)(String), int (*message_code_getter)(void));
    bool isConnected();
    void check();
};

class BluetoothConnection: public Connection {
private:
    BluetoothSerial SerialBT;
    void sendRaw(String topic, const char *payload);

public:
    BluetoothConnection(bool (*command_handler)(String), int (*message_code_getter)(void));
    bool isConnected();
};

#endif // _CONNECTION_H_
