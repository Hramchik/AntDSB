//
// Created by sanek on 20/12/2025.
//

#include "ConsoleControl.h"

#include <iostream>
#include <string>
#include <atomic>

#include "logger/Logger.h"

void ConsoleControlThread(std::atomic_bool& running_flag) {
    std::string line;
    std::cout << "Type 'exit' or 'quit' to stop the bot." << std::endl;

    while (running_flag && std::getline(std::cin, line)) {
        if (line == "exit" || line == "quit") {
            LogInfo("Console command received: " + line);
            running_flag = false;
            break;
        }
    }
}
