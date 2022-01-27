#include "alarms.h"
#include "bsp.h"
#include "infrared.h"
#include <CronAlarms.h>

const struct {
    const char *name;
    alarm_action_func_t func;
} alarm_actions[] = {
    {"relay_on",    [](){ Relay_Set(true);	        }},
    {"relay_off",   [](){ Relay_Set(false);	        }},
    {"relay_flip",  [](){ Relay_Flip();		        }},
    {"led_on",      [](){ LED_Set(true);	        }},
    {"led_off",     [](){ LED_Set(false);	        }},
    {"led_flip",    [](){ LED_Flip();		        }},
    {"beeper_on",   [](){ Beeper_Set(true);	        }},
    {"beeper_off",  [](){ Beeper_Set(false);	    }},
    {"beeper_flip", [](){ Beeper_Flip();		    }},
    {"ir_preset_0", [](){ Infrared_SendPreset(0);   }},
    {"ir_preset_1", [](){ Infrared_SendPreset(1);   }},
    {"ir_preset_2", [](){ Infrared_SendPreset(2);   }},
    {"ir_preset_3", [](){ Infrared_SendPreset(3);   }},
};

alarm_action_func_t alarm_action_get(String str) {
    for (auto &&action : alarm_actions)
        if (str.equalsIgnoreCase(action.name))
            return action.func;
    return NULL;
}

void alarm_add(const char *cron_string, void (*handler)(void), bool is_oneshot) {
    int id = Cron.create((char *)cron_string, handler, is_oneshot);
    log_i("new alarm, id:%d", id);
}

void alarm_remove(void (*handler)(void)) {
    CronEventClass *alarms = Cron.getAlarms();
    for (int i = 0; i < dtNBR_ALARMS; i++) {
        if (alarms[i].onTickHandler == handler) {
            alarms[i].isEnabled = false;
            log_i("alarm removed, id:%d", i);
        }
    }
}

void alarm_clear() {
    for (int i = 0; i < dtNBR_ALARMS; i++)
        Cron.free(i);
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
    Cron.delay(0);
}
