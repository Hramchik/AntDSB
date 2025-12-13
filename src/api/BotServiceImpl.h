//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_BOTSERVICEIMPL_H
#define ANTDSB_BOTSERVICEIMPL_H

#include "discord/DiscordBot.h"
#include "bot.grpc.pb.h"   // из generated
#include <grpcpp/grpcpp.h>

class BotServiceImpl final : public antdsb::BotService::Service {
public:
    explicit BotServiceImpl(DiscordBot& bot);

    ::grpc::Status GetStatus(::grpc::ServerContext* context,
                             const antdsb::StatusRequest* request,
                             antdsb::StatusReply* reply) override;

    ::grpc::Status SendMessage(::grpc::ServerContext* context,
                               const antdsb::SendMessageRequest* request,
                               antdsb::SendMessageReply* reply) override;

private:
    DiscordBot& bot_;
};

#endif //ANTDSB_BOTSERVICEIMPL_H