//
// Created by sanek on 20/12/2025.
//

#include "DiscordLogging.h"
#include "logger/Logger.h"

#include <unordered_map>

std::mutex DiscordLogging::messagesMutex;
std::unordered_map<uint64_t, std::deque<LoggedMessage>> DiscordLogging::messagesByChannel;

namespace {
    std::unordered_map<dpp::snowflake, dpp::snowflake> g_last_voice_channel;
    std::unordered_map<dpp::snowflake, std::string>    g_channel_name_cache;
    std::unordered_map<dpp::snowflake, std::string>    g_username_cache;

    std::string get_username(dpp::cluster* cluster,
                             dpp::snowflake guild_id,
                             dpp::snowflake user_id)
    {
        auto it = g_username_cache.find(user_id);
        if (it != g_username_cache.end())
            return it->second;

        if (dpp::user* cached = dpp::find_user(user_id)) {
            std::string name = cached->username;
            g_username_cache.emplace(user_id, name);
            return name;
        }

        if (cluster) {
            cluster->guild_get_member(
                guild_id,
                user_id,
                [](const dpp::confirmation_callback_t&) {}
            );
        }

        return std::to_string(user_id);
    }
} // namespace

// ===== Общая утилита по имени канала =====

std::string DiscordLogging::GetChannelNameCached(dpp::snowflake id) {
    if (!id) return {};

    auto it = g_channel_name_cache.find(id);
    if (it != g_channel_name_cache.end())
        return it->second;

    if (auto* ch = dpp::find_channel(id)) {
        auto name = ch->name;
        g_channel_name_cache.emplace(id, name);
        return name;
    }

    return {};
}

// ===== Регистрация логирования сообщений =====

void DiscordLogging::RegisterMessageLoggingHandlers(dpp::cluster& cluster) {
    cluster.on_message_create([](const dpp::message_create_t& event) {
        if (event.msg.author.is_bot()) {
            return;
        }

        const auto cid = static_cast<uint64_t>(event.msg.channel_id);
        std::string channel_name = DiscordLogging::GetChannelNameCached(event.msg.channel_id);

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

// ===== Регистрация логирования войс-событий =====

void DiscordLogging::RegisterVoiceLoggingHandlers(dpp::cluster& cluster) {
    dpp::cluster* cl = &cluster;

    cluster.on_voice_state_update([cl](const dpp::voice_state_update_t& event) {
        const dpp::voicestate& vs = event.state;
        dpp::snowflake user_id = vs.user_id;
        dpp::snowflake ch_id   = vs.channel_id;
        dpp::snowflake prev_ch_id = 0;

        auto it_prev = g_last_voice_channel.find(user_id);
        if (it_prev != g_last_voice_channel.end())
            prev_ch_id = it_prev->second;

        if (ch_id) {
            g_last_voice_channel[user_id] = ch_id;
        } else if (it_prev != g_last_voice_channel.end()) {
            g_last_voice_channel.erase(it_prev);
        }

        std::string action;
        dpp::snowflake log_channel_id = 0;

        if (!prev_ch_id && ch_id) {
            action = "[voice_join] ";
            log_channel_id = ch_id;
        } else if (prev_ch_id && !ch_id) {
            action = "[voice_leave] ";
            log_channel_id = prev_ch_id;
        } else if (prev_ch_id && ch_id && prev_ch_id != ch_id) {
            action = "[voice_move] ";
            log_channel_id = ch_id;
        } else {
            return;
        }

        long long cid = static_cast<long long>(log_channel_id);
        std::string channel_name = DiscordLogging::GetChannelNameCached(log_channel_id);
        std::string username     = get_username(cl, vs.guild_id, user_id);

        LogChannel(cid, channel_name, action + username);
    });
}

// ===== Запросы к кэшу =====

std::vector<std::pair<uint64_t, std::string>>
DiscordLogging::ListTextChannels() {
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
DiscordLogging::GetRecentMessages(uint64_t channel_id, uint32_t limit) {
    std::lock_guard<std::mutex> lg(messagesMutex);
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
