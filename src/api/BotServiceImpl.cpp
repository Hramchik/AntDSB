//
// Created by sanek on 13/12/2025.
//

#include "api/BotServiceImpl.h"

BotServiceImpl::BotServiceImpl(DiscordBot& bot)
    : bot_(bot) {}

::grpc::Status BotServiceImpl::GetStatus(::grpc::ServerContext*,
                                         const antdsb::StatusRequest*,
                                         antdsb::StatusReply* reply) {
    auto st = bot_.GetStatus();
    reply->set_running(st.running);
    reply->set_last_error(st.last_error);
    return ::grpc::Status::OK;
}

::grpc::Status BotServiceImpl::SendMessage(::grpc::ServerContext*,
                                           const antdsb::SendMessageRequest* request,
                                           antdsb::SendMessageReply* reply) {
    try {
        bot_.SendMessage(static_cast<dpp::snowflake>(request->channel_id()),
                         request->text());
        reply->set_ok(true);
        reply->set_error("");
        return ::grpc::Status::OK;
    } catch (const std::exception& e) {
        reply->set_ok(false);
        reply->set_error(e.what());
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, e.what());
    }
}