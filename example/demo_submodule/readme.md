# 以 submodule形式嵌入到demo 工程的一键可用模板

这个方案的要点：

- 把 `open62541pp` 作为子模块放到 `extern/open62541pp`，然后在主工程里用 `add_subdirectory(extern/open62541pp)` 一并构建并链接（CMake 会生成 `open62541pp::open62541pp` target）。
- 我同时给出如何用 **静态（默认）或共享（.so）** 两种构建方式（通过设置 `BUILD_SHARED_LIBS`）。
- 使用的 include/header 参照你现有的 `include/open62541pp` 结构（用 `#include <open62541pp/server.hpp>` 等）。

------

# 1. 初始化工程并添加子模块（在终端执行）

```bash
# 在你的工作目录下创建项目
mkdir -p ~/opc_ua/opc_demo_submodule
cd ~/opc_ua/opc_demo_submodule

# 初始化为 git 仓库（可选）
git init

# 添加 open62541pp 子模块（递归拉取子模块）
git submodule add --recursive https://github.com/open62541pp/open62541pp.git extern/open62541pp

# 如果子模块已经添加但未初始化，运行：
# git submodule update --init --recursive
```

------

# 2. 项目结构（执行完上面后应类似）

```
opc_demo_submodule/
├── CMakeLists.txt        # 顶层 CMake（下文给出）
├── server_demo.cpp       # 服务器源（下文给出）
├── client_demo.cpp       # 客户端源（下文给出）
└── extern/
    └── open62541pp/      # git submodule（源码）
```

------

# 3. 顶层 CMakeLists.txt（完整内容 — 直接创建）

将下面内容保存为项目根目录的 `CMakeLists.txt`：

```cmake
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
# set(CMAKE_C_COMPILER "/usr/bin/gcc-11" CACHE STRING "" FORCE)
# set(CMAKE_CXX_COMPILER "/usr/bin/g++-11" CACHE STRING "" FORCE)

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
```

说明：

- `OPC_BUILD_SHARED_OPEN62541PP` 控制是否使用 shared libs；默认 `OFF`（静态）。如果要生成 `.so`，可以在 configure 时传 `-DOPC_BUILD_SHARED_OPEN62541PP=ON` 或把 `OFF` 改为 `ON`。
- `UAPP_INTERNAL_OPEN62541` 强制子模块使用内嵌的 open62541 子项目（通常建议设 ON）。

------

# 4. server_demo.cpp（完整内容）

保存为 `server_demo.cpp`：

```cpp
// server_demo.cpp
#include <iostream>
#include <exception>

#include <open62541pp/server.hpp>
#include <open62541pp/node.hpp>
#include <open62541pp/open62541pp.hpp>

int main() {
    try {
        opcua::Server server;

        // 在 Objects 下添加一个变量 (ns=1;i=1000) 名为 "TheAnswer"
        opcua::Node objects(server, opcua::ObjectId::ObjectsFolder);
        auto myVar = objects.addVariable({1, 1000}, "TheAnswer");

        // 显式构造 Variant 写入初始值
        myVar.writeValue(opcua::Variant{42});

        std::cout << "Server running at opc.tcp://0.0.0.0:4840\n";
        server.run(); // 阻塞直到 stop()
    } catch (const std::exception &e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

------

# 5. client_demo.cpp（完整内容）

保存为 `client_demo.cpp`：

```cpp
// client_demo.cpp
#include <iostream>
#include <exception>

#include <open62541pp/client.hpp>
#include <open62541pp/node.hpp>
#include <open62541pp/open62541pp.hpp>

int main() {
    try {
        opcua::Client client;
        client.connect("opc.tcp://localhost:4840");

        opcua::Node node(client, opcua::NodeId{1, 1000});

        // 读取 Variant，并转换为 int
        auto v = node.readValue();
        int current = v.to<int>();
        std::cout << "Read current value: " << current << std::endl;

        // 写入新值
        node.writeValue(opcua::Variant{123});
        std::cout << "Wrote value 123" << std::endl;

        auto v2 = node.readValue();
        std::cout << "Read back: " << v2.to<int>() << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Client exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

------

# 6. 构建（静态或动态）

在项目根目录下执行：

```bash
mkdir -p build && cd build

# 若想用静态（默认），执行：
cmake -DCMAKE_BUILD_TYPE=Release ..

# 若你想生成动态库 (.so)，执行（示例开启 shared）：
# cmake -DCMAKE_BUILD_TYPE=Release -DOPC_BUILD_SHARED_OPEN62541PP=ON ..

# 若需要指定 gcc-11：
# cmake -DCMAKE_BUILD_TYPE=Release -DOPC_BUILD_SHARED_OPEN62541PP=ON -DCMAKE_C_COMPILER=/usr/bin/gcc-11 -DCMAKE_CXX_COMPILER=/usr/bin/g++-11 ..

cmake --build . -- -j$(nproc)
```

构建成功后，执行文件在 `build/bin/server_demo` 和 `build/bin/client_demo`（或 `build/` 根目录，取决子模块生成设置）。如果你把 `set RUNTIME_OUTPUT_DIRECTORY` 设为 `${CMAKE_BINARY_DIR}/bin`，则会在 `build/bin`。

------

# 7. 运行 demo

终端 A（运行 server）：

```bash
cd ~/opc_ua/opc_demo_submodule/build/bin
./server_demo
```

终端 B（运行 client）：

```bash
cd ~/opc_ua/opc_demo_submodule/build/bin
./client_demo
```

若你用了 shared libraries（`OPC_BUILD_SHARED_OPEN62541PP=ON`），可能需要设置运行时库路径（因为子模块生成的 .so 在 build 目录）：

```bash
export LD_LIBRARY_PATH=~/opc_ua/opc_demo_submodule/build/bin:$LD_LIBRARY_PATH
./server_demo
# 新终端
export LD_LIBRARY_PATH=~/opc_ua/opc_demo_submodule/build/bin:$LD_LIBRARY_PATH
./client_demo
```

------

# 8. 常见问题与解决（快速排查）

1. 子模块没有初始化 / 报找不到 CMakeLists：
   - 运行： `git submodule update --init --recursive`
   - 然后重新 `cmake ..`。
2. 子模块构建没有生成 .so（即你设了 shared 但仍只看到 .a）：
   - 确认 `cmake` 配置输出中 `BUILD_SHARED_LIBS` 的值。重新 `rm -rf build` 并以 `-DOPC_BUILD_SHARED_OPEN62541PP=ON` 重新 configure。
   - open62541pp 的 CMake 选项可能会覆盖 `BUILD_SHARED_LIBS`，可在顶层 `CMakeLists.txt` 里把 `set(BUILD_SHARED_LIBS ${OPC_BUILD_SHARED_OPEN62541PP} CACHE BOOL "" FORCE)` 保证传递。
3. 链接时出现 `undefined reference`：
   - 确保用同一编译器编译（例如都用 gcc-11）。可在 cmake 时指定 `-DCMAKE_C_COMPILER` / `-DCMAKE_CXX_COMPILER`。
   - 若仍有缺少系统库，按错误提示在顶层 `target_link_libraries` 添加 `pthread dl ssl crypto m rt`（不过使用 `open62541pp::open62541pp` target 通常会自动加好）。
4. 运行时报 `cannot open shared object file`：
   - 把生成 .so 的目录加入 `LD_LIBRARY_PATH`，或把库 `sudo cmake --install .` 安装到系统路径并运行 `sudo ldconfig`。