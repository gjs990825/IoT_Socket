#if !defined(_ALARMS_H_)
#define _ALARMS_H_

#include <WString.h>
#include <functional>

bool alarm_add(const char *cron_string, String action, bool is_oneshot);
bool alarm_remove(String name);
int alarm_get_count();
void alarm_clear();
void alarm_check();
int alarm_get_count();
int alarm_get_list_size();
String* alarm_get_list();

#endif // _ALARMS_H_
