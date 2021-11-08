#if !defined(_TASKS_H)
#define _TASKS_H

#include <Arduino.h>
#include <stddef.h>

void action_print_available();
void condition_print_available();
void trigger_type_print_available();

#define task_print_help()               \
    do                                  \
    {                                   \
        trigger_type_print_available(); \
        condition_print_available();    \
        action_print_available();       \
    } while (0)

bool task_add(void *handler, void *condition, void *action, float v1, float v2);
void task_add(const char *s);
void task_check(void);
void task_clear(void);
int task_get_count();

void alarm_add(const char *s, void(*handler)(void), bool is_oneshot);
void alarm_add(const char *s);
void alarm_clear();
int alarm_get_count();

void reset_to_default_state();

String get_state_string();

#endif // _TASKS_H
