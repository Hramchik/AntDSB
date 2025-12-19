//
// Created by sanek on 20/12/2025.
//
#include "../core/DiscordBot.h"
#include "logger/Logger.h"

#include <dpp/dpp.h>

void DiscordBot::RegisterMessageLoggingHandlers() {
    cluster->on_message_create([this](const dpp::message_create_t& event) {
        if (event.msg.author.is_bot()) {
            return;
        }

        const auto cid = static_cast<uint64_t>(event.msg.channel_id);
        std::string channel_name = GetChannelNameCached(event.msg.channel_id);

        std::string text = event.msg.author.username + ": " + event.msg.content;
        LogChannel(static_cast<long long>(cid), channel_name, text);

        LoggedMessage m;
        m.id         = static_cast<uint64_t>(event.msg.id);
        m.channel_id = cid;
        m.author_id  = static_cast<uint64_t>(event.msg.author.id);
        m.author     = event.msg.author.username;
        m.content    = event.msg.content;
        m.timestamp  = static_cast<uint64_t>(event.msg.sent);

        std::lock_guard<std::mutex> lg(messagesMutex);
        auto& dq = messagesByChannel[cid];
        dq.push_back(std::move(m));

        constexpr size_t MAX_PER_CHANNEL = 500;
        if (dq.size() > MAX_PER_CHANNEL) {
            dq.pop_front();
        }
    });
}
