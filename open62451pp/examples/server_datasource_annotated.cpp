/**
 * @file server_datasource_annotated.cpp
 * @brief OPC UA 服务器数据源示例 - 演示如何实现自定义的数据源来管理变量节点的数据
 * 
 * 本示例展示了 open62541pp 库中数据源系统的使用方法，包括：
 * 1. 创建自定义数据源类
 * 2. 实现数据读取和写入逻辑
 * 3. 将数据源绑定到变量节点
 * 4. 处理时间戳和数据值
 * 5. 管理内部数据存储
 * 
 * 功能说明：
 * - 数据源内部存储指定类型的数据
 * - 支持读取和写入操作
 * - 自动生成时间戳
 * - 可以绑定到任何变量节点
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/server.hpp>                    // 服务器核心功能
#include <open62541pp/services/nodemanagement.hpp>   // 节点管理服务

/**
 * @brief 模板化数据源类
 * 
 * 这是一个通用的数据源模板类，可以存储和管理任意类型 T 的数据。
 * 继承自 DataSourceBase，实现了必要的数据读取和写入接口。
 * 
 * 数据源的主要作用：
 * 1. 为变量节点提供数据存储
 * 2. 处理客户端的读取请求
 * 3. 处理客户端的写入请求
 * 4. 管理数据的时间戳
 * 
 * @tparam T 数据类型，如 int、double、string 等
 */
template <typename T>
struct DataSource : public opcua::DataSourceBase {
    /**
     * @brief 读取数据源中的数据
     * 
     * 当客户端读取变量节点时，此方法会被调用。
     * 将内部存储的数据设置到 DataValue 对象中，并可选地设置时间戳。
     * 
     * @param session 会话对象（此处未使用）
     * @param id 节点ID（此处未使用）
     * @param range 数值范围（此处未使用）
     * @param dv 数据值对象，用于返回数据
     * @param timestamp 是否设置时间戳
     * @return 状态码
     */
    opcua::StatusCode read(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        opcua::DataValue& dv,
        bool timestamp
    ) override {
        // 输出读取操作的调试信息
        std::cout << "从数据源读取值: " << data << std::endl;
        
        // 将内部存储的数据设置到 DataValue 中
        dv.setValue(opcua::Variant{data});
        
        // 如果请求时间戳，则设置当前时间作为源时间戳
        if (timestamp) {
            dv.setSourceTimestamp(opcua::DateTime::now());
            std::cout << "已设置源时间戳: " << opcua::toString(dv.sourceTimestamp()) << std::endl;
        }
        
        // 返回成功状态码
        return UA_STATUSCODE_GOOD;
    }

    /**
     * @brief 写入数据到数据源
     * 
     * 当客户端写入变量节点时，此方法会被调用。
     * 从 DataValue 对象中提取数据，并存储到内部存储中。
     * 
     * @param session 会话对象（此处未使用）
     * @param id 节点ID（此处未使用）
     * @param range 数值范围（此处未使用）
     * @param dv 包含要写入数据的 DataValue 对象
     * @return 状态码
     */
    opcua::StatusCode write(
        [[maybe_unused]] opcua::Session& session,
        [[maybe_unused]] const opcua::NodeId& id,
        [[maybe_unused]] const opcua::NumericRange* range,
        const opcua::DataValue& dv
    ) override {
        // 从 DataValue 中提取数据并存储到内部存储
        data = dv.value().to<T>();
        
        // 输出写入操作的调试信息
        std::cout << "向数据源写入值: " << data << std::endl;
        
        // 返回成功状态码
        return UA_STATUSCODE_GOOD;
    }

    // 内部数据存储
    T data{};
};

int main() {
    std::cout << "=== OPC UA 服务器数据源示例 ===" << std::endl;
    
    // 创建 OPC UA 服务器实例
    // 使用默认配置，包括默认端口和安全设置
    opcua::Server server;

    std::cout << "正在创建变量节点..." << std::endl;
    
    // 使用节点管理服务添加一个变量节点
    // 这个节点将绑定到自定义数据源
    const auto id = opcua::services::addVariable(
        server,                                    // 服务器实例
        opcua::ObjectId::ObjectsFolder,           // 父节点：Objects 文件夹
        {1, 1000},                                // 节点ID：命名空间1，标识符1000
        "DataSource",                             // 浏览名称
        opcua::VariableAttributes{}               // 变量属性
            .setAccessLevel(UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE)  // 读写权限
            .setDataType<int>(),                                                  // 数据类型：整数
        opcua::VariableTypeId::BaseDataVariableType,                             // 变量类型：基础数据变量
        opcua::ReferenceTypeId::HasComponent                                     // 引用类型：HasComponent
    ).value();  // 获取返回的节点ID

    std::cout << "变量节点创建成功，节点ID: " << opcua::toString(id) << std::endl;

    // 定义数据源实例
    // 这里使用整数类型的数据源，存储 32 位整数数据
    std::cout << "正在创建数据源..." << std::endl;
    DataSource<int> dataSource;
    
    // 设置初始值
    dataSource.data = 42;
    std::cout << "数据源初始值设置为: " << dataSource.data << std::endl;

    // 将数据源绑定到变量节点
    // 这样当客户端访问该节点时，就会使用我们的自定义数据源
    std::cout << "正在绑定数据源到变量节点..." << std::endl;
    opcua::setVariableNodeValueBackend(server, id, dataSource);
    
    std::cout << "数据源绑定完成！" << std::endl;
    std::cout << "\n=== 数据源功能说明 ===" << std::endl;
    std::cout << "1. 数据源内部存储一个整数值" << std::endl;
    std::cout << "2. 当客户端读取节点时，返回存储的值" << std::endl;
    std::cout << "3. 当客户端写入节点时，更新存储的值" << std::endl;
    std::cout << "4. 读取操作会自动生成时间戳" << std::endl;
    std::cout << "5. 所有操作都会在控制台输出调试信息" << std::endl;
    
    std::cout << "\n=== 测试说明 ===" << std::endl;
    std::cout << "1. 启动服务器后，使用 OPC UA 客户端工具连接" << std::endl;
    std::cout << "2. 浏览到 Objects/DataSource 节点" << std::endl;
    std::cout << "3. 尝试读取节点值，应该得到 42" << std::endl;
    std::cout << "4. 尝试写入新的值，比如 100" << std::endl;
    std::cout << "5. 再次读取，应该得到新写入的值" << std::endl;
    std::cout << "6. 观察控制台输出的数据源操作信息" << std::endl;

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
 * 2. 程序将创建一个带有自定义数据源的 OPC UA 服务器
 * 3. 服务器监听在默认端口 4840
 * 4. 在 Objects 文件夹下创建一个名为 "DataSource" 的变量节点
 * 5. 该节点绑定到自定义的数据源，支持读写操作
 * 
 * 数据源工作原理：
 * 
 * 1. 数据存储：
 *    - 数据源内部维护一个数据成员
 *    - 支持任意类型的数据（通过模板参数指定）
 *    - 数据在服务器运行期间持久存在
 * 
 * 2. 读取操作：
 *    - 客户端读取节点时触发 read 方法
 *    - 返回内部存储的数据
 *    - 可选地设置源时间戳
 * 
 * 3. 写入操作：
 *    - 客户端写入节点时触发 write 方法
 *    - 更新内部存储的数据
 *    - 支持类型转换和验证
 * 
 * 4. 时间戳管理：
 *    - 自动生成当前时间作为源时间戳
 *    - 可以用于数据质量监控
 *    - 支持历史数据记录
 * 
 * 扩展建议：
 * 
 * 1. 支持更多数据类型：
 *    - 浮点数、字符串、布尔值等
 *    - 数组和结构体类型
 *    - 自定义数据类型
 * 
 * 2. 添加数据验证：
 *    - 范围检查
 *    - 类型验证
 *    - 业务规则验证
 * 
 * 3. 实现数据持久化：
 *    - 文件存储
 *    - 数据库存储
 *    - 配置管理
 * 
 * 4. 添加数据统计：
 *    - 读写次数统计
 *    - 性能监控
 *    - 错误统计
 * 
 * 5. 支持数据源链：
 *    - 多个数据源组合
 *    - 数据转换和过滤
 *    - 数据聚合和计算
 * 
 * 注意事项：
 * 
 * - 数据源的生命周期与服务器绑定
 * - 多线程访问时需要考虑线程安全
 * - 大量数据时需要考虑内存管理
 * - 复杂数据类型需要实现序列化
 * 
 * 性能考虑：
 * 
 * - 简单数据类型操作很快
 * - 复杂数据类型可能需要内存分配
 * - 时间戳生成有一定开销
 * - 可以考虑缓存和优化
 */
