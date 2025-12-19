//
// Created by sanek on 20/12/2025.
//

#ifndef ANTDSB_DISCORDLOGGING_H
#define ANTDSB_DISCORDLOGGING_H

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <mutex>

#include <dpp/dpp.h>

struct LoggedMessage {
    uint64_t id;
    uint64_t channel_id;
    uint64_t author_id;
    std::string author;
    std::string content;
    uint64_t timestamp;
};

class DiscordLogging {
public:
    static void RegisterMessageLoggingHandlers(dpp::cluster& cluster);
    static void RegisterVoiceLoggingHandlers(dpp::cluster& cluster);

    static std::string GetChannelNameCached(dpp::snowflake id);

    static std::vector<std::pair<uint64_t, std::string>> ListTextChannels();
    static std::vector<LoggedMessage> GetRecentMessages(uint64_t channel_id, uint32_t limit);

private:
    static std::mutex messagesMutex;
    static std::unordered_map<uint64_t, std::deque<LoggedMessage>> messagesByChannel;
};

#endif //ANTDSB_DISCORDLOGGING_H