#if !defined(_TASKS_H_)
#define _TASKS_H_

#include <vector>
#include <WString.h>

void task_add(String cmd);
void task_check();
int task_get_count();
void task_clear();
std::vector<String> task_get();
void task_remove(String cmd);

#endif // _TASKS_H_
