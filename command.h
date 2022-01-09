#if !defined(_COMMAND_H_)
#define _COMMAND_H_

#include <WString.h>

void Command_CheckSerial();
void Command_Init();
bool Command_Run(String cmd);

void _task_check();

#endif // _COMMAND_H_
