//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_COMMANDREGISTRY_H
#define ANTDSB_COMMANDREGISTRY_H

#include <dpp/dpp.h>
#include <string>
#include <map>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class CommandRegistry {
public:
    struct CommandInfo {
        std::string name;
        std::string description;
        std::function<void(const dpp::slashcommand_t&)> callback;
    };

    static void RegisterCommand(const CommandInfo& cmd);
    static void RegisterAllCommands(dpp::cluster* cluster);
    static void HandleCommand(const dpp::slashcommand_t& event, dpp::cluster* cluster);

    static void StartCommandProcessor();
    static void StopCommandProcessor();

private:
    static std::map<std::string, CommandInfo>& GetCommands();

    struct PendingCommand {
        std::string name;
        dpp::slashcommand_t event;
    };

    static std::queue<PendingCommand>& GetCommandQueue();
    static std::mutex& GetQueueMutex();
    static std::condition_variable& GetQueueCV();
    static bool& GetProcessorRunning();
    static std::vector<std::thread>& GetWorkers();

    static void CommandWorker();
};

#endif //ANTDSB_COMMANDREGISTRY_H