#include "bluetooth_connection.h"
#include <BluetoothSerial.h>
#include "bsp.h"
#include "conf.h"
#include "json_helper.h"

BluetoothSerial SerialBT;

bool (*bluetooth_command_handler)(String cmd) = NULL;
int (*bluetooth_message_code_getter)(void);

void Bluetooth_SetCommandTools(bool (*handler)(String), int (*msg_code_getter)(void)) {
    bluetooth_command_handler = handler;
    bluetooth_message_code_getter = msg_code_getter;
}

void blueToothCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_DATA_IND_EVT) {
        if (bluetooth_command_handler != NULL) {
            String btString = SerialBT.readString();
            log_i("BT:%s", btString.c_str());

            bool status = bluetooth_command_handler(btString);

            if (bluetooth_message_code_getter != NULL) {
                Bluetooth_Ack(status, bluetooth_message_code_getter());
            } else {
                Bluetooth_Ack(status);
            }
            Bluetooth_Send();
            if (!status) {
                log_e("cmd:\"%s\" execute failed", btString.c_str());
            }
        }
    }
    if (event == ESP_SPP_CL_INIT_EVT) {
        log_i("BT Connected");
    }
}

void Bluetooth_Setup() {
    SerialBT.register_callback(blueToothCallback);
    SerialBT.begin(BLUETOOTH_NAME);
}

void Bluetooth_Send(const char *payload) {
    log_d("Bluetooth Upload:%s", payload);
    SerialBT.println(MQTT_TOPIC_STATE + payload);
}

void Bluetooth_Send() {
    Bluetooth_Send(json_helper_parse_report());
}

void Bluetooth_Ack(bool status, int msg_code) {
    log_i("Bluetooth ack:%d with msg code:%d", status, msg_code);
    char *json = json_helper_parse_ack(status, msg_code);
    SerialBT.println(MQTT_TOPIC_ACK + json);
}

bool Bluetooth_IsConnected() { return SerialBT.connected(); }
