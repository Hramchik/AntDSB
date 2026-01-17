//
// Created by sanek on 17/01/2026.
//

#ifndef ANTDSB_APPLICATIONLOGGER_H
#define ANTDSB_APPLICATIONLOGGER_H

#include <dpp/dpp.h>
#include <string>

namespace ApplicationLogger {
    inline dpp::snowflake LOG_CHANNEL_ID = 0;

    void SetLogChannel(dpp::snowflake channel_id);

    void LogApproval(
        dpp::cluster* cluster,
        dpp::snowflake moderator_id,
        const std::string& moderator_name,
        dpp::snowflake applicant_id,
        const std::string& applicant_name,
        const std::string& real_name,
        const std::string& age,
        const std::string& nickname
    );

    void LogRejection(
        dpp::cluster* cluster,
        dpp::snowflake moderator_id,
        const std::string& moderator_name,
        dpp::snowflake applicant_id,
        const std::string& applicant_name,
        const std::string& real_name,
        const std::string& age,
        const std::string& nickname,
        const std::string& reason = ""
    );
}

#endif //ANTDSB_APPLICATIONLOGGER_H