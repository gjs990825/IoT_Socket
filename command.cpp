#include "command.h"
#include "bsp.h"
#include "lwshell.h"
#include "sensors.hpp"
#include "infrared.h"
#include "mqtt_connection.h"
#include "alarms.h"
#include "tasks.h"
#include "json_helper.h"
#include <queue>
#include <vector>

typedef enum {
    MSG_NONE,
    MSG_ARGC_ERR, 
    MSG_FLAG_NOT_MATCH,
    MSG_OPERATION_SUCCESS,
    MSG_OPERATION_FAIL,
    MSG_INVALID_INPUT,
    MSG_INVALID_ACTION,
    MSG_INVALID_CONDITION,
    MSG_RESET_SUCCESS,
    MSG_COMMAND_DOES_NOT_EXIST,
    MSG_INFRARED_CAPTURE_START,
    MSG_INFRARED_CAPTURE_SUCCESS,
    MSG_INFRARED_CAPTURE_FAIL,
    MSG_TASK_ADD_SUCCESS,
    MSG_TASK_ADD_FAIL,
    MSG_TASK_DUPLICATED,
    MSG_TASK_TYPE_INVALID,
    MSG_TASK_REMOVE_SUCCESS,
    MSG_TASK_REMOVE_FAIL,
    MSG_ALARM_ADD_SUCCESS,
    MSG_ALARM_ADD_FAIL,
    MSG_ALARM_REMOVE_SUCCESS,
    MSG_ALARM_REMOVE_FAIL,
    MSG_WIFI_SETTING_UPDATING,
    MSG_TIME_SETTING_UPDATE_SUCCESS,
    MSG_TIME_SETTING_UPDATE_FAIL,
    MSG_SYSTEM_REBOOTING,
} msg_code_t;

const struct {
    msg_code_t id;
    const char *const msg;
} code_list[] = {
    {MSG_NONE,                          ""                              },
    {MSG_ARGC_ERR,                      "Argument number error"         },
    {MSG_FLAG_NOT_MATCH,                "Command flag not match"        },
    {MSG_OPERATION_SUCCESS,             "Operation success"             },
    {MSG_OPERATION_FAIL,                "Operation fail"                },
    {MSG_INVALID_INPUT,                 "Invalid input"                 },
    {MSG_INVALID_ACTION,                "Invalid action"                },
    {MSG_INVALID_CONDITION,             "Invalid condition"             },
    {MSG_RESET_SUCCESS,                 "Reset Success"                 },
    {MSG_COMMAND_DOES_NOT_EXIST,        "Command does not exist"        },
    {MSG_INFRARED_CAPTURE_START,        "Infrared capture start"        },
    {MSG_INFRARED_CAPTURE_SUCCESS,      "Infrared capture success"      },
    {MSG_INFRARED_CAPTURE_FAIL,         "Infrared capture fail"         },
    {MSG_TASK_ADD_SUCCESS,              "Task add success"              },
    {MSG_TASK_ADD_FAIL,                 "Task add fail"                 },
    {MSG_TASK_DUPLICATED,               "Task duplicated"               },
    {MSG_TASK_TYPE_INVALID,             "Task type invalid"             },
    {MSG_TASK_REMOVE_SUCCESS,           "Task remove success"           },
    {MSG_TASK_REMOVE_FAIL,              "Task remove fail"              },
    {MSG_ALARM_ADD_SUCCESS,             "Alarm add success"             },
    {MSG_ALARM_ADD_FAIL,                "Alarm add fail"                },
    {MSG_ALARM_REMOVE_SUCCESS,          "Alarm remove success"          },
    {MSG_ALARM_REMOVE_FAIL,             "Alarm remove fail"             },
    {MSG_WIFI_SETTING_UPDATING,         "WiFi setting updating"         },
    {MSG_TIME_SETTING_UPDATE_SUCCESS,   "TIME setting update success"   },
    {MSG_TIME_SETTING_UPDATE_FAIL,      "TIME setting update fail"      },
    {MSG_SYSTEM_REBOOTING,              "System rebooting"              },
};

const char *get_message_text(msg_code_t id) {
    for (auto &&i : code_list) {
        if (i.id == id) {
            return i.msg;
        }
    }
    return "";
}

msg_code_t command_msg;
void Command_SetMessage(msg_code_t id) {
    command_msg = id;
}

void Command_ClearMeaagae() {
    command_msg = MSG_NONE;
}

String Command_GetMessage() {
    msg_code_t id = command_msg;
    Command_ClearMeaagae();
    return get_message_text(id);
}

int Command_GetMessageCode() {
    msg_code_t id = command_msg;
    Command_ClearMeaagae();
    return (int)id;
}

bool arg_is_boolean(String arg) {
    return arg.equalsIgnoreCase("true") ||
        arg.equalsIgnoreCase("false");
}

bool arg_2_boolean(String arg) {
    return arg.equalsIgnoreCase("true");
}

int arg_2_int(String arg) {
    if (arg.equalsIgnoreCase("true")) {
        return 1;
    } else if (arg.equalsIgnoreCase("false")) {
        return 0;
    } else {
        return atoi(arg.c_str());
    }
}

void output_peripheral_arg_handler(OutputPeripheral *peripheral, String arg) {
    if (arg_is_boolean(arg)) {
        peripheral->set(arg_2_boolean(arg));
    } else {
        peripheral->set(arg_2_int(arg));
    }
}

#define CHECK_ARGC(required_num)                        \
            do {                                        \
                if (argc < required_num + 1) {          \
                    log_e("arg num err:%d", argc);      \
                    Command_SetMessage(MSG_ARGC_ERR);   \
                    return -1;                          \
                }                                       \
            } while(0) 

#define FLAG_NOT_MATCH()                                \
            do {                                        \
                log_e("flag not match");                \
                Command_SetMessage(MSG_FLAG_NOT_MATCH); \
                return -1;                              \
            } while(0)

#define CHECK_ACTION(action)                        \
    do {                                            \
        if (!Command_IsValid(action)) {             \
            log_e("invalid action:%s", action);     \
            Command_SetMessage(MSG_INVALID_ACTION); \
            return -1;                              \
        }                                           \
    } while (0)

// 0        1       2           3
// infrared send    preset_id
// infrared remove  preset_id
// infrared capture start       preset_id
// infrared cpature end
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
            CHECK_ARGC(3);
            Infrared_StartCapture(atoi(argv[3]));
            Command_SetMessage(MSG_INFRARED_CAPTURE_START);
        } else if (flag.equalsIgnoreCase("end")) {
            if (Infrared_EndCapture()) {
                Infrared_StorePreset();
                Command_SetMessage(MSG_INFRARED_CAPTURE_SUCCESS);
            } else {
                Command_SetMessage(MSG_INFRARED_CAPTURE_FAIL);
            }
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
    Serial.printf("cmd:\"%s\"\n", cmd.c_str());
    CommandQueue_Add(cmd);
    return 0;
}

// 0    1
// led  brightness
int32_t led_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    output_peripheral_arg_handler(&Led, argv[1]);
    return 0;
}

// 0    1
// pwm  duty
int32_t pwm_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    output_peripheral_arg_handler(&Pwm, argv[1]);
    return 0;
}

// 0        1
// relay    sta
int32_t relay_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    output_peripheral_arg_handler(&Relay, argv[1]);
    return 0;
}

// 0        1
// beeper   sta
int32_t beeper_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    output_peripheral_arg_handler(&Beeper, argv[1]);
    return 0;
}

const struct {
    const char *const name;
    OutputPeripheral *peripheral;
} flip_list[] = {
    {"relay"    , &Relay    },
    {"beeper"   , &Beeper   },
    {"led"      , &Led      },
    {"pwm"      , &Pwm      },
};

// 0    1
// flip peripheral
int32_t flip_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String peripheral = argv[1];
    for (auto &&f : flip_list) {
        if (peripheral.equalsIgnoreCase(f.name)) {
            f.peripheral->flip();
            return 0;
        }
    }
    FLAG_NOT_MATCH();
}

// 0        1       2           3       4
// alarm    add     cron_string action  is_oneshot 
// alarm    remove  action
// alarm    clear
int32_t alarm_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("clear")) {
        alarm_clear();
        Command_SetMessage(MSG_ALARM_REMOVE_SUCCESS);
    } else if (flag.equalsIgnoreCase("remove")) {
        CHECK_ARGC(2);
        bool ret = alarm_remove(argv[2]);
        Command_SetMessage(ret ? MSG_ALARM_REMOVE_SUCCESS : MSG_ALARM_REMOVE_FAIL);
    } else if (flag.equalsIgnoreCase("add")) {
        CHECK_ARGC(4);
        CHECK_ACTION(argv[3]);
        bool ret = alarm_add(argv[2], argv[3], arg_2_int(argv[4]));
        Command_SetMessage(ret ? MSG_ALARM_ADD_SUCCESS : MSG_ALARM_ADD_FAIL);
    } else if (flag.equalsIgnoreCase("check")) {
        String *alarm_list = alarm_get_list();
        for (int i = 0; i < alarm_get_list_size(); i++) {
            if (!alarm_list[i].isEmpty()) {
                Serial.println(alarm_list[i]);
            }
        }
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
    {"temperature",	[]() { return Sensors.getTemperature(); }       },
    {"pressure",	[]() { return Sensors.getPressure(); }          },
    {"brightness",  []() { return (float)Sensors.getBrightness(); } },
    {"true",		[]() { return 0.0f; }                           },
    {"false",		[]() { return 1.0f; }                           },
};

bool condition_is_valid(String str) {
    for (auto &&condition : conditions)
        if (str.equalsIgnoreCase(condition.name))
            return true;
    return false;
}

condition_func_t condition_get(String str) {
    for (auto &&condition : conditions)
        if (str.equalsIgnoreCase(condition.name))
            return condition.func;
    return NULL;
}

const char* task_types[] = {
    "higher",
    "lower",
    "equal",
    "not_equal",
    "linear",
    "interval",
    "inverse_interval",
};

bool task_type_is_valid(String type) {
    for (auto &&t : task_types)
        if (type.equalsIgnoreCase(t)) 
            return true;
    return false;
}

float condition_execute(condition_func_t func) { return func ? func() : 0.0; }

float map_float(float x, float in_min, float in_max, float out_min, float out_max) {
    float divisor = (in_max - in_min);
    if (divisor == 0) {
        return -1;
    }
    return (x - in_min) * (out_max - out_min) / divisor + out_min;
}

// 0        1       2           3           4+
// handler  action  condition   type        params+
// handler  relay   brightness  higher      1.5
// handler  led     condition   linear      in_l    in_h    out_l   out_h
// handler  led     condition   interval    low     high   
int32_t task_handler_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(3);
    String action = argv[1];
    condition_func_t condition = condition_get(argv[2]);
    String type = argv[3];

    CHECK_ACTION(action.c_str());
    
    if (condition == NULL) {
        log_e("invalid condition:%s", argv[1]);
        Command_SetMessage(MSG_INVALID_CONDITION);
        return -1;
    }

    float condition_result = condition_execute(condition);
    int result = 0;
    
    if (type.equalsIgnoreCase("higher")) {
        CHECK_ARGC(4);
        result = condition_result > atoff(argv[4]);
    } else if (type.equalsIgnoreCase("lower")) {
        CHECK_ARGC(4);
        result = condition_result < atoff(argv[4]);
    } else if (type.equalsIgnoreCase("equal")) {
        CHECK_ARGC(4);
        result = condition_result == atoff(argv[4]);
    } else if (type.equalsIgnoreCase("not_equal")) {
        CHECK_ARGC(4);
        result = condition_result != atoff(argv[4]);
    } else if (type.equalsIgnoreCase("linear")) {
        CHECK_ARGC(7);
        float inMin = atoff(argv[4]),
            inMax = atoff(argv[5]),
            outMin = atoff(argv[6]),
            outMax = atoff(argv[7]);

        result = (int)map_float(
            constrain(condition_result, inMin, inMax),
            inMin,
            inMax,
            outMin,
            outMax
        );
    } else if (type.equalsIgnoreCase("interval")) {
        CHECK_ARGC(5);
        float l = atoff(argv[4]), h = atoff(argv[5]);
        if (condition_result > h) {
            result = 1;
        } else if (condition_result < l) {
            result = 0;
        } else {
            // nothing to do.
            return 0;
        }
    } else if (type.equalsIgnoreCase("reverse_interval")) {
        CHECK_ARGC(5);
        float l = atoff(argv[4]), h = atoff(argv[5]);
        if (condition_result > h) {
            result = 0;
        } else if (condition_result < l) {
            result = 1;
        } else {
            // nothing to do.
            return 0;
        }
    } else {
        FLAG_NOT_MATCH();
    }

    String cmd = action + ' ' + result;
    CommandQueue_Add(cmd);
    
    return 0;
}

// 0    1       2       3           4       5
// task flag    action  condition   type    params+
// task add     
// task remove  action
// task clear
// task check
int32_t task_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("clear")) {
        task_clear();
        Command_SetMessage(MSG_TASK_REMOVE_SUCCESS);
    } else if (flag.equalsIgnoreCase("add")) {
        CHECK_ARGC(5);
        CHECK_ACTION(argv[2]);

        if (!condition_is_valid(argv[3])) {
            log_e("invalid condition:%s", argv[3]);
            Command_SetMessage(MSG_INVALID_CONDITION);
            return -1;
        }
        if (!task_type_is_valid(argv[4])) {
            log_e("invalid type:%s", argv[4]);
            Command_SetMessage(MSG_TASK_TYPE_INVALID);
            return -1;
        }
        String task = "task_handler ";
        for (int i = 2; i < argc; i++) {
            task += argv[i];
            task += ' ';
        }

        log_i("task to be added:%s", task.c_str());
        bool ret = task_add(task);
        Command_SetMessage(ret ? MSG_TASK_ADD_SUCCESS : MSG_TASK_DUPLICATED);
    } else if (flag.equalsIgnoreCase("remove")) {
        CHECK_ARGC(2);
        bool ret = task_remove(argv[2]);
        Command_SetMessage(ret ? MSG_TASK_REMOVE_SUCCESS : MSG_TASK_REMOVE_FAIL);
    } else if (flag.equalsIgnoreCase("check")) {
        for (auto &&cmd : task_get())
            Serial.println(cmd);
    } else {
        FLAG_NOT_MATCH();
    }    
    return 0;
}


// 0        1               2           3
// settings wifi            ssid        password
// settings wifi_instant    ssid        password
// settings time            timestamp
// settings reboot
// settings reboot_instant
int32_t settings_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("wifi")) {
        CHECK_ARGC(3);
        char buffer[128];
        sprintf(buffer, "settings wifi_instant \"%s\" \"%s\"", argv[2], argv[3]);
        CommandQueue_Add(buffer);
        Command_SetMessage(MSG_WIFI_SETTING_UPDATING);
    } else if (flag.equalsIgnoreCase("wifi_instant")) {
        CHECK_ARGC(3);
        log_w("update wifi setting:ssid:%s, pass:%s", argv[2], argv[3]);
        WiFi_UpdateSetting(argv[2], argv[3], Preferences_Get());
    } else if (flag.equalsIgnoreCase("time")) {
        CHECK_ARGC(2);
        bool ret = TimeStamp_Update(atol(argv[2]), Preferences_Get());
        Command_SetMessage(ret ? MSG_TIME_SETTING_UPDATE_SUCCESS : MSG_TIME_SETTING_UPDATE_FAIL);
    } else if (flag.equalsIgnoreCase("reboot")) {
        CommandQueue_Add("settings reboot_instant");
        Command_SetMessage(MSG_SYSTEM_REBOOTING);
    } else if (flag.equalsIgnoreCase("reboot_instant")) {
        log_w("System rebooting...");
        ESP.restart();
    } else {
        FLAG_NOT_MATCH();
    }
    return 0;
}

// 0    1
// mqtt send
int32_t mqtt_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("send")) {
        MQTT_Send();
    } else {
        FLAG_NOT_MATCH();
    }
    return 0;
}

// 0        1
// reset    default
int32_t reset_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String flag = argv[1];
    if (flag.equalsIgnoreCase("default")) {
        Command_Run("relay 0");
        Command_Run("beeper 0");
        Command_Run("led 0");
        Command_Run("motor 0");
        Command_Run("alarm clear");
        Command_Run("task clear");
        log_i("reset to default");
        Command_SetMessage(MSG_RESET_SUCCESS);
    } else if (flag.equalsIgnoreCase("wifi")) {
        WiFi_ForceUpdateSetting("", "", Preferences_Get());
        log_w("wifi setting reset");
    } else {
        FLAG_NOT_MATCH();
    }
    return 0;
}

// 0    1+
// echo ...
int32_t echo_cmd(int32_t argc, char** argv) {
    CHECK_ARGC(1);
    String cmd;
    for (int i = 1; i < argc - 1; i++) {
            cmd += argv[i];
            cmd += ' ';
    }
    Serial.println(cmd + argv[argc - 1]);
    return 0;
}

const struct {
    const char* cmd_name;
    lwshell_cmd_fn cmd_fn;
    const char* desc;
} lwshell_cmd_list[] = {
    {"run",             run_cmd,            "Run some command"          },
    {"task_handler",    task_handler_cmd,   "Task handler"              },
    {"settings",        settings_cmd,       "System settings"           },
    {"infrared",        infrared_cmd,       "Infrared management"       },
    {"preference",      preference_cmd,     "Preference management"     },
    {"led",             led_cmd,            "Led brightness control"    },
    {"pwm",             pwm_cmd,            "PWM duty control"          },
    {"relay",           relay_cmd,          "Relay control"             },
    {"beeper",          beeper_cmd,         "Beeper control"            },
    {"alarm",           alarm_cmd,          "Alarm setting"             },
    {"task",            task_cmd,           "Task setting"              },
    {"mqtt",            mqtt_cmd,           "MQTT send"                 },
    {"flip",            flip_cmd,           "Flip something"            },
    {"reset",           reset_cmd,          "Reset something"           },
    {"echo",            echo_cmd,           "Echo every input"          },
};

bool Command_IsValid(String command) {
    int index = command.indexOf(' ');
    if (index > 0) {
        command = command.substring(0, index);
    }
    for (auto &&cmd : lwshell_cmd_list) {
        if (command.equalsIgnoreCase(cmd.cmd_name)) {
            return true;
        }
    }
    return false;
}

void lwshell_register_cmd() {
    for (auto &&cmd : lwshell_cmd_list)
        lwshell_register_cmd(cmd.cmd_name, cmd.cmd_fn, cmd.desc);
}

void lwshell_output_function(const char* str, lwshell_t* lw) {
    Serial.printf("%s", str);
    if (*str == '\r')
        Serial.printf("\n");
}

void Command_OutputControl(bool sta) {
    lwshell_set_output_fn(sta ? lwshell_output_function : NULL);
}

void Command_Init() {
    // lwshell library init
    lwshell_init();
    Command_OutputControl(true);
    lwshell_register_cmd();
}

void Command_CheckSerial() {
    const unsigned int MAX_MESSAGE_LENGTH = 80;
    while (Serial.available() > 0) {
        static char message[MAX_MESSAGE_LENGTH];
        static unsigned int message_pos;

        char inByte = Serial.read();
        if (inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1)) {
            message[message_pos++] = inByte;
        } else {
            message[message_pos] = '\0';
            
            bool res = Command_Run(message);
            char *json = json_helper_parse_ack(res, Command_GetMessageCode());
            Serial.printf("Ack:%s\n", json);

            message_pos = 0;
        }
    }
}

lwshellr_t Command_RunRaw(String &cmd) {
    if (Command_IsValid(cmd)) {
        return lwshell_input(cmd.c_str(), cmd.length());
    }
    Command_SetMessage(MSG_COMMAND_DOES_NOT_EXIST);
    return lwshellERR;
}

bool Command_Run(std::vector<String> &cmd_list) {
    bool ret = true;
    int msg_code = MSG_NONE;
    for (auto &&cmd : cmd_list) {
        ret = ret && (Command_RunRaw(cmd) == lwshellOK);
        msg_code = max(msg_code, Command_GetMessageCode());
    }
    Command_SetMessage((msg_code_t)msg_code);
    return ret;
}

String split(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

bool Command_Run(String cmd) {
    cmd.trim();
    int index = cmd.indexOf('\n');
    if (index > 0) {
        std::vector<String> cmd_list;
        for (int i = 0; ; i++) {
            String tmp = split(cmd, '\n', i);
            if (tmp.isEmpty()) {
                break;
            }
            cmd_list.push_back(tmp + '\n');
        }
        return Command_Run(cmd_list);
    } else {
        Command_ClearMeaagae();
        cmd += '\n';
        return Command_RunRaw(cmd) == lwshellOK;
    }
}

std::queue<String> cmd_queue;
constexpr uint8_t MAX_CMD_QUEUE = 10;

bool CommandQueue_Add(String cmd) {
    if (cmd_queue.size() >= MAX_CMD_QUEUE) {
        log_e("command queue full");
        return false;
    }
    cmd_queue.push(cmd);
    return true;
}

bool CommandQueue_Add(std::vector<String> &cmd_list) {
    bool ret = true;
    for (auto &&cmd : cmd_list) {
        ret = ret && CommandQueue_Add(cmd);
        Serial.printf("Multi:%s\n", cmd.c_str());
    }
    return ret;
}

void CommandQueue_Handle() {
    if (cmd_queue.empty()) 
        return;
    if (!cmd_queue.empty()) {
        String cmd = cmd_queue.front();
        cmd_queue.pop();
        Command_Run(cmd);
    }
}
