//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_DISCORDBOT_H
#define ANTDSB_DISCORDBOT_H

#include <dpp/dpp.h>
#include <string>
#include <memory>
#include <thread>

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
    bool running;

    void RegisterEventHandlers();
    void BotThreadFunction();
};

#endif //ANTDSB_DISCORDBOT_H