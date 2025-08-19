cmake_minimum_required(VERSION 3.16)
project(opc_shared_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(open62541pp CONFIG REQUIRED)

add_executable(server_demo server_demo.cpp)
target_link_libraries(server_demo PRIVATE open62541pp::open62541pp)

add_executable(client_demo client_demo.cpp)
target_link_libraries(client_demo PRIVATE open62541pp::open62541pp)
