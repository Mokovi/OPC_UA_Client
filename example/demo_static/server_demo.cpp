// server_demo.cpp
#include <iostream>
#include <exception>

#include <open62541pp/server.hpp>
#include <open62541pp/node.hpp>
#include <open62541pp/open62541pp.hpp>

int main() {
    try {
        opcua::Server server;

        // 在 Objects 下添加一个变量 ns=1;i=1000 名为 "TheAnswer"
        opcua::Node objects(server, opcua::ObjectId::ObjectsFolder);
        auto myVar = objects.addVariable({1, 1000}, "TheAnswer");

        // 显式构造 Variant 写入（避免隐式转换错误）
        myVar.writeValue(opcua::Variant{42});

        std::cout << "Server running at opc.tcp://0.0.0.0:4840\n";
        server.run(); // 阻塞直到 stop() 被调用
    } catch (const std::exception &e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
