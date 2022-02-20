#if !defined(_JSON_HELPER_H_)
#define _JSON_HELPER_H_

#include "ArduinoJson.h"

char *json_helper_parse_report();
char *json_helper_parse_ack(bool status, int msg);

#endif // _JSON_HELPER_H_
