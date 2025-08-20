/**
 * @file client_find_servers_annotated.cpp
 * @brief OPC UA 客户端查找服务器示例 - 演示如何发现和获取 OPC UA 服务器信息
 * 
 * 本示例展示了 open62541pp 库中服务器发现功能的使用方法，包括：
 * 1. 查找网络中的 OPC UA 服务器
 * 2. 获取服务器的详细信息
 * 3. 获取服务器的端点信息
 * 4. 分析安全配置和身份验证选项
 * 5. 处理服务器发现结果
 * 
 * 功能说明：
 * - 自动发现网络中的 OPC UA 服务器
 * - 获取服务器的应用程序信息
 * - 分析服务器的安全配置
 * - 支持多种安全模式和策略
 */

#include <iostream>
#include <string_view>

// 包含必要的头文件
#include <open62541pp/client.hpp>    // 客户端核心功能

/**
 * @brief 将应用程序类型转换为可读的字符串
 * 
 * 这个函数将 OPC UA 的应用程序类型枚举值转换为人类可读的字符串，
 * 便于在输出中识别不同类型的应用程序。
 * 
 * @param applicationType 应用程序类型枚举值
 * @return 对应的字符串表示
 */
constexpr std::string_view toString(opcua::ApplicationType applicationType) {
    switch (applicationType) {
    case opcua::ApplicationType::Server:
        return "服务器";              // 纯服务器应用程序
    case opcua::ApplicationType::Client:
        return "客户端";              // 纯客户端应用程序
    case opcua::ApplicationType::ClientAndServer:
        return "客户端和服务器";      // 既是客户端又是服务器的应用程序
    case opcua::ApplicationType::DiscoveryServer:
        return "发现服务器";          // 专门用于服务器发现的应用程序
    default:
        return "未知类型";            // 未识别的应用程序类型
    }
}

/**
 * @brief 将消息安全模式转换为可读的字符串
 * 
 * 这个函数将 OPC UA 的消息安全模式枚举值转换为人类可读的字符串，
 * 便于在输出中识别不同的安全配置。
 * 
 * @param securityMode 消息安全模式枚举值
 * @return 对应的字符串表示
 */
constexpr std::string_view toString(opcua::MessageSecurityMode securityMode) {
    switch (securityMode) {
    case opcua::MessageSecurityMode::Invalid:
        return "无效";                // 无效的安全模式
    case opcua::MessageSecurityMode::None:
        return "无安全保护";          // 不提供安全保护
    case opcua::MessageSecurityMode::Sign:
        return "仅签名";              // 仅提供消息签名
    case opcua::MessageSecurityMode::SignAndEncrypt:
        return "签名和加密";          // 提供消息签名和加密
    default:
        return "无效的安全模式";      // 未识别的安全模式
    }
}

int main() {
    std::cout << "=== OPC UA 客户端查找服务器示例 ===" << std::endl;
    
    // 创建 OPC UA 客户端实例
    opcua::Client client;
    
    // 禁用客户端日志记录，避免干扰输出
    // 在生产环境中，可以根据需要启用日志记录
    client.config().setLogger([](auto&&...) {});

    std::cout << "正在查找网络中的 OPC UA 服务器..." << std::endl;
    std::cout << "搜索地址: opc.tcp://localhost:4840" << std::endl;
    
    // 在指定的网络地址上查找 OPC UA 服务器
    // 这个操作会发送发现请求，查找可用的服务器
    const auto servers = client.findServers("opc.tcp://localhost:4840");
    
    std::cout << "找到 " << servers.size() << " 个服务器" << std::endl;
    
    // 遍历所有找到的服务器
    size_t serverIndex = 0;
    for (const auto& server : servers) {
        const auto& name = server.applicationUri();
        
        std::cout << "\n=== 服务器 [" << serverIndex++ << "] ===" << std::endl;
        std::cout << "名称:             " << server.applicationName().text() << std::endl;
        std::cout << "应用程序 URI:     " << server.applicationUri() << std::endl;
        std::cout << "产品 URI:         " << server.productUri() << std::endl;
        std::cout << "应用程序类型:     " << toString(server.applicationType()) << std::endl;
        
        // 获取服务器的发现 URL 列表
        // 这些 URL 用于获取服务器的端点信息
        const auto discoveryUrls = server.discoveryUrls();
        std::cout << "发现 URL 列表:" << std::endl;
        
        if (discoveryUrls.empty()) {
            std::cout << "  未提供发现 URL，跳过端点搜索" << std::endl;
        } else {
            // 输出所有发现 URL
            for (const auto& url : discoveryUrls) {
                std::cout << "  - " << url << std::endl;
            }
            
            std::cout << "\n正在获取端点信息..." << std::endl;
            
            // 遍历每个发现 URL，获取服务器的端点信息
            for (const auto& url : discoveryUrls) {
                size_t endpointIndex = 0;
                
                // 获取指定 URL 上的所有端点
                // 端点包含连接和安全配置信息
                for (const auto& endpoint : client.getEndpoints(url)) {
                    std::cout << "\n  --- 端点 [" << endpointIndex++ << "] ---" << std::endl;
                    std::cout << "  端点 URL:        " << endpoint.endpointUrl() << std::endl;
                    std::cout << "  传输配置文件:   " << endpoint.transportProfileUri() << std::endl;
                    std::cout << "  安全模式:       " << toString(endpoint.securityMode()) << std::endl;
                    std::cout << "  安全策略:       " << endpoint.securityPolicyUri() << std::endl;
                    std::cout << "  安全级别:       " << endpoint.securityLevel() << std::endl;
                    
                    // 获取端点支持的用户身份令牌类型
                    const auto& tokens = endpoint.userIdentityTokens();
                    std::cout << "  用户身份令牌:" << std::endl;
                    
                    if (tokens.empty()) {
                        std::cout << "    无" << std::endl;
                    } else {
                        for (const auto& token : tokens) {
                            std::cout << "    - " << token.policyId() << std::endl;
                        }
                    }
                }
            }
        }
    }
    
    if (servers.empty()) {
        std::cout << "\n未找到任何 OPC UA 服务器" << std::endl;
        std::cout << "请确保：" << std::endl;
        std::cout << "1. 有 OPC UA 服务器在 localhost:4840 上运行" << std::endl;
        std::cout << "2. 网络连接正常" << std::endl;
        std::cout << "3. 防火墙允许 OPC UA 通信" << std::endl;
    } else {
        std::cout << "\n=== 服务器发现完成 ===" << std::endl;
        std::cout << "总共发现 " << servers.size() << " 个服务器" << std::endl;
        std::cout << "每个服务器都提供了详细的连接和安全信息" << std::endl;
    }
    
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 编译并运行此程序
 * 2. 程序将自动搜索网络中的 OPC UA 服务器
 * 3. 显示找到的服务器详细信息
 * 4. 分析每个服务器的端点和安全配置
 * 
 * 服务器发现工作原理：
 * 
 * 1. 发现请求：
 *    - 向指定的网络地址发送发现请求
 *    - 等待服务器响应
 *    - 收集服务器信息
 * 
 * 2. 服务器信息：
 *    - 应用程序名称和 URI
 *    - 产品信息
 *    - 应用程序类型
 *    - 发现 URL 列表
 * 
 * 3. 端点信息：
 *    - 连接 URL
 *    - 传输配置文件
 *    - 安全配置
 *    - 身份验证选项
 * 
 * 安全模式说明：
 * 
 * 1. 无安全保护 (None)：
 *    - 不提供任何安全保护
 *    - 适用于受信任的网络环境
 *    - 性能最好，但安全性最低
 * 
 * 2. 仅签名 (Sign)：
 *    - 提供消息完整性保护
 *    - 防止消息被篡改
 *    - 不提供机密性保护
 * 
 * 3. 签名和加密 (SignAndEncrypt)：
 *    - 提供完整的消息保护
 *    - 包括完整性和机密性
 *    - 安全性最高，但性能开销较大
 * 
 * 应用程序类型说明：
 * 
 * 1. 服务器 (Server)：
 *    - 纯服务器应用程序
 *    - 提供 OPC UA 服务
 *    - 不主动连接其他服务器
 * 
 * 2. 客户端 (Client)：
 *    - 纯客户端应用程序
 *    - 连接到其他服务器
 *    - 不提供 OPC UA 服务
 * 
 * 3. 客户端和服务器 (ClientAndServer)：
 *    - 混合应用程序
 *    - 既可以作为客户端连接其他服务器
 *    - 也可以作为服务器提供服务
 * 
 * 4. 发现服务器 (DiscoveryServer)：
 *    - 专门用于服务器发现
 *    - 维护服务器注册表
 *    - 帮助客户端找到可用的服务器
 * 
 * 扩展建议：
 * 
 * 1. 添加网络扫描功能：
 *    - 扫描多个网络地址
 *    - 支持 IP 范围扫描
 *    - 实现并发发现
 * 
 * 2. 改进结果过滤：
 *    - 按安全模式过滤
 *    - 按应用程序类型过滤
 *    - 按安全级别过滤
 * 
 * 3. 添加连接测试：
 *    - 测试端点连接性
 *    - 验证安全配置
 *    - 测试身份验证
 * 
 * 4. 实现结果缓存：
 *    - 缓存发现结果
 *    - 定期更新信息
 *    - 减少重复发现
 * 
 * 注意事项：
 * 
 * - 服务器发现需要网络访问权限
 * - 某些防火墙可能阻止发现请求
 * - 发现结果可能因网络状况而变化
 * - 生产环境中应该考虑安全策略
 * 
 * 性能考虑：
 * 
 * - 发现操作可能需要较长时间
 * - 网络延迟影响响应时间
 * - 大量服务器可能增加处理时间
 * - 可以考虑异步发现操作
 * 
 * 安全考虑：
 * 
 * - 验证服务器证书的有效性
 *    - 检查安全策略的适用性
 *    - 考虑网络环境的安全性
 *    - 避免在不安全网络中传输敏感信息
 */
