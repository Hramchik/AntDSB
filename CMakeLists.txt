cmake_minimum_required(VERSION 4.0 FATAL_ERROR)
project(AntDSB VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD_REQUIRED 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(dpp CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

set(SOURCES
        src/main.cpp
        src/config/ConfigManager.cpp
        src/discord/DiscordBot.cpp
        src/discord/BuiltInCommands.cpp
        src/discord/CommandRegistry.cpp
        src/discord/CallbackRegistry.cpp
        src/discord/callbacks/ApplicationCallback.cpp
        src/discord/ui/ApplicationUI.cpp
        src/utils/TokenValidator.cpp
        src/discord/callbacks/BuiltInCallbacks.cpp
        src/discord/callbacks/BuiltInCallbacks.h
)

add_executable(AntDSB ${SOURCES})

target_link_libraries(AntDSB PRIVATE
        yaml-cpp::yaml-cpp
        dpp::dpp
        CURL::libcurl
)

target_include_directories(AntDSB PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(AntDSB PRIVATE cxx_std_17)