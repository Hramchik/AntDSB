//
// Created by sanek on 01/12/2025.
//

#include "commands.h"
#include <iostream>

commands::commands()
    : pool_(std::thread::hardware_concurrency())
{}

const std::vector<commands::CommandInfo> commands::getCommands() {
    static const std::vector<commands::CommandInfo> commands = {
        {"help", "Heheheh", [](const dpp::slashcommand_t& event) {
            event.reply("Hui");
        }},
        {"ping", "Ping", [](const dpp::slashcommand_t& event) {
            event.reply("Обожаю такой кол");
        }}
    };
    return commands;
}

void commands::registerSlashCommands(dpp::cluster *bot) {
    if (!bot) {
        std::cerr << "Bot instance not ready" << std::endl;
        return;
    }

    if (dpp::run_once<struct register_all_commands>()) {
        std::vector<dpp::slashcommand> slash_commands;
        for (const auto& cmd : getCommands()) {
            slash_commands.emplace_back(cmd.name, cmd.description, bot->me.id);
        }

        bot->global_bulk_command_create(slash_commands);
        std::cout << "Registered " << slash_commands.size() << " slash commands" << std::endl;
    }
}

void commands::registerHandlers(dpp::cluster *bot) {
    if (!bot) return;

    handlers_.clear();

    for (const auto& cmd : getCommands()) {
        handlers_[cmd.name] = cmd.handler;
    }

    bot->on_slashcommand([this](const dpp::slashcommand_t& event) {
        std::string cmd_name = event.command.get_command_name();
        auto it = handlers_.find(cmd_name);

        if (it != handlers_.end()) {
            DiscordBot::getThreadPool().enqueue([handler = it->second, event]() {
                handler(event);
            });
        } else {
            event.reply("Неизвестная команда: " + cmd_name);
        }
    });
}