//
// Created by sanek on 05/12/2025.
//

#include <dpp/dpp.h>
#include <string>

#include "BuiltInCommands.h"
#include "CommandRegistry.h"
#include "logger/Logger.h"
#include "../ui/ApplicationUI.h"
#include "discord/tempvc/TempVC.h"  // NEW

static dpp::cluster* g_cluster = nullptr;

void BuiltInCommands::SetCluster(dpp::cluster* cluster) {
    g_cluster = cluster;
}

void BuiltInCommands::RegisterAll() {
    // уже существующая команда post-application
    CommandRegistry::RegisterCommand({
        "post-application",
        "Отправить сообщение с заявкой в канал",
        [](const dpp::slashcommand_t& event) {
            try {
                if (!g_cluster) {
                    event.reply("❌ Кластер не инициализирован");
                    return;
                }

                dpp::message msg_to_send = ApplicationUI::CreateApplicationEntryMessageV2();
                msg_to_send.channel_id = event.command.channel_id;

                LogInfo("[post-application] Sending v2 message to channel " +
                        std::to_string(event.command.channel_id));

                event.reply("✅ Сообщение отправлено");

                g_cluster->message_create(
                    msg_to_send,
                    [](const dpp::confirmation_callback_t& callback) {
                        if (callback.is_error()) {
                            LogError("[post-application] ERROR: " + callback.get_error().human_readable);
                        }
                    }
                );
            } catch (const std::exception& e) {
                LogError(std::string("[post-application] EXCEPTION: ") + e.what());
            }
        }
    });

    // NEW: команда createhub
    CommandRegistry::RegisterCommand({
        "createhub",
        "Создать канал-хаб для временных голосовых каналов",
        [](const dpp::slashcommand_t& event) {
            try {
                if (!g_cluster) {
                    event.reply(dpp::message("❌ Кластер не инициализирован").set_flags(dpp::m_ephemeral));
                    return;
                }

                auto& state = GetTempVCState();

                event.thinking(true);

                auto now = std::chrono::steady_clock::now();
                dpp::snowflake user_id = event.command.get_issuing_user().id;

                auto it_cd = state.last_created.find(user_id);
                if (it_cd != state.last_created.end()) {
                    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - it_cd->second).count();
                    if (diff < CREATE_CD_SEC) {
                        event.edit_original_response(
                            dpp::message()
                                .set_content(
                                    "Подождите ещё " +
                                    std::to_string(CREATE_CD_SEC - diff) +
                                    " секунд перед созданием нового хаба."
                                )
                        );
                        return;
                    }
                }

                dpp::guild* g = dpp::find_guild(event.command.guild_id);
                if (!g) {
                    event.edit_original_response(dpp::message("Не удалось найти сервер."));
                    return;
                }

                dpp::channel ch;
                ch.set_name("➕ Создать комнату")
                  .set_guild_id(g->id)
                  .set_type(dpp::CHANNEL_VOICE)
                  .set_user_limit(HUB_USER_LIMIT);

                g_cluster->channel_create(
                    ch,
                    [now, event](const dpp::confirmation_callback_t& cb) mutable {
                        auto& state = GetTempVCState();

                        if (cb.is_error()) {
                            event.edit_original_response(
                                dpp::message("Ошибка создания хаба: " + cb.get_error().message)
                            );
                            return;
                        }

                        auto created = std::get<dpp::channel>(cb.value);
                        state.hub_channels[created.id] = true;
                        state.last_created[event.command.get_issuing_user().id] = now;

                        event.edit_original_response(
                            dpp::message(
                                "Канал-хаб создан: <#" +
                                std::to_string(static_cast<uint64_t>(created.id)) +
                                ">"
                            )
                        );
                    }
                );
            } catch (const std::exception& e) {
                LogError(std::string("[createhub] EXCEPTION: ") + e.what());
                event.reply(
                    dpp::message("Произошла ошибка при создании хаба").set_flags(dpp::m_ephemeral)
                );
            }
        }
    });
}

