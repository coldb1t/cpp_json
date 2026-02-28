#include "cpp_json.h"
#include <iostream>

using namespace cpp_json;
int main() {
    auto j = json::obj({
        {"id", 9},
        {"bool", true},
        {"null_value", nullptr},
        {"array", json::arr({
            "cpp", 'c', 1.05, -0.0, false, nullptr, '\n'})},
        {"users", json::arr({
                json::obj(),
                json::obj({ {"id", 1},
                    {"data", json::obj({{"name", "Joe Griffin"}, {"phone", "+1-207-883-5350"}}) } }),
                json::obj({ {"id", 2},
                    {"data", json::obj({{"name", "John Clark"}, {"phone", "+1-817-473-1454"}}) } }),
            })},
        {"empty_array", json::arr({})},
        {"array_with_empty_objects", json::arr({json::obj(), json::obj()})}
    });

    std::cout << "JSON dump: " << std::endl;
    std::cout << j.dump(j_dump_format::Pretty) << std::endl;

    std::cout << "Root type: " << j.type_str() << "; Size: " << j.size() << std::endl;
    auto& id = j["id"]; id = 1.05;
    std::cout << "id (as uint16_t): " << id.as_copy<uint16_t>() << '\t'; // 1
    std::cout << "id: " << ++id << '\t' << std::endl; // ~2.05

    std::cout << "=========[ARRAY]=========" << std::endl;
    auto& array = j["array"];
    std::cout << "Type: " << array.type_str() << "; Size: " << array.size() << std::endl;

    std::cout << "element[0] (" << array[0] << ") equals to element[1] ("
        << array[1] << "): "
        << (array.at(0) == array[1] ? "true" : "false") << std::endl; // true

    std::cout << "Array iteration: [ ";
    // Iterating "json" class (type: Array)
    for (const auto& v : array) {
        std::cout << v << " (" << v.type_str() << ") ";
    }
    std::cout << "]" << std::endl;

    std::cout << "Array dump: " << array.dump(j_dump_format::Compact) << std::endl;

    return 0;
}