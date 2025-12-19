cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)
project(AntDSB VERSION 0.3.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(dpp CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(PROTO_SRC ${CMAKE_SOURCE_DIR}/proto/bot.proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

add_custom_command(
        OUTPUT
        ${GENERATED_DIR}/bot.pb.cc
        ${GENERATED_DIR}/bot.pb.h
        ${GENERATED_DIR}/bot.grpc.pb.cc
        ${GENERATED_DIR}/bot.grpc.pb.h
        COMMAND protobuf::protoc
        ARGS --grpc_out=${GENERATED_DIR}
        --cpp_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        -I ${CMAKE_SOURCE_DIR}/proto
        ${PROTO_SRC}
        DEPENDS ${PROTO_SRC}
        COMMENT "Generating gRPC sources from bot.proto"
)

add_library(antdsb_grpc
        ${GENERATED_DIR}/bot.pb.cc
        ${GENERATED_DIR}/bot.grpc.pb.cc
)

target_include_directories(antdsb_grpc PUBLIC ${GENERATED_DIR})
target_link_libraries(antdsb_grpc PUBLIC gRPC::grpc++ protobuf::libprotobuf)

set(SOURCES
        src/main.cpp
        src/config/ConfigManager.cpp
        src/discord/DiscordBot.cpp
        src/discord/BuiltInCommands.cpp
        src/discord/CommandRegistry.cpp
        src/discord/CallbackRegistry.cpp
        src/discord/DiscordBot_MessageLogging.cpp
        src/discord/DiscordBot_Queries.cpp
        src/discord/DiscordBot_VoiceLogging.cpp
        src/discord/ui/ApplicationUI.cpp
        src/discord/callbacks/BuiltInCallbacks.cpp
        src/utils/TokenValidator.cpp
        src/logger/Logger.cpp
        src/cli/ConsoleLoop.cpp
        src/api/BotServiceImpl.cpp
        src/api/GrpcServer.cpp
)

add_executable(AntDSB ${SOURCES}
        ${GENERATED_DIR}/bot.pb.cc
        ${GENERATED_DIR}/bot.grpc.pb.cc
)

add_dependencies(AntDSB antdsb_grpc)

target_link_libraries(AntDSB PRIVATE
        yaml-cpp::yaml-cpp
        dpp::dpp
        CURL::libcurl
        ZLIB::ZLIB
        antdsb_grpc
)

target_include_directories(AntDSB PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${GENERATED_DIR}
)

add_subdirectory(gui)
