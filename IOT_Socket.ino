#include "bsp.h"
#include "sensors.h"
#include "interactions.h"
#include "tasks.h"
#include <CronAlarms.h>
#include "infrared.h"
#include "command.h"

void setup() {
    // Basic peripheral
    Serial.begin(115200);
    LED_Setup();
    Beep_Setup();
    KEY_Setup();
    Relay_Setup();
    OLED_Setup();
    BMP280_Setup();
    MotorControl_Setup();

    // Restore settings
    Preferences_Init();

    // Connections setup
    BlueTooth_Setup();
    WIFI_Setup();
    NTP_Setup();

    // Command system
    Command_Init();

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
    OLED.printf("Light:%.2f", Sensors::getLight());
    OLED.printf("  [%s]", get_state_string().c_str()); // states indicator
    OLED.display();
}

void key1_press() {
    log_i("key1 press");
    Relay_Flip();
}

void key1_long_press() {
    if (!Infrared_IsCapturing()) {
        Infrared_StartCapture();
    } else {
        if (Infrared_EndCapture(0)) {
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
        Infrared_StartCapture();
    } else {
        if (Infrared_EndCapture(1)) {
            Infrared_StorePreset(Preferences_Get());
        } 
    }
}

void loop() {
    // test
    TASK(10) {
        static int speed;
        static int adding_value = 1;

        if (speed >= 100) {
            adding_value = -1;
        }
        else if (speed <= -100) {
            adding_value = 1;
        }
        speed += adding_value;
        MotorControl_SetSpeed(speed);
    }

    TASK(100) {
        Command_CheckSerial();
        CommandQueue_Handle();
    }

    TASK(10) {
        key_scan();
    }

    TASK(50) {
        if (key_is(K1, KEY_PRESS)) {
            while (key_is(K1, KEY_PRESS)) {
                key_scan();
            }
            if (key_is(K1, KEY_LONG_PRESS)) {
                key1_long_press();
                while (key_is(K1, KEY_LONG_PRESS)) {
                    key_scan();
                }
            } else {
                key1_press();
            }
        }

        if (key_is(K2, KEY_PRESS)) {
            while (key_is(K2, KEY_PRESS)) {
                key_scan();
            }
            if (key_is(K2, KEY_LONG_PRESS)) {
                key2_long_press();
                while (key_is(K2, KEY_LONG_PRESS)) {
                    key_scan();
                }
            } else {
                key2_press();
            }
        }
    }

    if (Infrared_IsCapturing()) {
        return;
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
        static unsigned long t = 0;
        if (get_retry_after() * 1000 + t < millis()) {
            t = millis();

            if (test_server_connection()) {  
                interactions();      
            } else {
                log_e("Service down:%d, retry after %ds\n", 
                    get_access_fail_count(),
                    get_retry_after());
            }
        }
    }

    TASK(1000 * 60 * 30) {
        Preference_UpdateTimeStamp();
    }

    Cron.delay(0);
}
