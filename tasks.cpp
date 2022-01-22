#include "tasks.h"
#include "bsp.h"
#include "sensors.h"
#include <list>
#include "infrared.h"

////////////////////// actions

typedef void (*action_func_t)(bool sta);

typedef struct {
    const char *name;
    action_func_t func;
} action_t;

const action_t actions[] = {
    {"relay",       [](bool sta){ Relay_Set(sta); 	        }},
    {"relay_on",    [](bool sta){ Relay_Set(true);	        }},
    {"relay_off",   [](bool sta){ Relay_Set(false);	        }},
    {"relay_flip",  [](bool sta){ Relay_Flip();		        }},
    {"led",         [](bool sta){ LED_Set(sta);		        }},
    {"led_on",      [](bool sta){ LED_Set(true);	        }},
    {"led_off",     [](bool sta){ LED_Set(false);	        }},
    {"led_flip",    [](bool sta){ LED_Flip();		        }},
    {"beeper",      [](bool sta){ Beeper_Set(sta);	        }},
    {"beeper_on",   [](bool sta){ Beeper_Set(true);	        }},
    {"beeper_off",  [](bool sta){ Beeper_Set(false);	    }},
    {"beeper_flip", [](bool sta){ Beeper_Flip();		    }},
    {"ir_preset_0", [](bool sta){ Infrared_SendPreset(0);   }},
    {"ir_preset_1", [](bool sta){ Infrared_SendPreset(1);   }},
    {"ir_preset_2", [](bool sta){ Infrared_SendPreset(2);   }},
    {"ir_preset_3", [](bool sta){ Infrared_SendPreset(3);   }},
};
const int actions_count = sizeof(actions) / sizeof(actions[0]);

void action_print_available() {
    Serial.println("Actions list:");
    for (int i = 0; i < actions_count; i++)
        Serial.printf("[%d]: %s\n", i, actions[i].name);
}

void *action_get(const char *s) {
    String str = String(s);
    for (auto &&action : actions)
        if (str.equalsIgnoreCase(action.name))
            return (void *)action.func;
    return NULL;
}

void action_execute(void *action, bool sta) { if (action != NULL) ((action_func_t)action)(sta); }

void action_execute(const char *s, bool sta) { action_execute(action_get(s), sta); }

/////////////////////////// conditions

typedef float (*condition_func_t)(void);

typedef struct {
    const char *name;
    condition_func_t func;
} condition_t;

const float F_TRUE = 1.0;
const float F_FALSE = 0.0;

const condition_t conditions[] = {
    {"temperature",	Sensors::getTemperature				},
    {"pressure",	Sensors::getPressure				},
    {"brightness",	Sensors::getBrightness				},
    {"true",		[]() -> float { return F_TRUE; }	},
    {"false",		[]() -> float { return F_FALSE; }	},
};
const int conditions_count = sizeof(conditions) / sizeof(conditions[0]);

void condition_print_available() {
    Serial.println("Conditions list:");
    for (int i = 0; i < conditions_count; i++)
        Serial.printf("[%d]: %s\n", i, conditions[i].name);
}

void *condition_get(const char *s) {
    String str = String(s);
    for (auto &&condition : conditions)
        if (str.equalsIgnoreCase(condition.name))
            return (void *)condition.func;
    return NULL;
}

float condition_result(void *condition) {  return condition ? ((condition_func_t)condition)() : F_FALSE; }

/////////////////////////// triggers

#define EXECUTE_USEING(OP) action_execute(action, condition_result(condition) OP v1)

void equal_handler(void *condition, void *action, float v1, float v2) { EXECUTE_USEING(==); }
void not_equal_handler(void *condition, void *action, float v1, float v2) { EXECUTE_USEING(!=); }
void bigger_handler(void *condition, void *action, float v1, float v2) { EXECUTE_USEING(>); }
void lower_handler(void *condition, void *action, float v1, float v2) { EXECUTE_USEING(<); }
void threshold_handler(void *condition, void *action, float v1, float v2) {
    float f = condition_result(condition);
    if (f > v1)
        action_execute(action, true);
    else if (f < v2)
        action_execute(action, false);
}

#undef EXECUTE_USEING

/////////////////////////// handlers

typedef enum {
    EQUAL,
    NOT_EQUAL,
    BIGGER,
    LOWER,
    THRESHOLD,
    INSTANT,
    NOT_A_TYPE,
} trigger_type_t;

typedef void (*handler_t)(void *condition, void *action, float v1, float v2);

typedef struct {
    trigger_type_t type;
    const char *name;
    handler_t handler;
} tigger_t;

tigger_t triggers[] = {
    {EQUAL,		"equal",		equal_handler		},
    {NOT_EQUAL,	"not_equal",	not_equal_handler	},
    {BIGGER,	"bigger",		bigger_handler		},
    {LOWER,		"lower",		lower_handler		},
    {THRESHOLD,	"threshold",	threshold_handler	},
    {INSTANT,	"instant",		NULL				},
};
int triggers_count = sizeof(triggers) / sizeof(triggers[0]);

void trigger_type_print_available() {
    Serial.println("Type list:");
    for (int i = 0; i < triggers_count; i++)
        Serial.printf("[%d]: %s\n", i, triggers[i].name);
}

void *trigger_function_get(trigger_type_t type) {
    for (auto &&trigger : triggers)
        if (trigger.type == type)
            return (void *)trigger.handler;
    return NULL;
}

trigger_type_t trigger_type_get(const char *s) {
    String str = String(s);
    for (auto &&trigger : triggers)
        if (str.equalsIgnoreCase(trigger.name))
            return trigger.type;
    return NOT_A_TYPE;
}

// 添加任务
// 格式 "action type [condition] [pramater1] [pramater2]"
void task_add(const char *s) {
    float v1 = 0, v2 = 0;
    char type_s[15], condition_s[15], action_s[15];
    int match = sscanf(s, "%s %s %s %f %f", action_s, type_s, condition_s, &v1, &v2);
    if (match > 0) {
        log_i("Action:%s, Type:%s, Condition:%s, V1:%f, V2:%f\n",
                      action_s, type_s, condition_s, v1, v2);
        trigger_type_t type = trigger_type_get(type_s);

        if (type == NOT_A_TYPE) {
            log_e("\"%s\" not a type!\n", type_s);
            return;
        }
        void *condition = condition_get(condition_s);
        void *action = action_get(action_s);
        void *handler = trigger_function_get(type);
        if (type == INSTANT) { // instant tasks
            bool sta = condition_result(condition) == F_TRUE;
            action_execute(action, sta);
            log_i("Instant task with %s\n", sta ? "TRUE" : "FALSE");
            return;
        } else {
            bool sta = task_add(handler, condition, action, v1, v2);
            log_i("Task add %s!\n", sta ? "SUCCESS" : "FAIL");
        }
    } else {
        log_e("TASK FORMAT ERROR!\n");
    }
}

typedef struct _task {
    void *handler, *condition, *action;
    float v1, v2;
    bool operator==(const struct _task t) const {
        return handler == t.handler &&
               condition == t.condition &&
               action == t.action;
    }
} task_t;

std::list<task_t> tasks;

bool task_add(void *handler, void *condition, void *action, float v1, float v2) {
    if (handler && condition && action) {
        task_t task = {handler, condition, action, v1, v2};
        tasks.push_back(task);
        tasks.unique(); // 去重，相同触发方式、条件、动作不能同时存在
        return true;
    }
    return false;
}

void task_do(task_t t) { 
    if (t.handler == NULL) return;
    ((handler_t)t.handler)(t.condition, t.action, t.v1, t.v2); 
}

void task_check(void) { std::for_each(tasks.begin(), tasks.end(), task_do); }

void task_clear(void) { tasks.clear(); }

int task_get_count() { return tasks.size(); }

#include <CronAlarms.h>

// 添加定时任务
// 格式 "type action isoneshot cronString"
void alarm_add(const char *s) {
    char action[15], isoneshot[10];
    int match = sscanf(s, "%s %s ", action, isoneshot);
    String ss = s, action_s = action, isoneshot_s = isoneshot;

    bool oneshot = isoneshot_s.equalsIgnoreCase("true");
    action_func_t func = (action_func_t)action_get(action);
    if (func == NULL) {
        log_e("Alarm add failed!");
        return;
    }
    ss.remove(0, action_s.length() + isoneshot_s.length() + 2);
    log_i("Add alarm With s:\"%s\", action:%s, isoneshot:%d\n", ss.c_str(), action, oneshot);
    alarm_add(ss.c_str(), (void(*)(void))func, oneshot);
    log_i("Alarm add Succeeded!");
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

bool setting_apply(String setting) {
    if (setting.startsWith("Wi-Fi ")) {
        setting.replace("Wi-Fi ", "");
        return Preferences_UpdateWIFISetting(setting);
    }
    return true;
}

void reset_to_default_state() {
    task_clear();
    alarm_clear();
    Relay_Set(false);
    LED_Set(false);
    Beeper_Set(false);
}

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
