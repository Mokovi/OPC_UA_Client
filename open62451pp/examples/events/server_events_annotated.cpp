/**
 * @file server_events_annotated.cpp
 * @brief OPC UA 服务器事件示例 - 演示如何在服务器中创建、配置和触发自定义事件
 * 
 * 本示例展示了 open62541pp 库中服务器事件系统的使用方法，包括：
 * 1. 启用事件通知功能
 * 2. 创建自定义事件对象
 * 3. 添加事件生成方法
 * 4. 配置事件属性和参数
 * 5. 动态触发事件
 * 
 * 功能说明：
 * - 在 Objects 文件夹上启用事件订阅功能
 * - 创建自定义事件对象，包含时间、严重性和消息属性
 * - 添加 GenerateEvent 方法，允许客户端触发事件
 * - 演示事件系统的完整工作流程
 * 
 * 事件系统工作原理：
 * - 事件是 OPC UA 中用于通知客户端状态变化的重要机制
 * - 事件可以包含多种属性，如时间戳、严重性、消息等
 * - 客户端可以订阅事件并接收实时通知
 * - 事件可以手动触发或由系统自动生成
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/event.hpp>     // 事件系统功能
#include <open62541pp/node.hpp>      // 节点操作
#include <open62541pp/server.hpp>    // 服务器核心功能

int main() {
    std::cout << "=== OPC UA 服务器事件示例 ===" << std::endl;
    
    // 创建 OPC UA 服务器实例
    opcua::Server server;
    
    std::cout << "正在配置事件系统..." << std::endl;
    
    // 获取 Objects 文件夹节点
    // 这是 OPC UA 地址空间中的标准根对象，客户端可以订阅此对象上的事件
    opcua::Node objectsNode{server, opcua::ObjectId::ObjectsFolder};
    
    // 在 Objects 文件夹上启用事件通知功能
    // EventNotifier::SubscribeToEvents 表示此节点可以订阅事件
    // 这是 OPC UA 事件系统的基础配置
    objectsNode.writeEventNotifier(opcua::EventNotifier::SubscribeToEvents);
    
    std::cout << "✓ 已在 Objects 文件夹上启用事件通知" << std::endl;
    std::cout << "客户端现在可以订阅此节点上的事件" << std::endl;
    
    // 创建自定义事件对象
    // 事件对象定义了事件的属性和结构
    // 客户端可以通过订阅来接收这些事件
    opcua::Event event{server};
    
    std::cout << "\n正在创建事件生成方法..." << std::endl;
    
    // 添加一个方法来生成和触发事件
    // 这个方法允许客户端动态触发事件，用于测试和演示
    objectsNode.addMethod(
        {1, 1000},  // 方法节点ID：命名空间1，标识符1000
        "GenerateEvent",  // 方法名称
        [&](opcua::Span<const opcua::Variant> input, opcua::Span<opcua::Variant>) {
            // 方法实现：从输入参数中提取事件属性并触发事件
            
            std::cout << "正在生成事件..." << std::endl;
            
            // 从输入参数中提取严重性级别
            // 严重性用于表示事件的重要程度（0-1000）
            const auto severity = input.at(0).scalar<uint16_t>();
            
            // 从输入参数中提取事件消息
            // 消息描述事件的具体内容
            const auto& message = input.at(1).scalar<opcua::String>();
            
            std::cout << "事件严重性: " << severity << std::endl;
            std::cout << "事件消息: " << message << std::endl;
            
            // 设置事件的时间戳为当前时间
            // 时间戳表示事件发生的确切时间
            event.writeTime(opcua::DateTime::now());
            
            // 设置事件的严重性级别
            // 严重性越高，事件越重要
            event.writeSeverity(severity);
            
            // 设置事件的消息内容
            // 消息提供事件的详细描述
            event.writeMessage({"", message});
            
            // 触发事件
            // 这会通知所有订阅此事件的客户端
            event.trigger();
            
            std::cout << "✓ 事件已成功触发！" << std::endl;
            std::cout << "所有订阅客户端将收到事件通知" << std::endl;
        },
        // 输入参数定义
        {
            // 第一个参数：严重性级别
            {"severity", {"", "Severity"}, opcua::DataTypeId::UInt16, opcua::ValueRank::Scalar},
            // 第二个参数：事件消息
            {"message", {"", "Message"}, opcua::DataTypeId::String, opcua::ValueRank::Scalar},
        },
        // 输出参数定义（此方法无输出参数）
        {}
    );
    
    std::cout << "✓ 事件生成方法创建完成！" << std::endl;
    std::cout << "方法名称: GenerateEvent" << std::endl;
    std::cout << "方法ID: {1, 1000}" << std::endl;
    
    std::cout << "\n=== 事件系统配置完成 ===" << std::endl;
    std::cout << "1. ✓ 事件通知功能已启用" << std::endl;
    std::cout << "2. ✓ 自定义事件对象已创建" << std::endl;
    std::cout << "3. ✓ 事件生成方法已添加" << std::endl;
    std::cout << "4. ✓ 事件属性已配置（时间、严重性、消息）" << std::endl;
    
    std::cout << "\n=== 使用方法 ===" << std::endl;
    std::cout << "1. 客户端可以订阅 Objects 文件夹上的事件" << std::endl;
    std::cout << "2. 客户端可以调用 GenerateEvent 方法来触发事件" << std::endl;
    std::cout << "3. 事件将包含时间戳、严重性和消息信息" << std::endl;
    std::cout << "4. 所有订阅客户端将实时收到事件通知" << std::endl;
    
    std::cout << "\n=== 事件参数说明 ===" << std::endl;
    std::cout << "严重性 (Severity):" << std::endl;
    std::cout << "  - 0-100: 信息级别事件" << std::endl;
    std::cout << "  - 101-500: 警告级别事件" << std::endl;
    std::cout << "  - 501-1000: 错误级别事件" << std::endl;
    std::cout << "消息 (Message): 事件的详细描述文本" << std::endl;
    
    std::cout << "\n正在启动服务器..." << std::endl;
    std::cout << "服务器地址: opc.tcp://localhost:4840" << std::endl;
    std::cout << "事件系统已就绪，等待客户端连接..." << std::endl;
    
    // 启动服务器并进入事件循环
    // 服务器将处理客户端连接、事件订阅和事件触发
    server.run();
    
    return 0;
}

/**
 * 事件系统详细说明：
 * 
 * 1. 事件通知机制：
 *    - EventNotifier 属性控制节点是否支持事件订阅
 *    - SubscribeToEvents 表示客户端可以订阅此节点上的事件
 *    - 客户端通过订阅来接收事件通知
 * 
 * 2. 事件对象结构：
 *    - 事件是特殊的对象类型，继承自 BaseEventType
 *    - 包含标准属性：时间、严重性、消息、源等
 *    - 可以添加自定义属性来扩展事件信息
 * 
 * 3. 事件触发流程：
 *    - 设置事件属性（时间、严重性、消息等）
 *    - 调用 trigger() 方法触发事件
 *    - 系统通知所有订阅客户端
 *    - 客户端接收事件数据
 * 
 * 4. 事件订阅管理：
 *    - 客户端创建订阅（Subscription）
 *    - 添加监控项（MonitoredItem）来监控事件
 *    - 配置事件过滤器来筛选感兴趣的事件
 *    - 接收事件通知并处理
 * 
 * 事件属性详解：
 * 
 * 1. 时间 (Time):
 *    - 表示事件发生的确切时间
 *    - 使用 OPC UA DateTime 类型
 *    - 客户端可以根据时间戳进行事件排序和分析
 * 
 * 2. 严重性 (Severity):
 *    - 表示事件的重要程度
 *    - 范围：0-1000
 *    - 帮助客户端确定事件的优先级
 * 
 * 3. 消息 (Message):
 *    - 事件的详细描述
 *    - 使用本地化字符串
 *    - 提供人类可读的事件信息
 * 
 * 4. 源 (Source):
 *    - 事件的来源节点
 *    - 帮助客户端了解事件的上下文
 *    - 可以用于事件路由和过滤
 * 
 * 事件过滤和路由：
 * 
 * 1. 事件过滤器：
 *    - 基于事件类型进行过滤
 *    - 基于属性值进行过滤
 *    - 基于时间范围进行过滤
 *    - 减少不必要的网络传输
 * 
 * 2. 事件路由：
 *    - 将事件发送到特定的客户端组
 *    - 基于客户端权限进行路由
 *    - 支持事件的分层分发
 * 
 * 3. 事件聚合：
 *    - 将多个相似事件合并
 *    - 减少事件风暴
 *    - 提高系统性能
 * 
 * 性能优化建议：
 * 
 * 1. 事件频率控制：
 *    - 限制事件生成频率
 *    - 使用事件缓冲和批处理
 *    - 实现智能事件抑制
 * 
 * 2. 网络优化：
 *    - 压缩事件数据
 *    - 使用高效的序列化格式
 *    - 实现事件优先级队列
 * 
 * 3. 内存管理：
 *    - 限制事件队列大小
 *    - 及时清理过期事件
 *    - 使用对象池减少分配
 * 
 * 实际应用场景：
 * 
 * 1. 工业自动化：
 *    - 设备状态变化通知
 *    - 报警和异常事件
 *    - 维护提醒和计划
 *    - 质量控制和检测结果
 * 
 * 2. 建筑自动化：
 *    - 环境参数变化
 *    - 设备故障报警
 *    - 能源使用统计
 *    - 安全事件通知
 * 
 * 3. 物联网应用：
 *    - 传感器数据异常
 *    - 设备连接状态
 *    - 数据采集完成
 *    - 系统配置变更
 * 
 * 4. 数据监控：
 *    - 数据质量异常
 *    - 阈值越限报警
 *    - 趋势变化检测
 *    - 统计分析结果
 * 
 * 安全考虑：
 * 
 * 1. 访问控制：
 *    - 限制事件订阅权限
 *    - 验证事件生成权限
 *    - 审计事件操作日志
 * 
 * 2. 数据保护：
 *    - 加密敏感事件数据
 *    - 限制事件信息泄露
 *    - 实现事件数据脱敏
 * 
 * 3. 网络安全：
 *    - 使用安全通道传输事件
 *    - 验证客户端身份
 *    - 防止事件数据篡改
 * 
 * 调试和测试：
 * 
 * 1. 事件日志：
 *    - 记录所有事件生成
 *    - 跟踪事件订阅状态
 *    - 监控事件传输性能
 * 
 * 2. 测试工具：
 *    - 使用 OPC UA 客户端工具测试事件
 *    - 验证事件属性设置
 *    - 测试事件过滤功能
 * 
 * 3. 性能监控：
 *    - 测量事件生成延迟
 *    - 监控网络带宽使用
 *    - 分析事件处理性能
 * 
 * 扩展建议：
 * 
 * 1. 添加更多事件类型：
 *    - 系统事件（启动、关闭、配置变更）
 *    - 用户事件（登录、注销、权限变更）
 *    - 业务事件（订单、支付、库存变更）
 * 
 * 2. 实现事件持久化：
 *    - 将事件存储到数据库
 *    - 支持事件历史查询
 *    - 实现事件回放功能
 * 
 * 3. 添加事件统计：
 *    - 事件频率统计
 *    - 事件类型分布
 *    - 客户端订阅统计
 * 
 * 4. 实现事件转发：
 *    - 将事件转发到其他系统
 *    - 支持多种传输协议
 *    - 实现事件负载均衡
 */
