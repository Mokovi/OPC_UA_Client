/**
 * @file client_connect_annotated.cpp
 * @brief OPC UA 客户端连接示例 - 演示如何连接到服务器并进行基本的读写操作
 * 
 * 本示例展示了 open62541pp 库中客户端的基本功能，包括：
 * 1. 解析命令行参数
 * 2. 配置用户身份验证（可选）
 * 3. 连接到 OPC UA 服务器
 * 4. 读取服务器时间
 * 5. 断开连接
 * 
 * 支持的命令行选项：
 * --username <用户名>    设置用户名进行身份验证
 * --password <密码>      设置密码进行身份验证
 * --help, -h            显示帮助信息
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/client.hpp>    // 客户端核心功能
#include <open62541pp/node.hpp>      // 节点操作

// 包含辅助函数头文件
#include "helper.hpp"  // CliParser - 命令行参数解析器

int main(int argc, char* argv[]) {
    // 创建命令行参数解析器
    // 解析传入的命令行参数
    const CliParser parser{argc, argv};
    
    // 检查参数数量是否足够，或者是否请求帮助
    // 至少需要 2 个参数：程序名和服务器 URL
    if (parser.nargs() < 2 || parser.hasFlag("-h") || parser.hasFlag("--help")) {
        // 显示使用说明和帮助信息
        std::cout
            << "用法: client_connect [选项] opc.tcp://<主机>:<端口>\n"
            << "选项:\n"
            << "  --username <用户名>    设置用户名进行身份验证\n"
            << "  --password <密码>      设置密码进行身份验证\n"
            << "  --help, -h            显示此帮助信息\n"
            << std::flush;
        
        // 返回错误代码 2，表示参数错误
        return 2;
    }

    // 从命令行参数中提取服务器 URL
    // 最后一个参数应该是服务器地址
    const auto serverUrl = parser.args()[parser.nargs() - 1];
    
    // 从命令行参数中提取用户名和密码（如果提供）
    const auto username = parser.value("--username");
    const auto password = parser.value("--password");

    // 输出连接信息
    std::cout << "正在连接到服务器: " << serverUrl << std::endl;
    if (username) {
        std::cout << "使用用户名进行身份验证: " << username.value() << std::endl;
    } else {
        std::cout << "使用匿名身份验证" << std::endl;
    }

    // 创建 OPC UA 客户端实例
    // 使用默认配置
    opcua::Client client;
    
    // 如果提供了用户名，则配置用户身份验证
    if (username) {
        std::cout << "配置用户身份验证..." << std::endl;
        
        // 创建用户名/密码身份令牌
        // 如果密码为空，则使用空字符串
        opcua::UserNameIdentityToken userToken{
            username.value(),           // 用户名
            password.value_or("")       // 密码，如果没有提供则使用空字符串
        };
        
        // 将身份令牌设置到客户端配置中
        client.config().setUserIdentityToken(userToken);
    }

    // 连接到服务器
    // 这将建立与服务器的 TCP 连接并创建会话
    std::cout << "正在建立连接..." << std::endl;
    try {
        client.connect(serverUrl);
        std::cout << "✓ 连接成功！" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "✗ 连接失败: " << e.what() << std::endl;
        return 1;
    }

    // 创建节点对象，用于访问服务器时间变量
    // 使用标准的服务器状态节点：Server_ServerStatus_CurrentTime
    std::cout << "正在读取服务器时间..." << std::endl;
    opcua::Node node{client, opcua::VariableId::Server_ServerStatus_CurrentTime};
    
    // 读取节点的值
    // 这个节点包含服务器的当前时间
    const auto result = node.readValue();
    
    // 检查读取操作是否成功
    if (result.hasValue()) {
        // 将 Variant 类型转换为 DateTime 类型
        const auto dt = result.value().to<opcua::DateTime>();
        
        // 格式化并输出时间
        std::cout << "✓ 读取成功！服务器时间 (UTC): " 
                  << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
    } else {
        std::cout << "✗ 读取失败，状态码: " << result.status() << std::endl;
    }

    // 断开与服务器的连接
    // 这将关闭会话和 TCP 连接
    std::cout << "正在断开连接..." << std::endl;
    client.disconnect();
    std::cout << "✓ 已断开连接" << std::endl;

    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 基本连接（匿名身份验证）：
 *    ./client_connect opc.tcp://localhost:4840
 * 
 * 2. 使用用户名/密码身份验证：
 *    ./client_connect --username myuser --password mypass opc.tcp://localhost:4840
 * 
 * 3. 连接到远程服务器：
 *    ./client_connect opc.tcp://192.168.1.100:4840
 * 
 * 4. 显示帮助信息：
 *    ./client_connect --help
 * 
 * 程序流程：
 * 1. 解析命令行参数
 * 2. 创建客户端实例
 * 3. 配置身份验证（如果提供）
 * 4. 连接到指定的服务器
 * 5. 读取服务器时间
 * 6. 断开连接并退出
 * 
 * 注意事项：
 * - 确保目标服务器正在运行并可访问
 * - 如果使用身份验证，确保用户名和密码正确
 * - 服务器 URL 格式：opc.tcp://<主机>:<端口>
 * - 默认端口通常是 4840
 * - 程序会在操作完成后自动断开连接
 * 
 * 错误处理：
 * - 参数错误：返回错误代码 2
 * - 连接失败：返回错误代码 1
 * - 成功执行：返回错误代码 0
 * 
 * 扩展建议：
 * - 可以添加更多命令行选项（超时设置、安全策略等）
 * - 可以实现批量读取多个节点
 * - 可以添加数据写入功能
 * - 可以实现连接重试机制
 */
