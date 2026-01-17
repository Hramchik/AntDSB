//
// Created by sanek on 17/01/2026.
//

#include "ApplicationLogger.h"
#include "logger/Logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace ApplicationLogger {

    void SetLogChannel(dpp::snowflake channel_id) {
        LOG_CHANNEL_ID = channel_id;
        LogInfo("[ApplicationLogger] Log channel set to: " + std::to_string(static_cast<uint64_t>(channel_id)));
    }

    std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d.%m.%Y %H:%M:%S");
        return oss.str();
    }

    void LogApproval(
        dpp::cluster* cluster,
        dpp::snowflake moderator_id,
        const std::string& moderator_name,
        dpp::snowflake applicant_id,
        const std::string& applicant_name,
        const std::string& real_name,
        const std::string& age,
        const std::string& nickname
    ) {
        if (!cluster || !LOG_CHANNEL_ID) {
            LogWarn("[ApplicationLogger] Cannot log approval: cluster or log channel not set");
            return;
        }

        std::string timestamp = GetTimestamp();

        dpp::embed embed = dpp::embed()
            .set_color(0x00FF00) // Зелёный
            .set_title("✅ Заявка одобрена")
            .set_timestamp(time(nullptr))
            .add_field("Модератор", "<@" + std::to_string(static_cast<uint64_t>(moderator_id)) + "> (" + moderator_name + ")", false)
            .add_field("Подавший заявку", "<@" + std::to_string(static_cast<uint64_t>(applicant_id)) + "> (" + applicant_name + ")", false)
            .add_field("Реальное имя", real_name, true)
            .add_field("Возраст", age, true)
            .add_field("Игровой ник", nickname, true)
            .set_footer(dpp::embed_footer().set_text("Время: " + timestamp));

        dpp::message msg(LOG_CHANNEL_ID, embed);

        cluster->message_create(msg, [](const dpp::confirmation_callback_t& callback) {
            if (callback.is_error()) {
                LogError("[ApplicationLogger] Failed to send approval log: " + callback.get_error().message);
            }
        });

        LogInfo("[ApplicationLogger] Approval logged: " + applicant_name + " by " + moderator_name);
    }

    void LogRejection(
        dpp::cluster* cluster,
        dpp::snowflake moderator_id,
        const std::string& moderator_name,
        dpp::snowflake applicant_id,
        const std::string& applicant_name,
        const std::string& real_name,
        const std::string& age,
        const std::string& nickname,
        const std::string& reason
    ) {
        if (!cluster || !LOG_CHANNEL_ID) {
            LogWarn("[ApplicationLogger] Cannot log rejection: cluster or log channel not set");
            return;
        }

        std::string timestamp = GetTimestamp();

        dpp::embed embed = dpp::embed()
            .set_color(0xFF0000) // Красный
            .set_title("❌ Заявка отклонена")
            .set_timestamp(time(nullptr))
            .add_field("Модератор", "<@" + std::to_string(static_cast<uint64_t>(moderator_id)) + "> (" + moderator_name + ")", false)
            .add_field("Подавший заявку", "<@" + std::to_string(static_cast<uint64_t>(applicant_id)) + "> (" + applicant_name + ")", false)
            .add_field("Реальное имя", real_name, true)
            .add_field("Возраст", age, true)
            .add_field("Игровой ник", nickname, true);

        if (!reason.empty()) {
            embed.add_field("Причина отклонения", reason, false);
        }

        embed.set_footer(dpp::embed_footer().set_text("Время: " + timestamp));

        dpp::message msg(LOG_CHANNEL_ID, embed);

        cluster->message_create(msg, [](const dpp::confirmation_callback_t& callback) {
            if (callback.is_error()) {
                LogError("[ApplicationLogger] Failed to send rejection log: " + callback.get_error().message);
            }
        });

        LogInfo("[ApplicationLogger] Rejection logged: " + applicant_name + " by " + moderator_name);
    }

}