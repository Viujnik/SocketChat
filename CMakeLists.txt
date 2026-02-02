cmake_minimum_required(VERSION 3.20)
project(SocketChat)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

add_executable(server
        src/server.cpp
        src/clients_handler.cpp
)
target_link_libraries(server Threads::Threads)

add_executable(client
        src/client.cpp
)
target_link_libraries(client Threads::Threads)
