//
// Created by sanek on 04/01/2026.
//

#ifndef ANTDSB_TEMPVC_H
#define ANTDSB_TEMPVC_H

#include <dpp/dpp.h>
#include <unordered_map>
#include <optional>
#include <chrono>
#include <random>

using dpp::snowflake;

inline constexpr int HUB_USER_LIMIT              = 0;
inline constexpr int TEMP_VC_USER_LIMIT_DEFAULT = 0;
inline constexpr int OWNER_ABSENCE_TIMEOUT_SEC  = 5 * 60;
inline constexpr int CREATE_CD_SEC              = 60;

struct temp_vc_info {
    snowflake owner_id;
    snowflake hub_channel_id;
    std::optional<std::chrono::steady_clock::time_point> owner_left_at;
};

struct TempVCState {
    std::unordered_map<snowflake, bool> hub_channels;
    std::unordered_map<snowflake, temp_vc_info> temp_channels;
    std::unordered_map<snowflake, std::chrono::steady_clock::time_point> last_created;
    std::unordered_map<snowflake, snowflake> last_voice_channel;
    std::mt19937 rng;

    TempVCState()
        : rng(std::random_device{}()) {}
};

TempVCState& GetTempVCState();

std::string TempVC_GetDisplayName(dpp::snowflake guild_id, dpp::snowflake user_id);

#endif //ANTDSB_TEMPVC_H
