#include "interactions.h"
#include "bsp.h"
#include "sensors.h"
#include "tasks.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>

const struct {
    uint8_t failedTimes;
    uint8_t retryAfter;
} fail_retries[] = {
    {1, 1},
    {3, 5},
    {6, 10},
    {15, 30},
    {30, 60},
};
constexpr uint8_t fail_retry_count = sizeof(fail_retries) / sizeof(fail_retries[0]);

int access_fail_count = 0;

int get_access_fail_count() { return access_fail_count; }
void update_access_fail_count(bool sta) { access_fail_count = sta ? 0 : access_fail_count + 1; }
bool get_server_state() { return access_fail_count == 0; }

uint8_t get_retry_after() {
    for (auto &&fail_retry : fail_retries)
        if (access_fail_count <= fail_retry.failedTimes)
            return fail_retry.retryAfter;
    return fail_retries[fail_retry_count - 1].retryAfter;
}

bool test_server_connection() {
    bool ret = Ping.ping(serverIPAdress, 1);
    update_access_fail_count(ret);
    return ret;
}

void Upload_Data() {
    HTTPClient http;
    String serverPath = serverDataHandler +
                        "?temperature=" + Sensors::getTemperature() +
                        "&pressure=" + Sensors::getPressure() +
                        "&brightness=" + Sensors::getBrightness() + 
                        "&relay=" + Relay_Get() + 
                        "&led=" + LED_Get() + 
                        "&beeper=" + Beeper_Get();

    log_d("Access:%s", serverPath.c_str());
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        log_d("HTTP Response code: %d", httpResponseCode);
        String payload = http.getString();
        log_d("%s", payload.c_str());
    } else {
        log_e("Error code: %d", httpResponseCode);
    }
    // Free resources
    http.end();
}

void Get_Command() {
    HTTPClient http;
    String serverPath = serverGetCommand;

    log_d("Access:%s", serverPath.c_str());
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        log_d("HTTP Response code: %d", httpResponseCode);
        String payload = http.getString();
        log_d("%s", payload.c_str());

        if (payload.startsWith(deletePrefix)) {
            payload.replace(deletePrefix, "");
            if (payload.startsWith(taskPrefix)) {
                task_clear();
                log_i("Task cleared");
            } else {
                alarm_clear();
                log_i("Alarm cleared");
            }
            return;
        }

        if (payload.startsWith(taskPrefix)) {
            log_d("Handleing pending command...");
            payload.replace(taskPrefix, "");
            task_add(payload.c_str());
        } else if (payload.startsWith(alarmPrefix)) {
            log_d("Handleing pending Alarm...");
            payload.replace(alarmPrefix, "");
            alarm_add(payload.c_str());
        } else if (payload.startsWith(settingPrefix)) {
            log_d("Handleing pending Setting...");
            payload.replace(settingPrefix, "");
            setting_apply(payload);
        }
    } else {
        log_e("Error code: %d", httpResponseCode);
    }
    // Free resources
    http.end();
}
