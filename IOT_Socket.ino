#include "bsp.h"
#include "sensors.h"
#include "interactions.h"
#include "tasks.h"
#include <CronAlarms.h>
#include "infrared.h"

#include <Preferences.h>
extern Preferences preferences;

#include "lwshell.h"

void serial_shell_check() {
    const unsigned int MAX_MESSAGE_LENGTH = 64;
    while (Serial.available() > 0) {
        static char message[MAX_MESSAGE_LENGTH];
        static unsigned int message_pos = 0;

        char inByte = Serial.read();
        if (inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1)) {
            message[message_pos++] = inByte;
        } else {
            message[message_pos] = '\0';
            lwshell_input(message, strlen(message));
            message_pos = 0;
        }
    }
}

int32_t ir_send_cmd(int32_t argc, char** argv) {
    if (argc < 2)
        return -1;
    Infrared_SendPreset(atoi(argv[1]));
    return 0;
}

int32_t ir_capture_cmd(int32_t argc, char** argv) {
    if (argc < 2)
        return -1;
    String flag = argv[1];
    if (flag.equalsIgnoreCase("start")) {
        Infrared_StartCapture();
    } else if (flag.equalsIgnoreCase("end")) {
        if (Infrared_EndCapture(atoi(argv[2])))
            Infrared_StorePreset(preferences);
    }
    return 0;
}

int32_t led_set_cmd(int32_t argc, char** argv) {
    if (argc < 2)
        return -1;
    LED_Set((uint8_t)atoi(argv[1]));
    return 0;
}

int32_t relay_set_cmd(int32_t argc, char** argv) {
    if (argc < 2)
        return -1;
    Relay_Set(atoi(argv[1]));
    return 0;
}

struct {
    const char* cmd_name;
    lwshell_cmd_fn cmd_fn;
    const char* desc;
} lwshell_cmd_list[] = {
    {"ir_send",     ir_send_cmd,    "Infrared send Nth preset"  },
    {"ir_capture",  ir_capture_cmd, "Infrared Capture"          },
    {"led_set",     led_set_cmd,    "Led brightness control"    },
    {"relay_set",   relay_set_cmd,  "Relay control"             },
};

void lwshell_register_cmd() {
    for (auto &&cmd : lwshell_cmd_list)
        lwshell_register_cmd(cmd.cmd_name, cmd.cmd_fn, cmd.desc);
}

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

    // lwshell library init
    lwshell_init();
    lwshell_set_output_fn(
        [](const char* str, lwshell_t* lw) {
            Serial.printf("%s", str);
            if (*str == '\r')
                Serial.printf("\n");
        }
    );
    lwshell_register_cmd();

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
    if (Infrared_IsCapturing()) {
        Infrared_StartCapture();
    } else {
        if (Infrared_EndCapture(0))
            Infrared_StorePreset(preferences);
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
    if (Infrared_IsCapturing()) {
        Infrared_StartCapture();
    } else {
        if (Infrared_EndCapture(1))
            Infrared_StorePreset(preferences);
    }
}

void loop() {
    TASK(100) {
        serial_shell_check();
    }

    TASK(10) {
        key_scan();
    }

    TASK(50) {
        int key = key_get_key();

        if (key_is(K1, KEY_PRESS)) {
            while (key_is(K1, KEY_PRESS))
                key_scan();
            if (key_is(K1, KEY_LONG_PRESS)) {
                key1_long_press();
                while (key_is(K1, KEY_LONG_PRESS))
                    key_scan();
            } else {
                key1_press();
            }
        }

        if (key_is(K2, KEY_PRESS)) {
            while (key_is(K2, KEY_PRESS))
                key_scan();
            if (key_is(K2, KEY_LONG_PRESS)) {
                key2_long_press();
                while (key_is(K2, KEY_LONG_PRESS))
                    key_scan();
            } else {
                key2_press();
            }
        }
    }

    if (Infrared_IsCapturing())
        return;

    TASK(300) {
        Sensors::updateAll();
        task_check();
    }

    TASK(500)
        OLED_UpdateInfo();
        
    TASK(1000) {
        static unsigned long _t = 0;
        if (get_retry_after() * 1000 + _t < millis()) {
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

    TASK(1000 * 60 * 30) {
        updateTimePreference();
    }

    Cron.delay(0);
}
