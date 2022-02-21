#include "connection.h"

void Connection::ack(bool status, int msg_code) {
    sendRaw(TOPIC_ACK, json_helper_parse_ack(status, msg_code));
}

Connection::Connection(bool (*command_handler)(String), int (*message_code_getter)(void))
    : cmd_handler(command_handler), msg_code_getter(message_code_getter) {}

void Connection::onCommandReceived(String cmd_string) {
    if (cmd_handler == NULL)
        return;

    bool status = cmd_handler(cmd_string);
    int code = msg_code_getter ? msg_code_getter() : 0;
    ack(status, code);

    if (status) {
        report();
    }
    else {
        log_e("cmd:\"%s\" execute failed", cmd_string.c_str());
    }
}

void Connection::report() {
    sendRaw(TOPIC_STATE, json_helper_parse_report());
}

void MqttConnection::sendRaw(String topic, const char *payload) {
    log_d("MQTT send:%s, %s", topic.c_str(), payload);
    if (!client.publish(topic, payload, false, QOS_AT_LEAST_ONCE)) {
        log_e("MQTT send failed");
    }
}

bool MqttConnection::connect() {
    if (!WIFI_IsConnected()) {
        log_e("no connection, mqtt connect failed");
        return false;
    }
    if (!client.connect(MQTT_CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
        log_e("disconnected");
        return false;
    }
    log_i("connected");
    client.subscribe(MQTT_TOPIC_COMMAND, QOS_EXACTLY_ONCE);
    return true;
}

MqttConnection::MqttConnection(bool (*command_handler)(String), int (*message_code_getter)(void))
    : Connection(command_handler, message_code_getter) {
    client.begin(SERVER_IP_ADDRESS_STRING.c_str(), net);
    MqttConnection *connection = this;
    client.onMessage([connection](String &topic, String &payload) -> void {
        if (topic.equals(connection->TOPIC_COMMAND)) {
            connection->onCommandReceived(payload);
        } else {
            log_w("unknown topic:%s", topic.c_str());
        }
    });
    connect();
}

bool MqttConnection::isConnected() { return client.connected(); }

void MqttConnection::check() {
    if (!client.loop()) {
        connect();
    }
}

String MqttConnection::getName() {
    return name;
}

void MqttConnection::reconnect() {
    connect();
}

void BluetoothConnection::sendRaw(String topic, const char *payload) {
    log_d("BT Send:%s, %s", topic.c_str(), payload);
    SerialBT.println(topic + payload);
}

BluetoothConnection::BluetoothConnection(bool (*command_handler)(String), int (*message_code_getter)(void))
    : Connection(command_handler, message_code_getter) {
    Connection *connection = this;
    SerialBT.onData([connection](const uint8_t *buffer, size_t size) -> void {
        char *msg = (char *)malloc(size + 1);
        memcpy(msg, buffer, size);
        msg[size] = '\0';

        Serial.print("BT:");
        Serial.println(msg);
        connection->onCommandReceived(msg);

        free(msg);
    });
    SerialBT.begin(BLUETOOTH_NAME);
}

bool BluetoothConnection::isConnected() { return SerialBT.connected(); }

String BluetoothConnection::getName() {
    return name;
}

Connection* ConnectionManager::getConnection() {
    for (auto &&conn : connections) {
        if (conn->isConnected()) {
            return conn;
        }
    }
    return NULL;
}

ConnectionManager::ConnectionManager(bool (*command_handler)(String), int (*message_code_getter)(void)) {
    connections.push_back(new BluetoothConnection(command_handler, message_code_getter));
    connections.push_back(new MqttConnection(command_handler, message_code_getter));
    currentConnection = getConnection();
}

void ConnectionManager::report() {
    currentConnection = getConnection();
    if (currentConnection != NULL) {
        currentConnection->report();
    } else {
        log_w("no available connection, try to reconnect");
        for (auto &&conn : connections) {
            conn->reconnect();
        }
    }
}

void ConnectionManager::check() {
    if (currentConnection != NULL) {
        currentConnection->check();
    }
}

bool ConnectionManager::isConnected(const char *name) {
    for (auto &&conn : connections) {
        if (conn->getName().equals(name)) {
            return conn->isConnected();
        }
    }
    return false;
}
