#include "bsp.h"
#include "sensors.h"
#include "infrared.h"
#include "command.h"
#include "json_helper.h"
#include "tasks.h"
#include "alarms.h"
#include "misc.h"

#include "connection.h"

MqttConnection* mqttConnection;
BluetoothConnection* bluetoothConnection;


void key1_press() {
    log_i("key1 press");
    Command_Run("flip relay");
}

void key1_long_press() {
    log_i("key1 long press");
}

void key2_press() {
    log_i("key2 press");
    Command_Run("reset default");
}

void key2_long_press() {
    log_i("key2 long press");
}

void setup() {
    // Peripheral and sensors
    Serial.begin(115200);
    OLED_Setup();
    KEY_Setup();
    Sensors.initialize();

    key_set_handler(key1_press,
                    key1_long_press,
                    key2_press,
                    key2_long_press);

    // Restore settings
    Preferences_Init();
    Preferences& pref = Preferences_Get();
    Infrared_RestorePreset(pref);
    WiFi_RestoreSettings(pref);
    TimeStamp_Restore(pref);

    // Command system
    Command_Init();

    // Connections
    WIFI_Setup();
    NTP_Setup();
    
    mqttConnection = new MqttConnection(Command_Run, Command_GetMessageCode);
    bluetoothConnection = new BluetoothConnection(Command_Run, Command_GetMessageCode);


    log_i("System setup finished");
}

void OLED_UpdateInfo() {
    OLED.clearDisplay();
    OLED.setCursor(0, 0);
    OLED.setTextSize(2);
    OLED.print(LocalTime_GetString());

    char indicator;
    if (bluetoothConnection->isConnected()) {
        indicator = 'B';
    } else if (mqttConnection->isConnected()) {
        indicator = 'M';
    } else if (WIFI_IsConnected()) {
        indicator = '!';
    } else {
        indicator = 'X';
    }
    OLED.printf(" %c\n", indicator);
    OLED.setTextSize(1);
    OLED.printf("\nTemperature:%.3f\n\n", Sensors.getTemperature());
    OLED.printf("Pressure:%.3f\n\n", Sensors.getPressure());
    OLED.printf("Brightness:%d", (int)Sensors.getBrightness());
    OLED.printf(" %s", get_state_string().c_str()); // states indicator
    OLED.display();
}

void loop() {
    TASK(10) {
        key_check();
    }

    TASK(100) {
        mqttConnection->check();
        Command_CheckSerial();
        CommandQueue_Handle();
    }

    TASK(500) {
        Sensors.updateAll();
        task_check();
        alarm_check();
        OLED_UpdateInfo();
    }
        
    TASK(1200) {
        if (bluetoothConnection->isConnected()) {
            bluetoothConnection->report();
        } else if (mqttConnection->isConnected()) {
            mqttConnection->report();
        }
    }

    TASK(1000 * 60 * 30) {
        TimeStamp_Update(Preferences_Get());
    }
}
