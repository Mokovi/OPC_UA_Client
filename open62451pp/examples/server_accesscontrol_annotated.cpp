/**
 * @file server_accesscontrol_annotated.cpp
 * @brief OPC UA 服务器访问控制示例 - 演示如何实现自定义的访问控制策略
 * 
 * 本示例展示了 open62541pp 库中访问控制系统的使用方法，包括：
 * 1. 继承默认访问控制类
 * 2. 实现自定义的会话激活逻辑
 * 3. 实现自定义的用户访问级别控制
 * 4. 使用会话属性存储用户权限信息
 * 5. 配置用户名/密码身份验证
 * 
 * 功能说明：
 * - 如果用户以 "admin" 用户名登录，则获得管理员权限
 * - 管理员可以读写变量节点，普通用户只能读取
 * - 使用会话属性存储用户权限状态
 * - 支持匿名用户访问
 */

#include <iostream>

// 包含必要的头文件
#include <open62541pp/node.hpp>                      // 节点操作
#include <open62541pp/plugin/accesscontrol_default.hpp>  // 默认访问控制基类
#include <open62541pp/server.hpp>                    // 服务器核心功能

using namespace opcua;

/**
 * @brief 自定义访问控制类
 * 
 * 继承自 AccessControlDefault，实现基于用户名的权限控制。
 * 如果用户以 "admin" 用户名登录，则获得管理员权限，可以读写变量节点。
 * 普通用户和匿名用户只能读取变量节点。
 * 
 * 注意：会话属性功能需要 open62541 v1.3 或更高版本。
 */
class AccessControlCustom : public AccessControlDefault {
public:
    // 继承基类的构造函数
    using AccessControlDefault::AccessControlDefault;

    /**
     * @brief 重写会话激活方法
     * 
     * 当用户成功登录后，此方法会被调用。在这里我们可以：
     * 1. 检查用户身份
     * 2. 设置用户权限
     * 3. 将权限信息存储到会话属性中
     * 
     * @param session 会话对象
     * @param endpointDescription 端点描述
     * @param secureChannelRemoteCertificate 安全通道远程证书
     * @param userIdentityToken 用户身份令牌
     * @return 状态码
     */
    StatusCode activateSession(
        Session& session,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const ExtensionObject& userIdentityToken
    ) override {
        // 检查用户身份令牌，判断是否为管理员
        const auto* token = userIdentityToken.decodedData<UserNameIdentityToken>();
        const bool isAdmin = (token != nullptr && token->userName() == "admin");
        
        // 输出用户权限信息
        std::cout << "用户登录成功，用户名: " 
                  << (token ? token->userName() : "匿名") << std::endl;
        std::cout << "用户具有管理员权限: " << (isAdmin ? "是" : "否") << std::endl;
        
        // 将管理员权限状态存储到会话属性中
        // 这样在后续的访问控制检查中就可以使用这个信息
        session.setSessionAttribute({0, "isAdmin"}, Variant{isAdmin});

        // 调用基类的会话激活方法，完成标准的会话激活流程
        return AccessControlDefault::activateSession(
            session, endpointDescription, secureChannelRemoteCertificate, userIdentityToken
        );
    }

    /**
     * @brief 重写用户访问级别获取方法
     * 
     * 当客户端尝试访问节点时，此方法会被调用来确定用户的访问权限。
     * 根据会话中存储的管理员权限信息，返回相应的访问级别。
     * 
     * @param session 会话对象
     * @param nodeId 要访问的节点ID
     * @return 访问级别掩码
     */
    Bitmask<AccessLevel> getUserAccessLevel(Session& session, const NodeId& nodeId) override {
        // 从会话属性中获取用户的管理员权限状态
        const bool isAdmin = session.getSessionAttribute({0, "isAdmin"}).scalar<bool>();
        
        // 输出访问控制检查信息
        std::cout << "检查节点访问权限，节点ID: " << opcua::toString(nodeId) << std::endl;
        std::cout << "用户具有管理员权限: " << (isAdmin ? "是" : "否") << std::endl;
        
        // 根据用户权限返回不同的访问级别
        if (isAdmin) {
            // 管理员：可以读取和写入
            std::cout << "授予管理员读写权限" << std::endl;
            return AccessLevel::CurrentRead | AccessLevel::CurrentWrite;
        } else {
            // 普通用户：只能读取
            std::cout << "授予普通用户只读权限" << std::endl;
            return AccessLevel::CurrentRead;
        }
    }
};

int main() {
    std::cout << "=== OPC UA 服务器访问控制示例 ===" << std::endl;
    
    // 警告：在生产环境中，明文传输用户名和密码是非常危险的！
    // 这里仅用于演示目的，实际应用中应该使用加密通信
    std::cout << "警告：此示例使用明文传输用户名和密码，仅用于演示！" << std::endl;
    
    // 创建自定义访问控制实例
    // 参数说明：
    // - 第一个参数：是否允许匿名访问
    // - 第二个参数：用户名和密码列表
    AccessControlCustom accessControl{
        true,  // 允许匿名用户访问
        {
            Login{String{"admin"}, String{"admin"}},  // 管理员账户
            Login{String{"user"}, String{"user"}},    // 普通用户账户
        }
    };

    // 创建服务器配置
    ServerConfig config;
    
    // 设置访问控制策略
    config.setAccessControl(accessControl);
    
    // 如果使用 open62541 v1.4 或更高版本，允许无策略密码
    // 这样可以支持明文密码（仅用于演示）
#if UAPP_OPEN62541_VER_GE(1, 4)
    config.allowNonePolicyPassword = true;
#endif

    // 使用配置创建服务器
    Server server{std::move(config)};

    // 在 Objects 文件夹下添加一个变量节点
    // 这个节点将用于演示访问控制功能
    std::cout << "正在创建演示变量节点..." << std::endl;
    
    Node{server, ObjectId::ObjectsFolder}
        .addVariable(
            {1, 1000},                    // 节点ID：命名空间1，标识符1000
            "Variable",                    // 浏览名称
            VariableAttributes{}           // 变量属性
                .setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)  // 设置访问级别
                .setDataType(DataTypeId::Int32)                                        // 数据类型：32位整数
                .setValueRank(ValueRank::Scalar)                                      // 值等级：标量
                .setValue(opcua::Variant{0})                                         // 初始值：0
        );

    std::cout << "变量节点创建完成！" << std::endl;
    std::cout << "\n=== 访问控制策略说明 ===" << std::endl;
    std::cout << "1. 匿名用户：可以连接到服务器，但只能读取变量值" << std::endl;
    std::cout << "2. 普通用户 (user/user)：可以读取变量值，但不能修改" << std::endl;
    std::cout << "3. 管理员 (admin/admin)：可以读取和修改变量值" << std::endl;
    std::cout << "\n=== 测试说明 ===" << std::endl;
    std::cout << "1. 启动服务器后，使用不同的客户端工具连接" << std::endl;
    std::cout << "2. 尝试使用不同的用户身份登录" << std::endl;
    std::cout << "3. 测试读取和写入变量节点的权限" << std::endl;
    std::cout << "4. 观察控制台输出的访问控制信息" << std::endl;

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
 * 2. 程序将创建一个带有访问控制的 OPC UA 服务器
 * 3. 服务器监听在默认端口 4840
 * 4. 在 Objects 文件夹下创建一个名为 "Variable" 的变量节点
 * 
 * 测试步骤：
 * 
 * 1. 使用 OPC UA 客户端工具连接到 localhost:4840
 * 2. 尝试匿名连接，测试读取变量值
 * 3. 使用用户名 "user" 和密码 "user" 登录，测试读取权限
 * 4. 使用用户名 "admin" 和密码 "admin" 登录，测试读写权限
 * 5. 观察控制台输出的访问控制信息
 * 
 * 访问控制逻辑：
 * 
 * 1. 会话激活时：
 *    - 检查用户身份令牌
 *    - 判断是否为管理员用户
 *    - 将权限信息存储到会话属性中
 * 
 * 2. 访问检查时：
 *    - 从会话属性中获取用户权限
 *    - 根据权限返回相应的访问级别
 *    - 管理员：读写权限
 *    - 普通用户：只读权限
 * 
 * 注意事项：
 * 
 * - 此示例仅用于演示，生产环境中应使用加密通信
 * - 会话属性功能需要 open62541 v1.3 或更高版本
 * - 访问控制检查会在每次节点访问时执行
 * - 可以根据需要扩展更复杂的权限控制逻辑
 * 
 * 扩展建议：
 * 
 * 1. 添加更细粒度的权限控制：
 *    - 按节点类型控制权限
 *    - 按用户组控制权限
 *    - 按时间段控制权限
 * 
 * 2. 实现动态权限管理：
 *    - 运行时修改用户权限
 *    - 支持权限继承
 *    - 实现权限缓存
 * 
 * 3. 添加审计日志：
 *    - 记录访问尝试
 *    - 记录权限变更
 *    - 记录安全事件
 * 
 * 4. 集成外部认证系统：
 *    - LDAP 认证
 *    - 数据库认证
 *    - 单点登录 (SSO)
 */
