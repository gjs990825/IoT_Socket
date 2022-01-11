#include "command.h"
#include "bsp.h"
#include "lwshell.h"
#include "sensors.h"
#include "infrared.h"
#include "tasks.h"

#define CHECK_ARGC(required_num)                    \
            do {                                    \
                if (argc < required_num + 1) {      \
                    log_e("arg num err:%d", argc);  \
                    return -1;                      \
                }                                   \
            } while(0) 

#define FLAG_NOT_MATCH() do { log_e("flag not match"); } while(0)

// 0        1       2           3
// infrared send    preset_id
// infrared remove  preset_id
// infrared capture start
// infrared cpature end         id
int32_t infrared_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("send")) {
        CHECK_ARGC(2);
        Infrared_SendPreset(atoi(argv[2]));
    } else if (flag.equalsIgnoreCase("remove")) {
        CHECK_ARGC(2);
        Infrared_RemovePreset(atoi(argv[2]), Preferences_Get());
    } else if (flag.equalsIgnoreCase("capture")) {
        CHECK_ARGC(2);
        flag = argv[2];
        if (flag.equalsIgnoreCase("start")) {
            Infrared_StartCapture();
        } else if (flag.equalsIgnoreCase("end")) {
            CHECK_ARGC(3);
            if (Infrared_EndCapture(atoi(argv[3])))
                Infrared_StorePreset(Preferences_Get());
        } else {
            FLAG_NOT_MATCH();
        }
    } else {
        FLAG_NOT_MATCH();
    }
    return 0;
}

// 0            1       2   3
// preference   add     key content
// preference   remove  key
// preference   check   key
int32_t preference_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("add")) {
        CHECK_ARGC(3);
        Preferences_Get().putString(argv[2], argv[3]);
    } else if (flag.equalsIgnoreCase("remove")) {
        CHECK_ARGC(2);
        Preferences_Get().remove(argv[2]);
    } else if (flag.equalsIgnoreCase("check")) {
        CHECK_ARGC(2);
        String res = Preferences_Get().getString(argv[2]);
        log_i("key:%s, content:%s", argv[2], res.c_str());
    } else {
        FLAG_NOT_MATCH();
    }
    return 0;
}

// 0    1   2
// run  cmd arg
int32_t run_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    Serial.printf("argc:%d\n", argc);
    Serial.printf("argv[0]:%s\n", argv[0]);
    String cmd = String();
    for (int i = 1; i < argc; i++) {
        cmd += argv[i];
        cmd += ' ';
    }
    cmd.trim();
    Serial.printf("cmd:\"%s\"\n", cmd.c_str());
    CommandQueue_Add(cmd);
    return 0;
}


// 0    1
// led  sta
int32_t led_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    LED_Set((uint8_t)atoi(argv[1]));
    return 0;
}

// 0        1
// relay    sta
int32_t relay_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    Relay_Set(atoi(argv[1]));
    return 0;
}

// 0    1
// beep sta
int32_t beep_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    Beep_Set(atoi(argv[1]));
    return 0;
}

typedef void (*alarm_action_func_t)();

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
    {"beep_on",     [](){ Beep_Set(true);	        }},
    {"beep_off",    [](){ Beep_Set(false);	        }},
    {"beep_flip",   [](){ Beep_Flip();		        }},
    {"ir_preset_0", [](){ Infrared_SendPreset(0);   }},
    {"ir_preset_1", [](){ Infrared_SendPreset(1);   }},
    {"ir_preset_2", [](){ Infrared_SendPreset(2);   }},
    {"ir_preset_3", [](){ Infrared_SendPreset(3);   }},
};

alarm_action_func_t alarm_action_get(const char *s) {
    String str = s;
    for (auto &&action : alarm_actions)
        if (str.equalsIgnoreCase(action.name))
            return action.func;
    return NULL;
}

// 0        1       2           3       4
// alarm    add     cron_string action  is_oneshot 
// alarm    remove  action
// alarm    clear
int32_t alarm_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("clear")) {
        CHECK_ARGC(1);
        alarm_clear();
    }
    if (flag.equalsIgnoreCase("remove")) {
        CHECK_ARGC(2);
        alarm_remove(alarm_action_get(argv[2]));
    } else if (flag.equalsIgnoreCase("add")) {
        CHECK_ARGC(4);
        alarm_add(argv[2], alarm_action_get(argv[3]), atoi(argv[4]));
    } else {
        FLAG_NOT_MATCH();
    }    
    return 0;
}

#include <vector>
std::vector<String> _tasks;

void _task_add(const char *cmd) {
    String cmd_s = cmd;
    cmd_s.trim();
    _tasks.push_back(cmd_s);

    // remove duplicate
    std::sort(_tasks.begin(), _tasks.end());
    _tasks.erase(std::unique(_tasks.begin(), _tasks.end()), _tasks.end());
}

void _task_clear() {
    _tasks.clear();
}

void _task_remove(const char *cmd) {
    _tasks.erase(std::remove_if(_tasks.begin(),
                                _tasks.end(),
                                [cmd](String s) -> bool {
                                    return s.equalsIgnoreCase(cmd);
                                }),
                _tasks.end());
}

void _task_check() {
    for (auto &&task : _tasks)
        Command_Run(task);
}

// 0    1       2
// task add     cmd
// task remove  cmd
// task clear
// task check
int32_t task_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("clear")) {
        _task_clear();
    } else if (flag.equalsIgnoreCase("add")) {
        CHECK_ARGC(2);
        _task_add(argv[2]);
    } else if (flag.equalsIgnoreCase("remove")) {
        CHECK_ARGC(2);
        _task_remove(argv[2]);
    } else if (flag.equalsIgnoreCase("check")) {
        for (auto &&cmd : _tasks)
            Serial.println(cmd);
    } else {
        FLAG_NOT_MATCH();
    }    
    return 0;
}


typedef float (*condition_func_t)();

const struct {
    const char *name;
    condition_func_t func;
} conditions[] = {
    {"temperature",	Sensors::getTemperature         },
    {"pressure",	Sensors::getPressure	        },
    {"light",		Sensors::getLight		        },
    {"true",		[]() -> float { return 0.0; }   },
    {"false",		[]() -> float { return 1.0; }   },
};

condition_func_t _condition_get(const char *s) {
    String str = String(s);
    for (auto &&condition : conditions)
        if (str.equalsIgnoreCase(condition.name))
            return condition.func;
    return NULL;
}

float condition_execute(condition_func_t func) { return func ? func() : 0.0; }

#define PARSE_BOOL(B) ((B) ? '1' : '0')

// 0        1           2       3
// higher   condition   action  param
// higher   light       relay   1.5
int32_t higher_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(3);
    condition_func_t condition = _condition_get(argv[1]);
    String action = argv[2];
    float set = atoff(argv[3]);
    
    if (condition == NULL) {
        log_e("invalid condition:%s", argv[1]);
        return -1;
    }

    float condition_result = condition_execute(condition);
    bool result = condition_result > set;

    String cmd = action + ' ' + PARSE_BOOL(result);
    CommandQueue_Add(cmd);
    
    return 0;
}

const struct {
    const char* cmd_name;
    lwshell_cmd_fn cmd_fn;
    const char* desc;
} lwshell_cmd_list[] = {
    {"run",         run_cmd,        "run some command"          },
    {"higher",      higher_cmd,     "higher handler"            },
    {"infrared",    infrared_cmd,   "Infrared management"       },
    {"preference",  preference_cmd, "Preference management"     },
    {"led",         led_cmd,        "Led brightness control"    },
    {"relay",       relay_cmd,      "Relay control"             },
    {"beep",        beep_cmd,       "Beeper control"            },
    {"alarm",       alarm_cmd,      "Alarm setting"             },
    {"task",        task_cmd,       "Task setting"              },
};

void lwshell_register_cmd() {
    for (auto &&cmd : lwshell_cmd_list)
        lwshell_register_cmd(cmd.cmd_name, cmd.cmd_fn, cmd.desc);
}

void Command_Init() {
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
}

void Command_CheckSerial() {
    const unsigned int MAX_MESSAGE_LENGTH = 80;
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

bool Command_Run(String cmd) {
    cmd += '\n';
    return lwshell_input(cmd.c_str(), cmd.length()) == lwshellOK;
}

#include <queue>
std::queue<String> cmd_queue;

constexpr uint8_t MAX_CMD_QUEUE = 10;

void CommandQueue_Add(String cmd) {
    if (cmd_queue.size() >= MAX_CMD_QUEUE) {
        log_e("command queue full");
        return;
    }
    cmd_queue.push(cmd);
}

void CommandQueue_Handle() {
    if (cmd_queue.empty()) 
        return;
    while (!cmd_queue.empty()) {
        String cmd = cmd_queue.front();
        cmd_queue.pop();
        Command_Run(cmd);
    }
}
