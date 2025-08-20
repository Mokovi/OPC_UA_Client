/**
 * @file client_subscription_annotated.cpp
 * @brief OPC UA 客户端订阅示例 - 演示如何创建和管理订阅以及监控项
 * 
 * 本示例展示了 open62541pp 库中客户端订阅系统的使用方法，包括：
 * 1. 创建和管理订阅
 * 2. 添加数据变化监控项
 * 3. 配置订阅和监控参数
 * 4. 处理数据变化通知
 * 5. 实现自动重连机制
 * 6. 信号处理和优雅关闭
 * 
 * 功能说明：
 * - 自动创建订阅和监控项
 * - 支持订阅参数配置
 * - 支持监控项参数配置
 * - 自动重连和会话恢复
 * - 优雅的信号处理
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

// 包含必要的头文件
#include <open62541pp/client.hpp>    // 客户端核心功能
#include <open62541pp/subscription.hpp>  // 订阅功能

// 全局运行状态标志，用于控制程序的运行和停止
// 使用 atomic 类型确保多线程环境下的安全性
inline static std::atomic<bool> isRunning = true;  // NOLINT(*global-variables)

/**
 * @brief 信号处理函数
 * 
 * 当程序接收到系统信号时，此函数会被调用。
 * 用于优雅地关闭程序，确保资源得到正确释放。
 * 
 * @param sig 接收到的信号类型
 */
static void signalHandler(int sig) noexcept {
    if (sig == SIGINT || sig == SIGTERM) {
        // SIGINT: Ctrl+C 中断信号
        // SIGTERM: 终止信号
        std::cout << "\n接收到信号 " << sig << "，正在优雅关闭..." << std::endl;
        isRunning = false;
    }
}

int main() {
    std::cout << "=== OPC UA 客户端订阅示例 ===" << std::endl;
    
    // 创建 OPC UA 客户端实例
    opcua::Client client;

    // 添加会话激活状态回调函数
    // 当客户端会话被激活时，此函数会被调用
    // 这是创建订阅和监控项的最佳时机
    client.onSessionActivated([&] {
        std::cout << "会话已激活，开始创建订阅和监控项..." << std::endl;
        
        // 创建订阅对象
        // 订阅是监控项的逻辑容器，管理发布间隔等参数
        opcua::Subscription sub{client};

        // 配置订阅参数
        // 这些参数影响数据发布的频率和行为
        opcua::SubscriptionParameters subscriptionParameters{};
        subscriptionParameters.publishingInterval = 1000.0;  // 发布间隔：1000 毫秒（1秒）
        sub.setSubscriptionParameters(subscriptionParameters);
        
        // 启用发布模式
        // 设置为 true 表示订阅开始发布数据
        sub.setPublishingMode(true);
        
        // 注意：可以通过以下方式删除订阅
        // sub.deleteSubscription();

        std::cout << "订阅创建完成，发布间隔: " << subscriptionParameters.publishingInterval << "ms" << std::endl;

        // 在订阅中创建数据变化监控项
        // 监控项用于监控特定节点的数据变化
        auto mon = sub.subscribeDataChange(
            opcua::VariableId::Server_ServerStatus_CurrentTime,  // 监控的节点ID：服务器当前时间
            opcua::AttributeId::Value,                          // 监控的属性：节点值
            [&](opcua::IntegerId subId, opcua::IntegerId monId, const opcua::DataValue& dv) {
                // 数据变化通知回调函数
                // 当监控的节点值发生变化时，此函数会被调用
                
                // 创建监控项对象，用于获取详细信息
                opcua::MonitoredItem item{client, subId, monId};
                
                // 输出数据变化通知的详细信息
                std::cout
                    << "收到数据变化通知:\n"
                    << "- 订阅ID:           " << item.subscriptionId() << "\n"
                    << "- 监控项ID:         " << item.monitoredItemId() << "\n"
                    << "- 节点ID:           " << opcua::toString(item.nodeId()) << "\n"
                    << "- 属性ID:           " << static_cast<int>(item.attributeId()) << "\n"
                    << "- 新值:             " << opcua::toString(dv) << std::endl;
            }
        );

        std::cout << "监控项创建完成" << std::endl;

        // 配置监控项参数
        // 这些参数影响数据采样的频率和行为
        opcua::MonitoringParametersEx monitoringParameters{};
        monitoringParameters.samplingInterval = 100.0;  // 采样间隔：100 毫秒（0.1秒）
        mon.setMonitoringParameters(monitoringParameters);
        
        // 设置监控模式为报告模式
        // 报告模式：当值发生变化时立即报告
        mon.setMonitoringMode(opcua::MonitoringMode::Reporting);
        
        // 注意：可以通过以下方式删除监控项
        // mon.deleteMonitoredItem();
        
        std::cout << "监控项配置完成，采样间隔: " << monitoringParameters.samplingInterval << "ms" << std::endl;
    });

    // 注册信号处理器
    // 用于捕获 Ctrl+C 等系统信号，实现优雅关闭
    std::signal(SIGINT, signalHandler);  // NOLINT

    std::cout << "正在启动客户端主循环..." << std::endl;
    std::cout << "按 Ctrl+C 停止程序" << std::endl;

    // 无限循环，自动重连到服务器
    // 如果连接断开，会自动尝试重新连接
    while (isRunning) {
        try {
            // 连接到服务器
            std::cout << "正在连接到服务器 opc.tcp://localhost:4840..." << std::endl;
            client.connect("opc.tcp://localhost:4840");
            std::cout << "✓ 连接成功！" << std::endl;
            
            // 运行客户端的主循环，处理回调和事件
            // 使用 runIterate 方法，每次迭代处理 100 毫秒的事件
            while (isRunning) {
                client.runIterate(100);
            }
            
        } catch (const opcua::BadStatus& e) {
            // 处理连接错误
            // 这是一个已知问题的临时解决方案
            // 参考：https://github.com/open62541pp/open62541pp/issues/51
            
            std::cout << "连接错误: " << e.what() << std::endl;
            
            // 强制断开连接，确保清理旧会话
            client.disconnect();
            
            std::cout << "3 秒后重试连接..." << std::endl;
            
            // 等待 3 秒后重试
            // 避免立即重连可能导致的资源竞争
            std::this_thread::sleep_for(std::chrono::seconds{3});
        }
    }

    std::cout << "程序正在关闭..." << std::endl;
    
    // 断开与服务器的连接
    try {
        client.disconnect();
        std::cout << "✓ 已断开连接" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "断开连接时发生错误: " << e.what() << std::endl;
    }
    
    std::cout << "=== 程序已退出 ===" << std::endl;
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 确保有一个 OPC UA 服务器在 localhost:4840 上运行
 * 2. 编译并运行此程序
 * 3. 程序将自动连接到服务器并创建订阅
 * 4. 监控服务器时间节点的变化
 * 5. 按 Ctrl+C 优雅地停止程序
 * 
 * 订阅系统工作原理：
 * 
 * 1. 订阅创建：
 *    - 在会话激活后自动创建订阅
 *    - 配置发布间隔等参数
 *    - 启用发布模式开始数据发布
 * 
 * 2. 监控项管理：
 *    - 在订阅中创建数据变化监控项
 *    - 配置采样间隔和监控模式
 *    - 设置数据变化通知回调
 * 
 * 3. 数据发布：
 *    - 服务器按发布间隔发送数据
 *    - 客户端接收并处理数据变化通知
 *    - 支持多种监控模式（报告、采样等）
 * 
 * 4. 连接管理：
 *    - 自动重连机制
 *    - 会话恢复和订阅重建
 *    - 错误处理和重试逻辑
 * 
 * 参数配置说明：
 * 
 * 1. 订阅参数：
 *    - publishingInterval: 发布间隔，影响数据更新频率
 *    - 较短的间隔提供更实时的数据，但增加网络负载
 *    - 较长的间隔减少网络负载，但数据更新较慢
 * 
 * 2. 监控项参数：
 *    - samplingInterval: 采样间隔，影响数据采样频率
 *    - 采样间隔应小于等于发布间隔
 *    - 监控模式决定何时发送数据变化通知
 * 
 * 扩展建议：
 * 
 * 1. 添加更多监控项：
 *    - 监控多个不同的节点
 *    - 支持不同类型的监控（数据变化、事件等）
 *    - 实现监控项的动态管理
 * 
 * 2. 改进错误处理：
 *    - 区分不同类型的错误
 *    - 实现指数退避重连策略
 *    - 添加错误统计和报告
 * 
 * 3. 优化性能：
 *    - 批量处理数据变化通知
 *    - 实现数据缓存和过滤
 *    - 支持条件监控和死区设置
 * 
 * 4. 添加配置管理：
 *    - 从配置文件读取参数
 *    - 支持运行时参数调整
 *    - 实现配置验证和默认值
 * 
 * 注意事项：
 * 
 * - 订阅和监控项在会话断开时会自动删除
 * - 重连后需要重新创建订阅和监控项
 * - 监控项的数量和频率会影响性能
 * - 生产环境中应该配置适当的参数
 * 
 * 性能考虑：
 * 
 * - 发布间隔影响网络负载和实时性
 * - 采样间隔影响数据精度和资源消耗
 * - 监控项数量影响内存和 CPU 使用
 * - 回调函数应该快速执行，避免阻塞
 * 
 * 安全考虑：
 * 
 * - 确保服务器支持所需的监控功能
 * - 验证节点访问权限
 * - 考虑数据加密和身份验证
 * - 监控异常访问和错误模式
 */
