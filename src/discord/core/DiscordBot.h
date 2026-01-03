//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_DISCORDBOT_H
#define ANTDSB_DISCORDBOT_H


#include <memory>
#include <thread>
#include <atomic>

#include <dpp/dpp.h>

class DiscordBot {
public:
    explicit DiscordBot(const std::string& token);
    ~DiscordBot();

    bool Start();
    void StartAsync();
    void Wait();
    void Stop();

    dpp::cluster& GetCluster();

private:
    void BotThreadFunction();
    void RegisterEventHandlers();
    void RegisterCoreHandlers();

    void RegisterTempVCHandlers(); // NEW

    std::unique_ptr<dpp::cluster> cluster;
    std::thread botThread;
    std::atomic<bool> running;
};

#endif // ANTDSB_DISCORDBOT_H
