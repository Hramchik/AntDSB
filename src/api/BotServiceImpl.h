//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_BOTSERVICEIMPL_H
#define ANTDSB_BOTSERVICEIMPL_H

#include "../discord/core/DiscordBot.h"
#include "bot.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <google/protobuf/empty.pb.h>

class BotServiceImpl final : public antdsb::BotService::Service {
public:
    explicit BotServiceImpl(DiscordBot& bot);

    ::grpc::Status GetStatus(::grpc::ServerContext* context,
                             const antdsb::StatusRequest* request,
                             antdsb::StatusReply* reply) override;

    ::grpc::Status SendMessage(::grpc::ServerContext* context,
                               const antdsb::SendMessageRequest* request,
                               antdsb::SendMessageReply* reply) override;

    ::grpc::Status StartBot(::grpc::ServerContext* context,
                            const google::protobuf::Empty* request,
                            antdsb::StatusReply* reply) override;

    ::grpc::Status StopBot(::grpc::ServerContext* context,
                           const google::protobuf::Empty* request,
                           antdsb::StatusReply* reply) override;

    ::grpc::Status RestartBot(::grpc::ServerContext* context,
                              const google::protobuf::Empty* request,
                              antdsb::StatusReply* reply) override;

    ::grpc::Status ListChannels(::grpc::ServerContext* context,
                                const antdsb::ListChannelsRequest* request,
                                antdsb::ListChannelsReply* reply) override;

    ::grpc::Status ListMessages(::grpc::ServerContext* context,
                                const antdsb::ListMessagesRequest* request,
                                antdsb::ListMessagesReply* reply) override;

private:
    DiscordBot& bot_;
};

#endif //ANTDSB_BOTSERVICEIMPL_H
