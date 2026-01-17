cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)
project(AntDSB VERSION 0.4.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(dpp CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

set(SOURCES
        src/main.cpp
        src/config/ConfigManager.cpp

        src/discord/core/DiscordBot.cpp
        src/discord/logging/ApplicationLogger.cpp
        src/discord/tempvc/TempVC.cpp
        src/discord/commands/BuiltInCommands.cpp
        src/discord/commands/CommandRegistry.cpp
        src/discord/callbacks/CallbackRegistry.cpp
        src/discord/callbacks/BuiltInCallbacks.cpp
        src/discord/logging/DiscordLogging.cpp
        src/discord/ui/ApplicationUI.cpp

        src/utils/TokenValidator.cpp
        src/logger/Logger.cpp
        src/cli/ConsoleControl.cpp
)

add_executable(AntDSB ${SOURCES})

target_link_libraries(AntDSB PRIVATE
        yaml-cpp::yaml-cpp
        dpp::dpp
        CURL::libcurl
        ZLIB::ZLIB
)

target_include_directories(AntDSB PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)