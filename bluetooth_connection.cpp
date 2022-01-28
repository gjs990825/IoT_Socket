#include "bluetooth_connection.h"
#include <BluetoothSerial.h>
#include "bsp.h"
#include "conf.h"
#include "json_helper.h"

BluetoothSerial SerialBT;

bool (*bluetooth_command_handler)(String cmd) = NULL;
String (*bluetooth_message_getter)(void);

void Bluetooth_SetCommandTools(bool (*handler)(String), String (*msg_getter)(void)) {
    bluetooth_command_handler = handler;
    bluetooth_message_getter = msg_getter;
}

void blueToothCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    if (event == ESP_SPP_DATA_IND_EVT) {
        if (bluetooth_command_handler != NULL) {
            String btString = SerialBT.readString();
            log_i("BT:%s", btString.c_str());

            bool status = bluetooth_command_handler(btString);

            if (bluetooth_message_getter != NULL) {
                Bluetooth_Ack(status, bluetooth_message_getter());
            } else {
                Bluetooth_Ack(status);
            }
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
    SerialBT.println(payload);
}

void Bluetooth_Ack(bool status, String msg) {
    log_i("Bluetooth ack:%d with msg:%s", status, msg.c_str());
    char *json = json_helper_parse_ack(status, msg);
    SerialBT.println(json);
}

bool Bluetooth_IsConnected() { return SerialBT.connected(); }
