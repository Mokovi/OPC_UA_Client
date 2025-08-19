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

        // 读取 Variant，并转换为 int
        auto v = node.readValue();
        int current = v.to<int>();
        std::cout << "Read current value: " << current << std::endl;

        // 写入新值
        node.writeValue(opcua::Variant{123});
        std::cout << "Wrote value 123" << std::endl;

        auto v2 = node.readValue();
        std::cout << "Read back: " << v2.to<int>() << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Client exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
