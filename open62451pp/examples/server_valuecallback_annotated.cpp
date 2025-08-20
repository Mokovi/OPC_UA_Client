/**
 * @file server_valuecallback_annotated.cpp
 * @brief OPC UA 服务器值回调示例 - 演示如何使用值回调来拦截变量的读写操作
 * 
 * 本示例展示了 open62541pp 库中值回调系统的使用方法，包括：
 * 1. 创建自定义值回调类
 * 2. 实现读写操作拦截
 * 3. 在回调中修改变量值
 * 4. 将回调绑定到变量节点
 * 5. 实现动态数据更新
 * 
 * 功能说明：
 * - 拦截变量的读取操作
 * - 在读取前自动更新变量值
 * - 实现动态数据源
 * - 支持时间戳管理
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/node.hpp>      // 节点操作
#include <open62541pp/server.hpp>    // 服务器核心功能

/**
 * @brief 当前时间值回调类
 * 
 * 这个类继承自 ValueCallbackBase，用于拦截变量的读写操作。
 * 在每次读取操作时，自动将变量的值更新为当前时间。
 * 
 * 主要功能：
 * 1. 拦截读取操作
 * 2. 自动更新时间值
 * 3. 输出操作日志
 * 4. 支持时间戳管理
 */
class CurrentTimeCallback : public opcua::ValueCallbackBase {
    /**
     * @brief 读取操作回调函数
     * 
     * 当客户端读取变量时，此方法会被调用。
     * 在这里我们可以：
     * 1. 获取当前变量值
     * 2. 更新为最新时间
     * 3. 记录操作日志
     * 4. 处理异常情况
     * 
     * @param session 会话对象
     * @param id 节点ID
     * @param range 数值范围（此处未使用）
     * @param value 当前的数据值
     */
    void onRead(
        opcua::Session& session,
        const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        const opcua::DataValue& value
    ) override {
        // 创建节点对象，用于访问和修改变量
        opcua::Node node{session.connection(), id};
        
        // 获取当前存储的时间值
        const auto timeOld = value.value().scalar<opcua::DateTime>();
        
        // 获取当前系统时间
        const auto timeNow = opcua::DateTime::now();
        
        // 输出操作日志
        std::cout << "读取操作拦截 - 节点ID: " << opcua::toString(id) << std::endl;
        std::cout << "读取前的时间值: " << timeOld.format("%Y-%m-%d %H:%M:%S") << std::endl;
        std::cout << "设置当前时间值: " << timeNow.format("%Y-%m-%d %H:%M:%S") << std::endl;
        
        // 将变量的值更新为当前时间
        // 这样客户端每次读取都会得到最新的时间
        node.writeValue(opcua::Variant{timeNow});
        
        std::cout << "时间值更新完成" << std::endl;
    }

    /**
     * @brief 写入操作回调函数
     * 
     * 当客户端写入变量时，此方法会被调用。
     * 在这个示例中，我们不处理写入操作，但可以在这里添加：
     * 1. 值验证逻辑
     * 2. 写入权限检查
     * 3. 数据转换处理
     * 4. 审计日志记录
     * 
     * @param session 会话对象（此处未使用）
     * @param id 节点ID（此处未使用）
     * @param range 数值范围（此处未使用）
     * @param value 要写入的数据值（此处未使用）
     */
    void onWrite(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        [[maybe_unused]] const opcua::DataValue& value
    ) override {
        // 在这个示例中，我们不处理写入操作
        // 但可以在这里添加写入拦截逻辑
    }
};

int main() {
    std::cout << "=== OPC UA 服务器值回调示例 ===" << std::endl;
    
    // 创建 OPC UA 服务器实例
    opcua::Server server;

    std::cout << "正在创建当前时间变量节点..." << std::endl;
    
    // 定义当前时间变量的节点ID
    // 使用命名空间1，字符串标识符"CurrentTime"
    const opcua::NodeId currentTimeId{1, "CurrentTime"};
    
    // 在 Objects 文件夹下创建当前时间变量节点
    // 这个节点将用于演示值回调功能
    opcua::Node{server, opcua::ObjectId::ObjectsFolder}
        .addVariable(currentTimeId, "CurrentTime")                    // 创建变量节点
        .writeDisplayName({"en-US", "Current time"})                 // 设置英文显示名称
        .writeDescription({"en-US", "Current time"})                 // 设置英文描述
        .writeDataType<opcua::DateTime>()                            // 设置数据类型为 DateTime
        .writeValue(opcua::Variant{opcua::DateTime::now()});        // 设置初始值为当前时间

    std::cout << "当前时间变量节点创建完成！" << std::endl;
    std::cout << "节点ID: " << opcua::toString(currentTimeId) << std::endl;
    
    std::cout << "\n正在创建值回调实例..." << std::endl;
    
    // 创建值回调实例
    // 这个回调将拦截变量的读写操作
    CurrentTimeCallback currentTimeCallback;
    
    std::cout << "正在将值回调绑定到变量节点..." << std::endl;
    
    // 将值回调绑定到变量节点
    // 这样当客户端访问该节点时，就会触发我们的回调函数
    opcua::setVariableNodeValueCallback(server, currentTimeId, currentTimeCallback);
    
    std::cout << "值回调绑定完成！" << std::endl;
    
    std::cout << "\n=== 值回调功能说明 ===" << std::endl;
    std::cout << "1. 变量名称: CurrentTime" << std::endl;
    std::cout << "2. 数据类型: DateTime (日期时间)" << std::endl;
    std::cout << "3. 回调功能: 每次读取时自动更新为当前时间" << std::endl;
    std::cout << "4. 绑定状态: 已绑定到变量节点" << std::endl;
    
    std::cout << "\n=== 工作原理 ===" << std::endl;
    std::cout << "1. 客户端连接到服务器" << std::endl;
    std::cout << "2. 客户端读取 CurrentTime 变量" << std::endl;
    std::cout << "3. 服务器触发 onRead 回调" << std::endl;
    std::cout << "4. 回调函数将变量值更新为当前时间" << std::endl;
    std::cout << "5. 客户端获得最新的时间值" << std::endl;
    
    std::cout << "\n=== 测试说明 ===" << std::endl;
    std::cout << "1. 启动服务器后，使用 OPC UA 客户端工具连接" << std::endl;
    std::cout << "2. 浏览到 Objects/CurrentTime 节点" << std::endl;
    std::endl;
    std::cout << "3. 多次读取该节点的值" << std::endl;
    std::cout << "4. 每次读取都会得到不同的时间值" << std::endl;
    std::cout << "5. 观察控制台输出的回调执行信息" << std::endl;

    // 启动服务器
    std::cout << "\n正在启动服务器..." << std::endl;
    std::cout << "服务器现在正在运行，等待客户端连接..." << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    
    server.run();
    
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 编译并运行此程序
 * 2. 程序将创建一个带有值回调功能的 OPC UA 服务器
 * 3. 服务器监听在默认端口 4840
 * 4. 在 Objects 文件夹下创建一个名为 "CurrentTime" 的变量节点
 * 5. 该节点绑定了值回调，每次读取都会更新为当前时间
 * 
 * 值回调系统工作原理：
 * 
 * 1. 回调注册：
 *    - 创建回调类实例
 *    - 实现必要的回调方法
 *    - 将回调绑定到变量节点
 *    - 服务器自动调用回调
 * 
 * 2. 读取拦截：
 *    - 客户端读取请求被拦截
 *    - 调用 onRead 回调方法
 *    - 在回调中修改变量值
 *    - 返回修改后的值给客户端
 * 
 * 3. 写入拦截：
 *    - 客户端写入请求被拦截
 *    - 调用 onWrite 回调方法
 *    - 在回调中验证或修改值
 *    - 决定是否允许写入
 * 
 * 回调方法说明：
 * 
 * 1. onRead 方法：
 *    - 在读取操作时被调用
 *    - 可以修改返回给客户端的值
 *    - 适合实现动态数据源
 *    - 可以添加访问控制逻辑
 * 
 * 2. onWrite 方法：
 *    - 在写入操作时被调用
 *    - 可以验证或修改要写入的值
 *    - 适合实现数据验证
 *    - 可以添加审计日志记录
 * 
 * 值回调的应用场景：
 * 
 * 1. 动态数据源：
 *    - 实时传感器数据
 *    - 系统状态信息
 *    - 计算得出的值
 *    - 外部数据源集成
 * 
 * 2. 数据验证和转换：
 *    - 输入值验证
 *    - 数据类型转换
 *    - 范围检查
 *    - 格式标准化
 * 
 * 3. 访问控制和审计：
 *    - 权限检查
 *    - 操作日志记录
 *    - 数据加密/解密
 *    - 合规性检查
 * 
 * 4. 业务逻辑集成：
 *    - 数据计算
 *    - 状态同步
 *    - 事件触发
 *    - 工作流集成
 * 
 * 扩展建议：
 * 
 * 1. 添加更多回调类型：
 *    - 条件回调（满足条件时执行）
 *    - 异步回调（非阻塞执行）
 *    - 批量回调（一次处理多个操作）
 *    - 链式回调（回调触发其他回调）
 * 
 * 2. 改进回调管理：
 *    - 回调分组和分类
 *    - 回调优先级管理
 *    - 回调依赖关系处理
 *    - 动态回调注册/注销
 * 
 * 3. 添加回调监控：
 *    - 执行次数统计
 *    - 执行时间监控
 *    - 错误率统计
 *    - 性能分析
 * 
 * 4. 实现回调配置：
 *    - 从配置文件读取参数
 *    - 运行时动态配置
 *    - 回调参数验证
 *    - 条件回调配置
 * 
 * 注意事项：
 * 
 * - 回调函数应该快速执行，避免阻塞
 * - 长时间运行的任务应该使用异步处理
 * - 回调中的异常应该被适当处理
 * - 考虑回调的内存和资源管理
 * 
 * 性能考虑：
 * 
 * - 回调执行时间影响响应性能
 * - 大量回调可能影响内存使用
 * - 复杂回调逻辑需要优化
 * - 考虑使用回调缓存和池化
 * 
 * 安全考虑：
 * 
 * - 回调函数应该有适当的访问控制
 * - 避免在回调中执行危险操作
 * - 验证回调参数的有效性
 * - 考虑回调的审计和日志记录
 * 
 * 实际应用场景：
 * 
 * 1. 工业自动化：
 *    - 传感器数据采集
 *    - 设备状态监控
 *    - 报警条件检查
 *    - 数据质量验证
 * 
 * 2. 建筑自动化：
 *    - 环境参数监控
 *    - 设备控制逻辑
 *    - 能源管理计算
 *    - 安全系统集成
 * 
 * 3. 物联网系统：
 *    - 设备数据聚合
 *    - 边缘计算处理
 *    - 数据格式转换
 *    - 协议适配
 */
