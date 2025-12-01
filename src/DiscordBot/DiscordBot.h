//
// Created by sanek on 01/12/2025.
//

#ifndef ANTDSB_DISCORDBOT_H
#define ANTDSB_DISCORDBOT_H

#include <dpp/dpp.h>
#include <string>
#include <memory>
#include <functional>

class ThreadPool;

extern dpp::cluster* botInstance;
extern std::unique_ptr<ThreadPool> threadPool;

class DiscordBot {
public:
    static void createCommand(const std::string& command, std::function<void(const dpp::slashcommand_t&)> handler);
    static void botInit(const std::string& token);
    static ThreadPool& getThreadPool() { return *threadPool; }
    static dpp::cluster* getBotInstance() { return botInstance; }
};

#endif //ANTDSB_DISCORDBOT_H