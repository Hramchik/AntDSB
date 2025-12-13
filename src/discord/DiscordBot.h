//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_DISCORDBOT_H
#define ANTDSB_DISCORDBOT_H

#include <dpp/dpp.h>
#include <string>
#include <memory>
#include <thread>
struct BotStatus {
    bool running = false;
    std::string last_error;
};

class DiscordBot {
public:
    explicit DiscordBot(const std::string& token);
    ~DiscordBot();

    bool Start();
    void StartAsync();
    void Stop();
    void Wait();

    dpp::cluster& GetCluster();

    BotStatus GetStatus() const;
    void SendMessage(dpp::snowflake channel_id, const std::string& text);

private:
    void BotThreadFunction();
    void RegisterEventHandlers();

    std::unique_ptr<dpp::cluster> cluster;
    std::thread botThread;
    std::atomic_bool running;
    mutable std::mutex status_mutex;
    BotStatus status;
};

#endif //ANTDSB_DISCORDBOT_H