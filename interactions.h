#if !defined(_INTERATIONS_H_)
#define _INTERATIONS_H_

void Upload_Data();
void Get_Command();

int get_access_fail_count();
int get_retry_after();
bool get_server_state();
bool test_server_connection();

#define interactions() \
    do                 \
    {                  \
        Upload_Data(); \
        Get_Command(); \
    } while (0)

static const char *taskPrefix = "Task ";
static const char *alarmPrefix = "Alarm ";
static const char *deletePrefix = "Delete ";
static const char *settingPrefix = "Setting ";

#endif // _INTERATIONS_H_
