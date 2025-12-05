//
// Created by sanek on 05/12/2025.
//

#include <iostream>

#include "CommandRegistry.h"
#include "utils/ThreadUtils.h"

std::map<std::string, CommandRegistry::CommandInfo>& CommandRegistry::GetCommands() {
    static std::map<std::string, CommandInfo> commands;
    return commands;
}

std::queue<CommandRegistry::PendingCommand>& CommandRegistry::GetCommandQueue() {
    static std::queue<PendingCommand> queue;
    return queue;
}

std::mutex& CommandRegistry::GetQueueMutex() {
    static std::mutex mtx;
    return mtx;
}

std::condition_variable& CommandRegistry::GetQueueCV() {
    static std::condition_variable cv;
    return cv;
}

bool& CommandRegistry::GetProcessorRunning() {
    static bool running = false;
    return running;
}

std::vector<std::thread>& CommandRegistry::GetWorkers() {
    static std::vector<std::thread> workers;
    return workers;
}

void CommandRegistry::RegisterCommand(const CommandInfo& cmd) {
    GetCommands()[cmd.name] = cmd;
    std::cout << "[CommandRegistry] Command registered: " << cmd.name << std::endl;
}

void CommandRegistry::RegisterAllCommands(dpp::cluster* cluster) {
    try {
        std::vector<dpp::slashcommand> commands;

        for (const auto& [name, info] : GetCommands()) {
            dpp::slashcommand cmd;
            cmd.name = info.name;
            cmd.description = info.description;
            commands.push_back(cmd);
        }

        cluster->global_bulk_command_create(commands);
        std::cout << "[CommandRegistry] All commands registered in Discord" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[CommandRegistry] Error registering commands: " << e.what() << std::endl;
    }
}

void CommandRegistry::HandleCommand(const dpp::slashcommand_t& event, dpp::cluster* cluster) {
    try {
        std::string cmd = event.command.get_command_name();
        auto& commands = GetCommands();

        auto it = commands.find(cmd);
        if (it != commands.end()) {
            it->second.callback(event);  // для простых команд
        } else {
            event.reply("Unknown command");
        }

    } catch (const std::exception& e) {
        std::cerr << "Error handling command: " << e.what() << std::endl;
        event.reply("Error executing command");
    }
}

void CommandRegistry::CommandWorker() {
    while (true) {
        PendingCommand pendingCmd;

        {
            std::unique_lock<std::mutex> lock(GetQueueMutex());
            GetQueueCV().wait(lock, [] {
                return !GetCommandQueue().empty() || !GetProcessorRunning();
            });

            if (!GetProcessorRunning() && GetCommandQueue().empty()) {
                return;
            }

            pendingCmd = GetCommandQueue().front();
            GetCommandQueue().pop();
        }

        try {
            auto& commands = GetCommands();
            auto it = commands.find(pendingCmd.name);

            if (it != commands.end()) {
                it->second.callback(pendingCmd.event);
            } else {
                pendingCmd.event.reply("Unknown command");
            }

        } catch (const std::exception& e) {
            std::cerr << "[CommandRegistry] Error processing command: " << e.what() << std::endl;
            pendingCmd.event.reply("Error executing command");
        }
    }
}

void CommandRegistry::StartCommandProcessor() {
    if (GetProcessorRunning()) {
        std::cout << "[CommandRegistry] Processor already running" << std::endl;
        return;
    }

    int numThreads = calc_thread_count(0.75); // 75% от hardware_concurrency[web:2][web:3]
    GetProcessorRunning() = true;

    auto& workers = GetWorkers();
    workers.clear();
    workers.reserve(numThreads);

    std::cout << "[CommandRegistry] Starting " << numThreads << " worker threads\n";

    for (int i = 0; i < numThreads; ++i) {
        workers.emplace_back(CommandWorker);
    }
}

void CommandRegistry::StopCommandProcessor() {
    {
        std::lock_guard<std::mutex> lock(GetQueueMutex());
        GetProcessorRunning() = false;
    }
    GetQueueCV().notify_all();

    auto& workers = GetWorkers();
    for (auto& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }
    workers.clear();

    std::cout << "[CommandRegistry] Processor stopped\n";
}