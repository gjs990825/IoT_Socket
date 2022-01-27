#include "bsp.h"
#include "sensors.h"
#include "interactions.h"
#include "tasks.h"
#include <CronAlarms.h>
#include "infrared.h"
#include "command.h"
#include "mqtt_connection.h"
#include "bluetooth_connection.h"
#include "json_helper.h"

void setup() {
    // Basic peripheral
    Serial.begin(115200);
    LED_Setup();
    Beeper_Setup();
    KEY_Setup();
    Relay_Setup();
    OLED_Setup();
    BMP280_Setup();
    MotorControl_Setup();

    // Restore settings
    Preferences_Init();

    // Connections setup
    WIFI_Setup();
    NTP_Setup();
    MQTT_Setup();
    Bluetooth_Setup();

    // Command system
    Command_Init();
    MQTT_SetCommandHandler(CommandQueue_Add);
    Bluetooth_SetCommandHandler(Command_Run);

    log_i("SYS OK");
}

void OLED_UpdateInfo() {
    OLED.clearDisplay();
    OLED.setCursor(0, 0);
    OLED.setTextSize(2);
    OLED.print(LocalTime_GetString());
    OLED.printf("%s\n", get_server_state() ? "" : " !"); // offline indicator
    OLED.setTextSize(1);
    OLED.printf("\nTemperature:%.3f\n\n", Sensors::getTemperature());
    OLED.printf("Pressure:%.3f\n\n", Sensors::getPressure());
    OLED.printf("Brightness:%d", (int)Sensors::getBrightness());
    OLED.printf(" %s", get_state_string().c_str()); // states indicator
    OLED.display();
}

void key1_press() {
    log_i("key1 press");
    Relay_Flip();
}

void key1_long_press() {
    if (!Infrared_IsCapturing()) {
        Infrared_StartCapture(0);
    } else {
        if (Infrared_EndCapture()) {
            Infrared_StorePreset(Preferences_Get());
        }
    }
    log_i("key1 long press");
}

void key2_press() {
    log_i("key2 press");
    reset_to_default_state();
    log_i("All reset!");
}

void key2_long_press() {
    log_i("key2 long press");
    if (!Infrared_IsCapturing()) {
        Infrared_StartCapture(1);
    } else {
        if (Infrared_EndCapture()) {
            Infrared_StorePreset(Preferences_Get());
        } 
    }
}

void loop() {
    TASK(200) {
        Command_CheckSerial();
        CommandQueue_Handle();
        Infrared_CheckCapture();
        if (!Infrared_IsCapturing()) {
            MQTT_Check();
        }
    }

    TASK(10) {
        key_scan();
    }

    TASK(50) {
        if (key_is(KEY1, KEY_PRESS)) {
            while (key_is(KEY1, KEY_PRESS)) {
                key_scan();
            }
            if (key_is(KEY1, KEY_LONG_PRESS)) {
                key1_long_press();
                while (key_is(KEY1, KEY_LONG_PRESS)) {
                    key_scan();
                }
            } else {
                key1_press();
            }
        }

        if (key_is(KEY2, KEY_PRESS)) {
            while (key_is(KEY2, KEY_PRESS)) {
                key_scan();
            }
            if (key_is(KEY2, KEY_LONG_PRESS)) {
                key2_long_press();
                while (key_is(KEY2, KEY_LONG_PRESS)) {
                    key_scan();
                }
            } else {
                key2_press();
            }
        }
    }

    TASK(450) {
        Sensors::updateAll();
        task_check();
        _task_check();
    }

    TASK(500) {
        OLED_UpdateInfo();
    }
        
    TASK(1500) {
        if (Bluetooth_IsConnected() || WIFI_IsConnected()) {
            const char* buffer = parse_json_buffer();;
            if (Bluetooth_IsConnected()) {
                Bluetooth_Send(buffer);
            }

            static unsigned long t = 0;
            if (get_retry_after() * 1000 + t < millis()) {
                t = millis();

                if (!Infrared_IsCapturing()) {
                    if (test_server_connection()) {
                        if (millis() - MQTT_GetLastSend() > 1000) {
                            MQTT_Send(buffer);
                        }
                    } else {
                        log_e("Service down:%d, retry after %ds\n", 
                            get_access_fail_count(),
                            get_retry_after());
                    }
                }
            }
        }
    }

    TASK(1000 * 60 * 30) {
        Preferences_UpdateTimeStamp();
    }

    Cron.delay(0);
}
