//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_DISCORDBOT_H
#define ANTDSB_DISCORDBOT_H

#include <dpp/dpp.h>
#include <string>
#include <memory>
#include <thread>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <vector>

struct LoggedMessage {
    uint64_t id;
    uint64_t channel_id;
    uint64_t author_id;
    std::string author;
    std::string content;
    uint64_t timestamp; // unix seconds
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

    // Список текстовых каналов (id + имя)
    std::vector<std::pair<uint64_t, std::string>> ListTextChannels() const;

    // Последние limit сообщений канала (от старых к новым)
    std::vector<LoggedMessage> GetRecentMessages(uint64_t channel_id, uint32_t limit);

private:
    std::unique_ptr<dpp::cluster> cluster;
    std::thread botThread;
    bool running;

    mutable std::mutex messagesMutex;
    std::unordered_map<uint64_t, std::deque<LoggedMessage>> messagesByChannel;

    void RegisterEventHandlers();
    void BotThreadFunction();
};

#endif //ANTDSB_DISCORDBOT_H
