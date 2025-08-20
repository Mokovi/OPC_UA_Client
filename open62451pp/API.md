# open62541pp API 文档

## 概述

open62541pp 是一个基于 open62541 的现代 C++ OPC UA 库，提供了类型安全、易用的 API 接口。该库支持 OPC UA 客户端和服务器功能，包括异步操作、类型系统、节点管理等。

## 目录

- [核心头文件](#核心头文件)
- [客户端 API](#客户端-api)
- [服务器 API](#服务器-api)
- [节点管理](#节点管理)
- [类型系统](#类型系统)
- [异步操作](#异步操作)
- [服务接口](#服务接口)
- [插件系统](#插件系统)
- [工具类](#工具类)

## 核心头文件

### open62541pp.hpp
主头文件，包含所有必要的组件：

```cpp
#include "open62541pp/open62541pp.hpp"
```

## 客户端 API

### client.hpp

#### ClientConfig 类
客户端配置类，继承自 `Wrapper<UA_ClientConfig>`。

**构造函数：**
```cpp
ClientConfig();  // 默认配置（无加密）
ClientConfig(const ByteString& certificate, const ByteString& privateKey, 
            Span<const ByteString> trustList, Span<const ByteString> revocationList = {});
```

**主要方法：**
```cpp
void setLogger(LogFunction func);                    // 设置日志函数
void setTimeout(uint32_t milliseconds) noexcept;     // 设置响应超时
void setUserIdentityToken(const AnonymousIdentityToken& token);  // 设置匿名身份令牌
void setUserIdentityToken(const UserNameIdentityToken& token);   // 设置用户名/密码身份令牌
void setUserIdentityToken(const X509IdentityToken& token);      // 设置X.509身份令牌
void setUserIdentityToken(const IssuedIdentityToken& token);    // 设置颁发身份令牌
void setSecurityMode(MessageSecurityMode mode) noexcept;       // 设置消息安全模式
```

#### Client 类
OPC UA 客户端类，继承自 `Wrapper<UA_Client>`。

**构造函数：**
```cpp
Client(ClientConfig&& config);
```

**主要方法：**
```cpp
void connect(const std::string& endpointUrl);        // 连接到服务器
void disconnect();                                   // 断开连接
bool isConnected() const noexcept;                  // 检查连接状态
Session getSession() const;                         // 获取会话
```

## 服务器 API

### server.hpp

#### ServerConfig 类
服务器配置类，继承自 `Wrapper<UA_ServerConfig>`。

**构造函数：**
```cpp
ServerConfig();  // 默认配置
ServerConfig(uint16_t port, const ByteString& certificate = {});  // 最小配置
ServerConfig(uint16_t port, const ByteString& certificate, const ByteString& privateKey,
            Span<const ByteString> trustList, Span<const ByteString> issuerList,
            Span<const ByteString> revocationList = {});  // 加密配置
```

#### Server 类
OPC UA 服务器类，继承自 `Wrapper<UA_Server>`。

**构造函数：**
```cpp
Server(ServerConfig&& config);
```

**主要方法：**
```cpp
void run();                                         // 运行服务器
void runIterate();                                  // 运行一次迭代
void stop();                                        // 停止服务器
bool isRunning() const noexcept;                    // 检查运行状态
```

## 节点管理

### node.hpp

#### Node 类
高级节点类，用于访问节点属性、浏览和填充地址空间。

**模板参数：**
- `Connection`: Server 或 Client 类型

**构造函数：**
```cpp
Node(Connection& connection, const NodeId& id);
Node(Connection& connection, NodeId&& id);
```

**主要方法：**
```cpp
Connection& connection() noexcept;                   // 获取服务器/客户端实例
const NodeId& id() const noexcept;                  // 获取节点ID
bool exists() noexcept;                             // 检查节点是否存在

// 节点管理
Node addFolder(const NodeId& id, std::string_view browseName, 
               const ObjectAttributes& attributes = {}, 
               const NodeId& referenceType = ReferenceTypeId::HasComponent);
Node addObject(const NodeId& id, std::string_view browseName,
               const ObjectAttributes& attributes = {},
               const NodeId& referenceType = ReferenceTypeId::HasComponent);
Node addVariable(const NodeId& id, std::string_view browseName,
                 const Variant& value, const VariableAttributes& attributes = {},
                 const NodeId& referenceType = ReferenceTypeId::HasComponent);
Node addMethod(const NodeId& id, std::string_view browseName,
               const MethodAttributes& attributes = {},
               const NodeId& referenceType = ReferenceTypeId::HasComponent);

// 异步版本
template<typename CompletionToken = DefaultCompletionToken>
auto addFolderAsync(...);
template<typename CompletionToken = DefaultCompletionToken>
auto addObjectAsync(...);
template<typename CompletionToken = DefaultCompletionToken>
auto addVariableAsync(...);
template<typename CompletionToken = DefaultCompletionToken>
auto addMethodAsync(...);
```

## 类型系统

### types.hpp

#### StatusCode 类
状态码包装类，继承自 `Wrapper<UA_StatusCode>`。

**构造函数：**
```cpp
StatusCode();  // 默认状态码 UA_STATUSCODE_GOOD
StatusCode(UA_StatusCode code);
```

**主要方法：**
```cpp
UA_StatusCode get() const noexcept;                 // 获取底层状态码
std::string_view name() const noexcept;             // 获取人类可读名称
bool isGood() const noexcept;                       // 检查是否良好
bool isUncertain() const noexcept;                  // 检查是否不确定
bool isBad() const noexcept;                        // 检查是否错误
void throwIfBad() const;                            // 如果错误则抛出异常
```

#### 基本数据类型
```cpp
// 字符串类型
class String : public Wrapper<UA_String>, public detail::StringLikeMixin<String, char>
class LocalizedText : public Wrapper<UA_LocalizedText>
class QualifiedName : public Wrapper<UA_QualifiedName>

// 数值类型
class Boolean : public Wrapper<UA_Boolean>
class SByte : public Wrapper<UA_SByte>
class Byte : public Wrapper<UA_Byte>
class Int16 : public Wrapper<UA_Int16>
class UInt16 : public Wrapper<UA_UInt16>
class Int32 : public Wrapper<UA_Int32>
class UInt32 : public Wrapper<UA_UInt32>
class Int64 : public Wrapper<UA_Int64>
class UInt64 : public Wrapper<UA_UInt64>
class Float : public Wrapper<UA_Float>
class Double : public Wrapper<UA_Double>

// 复杂类型
class DateTime : public Wrapper<UA_DateTime>
class Guid : public Wrapper<UA_Guid>
class ByteString : public Wrapper<UA_ByteString>
class XmlElement : public Wrapper<UA_XmlElement>
class NodeId : public Wrapper<UA_NodeId>
class ExpandedNodeId : public Wrapper<UA_ExpandedNodeId>
class ExtensionObject : public Wrapper<UA_ExtensionObject>
class DataValue : public Wrapper<UA_DataValue>
class Variant : public Wrapper<UA_Variant>

// 数组类型
template<typename T>
class Array : public Wrapper<UA_Array>
```

#### 身份令牌类型
```cpp
class AnonymousIdentityToken : public Wrapper<UA_AnonymousIdentityToken>
class UserNameIdentityToken : public Wrapper<UA_UserNameIdentityToken>
class X509IdentityToken : public Wrapper<UA_X509IdentityToken>
class IssuedIdentityToken : public Wrapper<UA_IssuedIdentityToken>
```

### datatype.hpp

#### DataType 类
数据类型类，继承自 `Wrapper<UA_DataType>`。

**主要方法：**
```cpp
const UA_NodeId& getTypeId() const noexcept;        // 获取类型ID
const char* getTypeName() const noexcept;            // 获取类型名称
size_t getMemSize() const noexcept;                  // 获取内存大小
UA_TypeKind getTypeKind() const noexcept;            // 获取类型种类
```

## 异步操作

### async.hpp

#### 异步结果类型
```cpp
template <class CompletionToken, typename T>
struct AsyncResult;

template <typename T, typename Initiation, typename CompletionToken, typename... Args>
auto asyncInitiate(Initiation&& initiation, CompletionToken&& token, Args&&... args);
```

#### 完成令牌类型
```cpp
struct UseFutureToken {};        // Future 完成令牌
struct UseDeferredToken {};      // 延迟完成令牌

inline constexpr UseFutureToken useFuture;
inline constexpr UseDeferredToken useDeferred;
```

#### 异步特性
```cpp
template <typename T>
struct AsyncTrait;

template <typename T>
constexpr bool isAsync = AsyncTrait<T>::value;
```

## 服务接口

### services/services.hpp

#### 命名空间
```cpp
namespace opcua::services
```

#### 主要服务模块
- `attribute.hpp` - 属性服务
- `attribute_highlevel.hpp` - 高级属性服务
- `method.hpp` - 方法服务
- `monitoreditem.hpp` - 监控项服务
- `nodemanagement.hpp` - 节点管理服务
- `subscription.hpp` - 订阅服务
- `view.hpp` - 视图服务

### services/attribute.hpp

#### 属性服务函数
```cpp
// 读取属性
template<typename Connection>
Result<DataValue> readAttribute(Connection& connection, const NodeId& nodeId, 
                               AttributeId attributeId = AttributeId::Value);

// 写入属性
template<typename Connection>
Result<void> writeAttribute(Connection& connection, const NodeId& nodeId,
                           const DataValue& value, AttributeId attributeId = AttributeId::Value);

// 异步版本
template<typename Connection, typename CompletionToken = DefaultCompletionToken>
auto readAttributeAsync(Connection& connection, const NodeId& nodeId,
                        AttributeId attributeId = AttributeId::Value, CompletionToken&& token = {});

template<typename Connection, typename CompletionToken = DefaultCompletionToken>
auto writeAttributeAsync(Connection& connection, const NodeId& nodeId,
                         const DataValue& value, AttributeId attributeId = AttributeId::Value,
                         CompletionToken&& token = {});
```

### services/nodemanagement.hpp

#### 节点管理服务函数
```cpp
// 添加文件夹
template<typename Connection>
Result<NodeId> addFolder(Connection& connection, const NodeId& parentNodeId,
                         const NodeId& requestedNodeId, std::string_view browseName,
                         const ObjectAttributes& attributes = {},
                         const NodeId& referenceType = ReferenceTypeId::HasComponent);

// 添加对象
template<typename Connection>
Result<NodeId> addObject(Connection& connection, const NodeId& parentNodeId,
                         const NodeId& requestedNodeId, std::string_view browseName,
                         const ObjectAttributes& attributes = {},
                         const NodeId& referenceType = ReferenceTypeId::HasComponent);

// 添加变量
template<typename Connection>
Result<NodeId> addVariable(Connection& connection, const NodeId& parentNodeId,
                           const NodeId& requestedNodeId, std::string_view browseName,
                           const Variant& value, const VariableAttributes& attributes = {},
                           const NodeId& referenceType = ReferenceTypeId::HasComponent);

// 添加方法
template<typename Connection>
Result<NodeId> addMethod(Connection& connection, const NodeId& parentNodeId,
                         const NodeId& requestedNodeId, std::string_view browseName,
                         const MethodAttributes& attributes = {},
                         const NodeId& referenceType = ReferenceTypeId::HasComponent);

// 异步版本
template<typename Connection, typename CompletionToken = DefaultCompletionToken>
auto addFolderAsync(Connection& connection, const NodeId& parentNodeId,
                    const NodeId& requestedNodeId, std::string_view browseName,
                    const ObjectAttributes& attributes = {},
                    const NodeId& referenceType = ReferenceTypeId::HasComponent,
                    CompletionToken&& token = {});
// ... 其他异步版本类似
```

## 插件系统

### plugin/accesscontrol.hpp

#### AccessControl 类
访问控制插件接口。

**主要方法：**
```cpp
virtual bool allowAddNode(const NodeId& nodeId, const NodeId& parentNodeId,
                          const NodeId& referenceTypeId, const NodeId& typeDefinition,
                          const QualifiedName& browseName, const NodeAttributes& attributes) = 0;

virtual bool allowDeleteNode(const NodeId& nodeId) = 0;
virtual bool allowAddReference(const NodeId& sourceNodeId, const NodeId& targetNodeId,
                              const NodeId& referenceTypeId) = 0;
virtual bool allowDeleteReference(const NodeId& sourceNodeId, const NodeId& targetNodeId,
                                 const NodeId& referenceTypeId) = 0;
virtual bool allowUpdateAttribute(const NodeId& nodeId, AttributeId attributeId,
                                 const DataValue& value) = 0;
virtual bool allowBrowse(const NodeId& nodeId) = 0;
virtual bool allowRead(const NodeId& nodeId) = 0;
virtual bool allowWrite(const NodeId& nodeId) = 0;
virtual bool allowCall(const NodeId& nodeId) = 0;
virtual bool allowHistoryRead(const NodeId& nodeId) = 0;
virtual bool allowHistoryUpdate(const NodeId& nodeId) = 0;
virtual bool allowSubscription(const NodeId& nodeId) = 0;
```

### plugin/accesscontrol_default.hpp

#### DefaultAccessControl 类
默认访问控制实现，继承自 `AccessControl`。

**构造函数：**
```cpp
DefaultAccessControl();
```

### plugin/log.hpp

#### Log 类
日志插件接口。

**主要方法：**
```cpp
virtual void log(LogLevel level, const std::string& category, const std::string& message) = 0;
```

### plugin/log_default.hpp

#### LogFunction 类型
```cpp
using LogFunction = std::function<void(LogLevel, const std::string&, const std::string&)>;
```

#### DefaultLog 类
默认日志实现，继承自 `Log`。

**构造函数：**
```cpp
DefaultLog(LogFunction func);
```

### plugin/nodestore.hpp

#### NodeStore 类
节点存储插件接口。

**主要方法：**
```cpp
virtual bool insertNode(const NodeId& nodeId, const Node& node) = 0;
virtual std::optional<Node> getNode(const NodeId& nodeId) = 0;
virtual bool removeNode(const NodeId& nodeId) = 0;
virtual void clear() = 0;
```

### plugin/create_certificate.hpp

#### 证书创建函数
```cpp
ByteString createCertificate(const std::string& commonName, const std::string& organization,
                            const std::string& organizationalUnit, const std::string& country,
                            const std::string& state, const std::string& locality,
                            uint32_t validityDays = 365);
```

## 工具类

### wrapper.hpp

#### Wrapper 类
通用包装器模板类。

**主要方法：**
```cpp
T& native() noexcept;                              // 获取原生对象引用
const T& native() const noexcept;                  // 获取原生对象常量引用
T* nativePtr() noexcept;                           // 获取原生对象指针
const T* nativePtr() const noexcept;               // 获取原生对象常量指针
```

### span.hpp

#### Span 类
非拥有视图容器，类似于 `std::span`。

**构造函数：**
```cpp
Span() noexcept;
Span(T* data, size_t size) noexcept;
template<size_t N>
Span(T (&arr)[N]) noexcept;
template<typename Container>
Span(Container& container) noexcept;
```

**主要方法：**
```cpp
T* data() const noexcept;                          // 获取数据指针
size_t size() const noexcept;                      // 获取大小
bool empty() const noexcept;                        // 检查是否为空
T& operator[](size_t index) const;                 // 索引访问
T* begin() const noexcept;                         // 开始迭代器
T* end() const noexcept;                           // 结束迭代器
```

### bitmask.hpp

#### Bitmask 类
位掩码模板类。

**主要方法：**
```cpp
bool test(BitType bit) const noexcept;             // 测试位
void set(BitType bit) noexcept;                    // 设置位
void reset(BitType bit) noexcept;                  // 重置位
void flip(BitType bit) noexcept;                   // 翻转位
bool all() const noexcept;                          // 检查所有位
bool any() const noexcept;                          // 检查任意位
bool none() const noexcept;                         // 检查无位
```

### callback.hpp

#### Callback 类
回调函数包装器。

**构造函数：**
```cpp
Callback() = default;
Callback(Function&& func);
```

**主要方法：**
```cpp
template<typename... Args>
auto operator()(Args&&... args) const;              // 调用回调函数
bool empty() const noexcept;                        // 检查是否为空
explicit operator bool() const noexcept;            // 布尔转换
```

### exception.hpp

#### 异常类
```cpp
class BadStatus : public std::runtime_error;        // 状态码错误异常
class BadAttributeId : public std::runtime_error;   // 属性ID错误异常
class BadNodeId : public std::runtime_error;        // 节点ID错误异常
class BadReferenceTypeId : public std::runtime_error; // 引用类型ID错误异常
class BadTypeDefinitionId : public std::runtime_error; // 类型定义ID错误异常
```

### result.hpp

#### Result 类
结果包装器模板类。

**构造函数：**
```cpp
Result() = default;
Result(const T& value);
Result(T&& value);
Result(const StatusCode& status);
```

**主要方法：**
```cpp
bool hasValue() const noexcept;                     // 检查是否有值
bool hasError() const noexcept;                     // 检查是否有错误
T& value();                                         // 获取值
const T& value() const;                             // 获取常量值
StatusCode status() const noexcept;                 // 获取状态码
void throwIfBad() const;                            // 如果错误则抛出异常
```

### typeconverter.hpp

#### TypeConverter 类
类型转换器。

**主要方法：**
```cpp
template<typename T>
static T fromVariant(const Variant& variant);

template<typename T>
static Variant toVariant(const T& value);
```

### typeregistry.hpp

#### 类型注册表函数
```cpp
const DataType& getDataType(const NodeId& typeId);
const DataType& getDataType(const std::string& typeName);
void registerDataType(const DataType& dataType);
```

## 监控项和订阅

### monitoreditem.hpp

#### MonitoredItem 类
监控项类。

**主要方法：**
```cpp
void setValueCallback(ValueCallback callback);
void setEventCallback(EventCallback callback);
void setStatusCallback(StatusCallback callback);
```

### subscription.hpp

#### Subscription 类
订阅类。

**主要方法：**
```cpp
MonitoredItem addDataChange(const NodeId& nodeId, MonitoringMode mode = MonitoringMode::Reporting);
MonitoredItem addEvent(const NodeId& nodeId, MonitoringMode mode = MonitoringMode::Reporting);
void removeMonitoredItem(const MonitoredItem& item);
```

## 会话管理

### session.hpp

#### Session 类
会话类。

**主要方法：**
```cpp
void activate(const UserIdentityToken& token);
void close();
bool isActive() const noexcept;
```

## 事件系统

### event.hpp

#### Event 类
事件类。

**主要方法：**
```cpp
void setSourceName(const std::string& name);
void setTime(const DateTime& time);
void setSeverity(uint16_t severity);
void setMessage(const LocalizedText& message);
```

## 通用工具

### common.hpp

#### 常量定义
```cpp
constexpr uint16_t NamespaceIndex = 0;              // 命名空间索引
constexpr uint32_t MaxStringLength = 65535;         // 最大字符串长度
constexpr uint32_t MaxArrayLength = 65535;          // 最大数组长度
constexpr uint32_t MaxByteStringLength = 65535;     // 最大字节字符串长度
constexpr uint32_t MaxXmlElementLength = 65535;     // 最大XML元素长度
```

## 使用示例

### 基本客户端示例
```cpp
#include "open62541pp/open62541pp.hpp"

int main() {
    // 创建客户端配置
    opcua::ClientConfig config;
    config.setTimeout(5000);
    
    // 创建客户端
    opcua::Client client(std::move(config));
    
    // 连接到服务器
    client.connect("opc.tcp://localhost:4840");
    
    // 创建节点对象
    opcua::Node<opcua::Client> node(client, opcua::NodeId(1, "MyVariable"));
    
    // 读取变量值
    auto result = node.readValue();
    if (result.hasValue()) {
        std::cout << "Value: " << result.value() << std::endl;
    }
    
    return 0;
}
```

### 基本服务器示例
```cpp
#include "open62541pp/open62541pp.hpp"

int main() {
    // 创建服务器配置
    opcua::ServerConfig config(4840);
    
    // 创建服务器
    opcua::Server server(std::move(config));
    
    // 添加变量节点
    opcua::Node<opcua::Server> node(server, opcua::NodeId(1, "MyVariable"));
    node.addVariable(opcua::NodeId(1, "MyVariable"), "MyVariable", 
                     opcua::Variant(42), opcua::VariableAttributes{});
    
    // 运行服务器
    server.run();
    
    return 0;
}
```

## 注意事项

1. **内存管理**: 所有类都使用 RAII 模式，自动管理资源
2. **异常安全**: 大部分操作都提供异常安全保证
3. **线程安全**: 客户端和服务器类不是线程安全的，需要在单线程中使用或外部同步
4. **异步操作**: 支持多种完成令牌类型，包括 Future 和延迟执行
5. **类型安全**: 提供编译时类型检查，减少运行时错误

## 版本信息

本文档基于 open62541pp 项目的最新版本编写。具体版本信息请参考项目的 CHANGELOG.md 文件。
