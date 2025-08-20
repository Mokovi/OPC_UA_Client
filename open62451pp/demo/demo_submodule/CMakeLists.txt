cmake_minimum_required(VERSION 3.16)
project(opc_demo_submodule CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------- 可选：在这里切换生成 shared lib 或 static lib ----------
# 如果你想生成动态库（.so），将下面注释取消（ON）
# 如果想使用静态库，注释或设置为 OFF
option(OPC_BUILD_SHARED_OPEN62541PP "Build open62541pp and open62541 as shared libraries" OFF)

# 将配置传递给子模块（通过 cache 强制传递）
set(BUILD_SHARED_LIBS ${OPC_BUILD_SHARED_OPEN62541PP} CACHE BOOL "Control shared vs static" FORCE)
# open62541pp option：若你想强制使用子模块内的 open62541，打开下面（通常子模块默认会使用）
set(UAPP_INTERNAL_OPEN62541 ON CACHE BOOL "Use internal open62541 submodule" FORCE)

# （可选）显示设置编译器，如果需要用 gcc-11 / g++-11，取消下面注释并修改路径
set(CMAKE_C_COMPILER "/usr/bin/gcc-11" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11" CACHE STRING "" FORCE)

# 把子模块加入构建（确保 extern/open62541pp 已存在）
add_subdirectory(extern/open62541pp)

# 现在 open62541pp::open62541pp target 应该可用
# 创建两个可执行
add_executable(server_demo server_demo.cpp)
add_executable(client_demo client_demo.cpp)

# 链接 open62541pp target（让 CMake 管理 include 和 link）
target_link_libraries(server_demo PRIVATE open62541pp::open62541pp)
target_link_libraries(client_demo PRIVATE open62541pp::open62541pp)

# 如果你想显示输出路径
set_target_properties(server_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(client_demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
