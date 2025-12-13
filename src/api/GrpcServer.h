//
// Created by sanek on 13/12/2025.
//

#ifndef ANTDSB_GRPCSERVER_H
#define ANTDSB_GRPCSERVER_H

#include <memory>
#include <string>
#include <thread>

class DiscordBot;
class BotServiceImpl;
namespace grpc { class Server; }

class GrpcServer {
public:
    explicit GrpcServer(DiscordBot& bot);
    ~GrpcServer();

    void Start(const std::string& address = "0.0.0.0:50051");
    void Shutdown();

private:
    void Run(const std::string& address);

    DiscordBot& bot_;
    std::unique_ptr<BotServiceImpl> service_;
    std::unique_ptr<grpc::Server> server_;
    std::thread server_thread_;
};

#endif //ANTDSB_GRPCSERVER_H