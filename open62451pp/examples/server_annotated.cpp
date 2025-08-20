/**
 * @file server_annotated.cpp
 * @brief OPC UA 服务器示例 - 演示如何创建和配置服务器，添加节点，以及读写操作
 * 
 * 本示例展示了 open62541pp 库中服务器端的基本功能，包括：
 * 1. 创建和配置服务器
 * 2. 添加变量节点到地址空间
 * 3. 设置节点的显示名称和描述
 * 4. 读写节点值
 * 5. 运行服务器
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/node.hpp>      // 节点操作
#include <open62541pp/server.hpp>    // 服务器核心功能

int main() {
    // 创建服务器配置对象
    // 使用默认配置，包括默认端口和安全设置
    opcua::ServerConfig config;
    
    // 设置应用程序名称 - 这是服务器的标识名称
    config.setApplicationName("open62541pp 服务器示例");
    
    // 设置应用程序 URI - 这是服务器的唯一标识符
    // 通常使用 URN 格式，确保全局唯一性
    config.setApplicationUri("urn:open62541pp.server.application");
    
    // 设置产品 URI - 指向产品的官方网站或文档
    config.setProductUri("https://open62541pp.github.io");

    // 使用配置创建服务器实例
    // std::move 确保配置对象的所有权转移给服务器
    opcua::Server server{std::move(config)};

    // 获取 Objects 文件夹节点作为父节点
    // Objects 是 OPC UA 地址空间中的标准根节点
    opcua::Node parentNode{server, opcua::ObjectId::ObjectsFolder};
    
    // 在 Objects 文件夹下添加一个整数变量节点
    // 这个节点将存储一个著名的答案：42
    opcua::Node myIntegerNode = parentNode.addVariable(
        {1, "TheAnswer"},                    // 节点ID：命名空间1，标识符"TheAnswer"
        "The Answer",                        // 浏览名称：在地址空间中显示的名称
        opcua::VariableAttributes{}          // 变量属性对象
            .setDisplayName({"en-US", "The Answer"})           // 设置英文显示名称
            .setDescription({"en-US", "Answer to the Ultimate Question of Life"})  // 设置描述
            .setDataType<int>()              // 设置数据类型为整数
    );

    // 向节点写入值（属性）
    // 这里写入著名的答案：42
    // 在《银河系漫游指南》中，42 是生命、宇宙和一切的终极答案
    std::cout << "正在向节点写入值: 42" << std::endl;
    myIntegerNode.writeValue(opcua::Variant{42});

    // 从节点读取值（属性）
    // 验证写入的值是否正确
    std::cout << "正在从节点读取值..." << std::endl;
    const auto readValue = myIntegerNode.readValue();
    
    // 检查读取操作是否成功
    if (readValue.hasValue()) {
        // 将 Variant 类型转换为整数并输出
        const int answer = readValue.value().to<int>();
        std::cout << "读取成功！答案是: " << answer << std::endl;
        
        // 验证值是否正确
        if (answer == 42) {
            std::cout << "✓ 验证成功：读取的值与写入的值一致" << std::endl;
        } else {
            std::cout << "✗ 验证失败：读取的值与写入的值不一致" << std::endl;
        }
    } else {
        std::cout << "✗ 读取操作失败，状态码: " << readValue.status() << std::endl;
    }

    // 输出服务器信息
    std::cout << "\n=== 服务器信息 ===" << std::endl;
    std::cout << "应用程序名称: " << server.config().applicationName() << std::endl;
    std::cout << "应用程序 URI: " << server.config().applicationUri() << std::endl;
    std::cout << "产品 URI: " << server.config().productUri() << std::endl;
    std::cout << "服务器状态: 正在启动..." << std::endl;

    // 启动服务器
    // 这将使服务器开始监听连接请求
    // 服务器将一直运行直到程序被终止
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
 * 2. 程序将创建一个 OPC UA 服务器，默认监听在端口 4840
 * 3. 服务器将在 Objects 文件夹下创建一个名为 "TheAnswer" 的变量节点
 * 4. 该节点的值为 42，数据类型为整数
 * 5. 服务器将一直运行，等待客户端连接
 * 
 * 客户端连接测试：
 * - 可以使用其他 OPC UA 客户端工具连接到 localhost:4840
 * - 浏览地址空间，找到 Objects/TheAnswer 节点
 * - 读取该节点的值，应该得到 42
 * 
 * 注意事项：
 * - 服务器使用默认配置，没有启用安全功能
 * - 默认端口是 4840，这是 OPC UA 的标准端口
 * - 程序会一直运行直到手动停止
 * - 如果端口被占用，可能需要修改配置或停止其他服务
 * 
 * 扩展建议：
 * - 可以添加更多类型的节点（对象、方法、事件等）
 * - 可以实现动态数据源，使节点值随时间变化
 * - 可以添加访问控制和安全功能
 * - 可以实现自定义的数据类型和结构
 */
