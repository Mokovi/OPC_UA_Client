/**
 * @file server_instantiation_annotated.cpp
 * @brief OPC UA 服务器实例化示例 - 演示如何创建对象类型和实例化对象
 * 
 * 本示例展示了 open62541pp 库中对象类型和实例化系统的使用方法，包括：
 * 1. 创建自定义对象类型
 * 2. 定义对象类型的层次结构
 * 3. 为对象类型添加变量
 * 4. 设置建模规则
 * 5. 实例化对象并设置属性值
 * 
 * 功能说明：
 * - 创建哺乳动物类型层次结构
 * - 演示继承和类型关系
 * - 自动创建实例的变量
 * - 使用建模规则控制实例化行为
 */

#include <open62541pp/node.hpp>
#include <open62541pp/server.hpp>

int main() {
    std::cout << "=== OPC UA 服务器实例化示例 ===" << std::endl;
    
    // 创建 OPC UA 服务器实例
    opcua::Server server;

    std::cout << "正在创建对象类型层次结构..." << std::endl;
    
    // 创建对象类型 `MammalType` 和 `DogType`：
    // (ObjectType) BaseObjectType
    // └─ (ObjectType) MammalType
    //    ├─ (Variable) Age
    //    └─ (ObjectType) DogType
    //       └─ (Variable) Name
    
    // 获取基础对象类型节点作为父节点
    // BaseObjectType 是 OPC UA 中所有对象类型的根类型
    opcua::Node nodeBaseObjectType{server, opcua::ObjectTypeId::BaseObjectType};
    
    std::cout << "1. 创建 MammalType 对象类型..." << std::endl;
    
    // 在 BaseObjectType 下创建 MammalType 对象类型
    // 这个类型将作为哺乳动物的基类
    auto nodeMammalType = nodeBaseObjectType.addObjectType(
        {1, 10000},                    // 节点ID：命名空间1，标识符10000
        "MammalType",                  // 浏览名称
        opcua::ObjectTypeAttributes{}  // 对象类型属性
            .setDisplayName({"en-US", "MammalType"})           // 英文显示名称
            .setDescription({"en-US", "A mammal"})             // 英文描述
    );

    std::cout << "2. 为 MammalType 添加 Age 变量..." << std::endl;
    
    // 为 MammalType 添加 Age 变量
    // 这个变量将存储哺乳动物的年龄
    nodeMammalType
        .addVariable(
            {1, 10001},                    // 节点ID：命名空间1，标识符10001
            "Age",                         // 浏览名称
            opcua::VariableAttributes{}    // 变量属性
                .setDisplayName({"en-US", "Age"})                                    // 英文显示名称
                .setDescription({"en-US", "This mammals age in months"})            // 英文描述
                .setValue(opcua::Variant{0U})                                       // 默认年龄：0 个月
        )
        .addModellingRule(opcua::ModellingRule::Mandatory);  // 建模规则：强制创建

    std::cout << "3. 创建 DogType 对象类型（继承自 MammalType）..." << std::endl;
    
    // 在 MammalType 下创建 DogType 对象类型
    // 这个类型继承自 MammalType，表示狗是哺乳动物的一种
    auto nodeDogType = nodeMammalType.addObjectType(
        {1, 10002},                    // 节点ID：命名空间1，标识符10002
        "DogType",                     // 浏览名称
        opcua::ObjectTypeAttributes{}  // 对象类型属性
            .setDisplayName({"en-US", "DogType"})                                    // 英文显示名称
            .setDescription({"en-US", "A dog, subtype of mammal"})                   // 英文描述
    );

    std::cout << "4. 为 DogType 添加 Name 变量..." << std::endl;
    
    // 为 DogType 添加 Name 变量
    // 这个变量将存储狗的名字
    nodeDogType
        .addVariable(
            {1, 10003},                    // 节点ID：命名空间1，标识符10003
            "Name",                        // 浏览名称
            opcua::VariableAttributes{}    // 变量属性
                .setDisplayName({"en-US", "Name"})                                    // 英文显示名称
                .setDescription({"en-US", "This dogs name"})                          // 英文描述
                .setValue(opcua::Variant{"unnamed dog"})                             // 默认名字："unnamed dog"
        )
        .addModellingRule(opcua::ModellingRule::Mandatory);  // 建模规则：强制创建

    std::cout << "对象类型层次结构创建完成！" << std::endl;
    
    std::cout << "\n=== 类型层次结构说明 ===" << std::endl;
    std::cout << "BaseObjectType (基础对象类型)" << std::endl;
    std::cout << "└─ MammalType (哺乳动物类型)" << std::endl;
    std::cout << "   ├─ Age 变量 (年龄，默认值: 0 个月)" << std::endl;
    std::cout << "   └─ DogType (狗类型，继承自哺乳动物)" << std::endl;
    std::cout << "      └─ Name 变量 (名字，默认值: \"unnamed dog\")" << std::endl;

    std::cout << "\n正在实例化一个名为 Bello 的狗对象..." << std::endl;
    
    // 实例化一个名为 Bello 的狗对象：
    // (Object) Objects
    // └─ (Object) Bello <DogType>
    //    ├─ (Variable) Age
    //    └─ (Variable) Name
    
    // 获取 Objects 文件夹节点作为父节点
    opcua::Node nodeObjects{server, opcua::ObjectId::ObjectsFolder};
    
    // 在 Objects 文件夹下创建 Bello 对象实例
    // 这个对象使用 DogType 作为其类型定义
    auto nodeBello = nodeObjects.addObject(
        {1, 20000},                    // 节点ID：命名空间1，标识符20000
        "Bello",                       // 浏览名称
        opcua::ObjectAttributes{}      // 对象属性
            .setDisplayName({"en-US", "Bello"})                                    // 英文显示名称
            .setDescription({"en-US", "A dog named Bello"})                        // 英文描述
        , nodeDogType.id()             // 类型定义：使用 DogType
    );

    std::cout << "Bello 对象创建完成！" << std::endl;
    
    std::cout << "\n正在设置 Age 和 Name 变量的值..." << std::endl;
    
    // 设置 Age 变量的值
    // 使用 browseChild 方法找到子节点，然后写入值
    nodeBello.browseChild({{1, "Age"}}).writeValue(opcua::Variant{3U});
    std::cout << "Age 变量设置为: 3 个月" << std::endl;
    
    // 设置 Name 变量的值
    nodeBello.browseChild({{1, "Name"}}).writeValue(opcua::Variant{"Bello"});
    std::cout << "Name 变量设置为: \"Bello\"" << std::endl;
    
    std::cout << "\n=== 实例化结果 ===" << std::endl;
    std::cout << "对象名称: Bello" << std::endl;
    std::cout << "对象类型: DogType" << std::endl;
    std::cout << "继承自: MammalType" << std::endl;
    std::cout << "包含变量:" << std::endl;
    std::cout << "  - Age: 3 个月" << std::endl;
    std::cout << "  - Name: \"Bello\"" << std::endl;
    
    std::cout << "\n正在启动服务器..." << std::endl;
    std::cout << "服务器现在正在运行，等待客户端连接..." << std::endl;
    std::cout << "客户端可以浏览到 Objects/Bello 节点" << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;

    // 启动服务器
    server.run();
    
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 编译并运行此程序
 * 2. 程序将创建一个带有对象类型层次结构的 OPC UA 服务器
 * 3. 服务器监听在默认端口 4840
 * 4. 在 Objects 文件夹下创建一个名为 "Bello" 的狗对象
 * 5. 该对象包含年龄和名字两个变量
 * 
 * 对象类型系统工作原理：
 * 
 * 1. 类型定义：
 *    - 创建对象类型描述
 *    - 定义类型的属性和变量
 *    - 建立类型之间的继承关系
 *    - 设置建模规则
 * 
 * 2. 实例化：
 *    - 基于类型定义创建对象实例
 *    - 自动创建类型中定义的变量
 *    - 设置实例的特定属性值
 *    - 维护类型和实例的关系
 * 
 * 3. 建模规则：
 *    - 控制实例化时的行为
 *    - 决定哪些变量自动创建
 *    - 影响地址空间的结构
 * 
 * 建模规则说明：
 * 
 * 1. Mandatory (强制)：
 *    - 实例化时必须创建
 *    - 不能删除或修改
 *    - 确保类型一致性
 * 
 * 2. Optional (可选)：
 *    - 实例化时可以选择创建
 *    - 可以动态添加或删除
 *    - 提供灵活性
 * 
 * 3. ExposedPlaceholder (暴露占位符)：
 *    - 为子节点提供占位符
 *    - 支持动态结构
 *    - 用于复杂建模
 * 
 * 类型继承系统：
 * 
 * 1. 继承关系：
 *    - DogType 继承自 MammalType
 *    - 自动获得父类型的所有变量
 *    - 可以添加自己的特定变量
 * 
 * 2. 类型安全：
 *    - 编译时检查类型兼容性
 *    - 运行时验证类型关系
 *    - 确保数据一致性
 * 
 * 3. 多态性：
 *    - 可以引用父类型
 *    - 支持类型转换
 *    - 实现灵活的类型系统
 * 
 * 扩展建议：
 * 
 * 1. 添加更多对象类型：
 *    - 其他动物类型（猫、鸟等）
 *    - 设备类型（传感器、执行器等）
 *    - 系统类型（控制器、网关等）
 * 
 * 2. 实现复杂建模规则：
 *    - 条件建模规则
 *    - 动态建模规则
 *    - 自定义建模规则
 * 
 * 3. 添加类型验证：
 *    - 类型约束检查
 *    - 值范围验证
 *    - 关系完整性检查
 * 
 * 4. 支持类型版本管理：
 *    - 类型版本控制
 *    - 向后兼容性
 *    - 类型迁移支持
 * 
 * 实际应用场景：
 * 
 * 1. 工业自动化：
 *    - 设备类型定义
 *    - 生产线建模
 *    - 质量控制对象
 * 
 * 2. 建筑自动化：
 *    - 房间类型定义
 *    - 设备类型建模
 *    - 能源管理对象
 * 
 * 3. 物联网系统：
 *    - 传感器类型定义
 *    - 网关类型建模
 *    - 数据采集对象
 * 
 * 注意事项：
 * 
 * - 类型定义应该仔细设计，考虑扩展性
 * - 建模规则影响实例化行为，需要谨慎选择
 * - 类型继承关系应该反映实际业务逻辑
 * - 考虑类型的向后兼容性
 * 
 * 性能考虑：
 * 
 * - 复杂的类型层次结构可能影响性能
 * - 大量实例化操作需要优化
 * - 类型验证可能增加开销
 * - 考虑使用类型缓存
 * 
 * 安全考虑：
 * 
 * - 类型定义应该有适当的访问控制
 *    - 验证类型关系的完整性
 *    - 防止恶意类型定义
 *    - 考虑类型的审计要求
 */
