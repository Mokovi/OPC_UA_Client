// client_demo.cpp
#include <iostream>
#include <exception>

#include <open62541pp/client.hpp>
#include <open62541pp/node.hpp>
#include <open62541pp/open62541pp.hpp>

int main() {
    try {
        opcua::Client client;
        client.connect("opc.tcp://localhost:4840");

        opcua::Node node(client, opcua::NodeId{1, 1000});

        // 先读取 Variant，然后转换为 int（更稳健）
        auto val = node.readValue();            // 返回 opcua::Variant
        int current = val.to<int>();            // 使用 Variant 的转换方法
        std::cout << "Read current value: " << current << std::endl;

        // 写值：显式用 Variant 包装
        node.writeValue(opcua::Variant{123});
        std::cout << "Wrote value 123" << std::endl;

        auto val2 = node.readValue();
        std::cout << "Read back: " << val2.to<int>() << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Client exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
