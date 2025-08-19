## 先决条件（在 Ubuntu 20.04 上执行）

1. 更新并安装基本工具与依赖（含 gcc-11 / g++-11）：

```bash
sudo apt update
sudo apt install -y build-essential git cmake ninja-build pkg-config python3 python3-pip \
                    libssl-dev ccache
# 安装 gcc-11 / g++-11（如果你已经有 gcc11 可跳过）
# Ubuntu 20.04 默认自带 GCC 9，你需要添加 官方 Toolchain PPA 才能装新版。
# 更新包索引
sudo apt update
sudo apt upgrade -y

# 添加 gcc 官方仓库
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt update

# 安装 gcc/g++ 11
sudo apt install -y gcc-11 g++-11

# 查看版本
gcc-11 --version
g++-11 --version

# 把系统默认的 gcc 切换到 11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11
# 手动选择 gcc 版本
sudo update-alternatives --config gcc
sudo update-alternatives --config g++
#确认
gcc --version
g++ --version
```

1. （可选）如果你想把 gcc-11 设为 cmake 的默认编译器，推荐在运行 cmake 时用 `-DCMAKE_C_COMPILER` / `-DCMAKE_CXX_COMPILER` 指定，示例在后面会给出。

------

## 1）获取 open62541pp 源码（带子模块）

```bash
cd ~/src
git clone --recursive https://github.com/open62541pp/open62541pp.git
cd open62541pp
# 确保子模块也更新（如果没用 --recursive）
git submodule update --init --recursive
```

（open62541pp README 推荐用 `--recursive`，仓库包含 open62541 子模块并且可以选择内部 open62541 构建）([GitHub](https://github.com/open62541pp/open62541pp))

------

## 2）构建 open62541pp（生成库）

下面的命令把 open62541pp 和内嵌的 open62541 一起构建，并启用 examples。我们用 `UAPP_INTERNAL_OPEN62541=ON`（表示使用仓库内的 open62541 子模块），并启用示例构建 `UAPP_BUILD_EXAMPLES=ON`。

```bash
# 在仓库目录内
mkdir -p build && cd build

# 如果需要指定 gcc-11 / g++-11（推荐）
cmake -DCMAKE_BUILD_TYPE=Release \
      -DUAPP_INTERNAL_OPEN62541=ON \
      -DUAPP_BUILD_EXAMPLES=ON \
      -DCMAKE_C_COMPILER=/usr/bin/gcc-11 \
      -DCMAKE_CXX_COMPILER=/usr/bin/g++-11 \
      ..

# 使用 make 或 ninja（上面装了 ninja，可用 -G Ninja）
cmake --build . -- -j$(nproc)
# or with config:
# cmake --build . --config Release
```

解释：

- `UAPP_INTERNAL_OPEN62541=ON`：使用仓库内集成的 open62541 子模块构建（可选，你也可以用系统已安装的 open62541）；open62541pp 支持两种方式。
- `UAPP_BUILD_EXAMPLES=ON`：会编译 `examples` 目录下的示例（client/server 等），便于直接跑 demo。([GitHub](https://github.com/open62541pp/open62541pp))
- 可以使用`BUILD_SHARED_LIBS=ON`：告诉 CMake 默认把 `add_library()` 生成 shared（open62541 会以 .so 形式构建）

**构建完成后**，你会在 `build` 目录下看到生成的库与可执行文件（examples）。如果你想把库安装到系统路径（以便后续 `find_package` 找到），执行：

```bash
sudo cmake --install .
```

（这会把 `open62541ppConfig.cmake` 安装到系统 CMake 路径，使 find_package 生效。）([GitHub](https://github.com/open62541pp/open62541pp))

太好了 — 既然静态版能跑了，我们把流程改成 **生成并使用动态库（.so）** 的方法。我会给你两条可选路径：

- 方法 A（推荐）：重新用 CMake 把 `open62541pp` / `open62541` 构建成 **共享库并安装到系统**，然后在 demo 用 `find_package(open62541pp)` 链接（更干净、类似第三方库使用方式）。
- 方法 B（快速）：只在 build 目录产生 `.so`，不安装，demo 直接链接到 build 下的 `.so` 并运行（适合临时验证）。

下面按步骤来 — 我会用你机器上的路径示例 `~/opc_ua/open62541pp`（你当前是 `/root/opc_ua/open62541pp`，两者等价，请以实际路径为准），并假定你使用 `gcc-11/g++-11`。

> 说明：open62541（核心 C 库）与 CMake 使用 `BUILD_SHARED_LIBS=ON` 来控制是否生成 shared libs；open62541pp 则有自己的选项（如 `UAPP_INTERNAL_OPEN62541` 控制是否使用内嵌 open62541 子模块）。我们将同时打开 `BUILD_SHARED_LIBS=ON` 与 `UAPP_INTERNAL_OPEN62541=ON` 以确保二者都以 shared 方式构建。([open62541.org](https://www.open62541.org/doc/master/building.html?utm_source=chatgpt.com), [GitHub](https://github.com/open62541pp/open62541pp?utm_source=chatgpt.com), [cmake.org](https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html?utm_source=chatgpt.com))

## 3) 确认 .so 已安装

```bash
ldconfig -p | grep open62541
ls /usr/local/lib | grep open62541
# 或
find /usr/local -type f -name "libopen62541*.so" -o -name "libopen62541pp*.so"
```

## 4) demo：用 find_package + targets（CMakeLists）

在你的 demo 项目目录（比如 `~/opc_ua/demo_shared`）写下 `server_demo.cpp` 与 `client_demo.cpp`（我在前面给出的示例可直接复用，只用 `opcua::Variant` 的写法即可）。这里给出 `CMakeLists.txt`（使用 `find_package`）：

```cmake
cmake_minimum_required(VERSION 3.16)
project(opc_shared_demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(open62541pp CONFIG REQUIRED)

add_executable(server_demo server_demo.cpp)
target_link_libraries(server_demo PRIVATE open62541pp::open62541pp)

add_executable(client_demo client_demo.cpp)
target_link_libraries(client_demo PRIVATE open62541pp::open62541pp)
```

## 5) 构建 demo

```bash
mkdir -p ~/opc_ua/demo_shared/build
cd ~/opc_ua/demo_shared/build
cmake -DCMAKE_C_COMPILER=/usr/bin/gcc-11 -DCMAKE_CXX_COMPILER=/usr/bin/g++-11 ..
cmake --build . -- -j$(nproc)
```