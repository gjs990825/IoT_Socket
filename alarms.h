#if !defined(_ALARMS_H_)
#define _ALARMS_H_

#include <WString.h>

typedef void (*alarm_action_func_t)();

alarm_action_func_t alarm_action_get(String str);
void alarm_add(const char *cron_string, void (*handler)(void), bool is_oneshot);
void alarm_remove(void (*handler)(void));
int alarm_get_count();
void alarm_clear();

#endif // _ALARMS_H_
