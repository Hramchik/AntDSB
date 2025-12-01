//
// Created by sanek on 01/12/2025.
//

#include <iostream>
#include "DiscordBot.h"
#include "../commands/commands.h"
#include "../utils/thread_pool.h"

commands cmd_manager;
dpp::cluster* botInstance = nullptr;
std::unique_ptr<ThreadPool> threadPool = nullptr;

void DiscordBot::botInit(const std::string& token) {
    threadPool = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());

    botInstance = new dpp::cluster(token);

    botInstance->on_log(dpp::utility::cout_logger());

    botInstance->on_ready([](const dpp::ready_t& event) {
        if (dpp::run_once<struct bot_ready>()) {
            cmd_manager.registerSlashCommands(botInstance);
            cmd_manager.registerHandlers(botInstance);
            std::cout << "Bot ready! ThreadPool size: " << threadPool->size() << std::endl;
        }
    });

    botInstance->start(dpp::st_wait);
}