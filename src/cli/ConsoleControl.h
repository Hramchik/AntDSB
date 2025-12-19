//
// Created by sanek on 20/12/2025.
//

#ifndef ANTDSB_CONSOLECONTROL_H
#define ANTDSB_CONSOLECONTROL_H

#include <atomic>

void ConsoleControlThread(std::atomic_bool& running_flag);

#endif //ANTDSB_CONSOLECONTROL_H