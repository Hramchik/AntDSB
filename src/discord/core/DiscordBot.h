//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_DISCORDBOT_H
#define ANTDSB_DISCORDBOT_H

#include <string>
#include <thread>
#include <memory>

#include <dpp/dpp.h>

class DiscordBot {
public:
    explicit DiscordBot(const std::string& token);
    ~DiscordBot();

    bool Start();
    void StartAsync();
    void Stop();
    void Wait();

    dpp::cluster& GetCluster();

private:
    std::unique_ptr<dpp::cluster> cluster;
    std::thread botThread;
    bool running{false};

    void RegisterEventHandlers();
    void RegisterCoreHandlers();
    void BotThreadFunction();
};

#endif // ANTDSB_DISCORDBOT_H
