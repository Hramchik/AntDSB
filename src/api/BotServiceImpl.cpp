//
// Created by sanek on 13/12/2025.
//

#include "api/BotServiceImpl.h"
#include "logger/Logger.h"

BotServiceImpl::BotServiceImpl(DiscordBot& bot)
    : bot_(bot) {}

::grpc::Status BotServiceImpl::GetStatus(::grpc::ServerContext*,
                                         const antdsb::StatusRequest*,
                                         antdsb::StatusReply* reply) {
    // LogDebug("[gRPC] GetStatus]");
    // Если RPC ответил — сервис жив, а статус бота определяется отдельно.
    reply->set_running(true);
    reply->set_last_error("");
    return ::grpc::Status::OK;
}

::grpc::Status BotServiceImpl::SendMessage(::grpc::ServerContext*,
                                           const antdsb::SendMessageRequest* request,
                                           antdsb::SendMessageReply* reply) {
    try {
        dpp::cluster& cl = bot_.GetCluster();
        dpp::message msg;
        msg.set_content(request->text());
        msg.channel_id = static_cast<dpp::snowflake>(request->channel_id());

        cl.message_create(msg);   // без лямбды и без reply из другого потока
        reply->set_ok(true);
        reply->set_error("");
        return ::grpc::Status::OK;
    } catch (const std::exception& e) {
        reply->set_ok(false);
        reply->set_error(std::string("Exception: ") + e.what());
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, e.what());
    }
}

::grpc::Status BotServiceImpl::StartBot(::grpc::ServerContext*,
                                        const google::protobuf::Empty*,
                                        antdsb::StatusReply* reply) {
    // LogDebug("[GRPC] StartBot]");
    bool ok = bot_.Start();
    reply->set_running(ok);
    reply->set_last_error(ok ? "" : "Failed to start bot");
    return ::grpc::Status::OK;
}

::grpc::Status BotServiceImpl::StopBot(::grpc::ServerContext*,
                                       const google::protobuf::Empty*,
                                       antdsb::StatusReply* reply) {
    // LogDebug("[grpc] StopBot]");
    bot_.Stop();
    reply->set_running(false);
    reply->set_last_error("");
    return ::grpc::Status::OK;
}

::grpc::Status BotServiceImpl::RestartBot(::grpc::ServerContext*,
                                          const google::protobuf::Empty*,
                                          antdsb::StatusReply* reply) {
    // LogDebug("[grpc] RestartBot]");
    bot_.Stop();
    bool ok = bot_.Start();
    reply->set_running(ok);
    reply->set_last_error(ok ? "" : "Failed to restart bot");
    return ::grpc::Status::OK;
}

::grpc::Status BotServiceImpl::ListChannels(::grpc::ServerContext*,
                                            const antdsb::ListChannelsRequest*,
                                            antdsb::ListChannelsReply* reply) {
    // LogDebug("gRPC ListChannels]");
    auto channels = bot_.ListTextChannels();
    for (auto& [id, name] : channels) {
        auto* out = reply->add_channels();
        out->set_id(id);
        out->set_name(name);
    }
    return ::grpc::Status::OK;
}

::grpc::Status BotServiceImpl::ListMessages(::grpc::ServerContext*,
                                            const antdsb::ListMessagesRequest* request,
                                            antdsb::ListMessagesReply* reply) {
    // LogDebug("GRPC ListMessages]");
    uint64_t channel_id = request->channel_id();
    uint32_t limit = request->limit();
    if (limit == 0)
        limit = 50;

    auto msgs = bot_.GetRecentMessages(channel_id, limit);
    for (const auto& m : msgs) {
        auto* out = reply->add_messages();
        out->set_id(m.id);
        out->set_author_id(m.author_id);
        out->set_author(m.author);
        out->set_content(m.content);
        out->set_timestamp(m.timestamp);
    }

    return ::grpc::Status::OK;
}
