/**
 * @file client_browse_annotated.cpp
 * @brief OPC UA 客户端浏览示例 - 演示如何浏览服务器的地址空间
 * 
 * 本示例展示了 open62541pp 库中客户端浏览功能的使用方法，包括：
 * 1. 连接到 OPC UA 服务器
 * 2. 浏览地址空间中的节点
 * 3. 获取节点的引用信息
 * 4. 处理浏览结果
 * 5. 递归浏览节点层次结构
 */

#include <iostream>
#include <string>

// 包含必要的头文件
#include <open62541pp/client.hpp>                    // 客户端核心功能
#include <open62541pp/services/attribute_highlevel.hpp>  // 高级属性服务
#include <open62541pp/services/view.hpp>             // 视图服务（浏览功能）

/**
 * @brief 递归浏览节点及其子节点
 * 
 * 这个函数递归地浏览指定节点下的所有子节点，展示地址空间的层次结构。
 * 通过递归调用，可以遍历整个地址空间树。
 * 
 * @param client OPC UA 客户端实例
 * @param nodeId 要浏览的节点ID
 * @param level 当前浏览的层次深度（用于缩进显示）
 * @param maxLevel 最大浏览深度，防止无限递归
 */
void browseNodeRecursively(
    const opcua::Client& client, 
    const opcua::NodeId& nodeId, 
    int level = 0, 
    int maxLevel = 3
) {
    // 检查是否达到最大浏览深度
    if (level >= maxLevel) {
        return;
    }
    
    // 创建缩进字符串，用于显示层次结构
    std::string indent(level * 2, ' ');
    
    try {
        // 创建浏览描述对象
        // 这个对象定义了我们要浏览的内容
        const opcua::BrowseDescription browseDesc{
            nodeId,                                    // 要浏览的节点ID
            opcua::BrowseDirection::Forward,           // 浏览方向：向前（查找引用）
            opcua::ReferenceTypeId::References,        // 引用类型：所有引用
            false,                                     // includeSubtypes：不包含子类型
            opcua::NodeId::null(),                     // nodeClassMask：所有节点类
            opcua::NodeId::null()                      // resultMask：所有结果掩码
        };
        
        // 执行浏览操作
        // 第三个参数是最大引用数量，0 表示不限制
        const auto browseResult = opcua::services::browse(client, browseDesc, 0);
        
        // 检查浏览操作是否成功
        if (browseResult.hasValue()) {
            const auto& references = browseResult.value().references();
            
            // 输出找到的引用数量
            std::cout << indent << "找到 " << references.size() << " 个引用:" << std::endl;
            
            // 遍历所有找到的引用
            for (const auto& reference : references) {
                // 获取引用的浏览名称
                const auto browseName = reference.browseName().name();
                
                // 获取引用的节点ID
                const auto& refNodeId = reference.nodeId();
                
                // 输出引用信息
                std::cout << indent << "  - " << browseName;
                
                // 如果节点ID有字符串标识符，也显示出来
                if (refNodeId.hasStringIdentifier()) {
                    std::cout << " (ID: " << refNodeId.identifier().string() << ")";
                }
                
                std::cout << std::endl;
                
                // 递归浏览子节点（增加层次深度）
                browseNodeRecursively(client, refNodeId, level + 1, maxLevel);
            }
        } else {
            // 浏览操作失败
            std::cout << indent << "浏览失败，状态码: " << browseResult.status() << std::endl;
        }
        
    } catch (const std::exception& e) {
        // 处理异常
        std::cout << indent << "浏览过程中发生异常: " << e.what() << std::endl;
    }
}

/**
 * @brief 浏览特定类型的节点
 * 
 * 这个函数演示如何浏览特定类型的节点，比如只浏览变量节点或对象节点。
 * 
 * @param client OPC UA 客户端实例
 * @param nodeId 要浏览的节点ID
 * @param nodeClass 要浏览的节点类
 */
void browseSpecificNodeClass(
    const opcua::Client& client, 
    const opcua::NodeId& nodeId, 
    opcua::NodeClass nodeClass
) {
    std::cout << "\n=== 浏览特定节点类 ===" << std::endl;
    
    try {
        // 创建浏览描述，指定节点类掩码
        const opcua::BrowseDescription browseDesc{
            nodeId,                                    // 要浏览的节点ID
            opcua::BrowseDirection::Forward,           // 浏览方向：向前
            opcua::ReferenceTypeId::References,        // 引用类型：所有引用
            false,                                     // 不包含子类型
            static_cast<opcua::NodeClassMask>(nodeClass),  // 只浏览指定节点类
            opcua::NodeId::null()                      // 所有结果掩码
        };
        
        // 执行浏览操作
        const auto browseResult = opcua::services::browse(client, browseDesc, 0);
        
        if (browseResult.hasValue()) {
            const auto& references = browseResult.value().references();
            
            // 获取节点类名称
            std::string nodeClassName;
            switch (nodeClass) {
                case opcua::NodeClass::Object: nodeClassName = "对象"; break;
                case opcua::NodeClass::Variable: nodeClassName = "变量"; break;
                case opcua::NodeClass::Method: nodeClassName = "方法"; break;
                case opcua::NodeClass::ObjectType: nodeClassName = "对象类型"; break;
                case opcua::NodeClass::VariableType: nodeClassName = "变量类型"; break;
                case opcua::NodeClass::ReferenceType: nodeClassName = "引用类型"; break;
                case opcua::NodeClass::DataType: nodeClassName = "数据类型"; break;
                case opcua::NodeClass::View: nodeClassName = "视图"; break;
                default: nodeClassName = "未知"; break;
            }
            
            std::cout << "找到 " << references.size() << " 个 " << nodeClassName << " 节点:" << std::endl;
            
            for (const auto& reference : references) {
                std::cout << "  - " << reference.browseName().name() << std::endl;
            }
        } else {
            std::cout << "浏览失败，状态码: " << browseResult.status() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "浏览过程中发生异常: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== OPC UA 客户端浏览示例 ===" << std::endl;
    
    // 创建 OPC UA 客户端实例
    opcua::Client client;
    
    try {
        // 连接到服务器
        std::cout << "正在连接到服务器 opc.tcp://localhost:4840..." << std::endl;
        client.connect("opc.tcp://localhost:4840");
        std::cout << "✓ 连接成功！" << std::endl;
        
        // 演示 1：浏览根节点
        std::cout << "\n1. 浏览根节点 (Objects 文件夹)" << std::endl;
        browseNodeRecursively(client, opcua::ObjectId::ObjectsFolder, 0, 2);
        
        // 演示 2：浏览服务器节点
        std::cout << "\n2. 浏览服务器节点" << std::endl;
        browseNodeRecursively(client, opcua::ObjectId::Server, 0, 2);
        
        // 演示 3：浏览特定类型的节点
        std::cout << "\n3. 浏览特定类型的节点" << std::endl;
        
        // 浏览变量节点
        browseSpecificNodeClass(client, opcua::ObjectId::ObjectsFolder, opcua::NodeClass::Variable);
        
        // 浏览对象节点
        browseSpecificNodeClass(client, opcua::ObjectId::ObjectsFolder, opcua::NodeClass::Object);
        
        // 演示 4：浏览服务器状态信息
        std::cout << "\n4. 浏览服务器状态信息" << std::endl;
        browseNodeRecursively(client, opcua::ObjectId::Server_ServerStatus, 0, 1);
        
        // 演示 5：获取节点的详细信息
        std::cout << "\n5. 获取节点详细信息" << std::endl;
        
        // 读取服务器时间节点的值
        try {
            const auto timeResult = opcua::services::readValue(
                client, 
                opcua::VariableId::Server_ServerStatus_CurrentTime
            );
            
            if (timeResult.hasValue()) {
                const auto dt = timeResult.value().scalar<opcua::DateTime>();
                std::cout << "服务器当前时间: " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "读取服务器时间失败: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "✗ 连接失败: " << e.what() << std::endl;
        return 1;
    }
    
    // 断开连接
    std::cout << "\n正在断开连接..." << std::endl;
    client.disconnect();
    std::cout << "✓ 已断开连接" << std::endl;
    
    std::cout << "\n=== 浏览示例完成 ===" << std::endl;
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 确保有一个 OPC UA 服务器在 localhost:4840 上运行
 * 2. 编译并运行此程序
 * 3. 程序将自动连接到服务器并执行以下操作：
 *    - 浏览 Objects 文件夹下的节点
 *    - 浏览服务器节点结构
 *    - 按类型浏览节点
 *    - 获取服务器状态信息
 * 
 * 浏览功能说明：
 * 
 * 1. 浏览方向：
 *    - Forward：向前浏览，查找引用
 *    - Backward：向后浏览，查找被引用
 *    - Both：双向浏览
 * 
 * 2. 引用类型：
 *    - References：所有引用
 *    - HierarchicalReferences：层次引用
 *    - Aggregates：聚合引用
 *    - Organizes：组织引用
 * 
 * 3. 节点类：
 *    - Object：对象节点
 *    - Variable：变量节点
 *    - Method：方法节点
 *    - ObjectType：对象类型节点
 *    - VariableType：变量类型节点
 *    - ReferenceType：引用类型节点
 *    - DataType：数据类型节点
 *    - View：视图节点
 * 
 * 注意事项：
 * 
 * - 浏览操作可能会返回大量数据，注意设置合理的限制
 * - 递归浏览时要注意设置最大深度，避免无限递归
 * - 某些节点可能没有访问权限，需要适当的错误处理
 * - 大型地址空间的浏览可能需要较长时间
 * 
 * 扩展建议：
 * 
 * 1. 添加过滤功能：
 *    - 按名称过滤
 *    - 按属性值过滤
 *    - 按数据类型过滤
 * 
 * 2. 实现分页浏览：
 *    - 分批获取结果
 *    - 支持"加载更多"功能
 * 
 * 3. 添加缓存机制：
 *    - 缓存浏览结果
 *    - 减少重复请求
 * 
 * 4. 实现异步浏览：
 *    - 使用异步 API
 *    - 支持进度回调
 */
