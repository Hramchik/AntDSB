//
// Created by sanek on 01/12/2025.
//

#ifndef ANTDSB_COMMANDS_H
#define ANTDSB_COMMANDS_H


#include <dpp/dpp.h>
#include <unordered_map>
#include <functional>
#include <string>
#include "../utils/thread_pool.h"

#include "../DiscordBot/DiscordBot.h"

class commands {
public:
    using CommandHandler = std::function<void(const dpp::slashcommand_t&)>;

    commands();
    void registerSlashCommands(dpp::cluster *bot);
    void registerHandlers(dpp::cluster *bot);

private:
    std::unordered_map<std::string, CommandHandler> handlers_;

    struct CommandInfo {
        std::string name;
        std::string description;
        CommandHandler handler;
    };

    const std::vector<CommandInfo> getCommands();

    ThreadPool pool_;
};

#endif //ANTDSB_COMMANDS_H