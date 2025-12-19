//
// Created by sanek on 20/12/2025.
//

#include "DiscordBot.h"
#include <dpp/dpp.h>

std::vector<std::pair<uint64_t, std::string>>
DiscordBot::ListTextChannels() const {
    std::vector<std::pair<uint64_t, std::string>> result;

    auto* cache = dpp::get_channel_cache();
    if (!cache) {
        return result;
    }

    for (const auto& [id, ch] : cache->get_container()) {
        if (!ch) continue;
        if (ch->is_text_channel()) {
            result.emplace_back(static_cast<uint64_t>(id), ch->name);
        }
    }

    return result;
}

std::vector<LoggedMessage>
DiscordBot::GetRecentMessages(uint64_t channel_id, uint32_t limit) {
    std::lock_guard lg(messagesMutex);
    std::vector<LoggedMessage> out;

    auto it = messagesByChannel.find(channel_id);
    if (it == messagesByChannel.end())
        return out;

    auto& dq = it->second;
    if (dq.empty())
        return out;

    if (limit == 0 || limit > dq.size())
        limit = static_cast<uint32_t>(dq.size());

    out.reserve(limit);
    auto start = dq.end();
    std::advance(start, -static_cast<long>(limit));

    for (auto it2 = start; it2 != dq.end(); ++it2)
        out.push_back(*it2);

    return out;
}