## 1) CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(opc_static_demo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== 设置 open62541pp build 根目录 ==========
set(OPEN62541PP_BUILD_DIR "/root/opc_ua/open62541pp/build" CACHE PATH "Open62541pp build dir")

# 直接指定 include 目录为仓库中的 include 文件夹（不要指到 build/bin）
set(OPEN62541PP_SRC_DIR "${OPEN62541PP_BUILD_DIR}/..")
set(OPEN62541PP_INCLUDE_DIR "${OPEN62541PP_SRC_DIR}/include")  # e.g. /root/opc_ua/open62541pp/include
set(OPEN62541PP_BIN_DIR "${OPEN62541PP_BUILD_DIR}/bin")        # e.g. /root/opc_ua/open62541pp/build/bin

message(STATUS "OPEN62541PP_INCLUDE_DIR = ${OPEN62541PP_INCLUDE_DIR}")
message(STATUS "OPEN62541PP_BIN_DIR     = ${OPEN62541PP_BIN_DIR}")

# 静态库文件
find_library(OPEN62541PP_LIB NAMES open62541pp PATHS ${OPEN62541PP_BIN_DIR} NO_DEFAULT_PATH)
find_library(OPEN62541_LIB   NAMES open62541   PATHS ${OPEN62541PP_BIN_DIR} NO_DEFAULT_PATH)

if(NOT OPEN62541PP_LIB OR NOT OPEN62541_LIB)
    message(FATAL_ERROR "找不到 libopen62541pp.a 或 libopen62541.a，请检查 ${OPEN62541PP_BIN_DIR}")
endif()

add_executable(server_demo server_demo.cpp)
add_executable(client_demo client_demo.cpp)

target_include_directories(server_demo PRIVATE ${OPEN62541PP_INCLUDE_DIR})
target_include_directories(client_demo PRIVATE ${OPEN62541PP_INCLUDE_DIR})

target_link_libraries(server_demo PRIVATE
    ${OPEN62541PP_LIB}
    ${OPEN62541_LIB}
    pthread dl ssl crypto m rt
)

target_link_libraries(client_demo PRIVATE
    ${OPEN62541PP_LIB}
    ${OPEN62541_LIB}
    pthread dl ssl crypto m rt
)

```

## 2) 构建 demo

```bash
cd ~/opc_ua/demo_static
rm -rf build
mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release \
      -DOPEN62541PP_BUILD_DIR=/root/opc_ua/open62541pp/build \
      -DCMAKE_C_COMPILER=/usr/bin/gcc-11 -DCMAKE_CXX_COMPILER=/usr/bin/g++-11 \
      ..

cmake --build . -- -j$(nproc)

```