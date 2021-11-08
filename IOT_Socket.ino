#include "bsp.h"
#include "sensors.h"
#include "interactions.h"
#include "tasks.h"
#include <CronAlarms.h>

void setup() {
    // Basic peripheral
    Serial.begin(115200);
    LED_Setup();
    Beep_Setup();
    KEY_Setup();
    Relay_Setup();
    OLED_Setup();
    BMP280_Setup();

    // Restore settings
    Preferences_Init();

    // Connections setup
    BlueTooth_Setup();
    WIFI_Setup();
    NTP_Setup();

    log_i("SYS OK");
    task_print_help();
    delay(500);
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

void loop() {
    TASK(10)
        key_scan();

    TASK(50) {
        int key = key_get_key();

        if (key_is(K1, KEY_PRESS)) {
            while (key_is(K1, KEY_PRESS))
                key_scan();
            Relay_Flip();
            log_i("Manualy Relay Flip");
        }

        if (key_is(K2, KEY_PRESS)) {
            while (key_is(K2, KEY_PRESS))
                key_scan();
            if (key_is(K2, KEY_LONG_PRESS)) {
                reset_to_default_state();
                log_i("All reset!");
                while (key_is(K2, KEY_LONG_PRESS))
                    key_scan();
            } else {
                LED_Flip();
            }
        }
    }

    TASK(300) {
        Sensors::updateAll();
        task_check();
    }

    TASK(500)
        OLED_UpdateInfo();
        
    TASK(1000) {
        static unsigned long _t = 0;
        if (get_retry_after() + _t < millis()) {
            _t = millis();

            if (test_server_connection()) {  
                interactions();      
            } else {
                log_e("Service down:%d, retry after %d\n", 
                    get_access_fail_count(),
                    get_retry_after());
            }
        }
    }

    TASK(120 * 1000) {
        updateTimePreference();
    }

    Cron.delay(0);
}
