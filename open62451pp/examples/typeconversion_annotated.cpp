/**
 * @file typeconversion_annotated.cpp
 * @brief OPC UA 类型转换示例 - 演示如何为任意类型添加与原生 UA_* 类型之间的转换
 * 
 * 本示例展示了 open62541pp 库中类型转换系统的使用方法，包括：
 * 1. 为自定义类型添加类型转换器
 * 2. 在 Variant 中使用自定义类型
 * 3. 处理基本类型和数组类型
 * 4. 使用不同的赋值方法
 * 
 * 在本示例中，我们为 C++17 的 std::byte 类型添加了与 UA_Byte 之间的转换支持。
 * 这种转换可以在库外部定义，展示了库的扩展性。
 */

#include <array>
#include <cstddef>  // std::byte, std::to_integer
#include <iostream>

// 包含必要的头文件
#include <open62541pp/typeconverter.hpp>  // 类型转换器
#include <open62541pp/types.hpp>          // 基本类型定义

// 在 opcua 命名空间中添加自定义类型转换器
namespace opcua {

/**
 * @brief std::byte 类型的类型转换器特化
 * 
 * 这个特化允许 std::byte 类型与 OPC UA 的 UA_Byte 类型之间进行转换。
 * 通过实现 fromNative 和 toNative 方法，我们可以在 Variant 中使用 std::byte。
 * 
 * @tparam std::byte 要转换的类型
 */
template <>
struct TypeConverter<std::byte> {
    // 定义对应的原生 OPC UA 类型
    using NativeType = UA_Byte;

    /**
     * @brief 从原生类型转换为 std::byte
     * 
     * 将 UA_Byte 类型的值转换为 std::byte 类型。
     * 对于基本类型，使用值传递；对于复杂类型，应使用 const NativeType& src。
     * 
     * @param src 源 UA_Byte 值
     * @return 转换后的 std::byte 值
     */
    [[nodiscard]] static std::byte fromNative(UA_Byte src) noexcept {
        return std::byte{src};
    }

    /**
     * @brief 从 std::byte 转换为原生类型
     * 
     * 将 std::byte 类型的值转换为 UA_Byte 类型。
     * 对于基本类型，使用值传递；对于复杂类型，应使用 const ValueType& src。
     * 
     * @param src 源 std::byte 值
     * @return 转换后的 UA_Byte 值
     */
    [[nodiscard]] static UA_Byte toNative(std::byte src) noexcept {
        return std::to_integer<UA_Byte>(src);
    }
};

}  // namespace opcua

int main() {
    std::cout << "=== OPC UA 类型转换示例 ===" << std::endl;
    
    // 创建 Variant 对象
    // Variant 是 OPC UA 中用于存储任意类型数据的容器
    opcua::Variant variant;

    // 演示 1：将 std::byte 写入 Variant
    std::cout << "\n1. 将 std::byte 写入 Variant" << std::endl;
    
    // 方法 1：使用 assign 方法
    std::cout << "   使用 assign 方法写入值 11..." << std::endl;
    variant.assign(std::byte{11});
    
    // 方法 2：使用赋值运算符
    std::cout << "   使用赋值运算符写入值 11..." << std::endl;
    variant = std::byte{11};
    
    std::cout << "   ✓ 写入完成" << std::endl;

    // 演示 2：从 Variant 读取原生类型
    std::cout << "\n2. 从 Variant 读取原生 UA_Byte 类型" << std::endl;
    
    // 读取 UA_Byte 类型（引用方式，避免拷贝）
    // 由于 std::byte 和 UA_Byte 之间的转换，这里可以直接读取
    const auto& valueNative = variant.scalar<UA_Byte>();
    std::cout << "   读取的 UA_Byte 值: " << static_cast<int>(valueNative) << std::endl;
    
    // 验证值是否正确
    if (static_cast<int>(valueNative) == 11) {
        std::cout << "   ✓ 值验证成功" << std::endl;
    } else {
        std::cout << "   ✗ 值验证失败" << std::endl;
    }

    // 演示 3：从 Variant 读取 std::byte 类型
    std::cout << "\n3. 从 Variant 读取 std::byte 类型" << std::endl;
    
    // 读取 std::byte 类型（需要拷贝和转换）
    // 使用 to<T>() 方法进行类型转换
    const auto value = variant.to<std::byte>();
    std::cout << "   读取的 std::byte 值: " << std::to_integer<int>(value) << std::endl;
    
    // 验证值是否正确
    if (std::to_integer<int>(value) == 11) {
        std::cout << "   ✓ 值验证成功" << std::endl;
    } else {
        std::cout << "   ✗ 值验证失败" << std::endl;
    }

    // 演示 4：将 std::byte 数组写入 Variant
    std::cout << "\n4. 将 std::byte 数组写入 Variant" << std::endl;
    
    // 创建包含 3 个 std::byte 的数组
    std::array<std::byte, 3> array{std::byte{1}, std::byte{2}, std::byte{3}};
    std::cout << "   创建数组: [" 
              << std::to_integer<int>(array[0]) << ", "
              << std::to_integer<int>(array[1]) << ", "
              << std::to_integer<int>(array[2]) << "]" << std::endl;
    
    // 方法 1：使用数组容器
    std::cout << "   方法 1: 使用 assign(array) 写入..." << std::endl;
    variant.assign(array);
    std::cout << "   数组大小: " << variant.arrayLength() << std::endl;
    
    // 方法 2：使用原始数组指针和大小
    std::cout << "   方法 2: 使用 assign(Span) 写入..." << std::endl;
    variant.assign(opcua::Span{array.data(), array.size()});
    std::cout << "   数组大小: " << variant.arrayLength() << std::endl;
    
    // 方法 3：使用迭代器对
    std::cout << "   方法 3: 使用 assign(iterator, iterator) 写入..." << std::endl;
    variant.assign(array.begin(), array.end());
    std::cout << "   数组大小: " << variant.arrayLength() << std::endl;
    
    std::cout << "   ✓ 数组写入完成" << std::endl;

    // 演示 5：验证数组内容
    std::cout << "\n5. 验证数组内容" << std::endl;
    
    // 检查数组长度
    const size_t arrayLength = variant.arrayLength();
    std::cout << "   数组长度: " << arrayLength << std::endl;
    
    if (arrayLength == 3) {
        std::cout << "   ✓ 数组长度正确" << std::endl;
        
        // 读取数组中的每个元素
        for (size_t i = 0; i < arrayLength; ++i) {
            const auto element = variant.array<UA_Byte>()[i];
            std::cout << "   元素 [" << i << "]: " << static_cast<int>(element) << std::endl;
        }
    } else {
        std::cout << "   ✗ 数组长度不正确" << std::endl;
    }

    std::cout << "\n=== 示例完成 ===" << std::endl;
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 编译并运行此程序
 * 2. 程序将演示以下功能：
 *    - 为 std::byte 类型添加类型转换支持
 *    - 在 Variant 中存储和读取 std::byte 值
 *    - 处理 std::byte 数组
 *    - 使用不同的赋值方法
 * 
 * 类型转换器的工作原理：
 * 
 * 1. 特化 TypeConverter 模板：
 *    - 为自定义类型提供 fromNative 和 toNative 方法
 *    - 定义对应的 NativeType
 * 
 * 2. 自动转换：
 *    - 当使用 assign() 或赋值运算符时，自动调用 toNative
 *    - 当使用 to<T>() 时，自动调用 fromNative
 * 
 * 3. 类型安全：
 *    - 编译时检查类型转换器的存在
 *    - 运行时安全的类型转换
 * 
 * 扩展建议：
 * 
 * 1. 为其他自定义类型添加转换器：
 *    - 结构体类型
 *    - 枚举类型
 *    - 容器类型
 * 
 * 2. 实现复杂的转换逻辑：
 *    - 数据验证
 *    - 错误处理
 *    - 性能优化
 * 
 * 3. 添加序列化支持：
 *    - 二进制序列化
 *    - XML 序列化
 *    - JSON 序列化
 * 
 * 注意事项：
 * 
 * - 类型转换器必须在 opcua 命名空间中定义
 * - 对于复杂类型，使用引用传递避免不必要的拷贝
 * - 确保转换函数的 noexcept 正确性
 * - 考虑转换失败的情况和错误处理
 * 
 * 性能考虑：
 * 
 * - 基本类型的转换通常很快
 * - 复杂类型的转换可能需要内存分配
 * - 考虑使用移动语义优化性能
 * - 对于频繁使用的类型，可以考虑缓存转换结果
 */
