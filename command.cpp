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

// 0        1
// ir_send  id
int32_t ir_send_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    Infrared_SendPreset(atoi(argv[1]));
    return 0;
}

// 0            1       2
// ir_capture   start
// ir_cpature   end     id
int32_t ir_capture_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("start")) {
        CHECK_ARGC(1);
        Infrared_StartCapture();
    } else if (flag.equalsIgnoreCase("end")) {
        CHECK_ARGC(2);
        if (Infrared_EndCapture(atoi(argv[2])))
            Infrared_StorePreset(Preferences_Get());
    } else {
        FLAG_NOT_MATCH();
    }
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

const struct {
    const char* cmd_name;
    lwshell_cmd_fn cmd_fn;
    const char* desc;
} lwshell_cmd_list[] = {
    {"ir_send",     ir_send_cmd,    "Infrared send Nth preset"  },
    {"ir_capture",  ir_capture_cmd, "Infrared Capture"          },
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

bool Command_Run(String cmd) {
    cmd += '\n';
    return lwshell_input(cmd.c_str(), cmd.length()) == lwshellOK;
}
