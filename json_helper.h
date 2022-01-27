#if !defined(_JSON_HELPER_H_)
#define _JSON_HELPER_H_

#include "ArduinoJson.h"

char *json_helper_parse_send();
char *json_helper_serialize();

extern DynamicJsonDocument jsonDoc;

#endif // _JSON_HELPER_H_
