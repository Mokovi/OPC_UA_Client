/**
 * @file client_method_async_annotated.cpp
 * @brief OPC UA 客户端异步方法调用示例 - 演示如何使用异步方式调用服务器方法
 * 
 * 本示例展示了 open62541pp 库中客户端异步方法调用的使用方法，包括：
 * 1. 使用 Future 模式的异步方法调用
 * 2. 使用回调模式的异步方法调用
 * 3. 在独立线程中运行客户端事件循环
 * 4. 异步操作的结果处理和状态检查
 * 
 * 功能说明：
 * - 连接到运行中的服务器方法示例
 * - 浏览并找到 Greet 方法节点
 * - 使用两种不同的异步调用方式
 * - 演示异步操作的生命周期管理
 * 
 * 注意：此示例需要服务器示例 `server_method` 正在运行
 * 
 * 异步方法调用工作原理：
 * - 异步调用不会阻塞主线程
 * - 支持 Future 和回调两种模式
 * - 需要在独立线程中运行客户端事件循环
 * - 提供非阻塞的方法调用体验
 */

#include <iostream>
#include <thread>

// 包含必要的头文件
#include <open62541pp/client.hpp>    // 客户端核心功能
#include <open62541pp/node.hpp>      // 节点操作

int main() {
    std::cout << "=== OPC UA 客户端异步方法调用示例 ===" << std::endl;
    
    // 创建 OPC UA 客户端实例
    opcua::Client client;
    
    std::cout << "正在连接到服务器..." << std::endl;
    std::cout << "服务器地址: opc.tcp://localhost:4840" << std::endl;
    
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
    
    std::cout << "\n正在启动客户端事件循环..." << std::endl;
    
    // 在独立线程中运行客户端事件循环
    // 这是异步操作的必要条件：客户端需要在后台处理网络事件
    // 使用 std::thread 创建新线程，避免阻塞主线程
    auto clientThread = std::thread([&] { 
        std::cout << "客户端事件循环线程已启动" << std::endl;
        client.run();  // 启动客户端事件循环
    });
    
    std::cout << "✓ 客户端事件循环已在独立线程中启动" << std::endl;
    
    // 等待一小段时间确保事件循环完全启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n=== 异步方法调用演示 ===" << std::endl;
    
    // 方式1：使用 Future 模式的异步方法调用
    // Future 模式提供同步风格的异步编程体验
    std::cout << "\n1. Future 模式异步调用..." << std::endl;
    {
        std::cout << "正在调用 Greet 方法，参数: \"Future World\"" << std::endl;
        
        // 使用 callMethodAsync 进行异步方法调用
        // opcua::useFuture 表示使用 Future 模式
        auto future = objectsNode.callMethodAsync(
            greetMethodNode.id(),                    // 要调用的方法节点ID
            {opcua::Variant{"Future World"}},       // 输入参数：包含字符串"Future World"的 Variant
            opcua::useFuture                         // 使用 Future 模式
        );
        
        std::cout << "异步调用已启动，等待操作完成..." << std::endl;
        
        // 等待异步操作完成
        // future.wait() 会阻塞当前线程直到操作完成
        future.wait();
        
        std::cout << "✓ Future 已就绪，获取方法输出" << std::endl;
        
        // 获取异步操作的结果
        auto result = future.get();
        
        // 检查操作状态码
        std::cout << "操作状态码: " << result.statusCode() << std::endl;
        
        // 获取并显示方法的输出参数
        // result.outputArguments() 返回输出参数的 Span
        // [0] 获取第一个输出参数
        // scalar<opcua::String>() 将参数转换为字符串类型
        std::cout << "方法返回结果: " << result.outputArguments()[0].scalar<opcua::String>() << std::endl;
        
        std::cout << "✓ Future 模式异步调用完成" << std::endl;
    }
    
    // 方式2：使用回调模式的异步方法调用
    // 回调模式提供事件驱动的异步编程体验
    std::cout << "\n2. 回调模式异步调用..." << std::endl;
    {
        std::cout << "正在调用 Greet 方法，参数: \"Callback World\"" << std::endl;
        
        // 使用 callMethodAsync 进行异步方法调用
        // 第三个参数是回调函数，在操作完成时被调用
        objectsNode.callMethodAsync(
            greetMethodNode.id(),                    // 要调用的方法节点ID
            {opcua::Variant{"Callback World"}},     // 输入参数：包含字符串"Callback World"的 Variant
            [](opcua::CallMethodResult& result) {   // 回调函数：处理异步操作结果
                std::cout << "✓ 回调函数被调用，操作完成！" << std::endl;
                
                // 检查操作状态码
                std::cout << "操作状态码: " << result.statusCode() << std::endl;
                
                // 获取并显示方法的输出参数
                std::cout << "方法返回结果: " << result.outputArguments()[0].scalar<opcua::String>() << std::endl;
            }
        );
        
        std::cout << "异步调用已启动，回调函数将在操作完成时被调用" << std::endl;
        
        // 等待一小段时间让回调函数有机会执行
        // 在实际应用中，这里通常会有更复杂的等待逻辑
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "✓ 回调模式异步调用完成" << std::endl;
    }
    
    std::cout << "\n=== 异步调用总结 ===" << std::endl;
    std::cout << "1. ✓ Future 模式：同步风格的异步编程" << std::endl;
    std::cout << "2. ✓ 回调模式：事件驱动的异步编程" << std::endl;
    std::cout << "3. ✓ 两种模式都成功调用了 Greet 方法" << std::endl;
    std::cout << "4. ✓ 异步操作不会阻塞主线程" << std::endl;
    
    std::cout << "\n正在停止客户端..." << std::endl;
    
    // 停止客户端事件循环
    // 这会通知事件循环线程退出
    client.stop();
    
    // 等待客户端线程完成
    // 确保所有异步操作都已完成
    clientThread.join();
    
    std::cout << "✓ 客户端已停止，线程已清理" << std::endl;
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    std::cout << "客户端异步方法调用演示成功完成" << std::endl;
    
    return 0;
}

/**
 * 异步方法调用详细说明：
 * 
 * 1. 异步调用模式：
 *    - Future 模式：提供同步风格的异步编程体验
 *    - 回调模式：提供事件驱动的异步编程体验
 *    - 两种模式都支持非阻塞的方法调用
 * 
 * 2. 事件循环管理：
 *    - 异步操作需要在事件循环中处理
 *    - 使用独立线程运行事件循环
 *    - 主线程可以继续执行其他任务
 * 
 * 3. 生命周期管理：
 *    - 异步操作的生命周期由客户端管理
 *    - 需要正确停止客户端和清理线程
 *    - 确保所有操作都已完成
 * 
 * Future 模式详解：
 * 
 * 1. 操作流程：
 *    - 调用 callMethodAsync 启动异步操作
 *    - 返回 Future 对象
 *    - 使用 wait() 等待操作完成
 *    - 使用 get() 获取操作结果
 * 
 * 2. 优点：
 *    - 代码结构清晰，易于理解
 *    - 支持异常传播
 *    - 可以组合多个异步操作
 * 
 * 3. 适用场景：
 *    - 需要等待操作完成的场景
 *    - 需要处理操作结果的场景
 *    - 需要异常处理的场景
 * 
 * 回调模式详解：
 * 
 * 1. 操作流程：
 *    - 调用 callMethodAsync 启动异步操作
 *    - 传入回调函数
 *    - 操作完成时自动调用回调函数
 *    - 在回调函数中处理结果
 * 
 * 2. 优点：
 *    - 非阻塞，立即返回
 *    - 适合事件驱动的架构
 *    - 可以处理多个并发操作
 * 
 * 3. 适用场景：
 *    - 不需要等待操作完成的场景
 *    - 事件驱动的应用架构
 *    - 需要处理多个并发操作的场景
 * 
 * 线程管理策略：
 * 
 * 1. 事件循环线程：
 *    - 负责处理网络事件和异步操作
 *    - 使用独立线程避免阻塞主线程
 *    - 需要正确启动和停止
 * 
 * 2. 主线程：
 *    - 负责业务逻辑和用户交互
 *    - 可以启动多个异步操作
 *    - 需要等待操作完成或处理回调
 * 
 * 3. 线程同步：
 *    - 使用 Future 进行线程同步
 *    - 使用回调函数处理异步结果
 *    - 确保线程安全的数据访问
 * 
 * 性能优化建议：
 * 
 * 1. 线程池管理：
 *    - 使用线程池管理多个客户端
 *    - 避免为每个客户端创建新线程
 *    - 实现线程复用和负载均衡
 * 
 * 2. 异步操作批处理：
 *    - 批量启动多个异步操作
 *    - 使用 Future 组合等待多个操作
 *    - 减少线程切换开销
 * 
 * 3. 内存管理：
 *    - 避免在回调函数中分配大量内存
 *    - 使用对象池减少内存分配
 *    - 及时清理不再需要的对象
 * 
 * 错误处理策略：
 * 
 * 1. 连接错误：
 *    - 检查网络连接状态
 *    - 实现自动重连机制
 *    - 处理连接超时和失败
 * 
 * 2. 方法调用错误：
 *    - 检查方法调用状态码
 *    - 处理参数验证错误
 *    - 实现重试机制
 * 
 * 3. 异步操作错误：
 *    - 在 Future 中捕获异常
 *    - 在回调函数中处理错误
 *    - 实现错误日志和监控
 * 
 * 实际应用场景：
 * 
 * 1. 工业自动化：
 *    - 批量设备控制命令
 *    - 实时数据采集
 *    - 系统状态监控
 * 
 * 2. 数据采集系统：
 *    - 多传感器数据读取
 *    - 历史数据查询
 *    - 数据导出操作
 * 
 * 3. 监控系统：
 *    - 多设备状态检查
 *    - 报警信息处理
 *    - 报表生成
 * 
 * 4. 测试系统：
 *    - 并发测试用例
 *    - 性能测试
 *    - 压力测试
 * 
 * 调试和测试：
 * 
 * 1. 日志记录：
 *    - 记录异步操作的生命周期
 *    - 监控操作状态和结果
 *    - 跟踪线程活动
 * 
 * 2. 性能监控：
 *    - 测量异步操作响应时间
 *    - 监控线程使用情况
 *    - 分析内存使用模式
 * 
 * 3. 错误诊断：
 *    - 记录详细的错误信息
 *    - 实现错误分类和统计
 *    - 提供错误恢复建议
 * 
 * 扩展建议：
 * 
 * 1. 添加更多异步操作：
 *    - 异步读取和写入
 *    - 异步浏览和订阅
 *    - 异步事件处理
 * 
 * 2. 实现操作队列：
 *    - 管理多个异步操作
 *    - 实现操作优先级
 *    - 支持操作取消
 * 
 * 3. 添加超时处理：
 *    - 设置操作超时时间
 *    - 实现超时回调
 *    - 处理超时错误
 * 
 * 4. 实现重试机制：
 *    - 自动重试失败的操作
 *    - 指数退避策略
 *    - 最大重试次数限制
 */
