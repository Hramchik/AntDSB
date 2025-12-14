//
// Created by sanek on 13/12/2025.
//

#include "api/BotServiceImpl.h"
#include <google/protobuf/empty.pb.h>

BotServiceImpl::BotServiceImpl(DiscordBot& bot)
    : bot_(bot) {}

::grpc::Status BotServiceImpl::GetStatus(::grpc::ServerContext*,
                                         const antdsb::StatusRequest*,
                                         antdsb::StatusReply* reply) {
    auto st = bot_.GetStatus();
    reply->set_running(st.running);
    reply->set_last_error(st.last_error);

    std::cout << "[GetStatus] running=" << st.running
              << " last_error=" << st.last_error << std::endl;

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

// Новый RPC: старт бота
::grpc::Status BotServiceImpl::StartBot(::grpc::ServerContext*,
                                        const google::protobuf::Empty*,
                                        antdsb::StatusReply* reply) {
    try {
        bot_.StartAsync();   // или Start(), как у тебя принято
        auto st = bot_.GetStatus();
        reply->set_running(st.running);
        reply->set_last_error(st.last_error);
        return ::grpc::Status::OK;
    } catch (const std::exception& e) {
        reply->set_running(false);
        reply->set_last_error(e.what());
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, e.what());
    }
}

// Новый RPC: остановка
::grpc::Status BotServiceImpl::StopBot(::grpc::ServerContext*,
                                       const google::protobuf::Empty*,
                                       antdsb::StatusReply* reply) {
    try {
        bot_.Stop();
        auto st = bot_.GetStatus();
        reply->set_running(st.running);
        reply->set_last_error(st.last_error);
        return ::grpc::Status::OK;
    } catch (const std::exception& e) {
        reply->set_running(false);
        reply->set_last_error(e.what());
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, e.what());
    }
}

// Новый RPC: рестарт
::grpc::Status BotServiceImpl::RestartBot(::grpc::ServerContext*,
                                          const google::protobuf::Empty*,
                                          antdsb::StatusReply* reply) {
    try {
        bot_.Stop();
        bot_.StartAsync();   // или Start()

        auto st = bot_.GetStatus();
        reply->set_running(st.running);
        reply->set_last_error(st.last_error);
        return ::grpc::Status::OK;
    } catch (const std::exception& e) {
        reply->set_running(false);
        reply->set_last_error(e.what());
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, e.what());
    }
}
