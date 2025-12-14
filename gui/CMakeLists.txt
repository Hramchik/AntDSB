cmake_minimum_required(VERSION 4.0.0)
project(AntDSB_GUI LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
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
        COMMENT "Generating gRPC sources for GUI from bot.proto"
)

set(GUI_SOURCES
        src/main_gui.cpp
        src/MainWindow.cpp
        src/BotClient.cpp
        ${GENERATED_DIR}/bot.pb.cc
        ${GENERATED_DIR}/bot.grpc.pb.cc
)

add_executable(AntDSB_GUI WIN32 ${GUI_SOURCES})

target_include_directories(AntDSB_GUI PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${GENERATED_DIR}
)

target_link_libraries(AntDSB_GUI PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets
        gRPC::grpc++ protobuf::libprotobuf
)

add_custom_command(TARGET AntDSB_GUI POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:AntDSB_GUI>/platforms"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "C:/Users/sanek/Documents/Programming/ClionPrg/AntDSB/build/Debug/vcpkg_installed/x64-windows/Qt6/plugins/platforms/qwindows.dll"
        "$<TARGET_FILE_DIR:AntDSB_GUI>/platforms/qwindows.dll"
        COMMENT "Copying qwindows.dll to output dir"
)
