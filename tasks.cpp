#include "tasks.h"
#include "command.h"
#include <algorithm>

std::vector<String> tasks;

void task_add(String cmd) {
    cmd.trim();
    tasks.push_back(cmd);

    // remove duplicate
    std::sort(tasks.begin(), tasks.end());
    tasks.erase(std::unique(tasks.begin(), tasks.end()), tasks.end());
}

void task_clear() {
    tasks.clear();
    log_i("tasks cleared");
}

void task_remove(String cmd) {
    tasks.erase(std::remove_if(tasks.begin(),
                                tasks.end(),
                                [cmd](String s) -> bool {
                                    return s.equalsIgnoreCase(cmd);
                                }),
                tasks.end());
}

void task_check() {
    // Command_OutputControl(false);
    for (auto &&task : tasks)
        Command_Run(task);
    // Command_OutputControl(true);
}

std::vector<String> task_get() { return tasks; }

int task_get_count() { return tasks.size(); }

