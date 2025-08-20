/**
 * @file client_method_annotated.cpp
 * @brief OPC UA 客户端方法调用示例 - 演示如何从客户端调用服务器上的方法
 * 
 * 本示例展示了 open62541pp 库中客户端方法调用的使用方法，包括：
 * 1. 连接到 OPC UA 服务器
 * 2. 浏览方法节点
 * 3. 调用远程方法
 * 4. 处理方法参数和返回值
 * 5. 错误处理和连接管理
 * 
 * 功能说明：
 * - 连接到运行中的服务器方法示例
 * - 调用 Greet 方法并传入参数
 * - 显示方法的返回结果
 * - 演示客户端-服务器方法交互
 * 
 * 注意：此示例需要服务器示例 `server_method` 正在运行
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/client.hpp>    // 客户端核心功能
#include <open62541pp/node.hpp>      // 节点操作

int main() {
    std::cout << "=== OPC UA 客户端方法调用示例 ===" << std::endl;
    
    // 创建 OPC UA 客户端实例
    opcua::Client client;
    
    std::cout << "正在连接到服务器..." << std::endl;
    std::cout << "服务器地址: opc.tcp://localhost:4840" << std::endl;
    
    try {
        // 连接到 OPC UA 服务器
        // 确保服务器示例 `server_method` 正在运行
        client.connect("opc.tcp://localhost:4840");
        std::cout << "✓ 连接成功！" << std::endl;
        
        std::cout << "\n正在浏览方法节点..." << std::endl;
        
        // 浏览 Objects 文件夹节点
        // 这是 OPC UA 地址空间中的标准根对象
        opcua::Node objectsNode{client, opcua::ObjectId::ObjectsFolder};
        
        // 浏览 Greet 方法节点
        // 使用 browseChild 方法找到指定名称的子节点
        // 参数 {{1, "Greet"}} 表示命名空间1，名称为"Greet"的节点
        const opcua::Node greetMethodNode = objectsNode.browseChild({{1, "Greet"}});
        
        std::cout << "✓ 找到 Greet 方法节点" << std::endl;
        std::cout << "方法节点ID: " << opcua::toString(greetMethodNode.id()) << std::endl;
        
        std::cout << "\n正在调用 Greet 方法..." << std::endl;
        std::cout << "输入参数: \"World\"" << std::endl;
        
        // 从父节点（Objects）调用方法
        // 这是 OPC UA 的标准做法：从父对象调用子方法
        const auto result = objectsNode.callMethod(
            greetMethodNode.id(),           // 要调用的方法节点ID
            {opcua::Variant{"World"}}      // 输入参数：包含字符串"World"的 Variant
        );
        
        std::cout << "✓ 方法调用成功！" << std::endl;
        
        // 获取并显示方法的输出参数
        // result.outputArguments() 返回输出参数的 Span
        // at(0) 获取第一个输出参数
        // scalar<opcua::String>() 将参数转换为字符串类型
        const auto greeting = result.outputArguments().at(0).scalar<opcua::String>();
        
        std::cout << "方法返回结果: " << greeting << std::endl;
        
        std::cout << "\n=== 方法调用总结 ===" << std::endl;
        std::cout << "1. 成功连接到服务器" << std::endl;
        std::cout << "2. 找到 Greet 方法节点" << std::endl;
        std::cout << "3. 调用方法，传入参数 \"World\"" << std::endl;
        std::cout << "4. 接收到问候语: " << greeting << std::endl;
        
    } catch (const opcua::BadStatus& e) {
        // 处理 OPC UA 特定的错误
        std::cout << "✗ OPC UA 错误: " << e.what() << std::endl;
        std::cout << "请确保服务器示例正在运行" << std::endl;
        return 1;
        
    } catch (const std::exception& e) {
        // 处理其他异常
        std::cout << "✗ 连接错误: " << e.what() << std::endl;
        std::cout << "请检查网络连接和服务器状态" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    std::cout << "客户端方法调用演示成功完成" << std::endl;
    
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 确保服务器示例 `server_method` 正在运行
 * 2. 编译并运行此程序
 * 3. 程序将连接到服务器并调用 Greet 方法
 * 4. 显示方法的返回结果
 * 
 * 方法调用工作原理：
 * 
 * 1. 连接建立：
 *    - 创建客户端实例
 *    - 连接到指定的服务器地址
 *    - 建立安全通道和会话
 * 
 * 2. 节点浏览：
 *    - 从根节点开始浏览
 *    - 找到目标方法节点
 *    - 获取方法的元数据信息
 * 
 * 3. 方法调用：
 *    - 准备输入参数
 *    - 发送方法调用请求
 *    - 等待服务器处理
 *    - 接收返回结果
 * 
 * 方法调用流程：
 * 
 * 1. 客户端准备：
 *    - 创建方法调用请求
 *    - 设置输入参数
 *    - 指定目标方法
 * 
 * 2. 网络传输：
 *    - 将请求序列化
 *    - 通过安全通道发送
 *    - 等待服务器响应
 * 
 * 3. 服务器处理：
 *    - 接收方法调用请求
 *    - 验证参数和权限
 *    - 执行方法逻辑
 *    - 准备返回结果
 * 
 * 4. 结果返回：
 *    - 服务器发送响应
 *    - 客户端接收结果
 *    - 解析输出参数
 *    - 处理返回数据
 * 
 * 参数处理说明：
 * 
 * 1. 输入参数：
 *    - 使用 Variant 包装数据
 *    - 支持多种数据类型
 *    - 自动类型转换
 * 
 * 2. 输出参数：
 *    - 从结果对象中提取
 *    - 使用适当的类型转换
 *    - 处理多个返回值
 * 
 * 3. 参数验证：
 *    - 检查参数数量
 *    - 验证数据类型
 *    - 处理空值情况
 * 
 * 错误处理策略：
 * 
 * 1. 连接错误：
 *    - 网络连接失败
 *    - 服务器不可达
 *    - 超时处理
 * 
 * 2. 方法调用错误：
 *    - 方法不存在
 *    - 参数类型不匹配
 *    - 权限不足
 * 
 * 3. 运行时错误：
 *    - 服务器内部错误
 *    - 方法执行失败
 *    - 资源不足
 * 
 * 扩展建议：
 * 
 * 1. 添加更多方法调用：
 *    - 调用不同的方法
 *    - 测试各种参数类型
 *    - 验证错误处理
 * 
 * 2. 改进用户界面：
 *    - 交互式参数输入
 *    - 实时结果显示
 *    - 调用历史记录
 * 
 * 3. 实现批量调用：
 *    - 同时调用多个方法
 *    - 异步方法调用
 *    - 调用队列管理
 * 
 * 4. 添加性能监控：
 *    - 调用响应时间
 *    - 成功率统计
 *    - 网络延迟分析
 * 
 * 实际应用场景：
 * 
 * 1. 工业控制：
 *    - 设备启动/停止命令
 *    - 参数配置设置
 *    - 状态查询操作
 *    - 报警确认处理
 * 
 * 2. 数据采集：
 *    - 触发数据采集
 *    - 配置采集参数
 *    - 查询采集状态
 *    - 导出采集数据
 * 
 * 3. 系统管理：
 *    - 用户权限管理
 *    - 系统配置更新
 *    - 日志查询操作
 *    - 备份恢复操作
 * 
 * 注意事项：
 * 
 * - 确保服务器正在运行且可访问
 * - 检查网络连接和防火墙设置
 * - 验证用户权限和访问控制
 * - 处理网络延迟和超时情况
 * 
 * 性能考虑：
 * 
 * - 方法调用频率影响网络负载
 *    - 大量参数增加传输时间
 *    - 复杂方法可能执行较慢
 *    - 考虑使用异步调用
 * 
 * 安全考虑：
 * 
 * - 验证服务器身份和证书
 *    - 使用适当的加密级别
 *    - 检查用户访问权限
 *    - 记录方法调用日志
 * 
 * 调试技巧：
 * 
 * 1. 启用详细日志：
 *    - 查看网络通信
 *    - 监控方法调用
 *    - 分析错误信息
 * 
 * 2. 使用 OPC UA 客户端工具：
 *    - 手动测试方法调用
 *    - 验证参数格式
 *    - 检查返回结果
 * 
 * 3. 逐步测试：
 *    - 先测试简单方法
 *    - 逐步增加复杂度
 *    - 验证边界情况
 */
