#include "misc.h"
#include "bsp.h"
#include "alarms.h"
#include "tasks.h"
#include <ESP32Ping.h>

const struct {
    bool (*sta)();
    const char mark;
} state_marks[] = {
    { [](){ return task_get_count() != 0; },    'T'},
    { [](){ return alarm_get_count() != 0; },   'A'},
    { [](){ return true; },                     '|'},
    { Relay_Get,                                'R'},
    { Beeper_Get,                               'B'},
    { [](){ return LED_Get() != 0; },           'L'},
};

String get_state_string() {
    String s;
    for (auto &&state_code : state_marks)
        s += state_code.sta() ? state_code.mark : '-';
    return s;
}

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
bool is_server_available() { return access_fail_count == 0; }

uint8_t get_retry_after() {
    for (auto &&fail_retry : fail_retries)
        if (access_fail_count <= fail_retry.failedTimes)
            return fail_retry.retryAfter;
    return fail_retries[fail_retry_count - 1].retryAfter;
}

bool test_server_connection() {
    bool ret = Ping.ping(SERVER_IP_ADDRESS, 1);
    update_access_fail_count(ret);
    return ret;
}
