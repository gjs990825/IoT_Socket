#if !defined(_COMMAND_H_)
#define _COMMAND_H_

#include <WString.h>

void Command_CheckSerial();
void Command_Init();
void Command_OutputControl(bool sta);
bool Command_IsValid(String command);
bool Command_Run(String cmd);

void CommandQueue_Add(String cmd);
void CommandQueue_Handle();

void _task_check();

#endif // _COMMAND_H_
