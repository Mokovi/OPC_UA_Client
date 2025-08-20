/**
 * @file server_method_annotated.cpp
 * @brief OPC UA 服务器方法示例 - 演示如何在服务器中创建和实现自定义方法
 * 
 * 本示例展示了 open62541pp 库中方法系统的使用方法，包括：
 * 1. 创建自定义方法节点
 * 2. 实现方法逻辑（使用 lambda 表达式）
 * 3. 定义方法输入和输出参数
 * 4. 处理不同类型的参数（字符串、数组、标量）
 * 5. 实现数组处理和转换逻辑
 * 
 * 功能说明：
 * - 创建问候方法，返回个性化的问候语
 * - 创建数组处理方法，对整数数组进行增量操作
 * - 演示参数验证和类型转换
 * - 支持复杂的数据类型处理
 */

#include <algorithm>

// 包含必要的头文件
#include <open62541pp/node.hpp>    // 节点操作
#include <open62541pp/server.hpp>  // 服务器核心功能

int main() {
    std::cout << "=== OPC UA 服务器方法示例 ===" << std::endl;
    
    // 创建 OPC UA 服务器实例
    opcua::Server server;

    // 获取 Objects 文件夹节点作为父节点
    // 这是 OPC UA 地址空间中的标准根对象
    opcua::Node objectsNode{server, opcua::ObjectId::ObjectsFolder};

    std::cout << "正在创建自定义方法..." << std::endl;
    
    // 添加一个方法来返回 "Hello " + 提供的名字
    // 这是一个简单的字符串处理方法
    std::cout << "1. 创建问候方法 (Greet)..." << std::endl;
    
    objectsNode.addMethod(
        {1, 1000},                    // 节点ID：命名空间1，标识符1000
        "Greet",                      // 方法名称
        [](opcua::Span<const opcua::Variant> input, opcua::Span<opcua::Variant> output) {
            // 方法实现逻辑（lambda 表达式）
            
            // 从输入参数中获取名字
            // input.at(0) 获取第一个输入参数，并转换为字符串类型
            const auto& name = input.at(0).scalar<opcua::String>();
            
            // 构造问候语：将 "Hello " 和提供的名字连接起来
            const auto greeting = std::string{"Hello "}.append(name);
            
            // 将结果设置到输出参数中
            // output.at(0) 设置第一个输出参数
            output.at(0) = greeting;
        },
        // 输入参数定义
        {
            {
                "name",                           // 参数名称
                {"en-US", "your name"},           // 显示名称（英文）
                opcua::DataTypeId::String,        // 数据类型：字符串
                opcua::ValueRank::Scalar          // 值等级：标量（单个值）
            }
        },
        // 输出参数定义
        {
            {
                "greeting",                       // 参数名称
                {"en-US", "greeting"},            // 显示名称（英文）
                opcua::DataTypeId::String,        // 数据类型：字符串
                opcua::ValueRank::Scalar          // 值等级：标量（单个值）
            }
        }
    );

    std::cout << "问候方法创建完成！" << std::endl;
    
    // 添加一个方法，接受一个包含 5 个整数的数组和一个标量作为输入
    // 返回数组的副本，其中每个条目都增加了标量值
    std::cout << "2. 创建数组增量方法 (IncInt32ArrayValues)..." << std::endl;
    
    objectsNode.addMethod(
        {1, 1001},                    // 节点ID：命名空间1，标识符1001
        "IncInt32ArrayValues",        // 方法名称
        [](opcua::Span<const opcua::Variant> input, opcua::Span<opcua::Variant> output) {
            // 方法实现逻辑（lambda 表达式）
            
            // 从输入参数中获取整数数组
            // input.at(0) 获取第一个输入参数（整数数组）
            const auto values = input.at(0).array<int32_t>();
            
            // 从输入参数中获取增量值
            // input.at(1) 获取第二个输入参数（标量增量）
            const auto delta = input.at(1).scalar<int32_t>();
            
            // 创建结果数组，大小与输入数组相同
            std::vector<int32_t> incremented(values.size());
            
            // 使用 std::transform 对数组中的每个元素应用增量操作
            // 这是一个函数式编程的示例，避免了传统的 for 循环
            std::transform(
                values.begin(),                    // 输入数组的开始迭代器
                values.end(),                      // 输入数组的结束迭代器
                incremented.begin(),               // 输出数组的开始迭代器
                [&](auto v) {                     // 转换函数（lambda 表达式）
                    return v + delta;              // 将每个值加上增量
                }
            );
            
            // 将结果数组设置到输出参数中
            output.at(0) = incremented;
        },
        // 输入参数定义
        {
            opcua::Argument{
                "int32 array",                    // 参数名称
                {"en-US", "int32[5] array"},     // 显示名称（英文）
                opcua::DataTypeId::Int32,         // 数据类型：32位整数
                opcua::ValueRank::OneDimension,  // 值等级：一维数组
                {5}                               // 数组维度：固定长度为 5
            },
            opcua::Argument{
                "int32 delta",                    // 参数名称
                {"en-US", "int32 delta"},         // 显示名称（英文）
                opcua::DataTypeId::Int32,         // 数据类型：32位整数
                opcua::ValueRank::Scalar          // 值等级：标量（单个值）
            },
        },
        // 输出参数定义
        {
            opcua::Argument{
                "each entry is incremented by the delta",  // 参数名称
                {"en-US", "int32[5] array"},              // 显示名称（英文）
                opcua::DataTypeId::Int32,                  // 数据类型：32位整数
                opcua::ValueRank::OneDimension,           // 值等级：一维数组
                {5}                                        // 数组维度：固定长度为 5
            },
        }
    );

    std::cout << "数组增量方法创建完成！" << std::endl;
    
    std::cout << "\n=== 方法功能说明 ===" << std::endl;
    std::cout << "1. Greet 方法：" << std::endl;
    std::cout << "   - 输入：名字（字符串）" << std::endl;
    std::cout << "   - 输出：问候语（字符串）" << std::endl;
    std::cout << "   - 功能：返回 \"Hello \" + 输入的名字" << std::endl;
    
    std::cout << "\n2. IncInt32ArrayValues 方法：" << std::endl;
    std::cout << "   - 输入1：整数数组（固定长度5）" << std::endl;
    std::cout << "   - 输入2：增量值（整数）" << std::endl;
    std::cout << "   - 输出：增量后的数组" << std::endl;
    std::cout << "   - 功能：将数组中的每个元素加上增量值" << std::endl;
    
    std::cout << "\n=== 方法调用说明 ===" << std::endl;
    std::cout << "1. 客户端可以连接到服务器" << std::endl;
    std::cout << "2. 浏览到 Objects 文件夹" << std::endl;
    std::cout << "3. 找到 Greet 和 IncInt32ArrayValues 方法节点" << std::endl;
    std::cout << "4. 调用方法并传入相应的参数" << std::endl;
    std::cout << "5. 接收方法的返回结果" << std::endl;

    // 启动服务器
    std::cout << "\n正在启动服务器..." << std::endl;
    std::cout << "服务器现在正在运行，等待客户端连接..." << std::endl;
    std::cout << "客户端可以调用创建的方法" << std::endl;
    std::cout << "按 Ctrl+C 停止服务器" << std::endl;
    
    server.run();
    
    return 0;
}

/**
 * 使用说明：
 * 
 * 1. 编译并运行此程序
 * 2. 程序将创建一个带有自定义方法的 OPC UA 服务器
 * 3. 服务器监听在默认端口 4840
 * 4. 在 Objects 文件夹下创建两个方法节点
 * 5. 客户端可以调用这些方法
 * 
 * 方法系统工作原理：
 * 
 * 1. 方法定义：
 *    - 创建方法节点
 *    - 定义输入和输出参数
 *    - 指定参数的数据类型和值等级
 *    - 设置显示名称和描述
 * 
 * 2. 方法实现：
 *    - 使用 lambda 表达式实现方法逻辑
 *    - 处理输入参数
 *    - 执行计算或操作
 *    - 设置输出参数
 * 
 * 3. 参数处理：
 *    - 支持标量和数组参数
 *    - 自动类型转换和验证
 *    - 处理不同维度的数组
 * 
 * 参数类型说明：
 * 
 * 1. 标量参数 (Scalar)：
 *    - 单个值
 *    - 如：整数、浮点数、字符串等
 *    - 适合简单的数值或文本输入
 * 
 * 2. 数组参数 (OneDimension)：
 *    - 一维数组
 *    - 可以指定固定长度或可变长度
 *    - 适合批量数据处理
 * 
 * 3. 多维数组参数：
 *    - 支持二维、三维等数组
 *    - 可以指定每个维度的长度
 *    - 适合矩阵或图像数据处理
 * 
 * 方法实现最佳实践：
 * 
 * 1. 参数验证：
 *    - 检查输入参数的有效性
 *    - 验证数组长度和类型
 *    - 处理边界情况
 * 
 * 2. 错误处理：
 *    - 捕获和处理异常
 *    - 返回适当的错误信息
 *    - 记录错误日志
 * 
 * 3. 性能优化：
 *    - 避免不必要的内存分配
 *    - 使用高效的算法
 *    - 考虑大数据集的处理
 * 
 * 扩展建议：
 * 
 * 1. 添加更多方法类型：
 *    - 数学计算方法
 *    - 字符串处理方法
 *    - 文件操作方法
 *    - 数据库查询方法
 * 
 * 2. 实现方法链：
 *    - 一个方法调用另一个方法
 *    - 实现复杂的工作流
 *    - 支持条件执行
 * 
 * 3. 添加方法监控：
 *    - 记录方法调用次数
 *    - 监控执行时间
 *    - 统计错误率
 * 
 * 4. 实现异步方法：
 *    - 支持长时间运行的操作
 *    - 非阻塞方法调用
 *    - 进度报告和取消支持
 * 
 * 实际应用场景：
 * 
 * 1. 工业自动化：
 *    - 设备控制命令
 *    - 参数配置方法
 *    - 诊断和测试方法
 *    - 报警处理方法
 * 
 * 2. 数据处理：
 *    - 数据转换方法
 *    - 统计分析方法
 *    - 报表生成方法
 *    - 数据验证方法
 * 
 * 3. 系统管理：
 *    - 配置管理方法
 *    - 用户管理方法
 *    - 日志管理方法
 *    - 备份恢复方法
 * 
 * 注意事项：
 * 
 * - 方法实现应该快速执行，避免长时间阻塞
 * - 复杂操作应该使用异步方法
 * - 参数验证应该在方法开始时进行
 * - 考虑方法的线程安全性
 * 
 * 性能考虑：
 * 
 * - 方法调用频率影响服务器性能
 * - 大量参数可能增加网络负载
 * - 复杂计算应该优化算法
 * - 考虑使用缓存和预计算
 * 
 * 安全考虑：
 * 
 * - 方法应该有适当的访问控制
 *    - 验证用户权限
 *    - 记录方法调用日志
 *    - 防止恶意方法调用
 *    - 考虑输入参数的安全性
 */
