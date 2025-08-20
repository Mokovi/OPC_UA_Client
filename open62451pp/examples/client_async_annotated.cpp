/**
 * @file client_async_annotated.cpp
 * @brief OPC UA 异步客户端示例 - 演示如何使用异步操作进行连接、读取、浏览和订阅
 * 
 * 本示例展示了 open62541pp 库中异步操作的使用方法，包括：
 * 1. 异步连接到服务器
 * 2. 异步读取变量值
 * 3. 异步浏览节点
 * 4. 异步创建订阅和监控项
 * 5. 使用回调函数处理异步操作结果
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/client.hpp>                    // 客户端核心功能
#include <open62541pp/services/attribute_highlevel.hpp>  // 高级属性服务
#include <open62541pp/services/monitoreditem.hpp>    // 监控项服务
#include <open62541pp/services/subscription.hpp>     // 订阅服务
#include <open62541pp/services/view.hpp>             // 视图服务

int main() {
    // 创建 OPC UA 客户端实例
    opcua::Client client;

    // 设置连接成功回调函数
    // 当客户端成功连接到服务器时，此函数会被调用
    client.onConnected([] { 
        std::cout << "客户端已连接到服务器" << std::endl; 
    });

    // 设置会话激活回调函数
    // 当客户端会话被激活时，此函数会被调用
    // 这是开始执行异步操作的最佳时机
    client.onSessionActivated([&] {
        std::cout << "会话已激活，开始执行异步操作" << std::endl;

        // 一旦客户端会话被激活，就安排异步操作
        // 1. 异步读取请求 - 读取服务器当前时间
        std::cout << "开始异步读取服务器时间..." << std::endl;
        opcua::services::readValueAsync(
            client,  // 客户端实例
            opcua::VariableId::Server_ServerStatus_CurrentTime,  // 要读取的变量节点ID
            [](opcua::Result<opcua::Variant>& result) {
                // 读取操作完成后的回调函数
                std::cout << "读取操作完成，状态码: " << result.code() << std::endl;
                
                // 检查操作是否成功
                if (result.hasValue()) {
                    // 从结果中提取 DateTime 类型的值
                    const auto dt = result.value().scalar<opcua::DateTime>();
                    // 格式化并输出时间
                    std::cout << "服务器时间 (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
                } else {
                    std::cout << "读取操作失败" << std::endl;
                }
            }
        );

        // 2. 异步浏览请求 - 浏览服务器对象
        std::cout << "开始异步浏览服务器对象..." << std::endl;
        const opcua::BrowseDescription bd{
            opcua::ObjectId::Server,                    // 从服务器对象开始浏览
            opcua::BrowseDirection::Forward,            // 向前浏览（查找引用）
            opcua::ReferenceTypeId::References          // 浏览所有引用类型
        };
        
        opcua::services::browseAsync(client, bd, 0, [](opcua::BrowseResult& result) {
            // 浏览操作完成后的回调函数
            std::cout << "浏览操作完成，找到 " << result.references().size() << " 个引用:\n";
            
            // 遍历所有找到的引用
            for (const auto& reference : result.references()) {
                std::cout << "- " << reference.browseName().name() << std::endl;
            }
        });

        // 3. 异步创建订阅
        std::cout << "开始异步创建订阅..." << std::endl;
        opcua::services::createSubscriptionAsync(
            client,                                     // 客户端实例
            opcua::SubscriptionParameters{},            // 使用默认订阅参数
            true,                                       // 启用发布
            {},                                         // 状态变化回调（此处为空）
            [](opcua::IntegerId subId) {
                // 订阅删除时的回调函数
                std::cout << "订阅已删除，订阅ID: " << subId << std::endl;
            },
            [&](opcua::CreateSubscriptionResponse& response) {
                // 订阅创建成功后的回调函数
                std::cout
                    << "订阅创建成功:\n"
                    << "- 状态码: " << response.responseHeader().serviceResult() << "\n"
                    << "- 订阅ID: " << response.subscriptionId() << std::endl;

                // 在订阅创建成功后，创建监控项来监控数据变化
                std::cout << "开始创建监控项..." << std::endl;
                opcua::services::createMonitoredItemDataChangeAsync(
                    client,                             // 客户端实例
                    response.subscriptionId(),           // 使用刚创建的订阅ID
                    opcua::ReadValueId{
                        opcua::VariableId::Server_ServerStatus_CurrentTime,  // 监控的变量
                        opcua::AttributeId::Value                             // 监控的属性（值）
                    },
                    opcua::MonitoringMode::Reporting,   // 监控模式：报告模式
                    opcua::MonitoringParametersEx{},    // 使用默认监控参数
                    
                    // 数据变化通知回调函数
                    [](opcua::IntegerId subId, opcua::IntegerId monId, const opcua::DataValue& dv) {
                        std::cout
                            << "收到数据变化通知:\n"
                            << "- 订阅ID: " << subId << "\n"
                            << "- 监控项ID: " << monId << "\n"
                            << "- 新值: " << opcua::toString(dv) << std::endl;
                    },
                    
                    {},  // 删除回调（此处为空）
                    
                    // 监控项创建完成回调函数
                    [](opcua::MonitoredItemCreateResult& result) {
                        std::cout
                            << "监控项创建完成:\n"
                            << "- 状态码: " << result.statusCode() << "\n"
                            << "- 监控项ID: " << result.monitoredItemId() << std::endl;
                    }
                );
            }
        );
    });

    // 设置会话关闭回调函数
    client.onSessionClosed([] { 
        std::cout << "会话已关闭" << std::endl; 
    });

    // 设置断开连接回调函数
    client.onDisconnected([] { 
        std::cout << "客户端已断开连接" << std::endl; 
    });

    // 异步连接到服务器
    // 使用默认的 OPC UA 服务器地址和端口
    std::cout << "正在异步连接到服务器 opc.tcp://localhost:4840..." << std::endl;
    client.connectAsync("opc.tcp://localhost:4840");
    
    // 运行客户端事件循环
    // 这将处理所有的异步操作和回调
    std::cout << "启动客户端事件循环..." << std::endl;
    client.run();
    
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 确保有一个 OPC UA 服务器在 localhost:4840 上运行
 * 2. 编译并运行此程序
 * 3. 程序将自动连接到服务器并执行以下操作：
 *    - 读取服务器当前时间
 *    - 浏览服务器对象结构
 *    - 创建订阅和监控项
 *    - 监控服务器时间的变化
 * 
 * 注意事项：
 * - 所有操作都是异步的，不会阻塞主线程
 * - 使用回调函数处理异步操作的结果
 * - 程序会一直运行直到手动停止或连接断开
 * - 如果连接失败，请检查服务器是否正在运行
 */
