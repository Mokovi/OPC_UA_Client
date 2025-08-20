/**
 * @file server_custom_datatypes_annotated.cpp
 * @brief OPC UA 服务器自定义数据类型示例 - 演示如何定义、注册和使用自定义数据类型
 * 
 * 本示例展示了 open62541pp 库中自定义数据类型系统的使用方法，包括：
 * 1. 注册自定义数据类型定义到服务器
 * 2. 创建数据类型节点
 * 3. 创建变量类型节点
 * 4. 创建使用自定义数据类型的变量节点
 * 5. 演示复杂数据类型的处理
 * 
 * 功能说明：
 * - 支持多种自定义数据类型：结构体、枚举、联合体
 * - 演示数组和标量数据类型的处理
 * - 展示数据类型继承和扩展机制
 * - 提供完整的数据类型注册和使用流程
 * 
 * 自定义数据类型系统工作原理：
 * - 允许定义超出 OPC UA 标准类型的数据结构
 * - 支持复杂的数据组织，如结构体、数组、枚举等
 * - 提供类型安全的序列化和反序列化
 * - 支持客户端和服务器之间的类型信息共享
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/node.hpp>      // 节点操作
#include <open62541pp/server.hpp>    // 服务器核心功能

// 包含自定义数据类型定义
// 这些定义在 custom_datatypes.hpp 中，包含 Point、Measurements、Opt、Uni、Color 等类型
#include "custom_datatypes.hpp"

int main() {
    std::cout << "=== OPC UA 服务器自定义数据类型示例 ===" << std::endl;
    
    // 创建 OPC UA 服务器实例
    opcua::Server server;
    
    std::cout << "正在配置自定义数据类型系统..." << std::endl;
    
    // 从公共头文件获取自定义类型定义
    // 这些类型定义包含了数据类型的结构、字段和元数据信息
    const auto& dataTypePoint = getPointDataType();
    const auto& dataTypeMeasurements = getMeasurementsDataType();
    const auto& dataTypeOpt = getOptDataType();
    const auto& dataTypeUni = getUniDataType();
    const auto& dataTypeColor = getColorDataType();
    
    std::cout << "✓ 已加载自定义数据类型定义" << std::endl;
    std::cout << "数据类型数量: 5" << std::endl;
    
    // 将自定义数据类型定义提供给服务器
    // 这是使用自定义数据类型的第一步：注册类型定义
    server.config().addCustomDataTypes({
        dataTypePoint,           // 3D 点结构体类型
        dataTypeMeasurements,    // 测量数据集合类型
        dataTypeOpt,            // 可选字段类型
        dataTypeUni,            // 联合体类型
        dataTypeColor,          // 颜色枚举类型
    });
    
    std::cout << "✓ 自定义数据类型已注册到服务器" << std::endl;
    
    std::cout << "\n正在创建数据类型节点..." << std::endl;
    
    // 获取结构体数据类型节点
    // 这是 OPC UA 中用于组织数据类型的标准节点
    opcua::Node structureDataTypeNode{server, opcua::DataTypeId::Structure};
    
    // 添加 Point 数据类型节点
    // 数据类型节点定义了数据类型的结构和属性
    structureDataTypeNode.addDataType(dataTypePoint.typeId(), "PointDataType");
    std::cout << "✓ 已创建 Point 数据类型节点" << std::endl;
    
    // 添加 Measurements 数据类型节点
    structureDataTypeNode.addDataType(dataTypeMeasurements.typeId(), "MeasurementsDataType");
    std::cout << "✓ 已创建 Measurements 数据类型节点" << std::endl;
    
    // 添加 Opt 数据类型节点
    structureDataTypeNode.addDataType(dataTypeOpt.typeId(), "OptDataType");
    std::cout << "✓ 已创建 Opt 数据类型节点" << std::endl;
    
    // 添加 Uni 数据类型节点
    structureDataTypeNode.addDataType(dataTypeUni.typeId(), "UniDataType");
    std::cout << "✓ 已创建 Uni 数据类型节点" << std::endl;
    
    // 获取枚举数据类型节点
    opcua::Node enumerationDataTypeNode{server, opcua::DataTypeId::Enumeration};
    
    // 添加 Color 枚举数据类型节点
    // 枚举类型需要额外的属性来定义枚举值
    auto colorDataTypeNode = enumerationDataTypeNode.addDataType(dataTypeColor.typeId(), "Color");
    
    // 为 Color 枚举添加枚举值属性
    // 这是枚举类型的标准属性，定义了所有可能的枚举值
    colorDataTypeNode.addProperty(
        {0, 0},  // 自动生成节点ID
        "EnumValues",
        opcua::VariableAttributes{}
            .setDataType<opcua::EnumValueType>()           // 数据类型为枚举值类型
            .setValueRank(opcua::ValueRank::OneDimension)  // 一维数组
            .setArrayDimensions({0})                       // 动态数组维度
            .setValue(
                opcua::Variant{opcua::Span<const opcua::EnumValueType>{
                    {0, {"", "Red"}, {}},      // 枚举值 0: "Red"
                    {1, {"", "Green"}, {}},    // 枚举值 1: "Green"
                    {2, {"", "Yellow"}, {}},   // 枚举值 2: "Yellow"
                }}
            )
    ).addModellingRule(opcua::ModellingRule::Mandatory);  // 添加强制建模规则
    
    std::cout << "✓ 已创建 Color 枚举数据类型节点" << std::endl;
    
    std::cout << "\n正在创建变量类型节点..." << std::endl;
    
    // 获取基础数据变量类型节点
    // 变量类型节点定义了变量的结构和默认值
    opcua::Node baseDataVariableTypeNode{server, opcua::VariableTypeId::BaseDataVariableType};
    
    // 创建 Point 变量类型节点
    // 变量类型节点定义了使用特定数据类型的变量的模板
    auto variableTypePointNode = baseDataVariableTypeNode.addVariableType(
        {1, 4243},  // 节点ID：命名空间1，标识符4243
        "PointType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypePoint.typeId())                    // 使用 Point 数据类型
            .setValueRank(opcua::ValueRank::ScalarOrOneDimension)   // 支持标量或一维数组
            .setValue(opcua::Variant{Point{1, 2, 3}, dataTypePoint})  // 设置默认值
    );
    std::cout << "✓ 已创建 Point 变量类型节点" << std::endl;
    
    // 创建 Measurements 变量类型节点
    auto variableTypeMeasurementNode = baseDataVariableTypeNode.addVariableType(
        {1, 4444},  // 节点ID：命名空间1，标识符4444
        "MeasurementsType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeMeasurements.typeId())     // 使用 Measurements 数据类型
            .setValueRank(opcua::ValueRank::Scalar)         // 标量类型
            .setValue(opcua::Variant{Measurements{}, dataTypeMeasurements})  // 设置默认值
    );
    std::cout << "✓ 已创建 Measurements 变量类型节点" << std::endl;
    
    // 创建 Opt 变量类型节点
    auto variableTypeOptNode = baseDataVariableTypeNode.addVariableType(
        {1, 4645},  // 节点ID：命名空间1，标识符4645
        "OptType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeOpt.typeId())               // 使用 Opt 数据类型
            .setValueRank(opcua::ValueRank::Scalar)          // 标量类型
            .setValue(opcua::Variant{Opt{}, dataTypeOpt})    // 设置默认值
    );
    std::cout << "✓ 已创建 Opt 变量类型节点" << std::endl;
    
    // 创建 Uni 变量类型节点
    auto variableTypeUniNode = baseDataVariableTypeNode.addVariableType(
        {1, 4846},  // 节点ID：命名空间1，标识符4846
        "UniType",
        opcua::VariableTypeAttributes{}
            .setDataType(dataTypeUni.typeId())               // 使用 Uni 数据类型
            .setValueRank(opcua::ValueRank::Scalar)          // 标量类型
            .setValue(opcua::Variant{Uni{}, dataTypeUni})    // 设置默认值
    );
    std::cout << "✓ 已创建 Uni 变量类型节点" << std::endl;
    
    std::cout << "\n正在创建变量节点..." << std::endl;
    
    // 获取 Objects 文件夹节点
    // 这是 OPC UA 地址空间中的标准根对象
    opcua::Node objectsNode{server, opcua::ObjectId::ObjectsFolder};
    
    // 创建使用 Point 数据类型的变量节点
    // 设置初始值为 (3.0, 4.0, 5.0)
    const Point point{3.0, 4.0, 5.0};
    objectsNode.addVariable(
        {1, "Point"},  // 节点ID：命名空间1，名称为"Point"
        "Point",
        opcua::VariableAttributes{}
            .setDataType(dataTypePoint.typeId())        // 使用 Point 数据类型
            .setValueRank(opcua::ValueRank::Scalar)     // 标量类型
            .setValue(opcua::Variant{point, dataTypePoint}),  // 设置初始值
        variableTypePointNode.id()                      // 指定变量类型
    );
    std::cout << "✓ 已创建 Point 变量节点，值: (" << point.x << ", " << point.y << ", " << point.z << ")" << std::endl;
    
    // 创建使用 Point 数组的变量节点
    // 设置初始值为两个点的数组
    const std::vector<Point> pointVec{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    objectsNode.addVariable(
        {1, "PointVec"},  // 节点ID：命名空间1，名称为"PointVec"
        "PointVec",
        opcua::VariableAttributes{}
            .setDataType(dataTypePoint.typeId())                    // 使用 Point 数据类型
            .setArrayDimensions({0})                                // 单维度但大小未知
            .setValueRank(opcua::ValueRank::OneDimension)           // 一维数组
            .setValue(opcua::Variant{pointVec, dataTypePoint}),     // 设置初始值
        variableTypePointNode.id()                                  // 指定变量类型
    );
    std::cout << "✓ 已创建 PointVec 数组变量节点，包含 " << pointVec.size() << " 个点" << std::endl;
    
    // 创建使用 Measurements 数据类型的变量节点
    // 设置初始值为包含描述和浮点数组的测量数据
    std::vector<float> measurementsValues{19.1F, 20.2F, 19.7F};
    const Measurements measurements{
        opcua::String{"Test description"},  // 测量描述
        measurementsValues.size(),           // 数据点数量
        measurementsValues.data(),           // 浮点数据数组
    };
    objectsNode.addVariable(
        {1, "Measurements"},  // 节点ID：命名空间1，名称为"Measurements"
        "Measurements",
        opcua::VariableAttributes{}
            .setDataType(dataTypeMeasurements.typeId())             // 使用 Measurements 数据类型
            .setValueRank(opcua::ValueRank::Scalar)                 // 标量类型
            .setValue(opcua::Variant{measurements, dataTypeMeasurements}),  // 设置初始值
        variableTypeMeasurementNode.id()                             // 指定变量类型
    );
    std::cout << "✓ 已创建 Measurements 变量节点，描述: " << measurements.description << std::endl;
    
    // 创建使用 Opt 数据类型的变量节点
    // 设置初始值为包含可选字段的 Opt 结构
    float optC = 10.10F;
    const Opt opt{3, nullptr, &optC};  // 选择字段 C，值为 10.10
    objectsNode.addVariable(
        {1, "Opt"},  // 节点ID：命名空间1，名称为"Opt"
        "Opt",
        opcua::VariableAttributes{}
            .setDataType(dataTypeOpt.typeId())           // 使用 Opt 数据类型
            .setValueRank(opcua::ValueRank::Scalar)      // 标量类型
            .setValue(opcua::Variant{opt, dataTypeOpt}), // 设置初始值
        variableTypeOptNode.id()                          // 指定变量类型
    );
    std::cout << "✓ 已创建 Opt 变量节点，选择字段 C，值: " << optC << std::endl;
    
    // 创建使用 Uni 数据类型的变量节点
    // 设置初始值为联合体类型，选择 OptionB 字段
    Uni uni{};
    uni.switchField = UniSwitch::OptionB;                    // 选择 OptionB 字段
    uni.fields.optionB = UA_STRING_STATIC("test string");    // 设置字符串值
    objectsNode.addVariable(
        {1, "Uni"},  // 节点ID：命名空间1，名称为"Uni"
        "Uni",
        opcua::VariableAttributes{}
            .setDataType(dataTypeUni.typeId())           // 使用 Uni 数据类型
            .setValueRank(opcua::ValueRank::Scalar)      // 标量类型
            .setValue(opcua::Variant{uni, dataTypeUni}), // 设置初始值
        variableTypeUniNode.id()                          // 指定变量类型
    );
    std::cout << "✓ 已创建 Uni 变量节点，选择字段 OptionB" << std::endl;
    
    // 创建使用 Color 枚举数据类型的变量节点
    // 设置初始值为绿色
    objectsNode.addVariable(
        {1, "Color"},  // 节点ID：命名空间1，名称为"Color"
        "Color",
        opcua::VariableAttributes{}
            .setDataType(dataTypeColor.typeId())           // 使用 Color 数据类型
            .setValueRank(opcua::ValueRank::Scalar)        // 标量类型
            .setValue(opcua::Variant{Color::Green, dataTypeColor})  // 设置初始值为绿色
    );
    std::cout << "✓ 已创建 Color 变量节点，值: Green" << std::endl;
    
    std::cout << "\n=== 自定义数据类型系统配置完成 ===" << std::endl;
    std::cout << "1. ✓ 数据类型定义已注册" << std::endl;
    std::cout << "2. ✓ 数据类型节点已创建" << std::endl;
    std::cout << "3. ✓ 变量类型节点已创建" << std::endl;
    std::cout << "4. ✓ 变量节点已创建并配置" << std::endl;
    
    std::cout << "\n=== 支持的数据类型 ===" << std::endl;
    std::cout << "1. Point - 3D 点结构体 (x, y, z)" << std::endl;
    std::cout << "2. Measurements - 测量数据集合 (描述 + 浮点数组)" << std::endl;
    std::cout << "3. Opt - 可选字段类型 (A, B, C 中选择一个)" << std::endl;
    std::cout << "4. Uni - 联合体类型 (多种字段类型)" << std::endl;
    std::cout << "5. Color - 颜色枚举 (Red, Green, Yellow)" << std::endl;
    
    std::cout << "\n=== 变量节点信息 ===" << std::endl;
    std::cout << "1. Point - 标量点，值: (3.0, 4.0, 5.0)" << std::endl;
    std::cout << "2. PointVec - 点数组，包含 2 个点" << std::endl;
    std::cout << "3. Measurements - 测量数据，3 个浮点值" << std::endl;
    std::cout << "4. Opt - 可选字段，选择 C，值: 10.10" << std::endl;
    std::cout << "5. Uni - 联合体，选择 OptionB 字段" << std::endl;
    std::cout << "6. Color - 颜色枚举，值: Green" << std::endl;
    
    std::cout << "\n正在启动服务器..." << std::endl;
    std::cout << "服务器地址: opc.tcp://localhost:4840" << std::endl;
    std::cout << "自定义数据类型系统已就绪，等待客户端连接..." << std::endl;
    
    // 启动服务器并进入事件循环
    // 服务器将处理客户端连接、数据类型查询和变量访问
    server.run();
    
    return 0;
}

/**
 * 自定义数据类型系统详细说明：
 * 
 * 1. 数据类型注册流程：
 *    - 定义数据类型结构和元数据
 *    - 将类型定义添加到服务器配置
 *    - 创建数据类型节点
 *    - 配置类型属性和约束
 * 
 * 2. 数据类型分类：
 *    - 结构体类型：包含多个命名字段
 *    - 枚举类型：预定义的常量值集合
 *    - 联合体类型：多种字段类型中选择一个
 *    - 数组类型：同类型元素的集合
 * 
 * 3. 类型安全机制：
 *    - 编译时类型检查
 *    - 运行时类型验证
 *    - 自动类型转换
 *    - 类型信息序列化
 * 
 * 数据类型设计原则：
 * 
 * 1. 结构清晰：
 *    - 字段命名有意义
 *    - 类型层次合理
 *    - 避免过度嵌套
 *    - 保持向后兼容
 * 
 * 2. 性能优化：
 *    - 减少内存拷贝
 *    - 优化序列化性能
 *    - 支持流式处理
 *    - 缓存类型信息
 * 
 * 3. 扩展性考虑：
 *    - 支持版本升级
 *    - 添加新字段
 *    - 保持兼容性
 *    - 文档化变更
 * 
 * 实际应用场景：
 * 
 * 1. 工业自动化：
 *    - 设备配置参数
 *    - 传感器数据格式
 *    - 控制命令结构
 *    - 状态信息定义
 * 
 * 2. 数据采集：
 *    - 测量数据结构
 *    - 时间序列格式
 *    - 质量控制参数
 *    - 校准信息记录
 * 
 * 3. 系统配置：
 *    - 用户权限设置
 *    - 网络配置参数
 *    - 安全策略定义
 *    - 日志格式规范
 * 
 * 4. 业务逻辑：
 *    - 订单数据结构
 *    - 产品规格定义
 *    - 工作流程配置
 *    - 报表格式模板
 * 
 * 性能优化建议：
 * 
 * 1. 内存管理：
 *    - 使用对象池减少分配
 *    - 避免不必要的拷贝
 *    - 优化字符串处理
 *    - 管理大对象生命周期
 * 
 * 2. 序列化优化：
 *    - 使用高效的编码格式
 *    - 实现增量序列化
 *    - 支持压缩传输
 *    - 缓存序列化结果
 * 
 * 3. 网络传输：
 *    - 批量传输数据
 *    - 异步处理请求
 *    - 实现数据分片
 *    - 支持断点续传
 * 
 * 安全考虑：
 * 
 * 1. 类型验证：
 *    - 验证输入数据格式
 *    - 检查字段值范围
 *    - 防止缓冲区溢出
 *    - 验证类型一致性
 * 
 * 2. 访问控制：
 *    - 限制类型定义权限
 *    - 控制类型修改操作
 *    - 审计类型变更日志
 *    - 实现类型版本控制
 * 
 * 3. 数据保护：
 *    - 加密敏感字段
 *    - 实现数据脱敏
 *    - 保护类型元数据
 *    - 防止类型信息泄露
 * 
 * 调试和测试：
 * 
 * 1. 类型验证：
 *    - 测试类型定义正确性
 *    - 验证序列化/反序列化
 *    - 检查类型兼容性
 *    - 测试边界条件
 * 
 * 2. 性能测试：
 *    - 测量类型创建时间
 *    - 监控内存使用情况
 *    - 测试网络传输性能
 *    - 分析瓶颈点
 * 
 * 3. 兼容性测试：
 *    - 测试不同客户端兼容性
 *    - 验证类型升级过程
 *    - 检查向后兼容性
 *    - 测试跨平台支持
 * 
 * 扩展建议：
 * 
 * 1. 添加更多数据类型：
 *    - 时间序列类型
 *    - 地理坐标类型
 *    - 多媒体数据类型
 *    - 加密数据类型
 * 
 * 2. 实现类型继承：
 *    - 支持类型扩展
 *    - 实现多态性
 *    - 支持接口定义
 *    - 实现类型组合
 * 
 * 3. 添加类型约束：
 *    - 字段值范围限制
 *    - 字符串长度约束
 *    - 数组大小限制
 *    - 自定义验证规则
 * 
 * 4. 实现类型版本管理：
 *    - 支持类型版本升级
 *    - 实现向后兼容
 *    - 管理类型变更历史
 *    - 支持类型回滚
 */
