/**
 * @file client_eventfilter_annotated.cpp
 * @brief OPC UA 客户端事件过滤器示例 - 演示如何使用事件过滤器来筛选和监控特定事件
 * 
 * 本示例展示了 open62541pp 库中客户端事件过滤器的使用方法，包括：
 * 1. 定义事件过滤器元素
 * 2. 组合多个过滤器条件
 * 3. 创建事件过滤器配置
 * 4. 订阅和监控过滤后的事件
 * 5. 处理事件通知和字段数据
 * 
 * 功能说明：
 * - 连接到运行中的事件服务器示例
 * - 定义基于事件类型和严重性的过滤器
 * - 使用逻辑运算符组合过滤器条件
 * - 实时接收和显示过滤后的事件
 * 
 * 注意：此示例应该在 `server_events` 运行时执行
 * 可以使用 UaExpert 等工具通过 `GenerateEvent` 方法节点触发事件
 * 
 * 事件过滤器工作原理：
 * - 事件过滤器由选择子句（Select Clause）和条件子句（Where Clause）组成
 * - 选择子句定义要返回的事件字段
 * - 条件子句定义事件筛选条件
 * - 支持复杂的逻辑表达式和属性比较
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/client.hpp>        // 客户端核心功能
#include <open62541pp/node.hpp>          // 节点操作
#include <open62541pp/subscription.hpp>  // 订阅管理

int main() {
    std::cout << "=== OPC UA 客户端事件过滤器示例 ===" << std::endl;
    
    // 创建 OPC UA 客户端实例
    opcua::Client client;
    
    std::cout << "正在连接到服务器..." << std::endl;
    std::cout << "服务器地址: opc.tcp://localhost:4840" << std::endl;
    
    // 连接到 OPC UA 服务器
    // 确保服务器示例 `server_events` 正在运行
    client.connect("opc.tcp://localhost:4840");
    std::cout << "✓ 连接成功！" << std::endl;
    
    std::cout << "\n正在配置事件过滤器..." << std::endl;
    
    // 定义过滤器元素1：基于事件类型过滤
    // 只接受 BaseEventType 类型的事件
    const opcua::ContentFilterElement filterBaseEventType{
        opcua::FilterOperator::OfType,  // 过滤操作符：类型匹配
        {
            // 操作数：BaseEventType 节点ID
            opcua::LiteralOperand{opcua::NodeId{opcua::ObjectTypeId::BaseEventType}},
        }
    };
    std::cout << "✓ 已创建事件类型过滤器：只接受 BaseEventType 事件" << std::endl;
    
    // 定义过滤器元素2：基于严重性过滤
    // 只接受严重性小于 200 的事件
    const opcua::ContentFilterElement filterLowSeverity{
        opcua::FilterOperator::LessThan,  // 过滤操作符：小于
        {
            // 第一个操作数：事件严重性属性
            opcua::SimpleAttributeOperand(
                opcua::ObjectTypeId::BaseEventType,  // 事件类型
                {{0, "Severity"}},                   // 属性路径：Severity 属性
                opcua::AttributeId::Value            // 属性ID：值属性
            ),
            // 第二个操作数：比较值 200
            opcua::LiteralOperand{200},
        }
    };
    std::cout << "✓ 已创建严重性过滤器：只接受严重性 < 200 的事件" << std::endl;
    
    // 组合过滤器：使用逻辑运算符
    // 结果：只接受 BaseEventType 类型且严重性 >= 200 的事件
    // 注意：!filterLowSeverity 表示严重性 >= 200
    const opcua::ContentFilter filterCombined = filterBaseEventType && !filterLowSeverity;
    
    std::cout << "✓ 已组合过滤器条件：" << std::endl;
    std::cout << "  - 事件类型：BaseEventType" << std::endl;
    std::cout << "  - 严重性：>= 200（高严重性事件）" << std::endl;
    
    // 创建事件过滤器配置
    // 事件过滤器包含选择子句和条件子句
    const opcua::EventFilter eventFilter{
        // 选择子句：定义要返回的事件字段
        {
            // 时间字段：事件发生的时间
            {opcua::ObjectTypeId::BaseEventType, {{0, "Time"}}, opcua::AttributeId::Value},
            // 严重性字段：事件的严重性级别
            {opcua::ObjectTypeId::BaseEventType, {{0, "Severity"}}, opcua::AttributeId::Value},
            // 消息字段：事件的描述消息
            {opcua::ObjectTypeId::BaseEventType, {{0, "Message"}}, opcua::AttributeId::Value},
        },
        // 条件子句：使用组合后的过滤器
        filterCombined
    };
    
    std::cout << "✓ 事件过滤器配置完成" << std::endl;
    std::cout << "选择字段：时间、严重性、消息" << std::endl;
    
    std::cout << "\n正在创建事件订阅..." << std::endl;
    
    // 创建订阅对象
    // 订阅用于接收服务器的事件通知
    opcua::Subscription sub{client};
    
    // 订阅事件：使用配置的事件过滤器
    // 监控 Server 对象上的事件
    sub.subscribeEvent(
        opcua::ObjectId::Server,  // 监控的节点：Server 对象
        eventFilter,               // 事件过滤器配置
        [&](opcua::IntegerId subId,           // 订阅ID
            opcua::IntegerId monId,           // 监控项ID
            opcua::Span<const opcua::Variant> eventFields) {  // 事件字段数据
            
            // 创建监控项对象，用于获取详细信息
            opcua::MonitoredItem item{client, subId, monId};
            
            std::cout << "\n=== 收到事件通知 ===" << std::endl;
            
            // 显示监控项的基本信息
            std::cout << "订阅ID:        " << item.subscriptionId() << std::endl;
            std::cout << "监控项ID:      " << item.monitoredItemId() << std::endl;
            std::cout << "节点ID:        " << opcua::toString(item.nodeId()) << std::endl;
            std::cout << "属性ID:        " << static_cast<int>(item.attributeId()) << std::endl;
            
            // 解析事件字段数据
            // 字段顺序与选择子句中定义的顺序一致
            
            // 第一个字段：时间
            const auto& time = eventFields.at(0).scalar<opcua::DateTime>();
            std::cout << "时间:          " << time.format("%Y-%m-%d %H:%M:%S") << std::endl;
            
            // 第二个字段：严重性
            const auto& severity = eventFields.at(1).scalar<uint16_t>();
            std::cout << "严重性:        " << severity << std::endl;
            
            // 第三个字段：消息
            const auto& message = eventFields.at(2).scalar<opcua::LocalizedText>();
            std::cout << "消息:          " << message.text() << std::endl;
            
            std::cout << "=== 事件通知结束 ===" << std::endl;
        }
    );
    
    std::cout << "✓ 事件订阅已创建" << std::endl;
    std::cout << "正在监控 Server 对象上的事件..." << std::endl;
    std::cout << "过滤器条件：BaseEventType 且严重性 >= 200" << std::endl;
    
    std::cout << "\n=== 使用说明 ===" << std::endl;
    std::cout << "1. 使用 UaExpert 或其他 OPC UA 客户端工具" << std::endl;
    std::cout << "2. 连接到服务器并找到 GenerateEvent 方法节点" << std::endl;
    std::cout << "3. 调用方法并设置严重性 >= 200 的参数" << std::endl;
    std::cout << "4. 观察此客户端程序的事件通知输出" << std::endl;
    
    std::cout << "\n=== 事件过滤器配置总结 ===" << std::endl;
    std::cout << "1. ✓ 事件类型过滤：BaseEventType" << std::endl;
    std::cout << "2. ✓ 严重性过滤：>= 200（高严重性）" << std::endl;
    std::cout << "3. ✓ 选择字段：时间、严重性、消息" << std::endl;
    std::cout << "4. ✓ 监控节点：Server 对象" << std::endl;
    
    std::cout << "\n正在启动客户端事件循环..." << std::endl;
    std::cout << "等待事件通知..." << std::endl;
    
    // 运行客户端的主循环来处理回调和事件
    // 这会阻塞直到调用 client.stop() 或抛出异常
    client.run();
    
    return 0;
}

/**
 * 事件过滤器系统详细说明：
 * 
 * 1. 过滤器架构：
 *    - 选择子句：定义要返回的事件字段
 *    - 条件子句：定义事件筛选条件
 *    - 支持复杂的逻辑表达式
 *    - 可组合多个过滤条件
 * 
 * 2. 过滤操作符：
 *    - OfType：类型匹配
 *    - LessThan：小于比较
 *    - GreaterThan：大于比较
 *    - Equals：相等比较
 *    - Not：逻辑非
 *    - And：逻辑与
 *    - Or：逻辑或
 * 
 * 3. 操作数类型：
 *    - LiteralOperand：字面量值
 *    - SimpleAttributeOperand：简单属性引用
 *    - ElementOperand：元素引用
 *    - AttributeOperand：属性引用
 * 
 * 事件过滤器设计原则：
 * 
 * 1. 性能优化：
 *    - 使用简单的过滤条件
 *    - 避免复杂的嵌套表达式
 *    - 合理选择监控节点
 *    - 限制选择字段数量
 * 
 * 2. 功能完整性：
 *    - 包含必要的事件信息
 *    - 支持业务逻辑需求
 *    - 提供足够的筛选精度
 *    - 保持过滤器的一致性
 * 
 * 3. 可维护性：
 *    - 使用清晰的命名
 *    - 模块化过滤器组件
 *    - 提供配置选项
 *    - 支持动态调整
 * 
 * 实际应用场景：
 * 
 * 1. 工业监控：
 *    - 高严重性报警事件
 *    - 设备状态变化事件
 *    - 安全相关事件
 *    - 维护提醒事件
 * 
 * 2. 系统管理：
 *    - 用户登录/登出事件
 *    - 权限变更事件
 *    - 配置修改事件
 *    - 系统错误事件
 * 
 * 3. 数据采集：
 *    - 数据质量异常事件
 *    - 采集完成事件
 *    - 存储空间警告事件
 *    - 网络连接事件
 * 
 * 4. 业务逻辑：
 *    - 订单状态变更事件
 *    - 库存变化事件
 *    - 支付处理事件
 *    - 客户交互事件
 * 
 * 性能优化建议：
 * 
 * 1. 过滤器优化：
 *    - 使用索引字段进行过滤
 *    - 避免字符串比较操作
 *    - 使用数值范围过滤
 *    - 实现过滤器缓存
 * 
 * 2. 网络优化：
 *    - 批量传输事件数据
 *    - 压缩事件字段内容
 *    - 实现事件优先级
 *    - 支持事件聚合
 * 
 * 3. 内存管理：
 *    - 限制事件队列大小
 *    - 及时清理过期事件
 *    - 使用对象池
 *    - 避免内存泄漏
 * 
 * 安全考虑：
 * 
 * 1. 访问控制：
 *    - 限制事件订阅权限
 *    - 验证过滤器配置
 *    - 审计事件访问日志
 *    - 实现事件脱敏
 * 
 * 2. 数据保护：
 *    - 加密敏感事件数据
 *    - 限制事件信息泄露
 *    - 保护事件元数据
 *    - 实现事件签名
 * 
 * 3. 网络安全：
 *    - 使用安全通道传输
 *    - 验证事件来源
 *    - 防止事件篡改
 *    - 实现事件认证
 * 
 * 调试和测试：
 * 
 * 1. 过滤器测试：
 *    - 测试各种过滤条件
 *    - 验证逻辑表达式
 *    - 检查边界条件
 *    - 测试异常情况
 * 
 * 2. 性能测试：
 *    - 测量过滤延迟
 *    - 监控内存使用
 *    - 测试并发性能
 *    - 分析瓶颈点
 * 
 * 3. 兼容性测试：
 *    - 测试不同客户端
 *    - 验证事件格式
 *    - 检查字段映射
 *    - 测试版本兼容性
 * 
 * 扩展建议：
 * 
 * 1. 添加更多过滤条件：
 *    - 时间范围过滤
 *    - 地理位置过滤
 *    - 用户权限过滤
 *    - 自定义属性过滤
 * 
 * 2. 实现动态过滤器：
 *    - 运行时修改过滤条件
 *    - 支持过滤器模板
 *    - 实现过滤器继承
 *    - 支持过滤器组合
 * 
 * 3. 添加过滤器管理：
 *    - 过滤器配置持久化
 *    - 过滤器版本管理
 *    - 过滤器性能监控
 *    - 过滤器使用统计
 * 
 * 4. 实现高级功能：
 *    - 事件模式匹配
 *    - 事件序列检测
 *    - 事件关联分析
 *    - 事件预测功能
 */
