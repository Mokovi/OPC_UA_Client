/**
 * @file server_logger_annotated.cpp
 * @brief OPC UA 服务器日志示例 - 演示如何配置和使用自定义日志系统
 * 
 * 本示例展示了 open62541pp 库中日志系统的使用方法，包括：
 * 1. 创建自定义日志函数
 * 2. 配置服务器日志记录器
 * 3. 处理不同的日志级别和类别
 * 4. 格式化日志输出
 * 5. 在服务器配置中设置日志器
 * 
 * 功能说明：
 * - 支持多种日志级别（Trace、Debug、Info、Warning、Error、Fatal）
 * - 支持多种日志类别（网络、安全通道、会话、服务器、客户端等）
 * - 可以自定义日志格式和输出方式
 * - 支持在服务器配置中设置日志器
 */

#include <iostream>
#include <string_view>

// 包含必要的头文件
#include <open62541pp/server.hpp>    // 服务器核心功能

/**
 * @brief 将日志级别转换为可读的字符串
 * 
 * 这个函数将 OPC UA 的日志级别枚举值转换为人类可读的字符串，
 * 便于在日志输出中识别不同的日志级别。
 * 
 * @param level 日志级别枚举值
 * @return 对应的字符串表示
 */
constexpr std::string_view toString(opcua::LogLevel level) {
    switch (level) {
    case opcua::LogLevel::Trace:
        return "trace";      // 跟踪级别：最详细的日志信息
    case opcua::LogLevel::Debug:
        return "debug";      // 调试级别：调试信息
    case opcua::LogLevel::Info:
        return "info";       // 信息级别：一般信息
    case opcua::LogLevel::Warning:
        return "warning";    // 警告级别：警告信息
    case opcua::LogLevel::Error:
        return "error";      // 错误级别：错误信息
    case opcua::LogLevel::Fatal:
        return "fatal";      // 致命级别：致命错误信息
    default:
        return "unknown";    // 未知级别：未识别的日志级别
    }
}

/**
 * @brief 将日志类别转换为可读的字符串
 * 
 * 这个函数将 OPC UA 的日志类别枚举值转换为人类可读的字符串，
 * 便于在日志输出中识别不同的日志来源。
 * 
 * @param category 日志类别枚举值
 * @return 对应的字符串表示
 */
constexpr std::string_view toString(opcua::LogCategory category) {
    switch (category) {
    case opcua::LogCategory::Network:
        return "network";        // 网络相关日志
    case opcua::LogCategory::SecureChannel:
        return "channel";        // 安全通道相关日志
    case opcua::LogCategory::Session:
        return "session";        // 会话相关日志
    case opcua::LogCategory::Server:
        return "server";         // 服务器相关日志
    case opcua::LogCategory::Client:
        return "client";         // 客户端相关日志
    case opcua::LogCategory::Userland:
        return "userland";       // 用户代码相关日志
    case opcua::LogCategory::SecurityPolicy:
        return "securitypolicy"; // 安全策略相关日志
    default:
        return "unknown";        // 未知类别：未识别的日志类别
    }
}

int main() {
    std::cout << "=== OPC UA 服务器日志示例 ===" << std::endl;
    
    // 创建自定义日志函数
    // 这是一个 lambda 表达式，定义了日志的输出格式
    auto logger = [](auto level, auto category, auto msg) {
        // 日志格式：[级别] [类别] 消息内容
        std::cout << "[" << toString(level) << "] " 
                  << "[" << toString(category) << "] " 
                  << msg << std::endl;
    };

    std::cout << "正在创建服务器配置..." << std::endl;
    
    // 创建服务器配置，指定端口为 4840
    // 第二个参数是证书，这里使用空证书（无加密）
    opcua::ServerConfig config{4840, {}};
    
    // 在配置中设置日志器
    // 这样服务器在运行过程中就会使用我们自定义的日志函数
    std::cout << "正在设置自定义日志器..." << std::endl;
    config.setLogger(logger);

    // 使用配置构造服务器实例
    // std::move 确保配置对象的所有权转移给服务器
    std::cout << "正在创建服务器实例..." << std::endl;
    opcua::Server server{std::move(config)};

    // 演示：也可以在服务器构造后设置日志器
    // 这种方式与在配置中设置效果相同
    std::cout << "正在设置备用日志器..." << std::endl;
    server.config().setLogger(logger);

    std::cout << "服务器配置完成！" << std::endl;
    std::cout << "\n=== 日志系统说明 ===" << std::endl;
    std::cout << "1. 日志级别：从 trace 到 fatal，级别逐渐提高" << std::endl;
    std::cout << "2. 日志类别：标识日志的来源和类型" << std::endl;
    std::cout << "3. 日志格式：[级别] [类别] 消息内容" << std::endl;
    std::cout << "4. 自定义日志器：可以完全控制日志的输出方式" << std::endl;
    
    std::cout << "\n=== 日志级别说明 ===" << std::endl;
    std::cout << "- Trace: 最详细的跟踪信息，用于调试" << std::endl;
    std::cout << "- Debug: 调试信息，帮助开发者理解程序行为" << std::endl;
    std::cout << "- Info: 一般信息，记录程序正常运行状态" << std::endl;
    std::cout << "- Warning: 警告信息，表示可能的问题" << std::endl;
    std::cout << "- Error: 错误信息，表示程序执行中的错误" << std::endl;
    std::cout << "- Fatal: 致命错误，表示程序无法继续运行" << std::endl;
    
    std::cout << "\n=== 日志类别说明 ===" << std::endl;
    std::cout << "- Network: 网络连接、数据传输等" << std::endl;
    std::cout << "- SecureChannel: 安全通道建立、加密通信等" << std::endl;
    std::cout << "- Session: 会话管理、用户认证等" << std::endl;
    std::cout << "- Server: 服务器内部状态、配置等" << std::endl;
    std::cout << "- Client: 客户端连接、请求处理等" << std::endl;
    std::cout << "- Userland: 用户代码、自定义功能等" << std::endl;
    std::cout << "- SecurityPolicy: 安全策略、访问控制等" << std::endl;

    // 启动服务器
    std::cout << "\n正在启动服务器..." << std::endl;
    std::cout << "服务器现在正在运行，监听端口 4840" << std::endl;
    std::cout << "观察控制台输出，可以看到各种日志信息" << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    
    server.run();
    
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 编译并运行此程序
 * 2. 程序将创建一个带有自定义日志系统的 OPC UA 服务器
 * 3. 服务器监听在端口 4840
 * 4. 所有服务器活动都会通过自定义日志器输出到控制台
 * 5. 观察不同级别和类别的日志信息
 * 
 * 日志系统工作原理：
 * 
 * 1. 日志级别：
 *    - 服务器根据事件的重要性选择日志级别
 *    - 可以通过配置过滤特定级别的日志
 *    - 不同级别用于不同的用途（调试、监控、错误处理等）
 * 
 * 2. 日志类别：
 *    - 标识日志的来源和类型
 *    - 便于分类管理和过滤
 *    - 帮助快速定位问题
 * 
 * 3. 日志格式：
 *    - 统一的格式便于解析和处理
 *    - 包含时间戳、级别、类别、消息等
 *    - 可以自定义格式满足特定需求
 * 
 * 扩展建议：
 * 
 * 1. 日志输出到文件：
 *    - 将日志写入文件而不是控制台
 *    - 支持日志文件轮转
 *    - 实现日志文件压缩和归档
 * 
 * 2. 日志过滤和级别控制：
 *    - 运行时动态调整日志级别
 *    - 按类别过滤日志
 *    - 支持正则表达式过滤
 * 
 * 3. 结构化日志：
 *    - 使用 JSON 格式输出日志
 *    - 包含更多元数据信息
 *    - 便于日志分析工具处理
 * 
 * 4. 日志聚合和分析：
 *    - 将日志发送到中央日志服务器
 *    - 实现日志搜索和分析
 *    - 支持日志告警和监控
 * 
 * 5. 性能优化：
 *    - 异步日志输出
 *    - 日志缓冲和批量写入
 *    - 日志采样和限流
 * 
 * 注意事项：
 * 
 * - 日志输出可能影响性能，特别是在高频率场景下
 * - 生产环境中应该配置适当的日志级别
 * - 日志文件可能占用大量磁盘空间
 * - 敏感信息不应该记录在日志中
 * 
 * 性能考虑：
 * 
 * - 字符串转换使用 constexpr 函数，编译时优化
 * - 日志函数使用 lambda 表达式，避免函数调用开销
 * - 可以根据需要禁用某些类别的日志
 * - 考虑使用条件编译控制日志输出
 * 
 * 安全考虑：
 * 
 * - 日志中不应包含密码、密钥等敏感信息
 * - 用户输入应该进行适当的过滤和转义
 * - 日志文件应该有适当的访问权限控制
 * - 考虑日志的审计和合规要求
 */
