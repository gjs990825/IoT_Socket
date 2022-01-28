#if !defined(_ALARMS_H_)
#define _ALARMS_H_

#include <WString.h>
#include <functional>

bool alarm_add(const char *cron_string, String cmd, bool is_oneshot);
bool alarm_remove(String name);
int alarm_get_count();
void alarm_clear();
void alarm_check();

#endif // _ALARMS_H_
