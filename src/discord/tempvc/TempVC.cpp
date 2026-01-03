//
// Created by sanek on 04/01/2026.
//

#include "TempVC.h"

TempVCState& GetTempVCState() {
    static TempVCState state;
    return state;
}

std::string TempVC_GetDisplayName(dpp::snowflake guild_id, dpp::snowflake user_id) {
    dpp::guild* g = dpp::find_guild(guild_id);
    if (!g) {
        return "TempVC-" + std::to_string(static_cast<uint64_t>(user_id));
    }

    auto it = g->members.find(user_id);
    if (it != g->members.end()) {
        const dpp::guild_member& gm = it->second;

        std::string nick = gm.get_nickname();
        if (!nick.empty()) {
            return nick;
        }

        if (gm.get_user()) {
            return gm.get_user()->username;
        }
    }

    return "TempVC-" + std::to_string(static_cast<uint64_t>(user_id));
}
