//
// Created by sanek on 05/12/2025.
//

#include <iostream>
#include <thread>

#include "DiscordBot.h"
#include "DiscordCluster.h"
#include "CommandRegistry.h"
#include "CallbackRegistry.h"
#include "BuiltInCommands.h"
#include "callbacks/BuiltInCallbacks.h"
#include "utils/ThreadUtils.h"
#include "logger/Logger.h"

namespace {
    std::unordered_map<dpp::snowflake, dpp::snowflake> g_last_voice_channel;
    std::unordered_map<dpp::snowflake, std::string>    g_channel_name_cache;
    std::unordered_map<dpp::snowflake, std::string>    g_username_cache;

    std::string get_channel_name(dpp::snowflake id) {
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

    std::string get_username(dpp::cluster* cluster,
                             dpp::snowflake guild_id,
                             dpp::snowflake user_id) {
        auto it = g_username_cache.find(user_id);
        if (it != g_username_cache.end())
            return it->second;

        if (dpp::user* cached = dpp::find_user(user_id)) {
            std::string name = cached->username;
            g_username_cache.emplace(user_id, name);
            return name;
        }

        if (cluster) {
            cluster->guild_get_member(guild_id, user_id,
                [](const dpp::confirmation_callback_t&){});
        }

        return std::to_string(user_id);
    }
}

DiscordBot::DiscordBot(const std::string& token) : running(false) {
    try {
        unsigned int hw = std::thread::hardware_concurrency();
        if (hw == 0) hw = 4;
        LogDebug("[DiscordBot] HW threads: " + std::to_string(hw));

        uint32_t intents = dpp::i_default_intents
                        | dpp::i_message_content
                        | dpp::i_direct_messages
                        | dpp::i_guild_voice_states;

        cluster = std::make_unique<dpp::cluster>(token, intents);
        DiscordCluster::SetCluster(cluster.get());

        LogInfo("[DiscordBot] Initialized successfully");
    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error initializing: " + except);
    }
}

DiscordBot::~DiscordBot() {
    Stop();
}

bool DiscordBot::Start() {
    try {
        if (!cluster) {
            LogError("[DiscordBot] Cluster is not initialized");
            return false;
        }

        running = true;
        RegisterEventHandlers();
        cluster->start(dpp::st_wait);

        LogInfo("[DiscordBot] Started (sync)");
        return true;

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error starting: " + except);
        return false;
    }
}

void DiscordBot::StartAsync() {
    if (running) {
        LogInfo("[DiscordBot] Already running");
        return;
    }

    running = true;
    botThread = std::thread(&DiscordBot::BotThreadFunction, this);
    std::ostringstream oss;
    oss << botThread.get_id();
    LogInfo("[DiscordBot] Started async (thread: " + oss.str() + ")");
}

void DiscordBot::BotThreadFunction() {
    try {
        if (!cluster) {
            LogError("[DiscordBot] Cluster is not initialized");
            return;
        }

        RegisterEventHandlers();
        cluster->start(dpp::st_wait);

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error in bot thread: " + except);
    }
}

void DiscordBot::Wait() {
    if (botThread.joinable()) {
        botThread.join();
        LogInfo("[DiscordBot] Bot thread finished");
    }
}

void DiscordBot::Stop() {
    if (!running) {
        return;
    }

    running = false;
    CommandRegistry::StopCommandProcessor();

    if (cluster) {
        cluster->shutdown();
        LogInfo("[DiscordBot] Stopped");
    }

    if (botThread.joinable()) {
        botThread.join();
    }
}

dpp::cluster& DiscordBot::GetCluster() {
    return *cluster;
}

void DiscordBot::RegisterEventHandlers() {
    try {
        BuiltInCommands::SetCluster(cluster.get());

        BuiltInCommands::RegisterAll();

        BuiltInCallbacks::RegisterAll();

        CommandRegistry::StartCommandProcessor();

        cluster->on_ready([this](const dpp::ready_t& event) {
            if (dpp::run_once<struct register_bot_commands>()) {
                std::ostringstream oss;
                oss << std::this_thread::get_id();
                LogInfo("[DiscordBot] Bot is ready (thread: "
                          + oss.str() + ")");
                CommandRegistry::RegisterAllCommands(cluster.get());
            }
        });

        cluster->on_slashcommand([this](const dpp::slashcommand_t& event) {
            CommandRegistry::HandleCommand(event, cluster.get());
        });


        cluster->on_button_click([this](const dpp::button_click_t& event) {
            LogInfo("[DiscordBot] Button clicked: " + event.custom_id);
                CallbackRegistry::HandleButtonClick(event);
        });

        cluster->on_form_submit([this](const dpp::form_submit_t& event) {
            LogInfo("[DiscordBot] Form submitted: " + event.custom_id);
                CallbackRegistry::HandleFormSubmit(event);
        });

        cluster->on_message_create([this](const dpp::message_create_t& event) {
            if (event.msg.author.is_bot()) {
                return;
            }

            const auto cid = static_cast<long long>(event.msg.channel_id);
            std::string channel_name = get_channel_name(event.msg.channel_id);
            std::string text = event.msg.author.username + ": " + event.msg.content;

            LogChannel(cid, channel_name, text);
        });

        cluster->on_guild_member_add([this](const dpp::guild_member_add_t& event) {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            LogInfo("[DiscordBot][" + oss.str() + "] " + "New member joined");
        });

        cluster->on_voice_state_update([this](const dpp::voice_state_update_t& event) {
            dpp::cluster* cl = cluster.get();
            const dpp::voicestate& vs = event.state;
            dpp::snowflake user_id    = vs.user_id;
            dpp::snowflake ch_id      = vs.channel_id;

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
            std::string channel_name = get_channel_name(log_channel_id);
            std::string username     = get_username(cl, vs.guild_id, user_id);

            LogChannel(cid, channel_name, action + username);
        });

        std::ostringstream oss;
        oss << std::this_thread::get_id();
        LogInfo("[DiscordBot] Handlers registered (thread: " + oss.str() + ")");

    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error registering handlers: " + except);
    }
}