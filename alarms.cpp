#include "alarms.h"
#include "bsp.h"
#include "infrared.h"
#include "CronAlarms.h"
#include <functional>
#include "command.h"

typedef std::function<void(void)> alarm_handler_t;

String alarm_name_list[dtNBR_ALARMS];

bool alarm_add(const char *cron_string, String action, bool is_oneshot) {
    alarm_handler_t handler = [action]() { Command_Run(action); };
    int id = Cron.create((char *)cron_string, handler, is_oneshot);
    if (id == dtINVALID_ALARM_ID) {
        log_e("alarm create error");
        return false;
    }
    alarm_name_list[id] = cron_string + ' ' + action + ' ' + is_oneshot;
    log_i("new alarm, id:%d, action:%s", id, action.c_str());
    return true;
}

void alarm_remove_name(int i) { alarm_name_list[i].clear(); }

bool alarm_remove(String name) {
    bool ret = false;
    for (int i = 0; i < dtNBR_ALARMS; i++) {
        if (alarm_name_list[i].indexOf(name) > 0) {
            ret = true;
            Cron.free(i);
            alarm_name_list[i].clear();
            log_i("alarm %d:%s removed", i, name.c_str());
        }
    }
    return ret;
}

void alarm_clear() {
    for (int i = 0; i < dtNBR_ALARMS; i++) {
        Cron.free(i);
        alarm_name_list[i].clear();
    }
    log_i("alarms cleared");
}

int alarm_get_count() {
    int count = 0;
    for (int i = 0; i < dtNBR_ALARMS; i++)
        if (Cron.isAllocated(i))
            count++;
    return count;
}

int alarm_get_list_size() { return dtNBR_ALARMS; }

String* alarm_get_list() {
    return alarm_name_list;
}

void alarm_check() {
    // Command_OutputControl(false);
    Cron.delay();
    // Command_OutputControl(true);
}
