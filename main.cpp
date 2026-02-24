#include <iostream>

#include "cpp_json.h"

using namespace cpp_json;
int main() {
    auto&& j = json::obj({
    {"id", 42},
    {"key", "value"},
    {"array", json::arr({
        "cpp", "json", 1, false, true, nullptr, 'c', 0x5, '\n'})}
    });
    std::cout << "Root object type: " << j.type_str() << "; objects count: " << j.size() << std::endl;
    auto& id = j["id"];
    std::cout << "id: " << id.as_num() << std::endl;
    id = 0;
    std::cout << "id: " << id.as_num() << std::endl;
    id = "66";
    std::cout << "id: " << id.as_str() << std::endl;
    std::cout << "key: " << j["key"].as<std::string>() << std::endl << std::endl;

    auto& array = j["array"];
    array.push_back(9999);
    array.push_back("1733");
    std::cout << "array type: " << array.type_str() << "; size: " << array.size() << std::endl;
    std::cout << "[array content]" << std::endl;
    std::cout << (array.at(0) != array[1]) << std::endl;
    auto& a_0 = array.at(0);
    std::cout << "a_0: " << a_0.as_str() << std::endl;
    for (const auto& v : array.as<json::array>()) {
        switch (v.type()) {
            default:
                std::cout << v.type_str() << std::endl;
                break;

            case String:
                std::cout << "str: " << v.as<std::string>() << std::endl;
                break;

            case Number:
                std::cout << "num: " << v.as<double>() << std::endl;
                break;

            case Boolean:
                std::cout << "bool: " << v.as<bool>() << std::endl;
                break;

            case Null:
                std::cout << "null: " << v.as<std::nullptr_t>() << std::endl;
                break;
        }
    }

    return 0;
}