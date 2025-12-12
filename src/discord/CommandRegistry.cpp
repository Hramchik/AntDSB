//
// Created by sanek on 05/12/2025.
//

#include <iostream>

#include "CommandRegistry.h"
#include "utils/ThreadUtils.h"
#include "logger/Logger.h"

CommandRegistry::CommandMap& CommandRegistry::GetCommands() {
    static CommandMap commands;
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
    LogInfo("[CommandRegistry] Command registered: " + cmd.name);
}

void CommandRegistry::RegisterAllCommands(dpp::cluster* cluster) {
    try {
        std::vector<dpp::slashcommand> commands;
        commands.reserve(GetCommands().size());

        for (const auto& [name, info] : GetCommands()) {
            dpp::slashcommand cmd;
            cmd.name        = info.name;
            cmd.description = info.description;
            commands.push_back(cmd);
        }

        cluster->global_bulk_command_create(commands);
        LogInfo("[CommandRegistry] All commands registered in Discord");

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[CommandRegistry] Error registering commands: " + except);
    }
}

void CommandRegistry::HandleCommand(const dpp::slashcommand_t& event, dpp::cluster* /*cluster*/) {
    try {
        std::string cmd = event.command.get_command_name();
        auto& commands  = GetCommands();

        auto it = commands.find(cmd);
        if (it != commands.end()) {
            it->second.callback(event);
        } else {
            event.reply(dpp::message("Unknown command").set_flags(dpp::m_ephemeral));
        }

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[CommandRegistry] Error handling command: " + except);
        event.reply(dpp::message("Error executing command").set_flags(dpp::m_ephemeral));
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

            pendingCmd = std::move(GetCommandQueue().front());
            GetCommandQueue().pop();
        }

        try {
            auto& commands = GetCommands();
            auto it = commands.find(pendingCmd.name);

            if (it != commands.end()) {
                it->second.callback(pendingCmd.event);
            } else {
                pendingCmd.event.reply(dpp::message("Unknown command").set_flags(dpp::m_ephemeral));
            }

        } catch (const std::exception& e) {
            std::string except = e.what();
            LogError("[CommandRegistry] Error processing command: " + except);
            pendingCmd.event.reply(dpp::message("Error executing command").set_flags(dpp::m_ephemeral));
        }
    }
}

void CommandRegistry::StartCommandProcessor() {
    if (GetProcessorRunning()) {
        LogInfo("[CommandRegistry] Processor already running");
        return;
    }

    int numThreads = calc_thread_count(0.75);
    GetProcessorRunning() = true;

    auto& workers = GetWorkers();
    workers.clear();
    workers.reserve(numThreads);

    LogInfo("[CommandRegistry] Starting " + std::to_string(numThreads) + " worker threads");

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

    LogInfo("[CommandRegistry] Processor stopped");
}