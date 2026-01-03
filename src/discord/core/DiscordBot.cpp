//
// Created by sanek on 05/12/2025.
//

#include "DiscordBot.h"

#include <sstream>
#include <thread>

#include "discord/core/DiscordCluster.h"
#include "discord/commands/CommandRegistry.h"
#include "discord/callbacks/CallbackRegistry.h"
#include "discord/commands/BuiltInCommands.h"
#include "discord/callbacks/BuiltInCallbacks.h"
#include "discord/logging/DiscordLogging.h"
#include "discord/tempvc/TempVC.h"   // NEW

#include "logger/Logger.h"

DiscordBot::DiscordBot(const std::string& token)
    : running(false)
{
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

        RegisterCoreHandlers();
        DiscordLogging::RegisterMessageLoggingHandlers(*cluster);
        DiscordLogging::RegisterVoiceLoggingHandlers(*cluster);

        std::ostringstream oss;
        oss << std::this_thread::get_id();
        LogInfo("[DiscordBot] Handlers registered (thread: " + oss.str() + ")");
    } catch (const std::exception& e) {
        std::string except = e.what();
        LogError("[DiscordBot] Error registering handlers: " + except);
    }
}

void DiscordBot::RegisterCoreHandlers() {
    cluster->on_ready([this](const dpp::ready_t&) {
        if (dpp::run_once<struct register_commands_tag>()) {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            LogInfo("[DiscordBot] Bot is ready (thread: " + oss.str() + ")");
            CommandRegistry::RegisterAllCommands(cluster.get());
        }
    });

    cluster->on_slashcommand([this](const dpp::slashcommand_t& event) {
        CommandRegistry::HandleCommand(event, cluster.get());
    });

    cluster->on_button_click([this](const dpp::button_click_t& event) {
        LogInfo(std::string("[DiscordBot] Button clicked: ") + event.custom_id);
        CallbackRegistry::HandleButtonClick(event);
    });

    cluster->on_form_submit([this](const dpp::form_submit_t& event) {
        LogInfo(std::string("[DiscordBot] Form submitted: ") + event.custom_id);
        CallbackRegistry::HandleFormSubmit(event);
    });

    cluster->on_guild_member_add([this](const dpp::guild_member_add_t&) {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        LogInfo("[DiscordBot][" + oss.str() + "] New member joined");
    });

    // ====== NEW: TempVC ======

    // авто-инициализация хабов на guild_create
    cluster->on_guild_create([this](const dpp::guild_create_t& ev) {
        auto& state = GetTempVCState();

        dpp::guild* g = dpp::find_guild(ev.created.id);
        if (!g) {
            LogError("[TempVC] Auto-init hubs: guild not in cache on guild_create");
            return;
        }

        state.hub_channels.clear();

        for (dpp::snowflake cid : g->channels) {
            dpp::channel* ch = dpp::find_channel(cid);
            if (!ch)
                continue;

            if (ch->get_type() == dpp::CHANNEL_VOICE &&
                ch->name == "➕ Создать комнату") {
                state.hub_channels[ch->id] = true;
            }
        }

        LogInfo("[TempVC] Auto-init hubs: " + std::to_string(state.hub_channels.size()) + " hub(s) loaded");
    });

    // voice_state_update
    cluster->on_voice_state_update([this](const dpp::voice_state_update_t& ev) {
        auto& state = GetTempVCState();

        const auto& vs = ev.state;
        dpp::snowflake user_id  = vs.user_id;
        dpp::snowflake guild_id = vs.guild_id;

        dpp::snowflake new_channel = vs.channel_id;
        dpp::snowflake old_channel = 0;

        auto it_prev = state.last_voice_channel.find(user_id);
        if (it_prev != state.last_voice_channel.end())
            old_channel = it_prev->second;

        if (new_channel)
            state.last_voice_channel[user_id] = new_channel;
        else
            state.last_voice_channel.erase(user_id);

        // зашёл в хаб
        if (new_channel && state.hub_channels.contains(new_channel) && new_channel != old_channel) {
            dpp::guild* g = dpp::find_guild(guild_id);
            if (!g) return;

            dpp::channel* hub_ch = dpp::find_channel(new_channel);
            if (!hub_ch) return;

            std::string vc_name = TempVC_GetDisplayName(guild_id, user_id);

            dpp::channel vc;
            vc.set_name(vc_name)
              .set_guild_id(guild_id)
              .set_type(dpp::CHANNEL_VOICE)
              .set_user_limit(TEMP_VC_USER_LIMIT_DEFAULT)
              .set_parent_id(hub_ch->parent_id);

            cluster->channel_create(
                vc,
                [user_id, guild_id, new_channel](const dpp::confirmation_callback_t& cb) {
                    auto& state = GetTempVCState();

                    if (cb.is_error()) {
                        LogError(std::string("[TempVC] Failed to create temp VC: ") + cb.get_error().message);
                        return;
                    }

                    auto created_vc = std::get<dpp::channel>(cb.value);
                    temp_vc_info info{ user_id, new_channel, std::nullopt };
                    state.temp_channels[created_vc.id] = info;

                    // перенос участника
                    DiscordCluster::GetCluster()->guild_member_move(
                        created_vc.id,
                        guild_id,
                        user_id,
                        [user_id, created_vc](const dpp::confirmation_callback_t& cb2) {
                            if (cb2.is_error()) {
                                LogError(
                                    "[TempVC] Move failed for user " +
                                    std::to_string(static_cast<uint64_t>(user_id)) +
                                    " to channel " +
                                    std::to_string(static_cast<uint64_t>(created_vc.id)) +
                                    ": " + cb2.get_error().message
                                );
                            }
                        }
                    );

                    // панель управления в голосовом канале
                    dpp::message m;
                    m.set_channel_id(created_vc.id);

                    m.set_content(
                        "Настройки голосового канала для <#" +
                        std::to_string(static_cast<uint64_t>(created_vc.id)) +
                        ">:\n"
                        "• Кол-во участников\n"
                        "• Переименовать\n"
                        "• Видимость\n"
                        "• Права доступа"
                    );

                    dpp::component row;
                    row.set_type(dpp::cot_action_row);

                    row.add_component(
                        dpp::component()
                            .set_type(dpp::cot_button)
                            .set_style(dpp::cos_primary)
                            .set_label("Участники")
                            .set_id("vc_cfg_users")
                    );

                    row.add_component(
                        dpp::component()
                            .set_type(dpp::cot_button)
                            .set_style(dpp::cos_secondary)
                            .set_label("Переименовать")
                            .set_id("vc_cfg_rename")
                    );

                    row.add_component(
                        dpp::component()
                            .set_type(dpp::cot_button)
                            .set_style(dpp::cos_secondary)
                            .set_label("Видимость")
                            .set_id("vc_cfg_visibility")
                    );

                    row.add_component(
                        dpp::component()
                            .set_type(dpp::cot_button)
                            .set_style(dpp::cos_secondary)
                            .set_label("Права")
                            .set_id("vc_cfg_perms")
                    );

                    m.add_component(row);
                    DiscordCluster::GetCluster()->message_create(m);
                }
            );
        }

        auto handle_leave = [&](dpp::snowflake ch_id) {
            auto it = state.temp_channels.find(ch_id);
            if (it == state.temp_channels.end())
                return;

            auto& info = it->second;

            dpp::channel* ch = dpp::find_channel(ch_id);
            if (!ch)
                return;

            std::vector<dpp::snowflake> members;
            auto vmap = ch->get_voice_members();
            for (auto vm_it = vmap.begin(); vm_it != vmap.end(); ++vm_it)
                members.push_back(vm_it->first);

            if (members.empty()) {
                DiscordCluster::GetCluster()->channel_delete(ch_id);
                state.temp_channels.erase(it);
                return;
            }

            if (user_id == info.owner_id)
                info.owner_left_at = std::chrono::steady_clock::now();
        };

        if (old_channel && old_channel != new_channel)
            handle_leave(old_channel);

        if (new_channel) {
            auto it = state.temp_channels.find(new_channel);
            if (it != state.temp_channels.end() && user_id == it->second.owner_id)
                it->second.owner_left_at.reset();
        }
    });

    // таймер
    cluster->start_timer([this](dpp::timer) {
        auto& state = GetTempVCState();
        auto now = std::chrono::steady_clock::now();

        for (auto it = state.temp_channels.begin(); it != state.temp_channels.end(); ) {
            dpp::snowflake vc_id = it->first;
            auto& info           = it->second;

            dpp::channel* ch = dpp::find_channel(vc_id);
            if (!ch) {
                it = state.temp_channels.erase(it);
                continue;
            }

            std::vector<dpp::snowflake> members;
            auto vmap = ch->get_voice_members();
            for (auto vm_it = vmap.begin(); vm_it != vmap.end(); ++vm_it)
                members.push_back(vm_it->first);

            if (members.empty()) {
                DiscordCluster::GetCluster()->channel_delete(vc_id);
                it = state.temp_channels.erase(it);
                continue;
            }

            if (info.owner_left_at) {
                auto diff = std::chrono::duration_cast<std::chrono::seconds>(
                    now - *info.owner_left_at
                ).count();

                if (diff >= OWNER_ABSENCE_TIMEOUT_SEC) {
                    std::uniform_int_distribution<size_t> dist(0, members.size() - 1);
                    dpp::snowflake new_owner = members[dist(state.rng)];
                    info.owner_id = new_owner;
                    info.owner_left_at.reset();

                    dpp::guild* g = dpp::find_guild(ch->guild_id);
                    if (g) {
                        dpp::snowflake text_ch = 0;
                        const std::vector<dpp::snowflake>& chans = g->channels;
                        for (dpp::snowflake cid : chans) {
                            dpp::channel* ch2 = dpp::find_channel(cid);
                            if (ch2 && ch2->is_text_channel()) {
                                text_ch = cid;
                                break;
                            }
                        }
                        if (text_ch) {
                            dpp::message m;
                            m.set_channel_id(text_ch);
                            m.set_content(
                                "Владельцем временного канала `" +
                                std::to_string(static_cast<uint64_t>(vc_id)) +
                                "` теперь является <@" +
                                std::to_string(static_cast<uint64_t>(new_owner)) +
                                ">."
                            );
                            DiscordCluster::GetCluster()->message_create(m);
                        }
                    }
                }
            }

            ++it;
        }
    }, 10);
}

