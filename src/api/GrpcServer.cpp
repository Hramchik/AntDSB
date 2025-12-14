//
// Created by sanek on 13/12/2025.
//

#include "GrpcServer.h"
#include "BotServiceImpl.h"
#include "logger/Logger.h"

#include <grpcpp/server_builder.h>

GrpcServer::GrpcServer(DiscordBot& bot)
    : bot_(bot) {}

GrpcServer::~GrpcServer() {
    Shutdown();
}

void GrpcServer::Start(const std::string& address) {
    if (server_)
        return;

    // Правильно: создаём BotServiceImpl с ссылкой на живой DiscordBot
    service_ = std::make_unique<BotServiceImpl>(bot_);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(service_.get());

    server_ = builder.BuildAndStart();
    LogInfo("[gRPC] Server listening on " + address);

    server_thread_ = std::thread([this] {
        server_->Wait();
        LogInfo("[gRPC] Server stopped");
    });
}

void GrpcServer::Shutdown() {
    if (!server_)
        return;

    server_->Shutdown();
    if (server_thread_.joinable()) {
        server_thread_.join();
    }

    server_.reset();
    service_.reset();
}
