//
// Created by sanek on 06/12/2025.
//

#include <iostream>
#include <variant>
#include <dpp/dpp.h>

#include "BuiltInCallbacks.h"
#include "config/ConfigManager.h"
#include "CallbackRegistry.h"
#include "discord/ui/ApplicationUI.h"
#include "../core/DiscordCluster.h"
#include "logger/Logger.h"

namespace {
    void HandleTicketDecision(const dpp::button_click_t& event, bool approved) {
        dpp::cluster* cluster = DiscordCluster::GetCluster();
        if (!cluster) {
            LogError("[BuiltInCallbacks] Cluster is null in HandleTicketDecision");
            return;
        }

        const std::string& cid = event.custom_id;
        auto pos = cid.find(':');
        if (pos == std::string::npos) {
            LogError("[BuiltInCallbacks] Invalid ticket button custom_id: " + cid);
            return;
        }

        std::string user_id_str = cid.substr(pos + 1);
        dpp::snowflake target_user_id = 0;
        try {
            target_user_id = static_cast<dpp::snowflake>(std::stoull(user_id_str));
        } catch (...) {
            LogError("[BuiltInCallbacks] Failed to parse user_id from custom_id: " + cid);
            return;
        }

        std::string dm_text = approved
            ? "Ваша заявка одобрена! Добро пожаловать."
            : "К сожалению, ваша заявка отклонена.";

        dpp::snowflake channel_id = event.command.channel_id;
        dpp::snowflake message_id = event.command.message_id;

        // DM пользователю
        cluster->direct_message_create(target_user_id, dpp::message(dm_text));

        // Ответ модератору (ephemeral)
        event.reply(
            dpp::message(
                approved
                    ? "Заявка одобрена. Канал будет удалён через 10 секунд."
                    : "Заявка отклонена. Канал будет удалён через 10 секунд."
            ).set_flags(dpp::m_ephemeral)
        );

        // Собираем обновлённый embed со статусом
        dpp::embed ticket_embed;
        ticket_embed.set_color(approved ? 0x2ECC71 : 0xE74C3C);
        ticket_embed.set_title("Статус заявки");

        ticket_embed.add_field(
            "Статус",
            approved ? "Заявка одобрена модератором." : "Заявка отклонена модератором.",
            false
        );

        ticket_embed.set_timestamp(std::time(nullptr));

        // Редактируем существующее сообщение: убираем кнопки и ставим новый embed
        dpp::message updated_msg;
        updated_msg.id = message_id;
        updated_msg.channel_id = channel_id;
        updated_msg.components.clear();      // убрать все компоненты (кнопки)
        updated_msg.embeds.clear();         // на всякий случай очистить старые embed'ы
        updated_msg.add_embed(ticket_embed); // твой уже собранный embed

        DiscordCluster::GetCluster()->message_edit(updated_msg);

        // Таймер на удаление канала
        cluster->start_timer(
            [cluster, channel_id](dpp::timer t) {
                cluster->channel_delete(channel_id);
                cluster->stop_timer(t);
            },
            10
        );
    }
} // namespace

void BuiltInCallbacks::RegisterAll() {

    CallbackRegistry::RegisterButtonCallback({
        "btn_apply_modal",
        [](const dpp::button_click_t& event) {
            LogDebug("[BuiltInCallbacks] Button clicked, creating modal...");

            try {
                dpp::interaction_modal_response modal = ApplicationUI::CreateApplicationModal();

                LogDebug("[BuiltInCallbacks] Modal created successfully");
                LogDebug("[BuiltInCallbacks] Modal custom_id: " + modal.custom_id);
                LogDebug("[BuiltInCallbacks] Modal title: " + modal.title);
                LogDebug("[BuiltInCallbacks] Modal components: " +
                         std::to_string(modal.components.size()) + " rows");

                event.dialog(modal);
                LogDebug("[BuiltInCallbacks] Dialog sent successfully!");

            } catch (const std::exception& e) {
                LogError(std::string("[BuiltInCallbacks] EXCEPTION: ") + e.what());
            } catch (...) {
                LogError("[BuiltInCallbacks] UNKNOWN EXCEPTION");
            }
        }
    });

    CallbackRegistry::RegisterButtonCallback({
        "ticket_approve",
        [](const dpp::button_click_t& event) {
            HandleTicketDecision(event, true);
        }
    });

    CallbackRegistry::RegisterButtonCallback({
        "ticket_reject",
        [](const dpp::button_click_t& event) {
            HandleTicketDecision(event, false);
        }
    });

    CallbackRegistry::RegisterFormCallback({
        "modal_apply",
        [](const dpp::form_submit_t& event) {
            LogDebug("[BuiltInCallbacks] Application form submitted");

            const auto& c = event.components;
            std::string name     = std::get<std::string>(c[0].components[0].value);
            std::string age_str  = std::get<std::string>(c[1].components[0].value);
            std::string nickname = std::get<std::string>(c[2].components[0].value);
            std::string about    = std::get<std::string>(c[3].components[0].value);
            std::string source   = std::get<std::string>(c[4].components[0].value);

            int age = 0;
            try { age = std::stoi(age_str); } catch (...) { age = 0; }

            LogDebug(
                std::string("[BuiltInCallbacks] Application details:\n") +
                "  Name: " + name + "\n" +
                "  Age: " + std::to_string(age) + "\n" +
                "  Nickname: " + nickname + "\n" +
                "  About: " + about + "\n" +
                "  Source: " + source
            );

            dpp::cluster* cluster = DiscordCluster::GetCluster();
            if (!cluster) {
                LogError("[BuiltInCallbacks] Cluster is null in form callback");
                return;
            }

            dpp::snowflake guild_id = event.command.guild_id;
            dpp::snowflake user_id  = event.command.get_issuing_user().id;

            std::string display_name =
                !event.command.usr.global_name.empty()
                    ? event.command.usr.global_name
                    : event.command.usr.username;
            std::string avatar_url = event.command.usr.get_avatar_url();

            const std::string config_path = "config/config.yml";
            uint64_t ticket_cat_raw =
                ConfigManager::ReadConfigValue<uint64_t>(
                    config_path,
                    "channels.applications_category_id",
                    0
                );
            uint64_t apps_role_raw =
                ConfigManager::ReadConfigValue<uint64_t>(
                    config_path,
                    "roles.support_role_id",
                    0
                );

            dpp::snowflake tickets_category_id = ticket_cat_raw;
            dpp::snowflake apps_role_id        = apps_role_raw;

            dpp::snowflake everyone_id = guild_id;

            std::vector<dpp::permission_overwrite> overwrites;

            overwrites.emplace_back(
                everyone_id,
                0,
                dpp::p_view_channel,
                dpp::ot_role
            );

            overwrites.emplace_back(
                user_id,
                dpp::p_view_channel | dpp::p_send_messages,
                0,
                dpp::ot_member
            );

            if (apps_role_id) {
                overwrites.emplace_back(
                    apps_role_id,
                    dpp::p_view_channel | dpp::p_send_messages,
                    0,
                    dpp::ot_role
                );
            }

            dpp::channel ticket_ch;
            ticket_ch.set_name("заявка-" + display_name)
                     .set_guild_id(guild_id)
                     .set_parent_id(tickets_category_id)
                     .set_type(dpp::CHANNEL_TEXT);
            ticket_ch.permission_overwrites = overwrites;

            cluster->channel_create(
                ticket_ch,
                [name = std::move(name),
                 nickname = std::move(nickname),
                 about = std::move(about),
                 source = std::move(source),
                 age,
                 apps_role_id,
                 display_name,
                 avatar_url,
                 user_id]
                (const dpp::confirmation_callback_t& cc) {

                    if (cc.is_error()) {
                        LogError("[BuiltInCallbacks] Failed to create ticket channel: " +
                                 cc.http_info.body);
                        return;
                    }

                    auto created = std::get<dpp::channel>(cc.value);
                    dpp::snowflake ticket_channel_id = created.id;
                    LogDebug("[BuiltInCallbacks] Ticket channel created, id=" +
                             std::to_string(ticket_channel_id));

                    dpp::cluster* c = DiscordCluster::GetCluster();
                    if (!c) return;

                    dpp::embed ticket_embed;
                    ticket_embed.set_color(0xF39C12);

                    dpp::embed_author author;
                    author.name     = display_name;
                    author.icon_url = avatar_url;
                    ticket_embed.set_author(author);

                    ticket_embed.set_title("Заявка " + display_name);
                    ticket_embed.add_field("Реальное имя", name, true);
                    ticket_embed.add_field("Игровой ник", nickname, true);
                    ticket_embed.add_field("Возраст", std::to_string(age), true);
                    ticket_embed.add_field("\u200b", "\u200b", true);
                    ticket_embed.add_field("О себе",       about,  false);
                    ticket_embed.add_field("Откуда узнал", source, false);

                    std::string status_text = "На рассмотрении";
                    ticket_embed.add_field("Статус", status_text, false);

                    ticket_embed.set_footer(
                        dpp::embed_footer().set_text("Ожидание решения модераторов")
                    );
                    ticket_embed.set_timestamp(std::time(nullptr));

                    std::string uid_str = std::to_string(user_id);

                    dpp::component approve_btn = dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_success)
                        .set_label("Одобрить")
                        .set_id("ticket_approve:" + uid_str);

                    dpp::component reject_btn = dpp::component()
                        .set_type(dpp::cot_button)
                        .set_style(dpp::cos_danger)
                        .set_label("Отклонить")
                        .set_id("ticket_reject:" + uid_str);

                    dpp::message ticket_msg;
                    ticket_msg.channel_id = ticket_channel_id;

                    if (apps_role_id) {
                        ticket_msg.set_content("<@&" + std::to_string(apps_role_id) + ">");
                    }

                    ticket_msg.add_embed(ticket_embed);
                    ticket_msg.add_component(
                        dpp::component().add_component(approve_btn)
                                        .add_component(reject_btn)
                    );

                    c->message_create(ticket_msg,
                        [](const dpp::confirmation_callback_t& mcc) {
                            if (mcc.is_error()) {
                                LogError("[BuiltInCallbacks] Failed to send ticket message: " +
                                         mcc.http_info.body);
                                return;
                            }

                            auto msg = std::get<dpp::message>(mcc.value);
                            LogDebug("[BuiltInCallbacks] Ticket message sent OK, id=" +
                                     std::to_string(msg.id));
                        }
                    );
                }
            );

            event.reply(dpp::message("Тикет успешно создан").set_flags(dpp::m_ephemeral));
        }
    });
}
