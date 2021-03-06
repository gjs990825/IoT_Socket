#if !defined(_COMMAND_H_)
#define _COMMAND_H_

#include <WString.h>
#include <vector>

void Command_Init();
void Command_CheckSerial();
void Command_OutputControl(bool sta);
bool Command_IsValid(String command);
bool Command_Run(String cmd);
void Command_ClearMeaagae();
int Command_GetMessageCode();

bool CommandQueue_Add(String cmd);
bool CommandQueue_Add(std::vector<String> &cmd_list);
void CommandQueue_Handle();

#endif // _COMMAND_H_
