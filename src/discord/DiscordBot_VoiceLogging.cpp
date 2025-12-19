//
// Created by sanek on 20/12/2025.
//
#include "DiscordBot.h"
#include "logger/Logger.h"

#include <unordered_map>
#include <dpp/dpp.h>

namespace {
    std::unordered_map<dpp::snowflake, dpp::snowflake> g_last_voice_channel;
    std::unordered_map<dpp::snowflake, std::string>    g_channel_name_cache;
    std::unordered_map<dpp::snowflake, std::string>    g_username_cache;
}

// Глобальная функция, объявлена в DiscordBot.h
std::string GetChannelNameCached(dpp::snowflake id) {
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

namespace {
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

void DiscordBot::RegisterVoiceLoggingHandlers() {
    dpp::cluster* cl = cluster.get();

    cluster->on_voice_state_update([cl](const dpp::voice_state_update_t& event) {
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
        std::string channel_name = GetChannelNameCached(log_channel_id);
        std::string username     = get_username(cl, vs.guild_id, user_id);

        LogChannel(cid, channel_name, action + username);
    });
}
