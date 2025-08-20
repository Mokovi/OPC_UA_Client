/**
 * @file client_custom_datatypes_annotated.cpp
 * @brief OPC UA 客户端自定义数据类型示例 - 演示如何读取和处理服务器上的自定义数据类型
 * 
 * 本示例展示了 open62541pp 库中客户端自定义数据类型的处理方法，包括：
 * 1. 注册自定义数据类型定义到客户端
 * 2. 读取使用自定义数据类型的变量节点
 * 3. 处理不同类型的自定义数据（结构体、数组、联合体、枚举）
 * 4. 解析和显示自定义数据的内容
 * 5. 处理数组和可选字段的特殊情况
 * 
 * 功能说明：
 * - 连接到运行自定义数据类型服务器示例
 * - 读取各种自定义数据类型的变量
 * - 演示不同类型数据的解析方法
 * - 展示客户端如何处理复杂的自定义类型
 * 
 * 注意：此示例需要服务器示例 `server_custom_datatypes` 正在运行
 * 
 * 自定义数据类型处理工作原理：
 * - 客户端需要先注册数据类型定义才能正确解析数据
 * - 标量自定义类型会自动解包为原始数据结构
 * - 数组类型需要手动处理 ExtensionObject 解包
 * - 支持结构体、枚举、联合体等复杂类型
 */

#include <iostream>
#include <string>

// 包含必要的头文件
#include <open62541pp/client.hpp>        // 客户端核心功能
#include <open62541pp/node.hpp>          // 节点操作

// 包含自定义数据类型定义
// 这些定义在 custom_datatypes.hpp 中，包含 Point、Measurements、Opt、Uni、Color 等类型
#include "custom_datatypes.hpp"

int main() {
    std::cout << "=== OPC UA 客户端自定义数据类型示例 ===" << std::endl;
    
    // 创建 OPC UA 客户端实例
    opcua::Client client;
    
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
    
    // 将自定义数据类型定义提供给客户端
    // 这是使用自定义数据类型的必要步骤：客户端需要知道如何解析这些类型
    client.config().addCustomDataTypes({
        dataTypePoint,           // 3D 点结构体类型
        dataTypeMeasurements,    // 测量数据集合类型
        dataTypeOpt,            // 可选字段类型
        dataTypeUni,            // 联合体类型
        dataTypeColor,          // 颜色枚举类型
    });
    
    std::cout << "✓ 自定义数据类型已注册到客户端" << std::endl;
    
    std::cout << "\n正在连接到服务器..." << std::endl;
    std::cout << "服务器地址: opc.tcp://localhost:4840" << std::endl;
    
    // 连接到 OPC UA 服务器
    // 确保服务器示例 `server_custom_datatypes` 正在运行
    client.connect("opc.tcp://localhost:4840");
    std::cout << "✓ 连接成功！" << std::endl;
    
    std::cout << "\n=== 开始读取自定义数据类型变量 ===" << std::endl;
    
    // 声明 Variant 变量用于存储读取的数据
    opcua::Variant variant;
    
    // 1. 读取 Point 变量（3D 点结构体）
    std::cout << "\n1. 读取 Point 变量..." << std::endl;
    variant = opcua::Node{client, {1, "Point"}}.readValue();
    
    // 检查变量是否使用 Point 数据类型
    if (variant.isType(dataTypePoint)) {
        // 将数据转换为 Point 指针
        const auto* p = static_cast<Point*>(variant.data());
        
        std::cout << "✓ Point 数据读取成功：" << std::endl;
        std::cout << "  - x = " << p->x << std::endl;
        std::cout << "  - y = " << p->y << std::endl;
        std::cout << "  - z = " << p->z << std::endl;
    } else {
        std::cout << "✗ Point 数据类型不匹配" << std::endl;
    }
    
    // 2. 读取 PointVec 变量（Point 数组）
    std::cout << "\n2. 读取 PointVec 变量（Point 数组）..." << std::endl;
    variant = opcua::Node{client, {1, "PointVec"}}.readValue();
    
    // 重要说明：数组类型的处理
    // Variant 将非内置数据类型存储为 ExtensionObject
    // 如果数据类型已知，open62541 会在编码层透明地解包标量对象
    // 但是数组不能轻易解包，因为数组是 ExtensionObject 的数组
    // 解包后的对象数组在内存中不是连续的，需要手动处理
    if (variant.isArray() && variant.isType<opcua::ExtensionObject>()) {
        std::cout << "✓ PointVec 数组数据读取成功：" << std::endl;
        
        // 遍历 ExtensionObject 数组
        size_t i = 0;
        for (auto&& extObj : variant.array<opcua::ExtensionObject>()) {
            // 从 ExtensionObject 中解码 Point 数据
            const auto* p = static_cast<Point*>(extObj.decodedData());
            
            std::cout << "  PointVec[" << i++ << "]:" << std::endl;
            std::cout << "    - x = " << p->x << std::endl;
            std::cout << "    - y = " << p->y << std::endl;
            std::cout << "    - z = " << p->z << std::endl;
        }
    } else {
        std::cout << "✗ PointVec 数据类型不匹配或不是数组" << std::endl;
    }
    
    // 3. 读取 Measurements 变量（测量数据集合）
    std::cout << "\n3. 读取 Measurements 变量..." << std::endl;
    variant = opcua::Node{client, {1, "Measurements"}}.readValue();
    
    if (variant.isType(dataTypeMeasurements)) {
        // 将数据转换为 Measurements 指针
        const auto* m = static_cast<Measurements*>(variant.data());
        
        std::cout << "✓ Measurements 数据读取成功：" << std::endl;
        std::cout << "  - 描述: " << m->description << std::endl;
        
        // 遍历测量值数组
        size_t i = 0;
        for (auto&& value : opcua::Span{m->measurements, m->measurementsSize}) {
            std::cout << "  - measurements[" << i++ << "] = " << value << std::endl;
        }
    } else {
        std::cout << "✗ Measurements 数据类型不匹配" << std::endl;
    }
    
    // 4. 读取 Opt 变量（可选字段类型）
    std::cout << "\n4. 读取 Opt 变量（可选字段类型）..." << std::endl;
    variant = opcua::Node{client, {1, "Opt"}}.readValue();
    
    // 定义格式化可选字段的 Lambda 函数
    // 如果指针为 nullptr，显示 "NULL"，否则显示值
    auto formatOptional = [](const auto* ptr) {
        return ptr == nullptr ? "NULL" : std::to_string(*ptr);
    };
    
    if (variant.isScalar() && variant.isType(dataTypeOpt)) {
        // 将数据转换为 Opt 指针
        const auto* opt = static_cast<Opt*>(variant.data());
        
        std::cout << "✓ Opt 数据读取成功：" << std::endl;
        std::cout << "  - a = " << opt->a << std::endl;
        std::cout << "  - b = " << formatOptional(opt->b) << std::endl;
        std::cout << "  - c = " << formatOptional(opt->c) << std::endl;
    } else {
        std::cout << "✗ Opt 数据类型不匹配或不是标量" << std::endl;
    }
    
    // 5. 读取 Uni 变量（联合体类型）
    std::cout << "\n5. 读取 Uni 变量（联合体类型）..." << std::endl;
    variant = opcua::Node{client, {1, "Uni"}}.readValue();
    
    if (variant.isType(dataTypeUni)) {
        // 将数据转换为 Uni 指针
        const auto* uni = static_cast<Uni*>(variant.data());
        
        std::cout << "✓ Uni 数据读取成功：" << std::endl;
        std::cout << "  - switchField = " << static_cast<int>(uni->switchField) << std::endl;
        
        // 根据 switchField 的值访问相应的字段
        if (uni->switchField == UniSwitch::OptionA) {
            std::cout << "  - optionA = " << uni->fields.optionA << std::endl;
        }
        if (uni->switchField == UniSwitch::OptionB) {
            // 将 UA_String 转换为 open62541pp::String 进行显示
            std::cout << "  - optionB = " << opcua::String{uni->fields.optionB} << std::endl;
        }
    } else {
        std::cout << "✗ Uni 数据类型不匹配" << std::endl;
    }
    
    // 6. 读取 Color 变量（颜色枚举）
    std::cout << "\n6. 读取 Color 变量（颜色枚举）..." << std::endl;
    variant = opcua::Node{client, {1, "Color"}}.readValue();
    
    if (variant.isType<int32_t>()) {
        // 枚举值以整数形式存储
        int32_t colorValue = variant.scalar<int32_t>();
        
        std::cout << "✓ Color 数据读取成功：" << std::endl;
        std::cout << "  - 枚举值: " << colorValue << std::endl;
        
        // 将枚举值转换为可读的颜色名称
        std::string colorName;
        switch (colorValue) {
            case 0: colorName = "Red"; break;
            case 1: colorName = "Green"; break;
            case 2: colorName = "Yellow"; break;
            default: colorName = "Unknown"; break;
        }
        std::cout << "  - 颜色名称: " << colorName << std::endl;
    } else {
        std::cout << "✗ Color 数据类型不匹配" << std::endl;
    }
    
    std::cout << "\n=== 自定义数据类型读取完成 ===" << std::endl;
    std::cout << "1. ✓ Point - 3D 点结构体" << std::endl;
    std::cout << "2. ✓ PointVec - Point 数组" << std::endl;
    std::cout << "3. ✓ Measurements - 测量数据集合" << std::endl;
    std::cout << "4. ✓ Opt - 可选字段类型" << std::endl;
    std::cout << "5. ✓ Uni - 联合体类型" << std::endl;
    std::cout << "6. ✓ Color - 颜色枚举" << std::endl;
    
    std::cout << "\n=== 示例完成 ===" << std::endl;
    std::cout << "客户端自定义数据类型处理演示成功完成" << std::endl;
    
    return 0;
}

/**
 * 自定义数据类型处理详细说明：
 * 
 * 1. 数据类型注册：
 *    - 客户端必须注册数据类型定义
 *    - 支持结构体、枚举、联合体等类型
 *    - 注册后才能正确解析数据
 *    - 类型定义必须与服务器一致
 * 
 * 2. 数据解析机制：
 *    - 标量类型自动解包
 *    - 数组类型需要手动处理
 *    - 支持类型检查和验证
 *    - 提供类型安全的访问
 * 
 * 3. 特殊类型处理：
 *    - 可选字段的空值处理
 *    - 联合体的字段选择
 *    - 枚举值的数值转换
 *    - 字符串的本地化处理
 * 
 * 数据类型处理策略：
 * 
 * 1. 类型安全：
 *    - 使用 isType 检查类型
 *    - 验证数据类型匹配
 *    - 安全的类型转换
 *    - 处理类型不匹配情况
 * 
 * 2. 错误处理：
 *    - 检查数据类型一致性
 *    - 处理空值和无效数据
 *    - 提供有意义的错误信息
 *    - 实现优雅的降级处理
 * 
 * 3. 性能优化：
 *    - 避免重复类型检查
 *    - 使用高效的数据访问
 *    - 减少内存拷贝
 *    - 优化循环和迭代
 * 
 * 实际应用场景：
 * 
 * 1. 工业自动化：
 *    - 设备配置参数
 *    - 传感器数据结构
 *    - 控制命令格式
 *    - 状态信息定义
 * 
 * 2. 数据采集：
 *    - 测量数据格式
 *    - 时间序列结构
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
 *    - 避免不必要的数据拷贝
 *    - 使用引用和指针
 *    - 及时释放临时对象
 *    - 实现对象池和缓存
 * 
 * 2. 类型检查优化：
 *    - 缓存类型检查结果
 *    - 批量处理相同类型
 *    - 使用类型标签优化
 *    - 减少运行时类型查询
 * 
 * 3. 数据访问优化：
 *    - 批量读取数据
 *    - 使用高效的数据结构
 *    - 实现数据预取
 *    - 优化循环和迭代
 * 
 * 安全考虑：
 * 
 * 1. 类型验证：
 *    - 验证数据类型一致性
 *    - 检查字段值范围
 *    - 防止缓冲区溢出
 *    - 验证数据完整性
 * 
 * 2. 访问控制：
 *    - 限制数据类型访问权限
 *    - 验证数据读取权限
 *    - 审计数据访问日志
 *    - 实现数据脱敏
 * 
 * 3. 数据保护：
 *    - 加密敏感数据
 *    - 保护类型元数据
 *    - 防止数据泄露
 *    - 实现数据签名
 * 
 * 调试和测试：
 * 
 * 1. 类型验证测试：
 *    - 测试各种数据类型
 *    - 验证类型匹配
 *    - 检查边界条件
 *    - 测试异常情况
 * 
 * 2. 性能测试：
 *    - 测量数据读取时间
 *    - 监控内存使用
 *    - 测试并发性能
 *    - 分析瓶颈点
 * 
 * 3. 兼容性测试：
 *    - 测试不同服务器
 *    - 验证数据类型版本
 *    - 检查字段映射
 *    - 测试向后兼容性
 * 
 * 扩展建议：
 * 
 * 1. 添加更多数据类型：
 *    - 时间序列类型
 *    - 地理坐标类型
 *    - 多媒体数据类型
 *    - 加密数据类型
 * 
 * 2. 实现类型管理：
 *    - 动态类型注册
 *    - 类型版本管理
 *    - 类型缓存机制
 *    - 类型继承支持
 * 
 * 3. 添加数据处理：
 *    - 数据验证和清洗
 *    - 数据转换和映射
 *    - 数据聚合和统计
 *    - 数据导出和导入
 * 
 * 4. 实现高级功能：
 *    - 类型自动发现
 *    - 动态类型创建
 *    - 类型关系分析
 *    - 类型性能优化
 */
