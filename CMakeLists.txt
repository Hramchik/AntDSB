cmake_minimum_required(VERSION 4.0 FATAL_ERROR)
project(AntDSB VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD_REQUIRED 20)

find_package(dpp CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_executable(AntDSB src/main.cpp
        src/DiscordBot/DiscordBot.cpp
        src/DiscordBot/createCommand.cpp
        src/commands/commands.cpp
        src/configParser/configParser.cpp
        src/utils/thread_pool.cpp
)

target_link_libraries(AntDSB PRIVATE
        dpp::dpp
        yaml-cpp::yaml-cpp
)