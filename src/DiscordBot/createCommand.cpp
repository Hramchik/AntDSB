//
// Created by sanek on 01/12/2025.
//

#include "DiscordBot.h"

void DiscordBot::createCommand(const std::string &command, std::function<void(const dpp::slashcommand_t &)> handler) {
    if (!botInstance) {
        std::cerr << "Bot command not initialized" << std::endl;
        return;
    }

    botInstance->on_slashcommand([command, handler](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == command) {
            handler(event);
        }
    });
}
