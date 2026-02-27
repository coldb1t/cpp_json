#include "cpp_json.h"

#include <iostream>

using namespace cpp_json;
int main() {
    auto j = json::obj({
    {"id", 42},
    {"key", "value"},
    {"bool", true},
    {"null_value\n", nullptr},
    {"array", json::arr({
        "cpp", "cpp", 1.05, -0.0, false, true, nullptr, 'c', 0x5, '\n'})},
    {"users", json::arr({
            json::obj(),
            json::obj({ {"id", 1}, {"data", json::obj({{"name", "Joe"}, {"phone", nullptr}}) } }),
            json::obj({ { "id", 2 } }),
            json::obj(),
            json::obj({ {"id", 3}, {"data", json::obj({{"name", "Joe"}, {"phone", nullptr}}) } }),
            json::obj({ {"id", 4}, {"data", json::obj({{"name", "Joe"}, {"phone", nullptr}}) } }),
        })},
    {"empty_array", json::arr({})},
    {"array2", json::arr({json::obj(), json::obj()})}
    });

    std::cout << "JSON dump: " << std::endl;
    std::cout << j << std::endl << std::endl;

    std::cout << "Root type: " << j.type_str() << "; Count: " << j.size() << std::endl;
    auto& id = j["id"]; id = 2;
    std::cout << "id (uint16_t): " << id.as_copy<std::uint16_t>() << '\t'; // 2
    std::cout << "id: " << (id++) << '\t'; // 3
    std::cout << "id: " << id << '\t'; // 3
    std::cout << "key: " << j["key"].as<std::string>() << std::endl; // value

    std::cout << "=========[ARRAY]=========" << std::endl;
    auto& array = j["array"];
    array.push_back(9999); array.push_back("1733");
    std::cout << "Type: " << array.type_str() << "; Size: " << array.size() << std::endl;

    std::cout << "[ARRAY CONTENT]" << std::endl;
    std::cout << "element [0] (" << array[0] << ") equals to [1] ("
        << (array.at(0) == array[1] ? "true" : "false") << ")" << std::endl; // true

    std::cout << "Array iteration: [";
    // Iterating "json" class (type: Array)
    for (const auto& v : array) {
        switch (v.type()) {
            default:
                std::cout << "Default: " << v.type_str() << "; ";
                break;

            case String:
                std::cout << "String: " << v.as<std::string>() << "; ";
                break;

            case Number:
                std::cout << "Number: " << v.as<double>() << "; ";
                break;

            case Boolean:
                std::cout << "Boolean: " << v.as<bool>() << "; ";
                break;

            case Null:
                std::cout << "Null: " << v.as<std::nullptr_t>() << "; ";
                break;
        }
    }
    std::cout << "]" << std::endl;

    std::cout << "Array dump: " << array << std::endl;

    return 0;
}