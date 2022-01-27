#if !defined(_MISC_H_)
#define _MISC_H_

#include <WString.h>

String get_state_string();

int get_access_fail_count();
void update_access_fail_count(bool sta);
uint8_t get_retry_after();
bool is_server_available();
bool test_server_connection();

#endif // _MISC_H_
