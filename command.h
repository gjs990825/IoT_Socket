#if !defined(_COMMAND_H_)
#define _COMMAND_H_

#include <WString.h>

void Command_CheckSerial();
void Command_Init();
void Command_OutputControl(bool sta);
bool Command_IsValid(String command);
bool Command_Run(String cmd);

bool CommandQueue_Add(String cmd);
void CommandQueue_Handle();

#endif // _COMMAND_H_
