#include "alarms.h"
#include "bsp.h"
#include "infrared.h"
#include "CronAlarms.h"
#include <functional>
#include "command.h"

typedef std::function<void(void)> alarm_handler_t;

String alarm_name_list[dtNBR_ALARMS];

bool alarm_add(const char *cron_string, String cmd, bool is_oneshot) {
    alarm_handler_t handler = [cmd]() { Command_Run(cmd); };
    int id = Cron.create((char *)cron_string, handler, is_oneshot);
    if (id == dtINVALID_ALARM_ID) {
        log_e("alarm create error");
        return false;
    }
    String name = cmd.substring(0, cmd.indexOf(" "));
    alarm_name_list[id] = name;
    log_i("new alarm, id:%d, name:%s", id, name.c_str());
    return true;
}

bool alarm_remove(String name) {
    bool ret = false;
    for (int i = 0; i < dtNBR_ALARMS; i++) {
        if (alarm_name_list[i].equalsIgnoreCase(name)) {
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

void alarm_check() {
    // Command_OutputControl(false);
    Cron.delay(0);
    // Command_OutputControl(true);
}
