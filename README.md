# cpp_json

A small, single-header mini JSON library implemented in modern C++ (**C++20**).  
It focuses on a clean, type-safe API using `std::variant`, with convenient helpers for runtime type checks and typed access.
Tested on macOS, built with CLion.
Example usage in `main.cpp`

## Example
```
#include <iostream>

#include "cpp_json.h"

using namespace cpp_json;
int main() {
    auto j = json::obj({
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
    const auto& a_0 = array.at(0);
    std::cout << "a_0: " << a_0.as<std::string>() << std::endl;
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
```

## Output
```
Root object type: Object; objects count: 3
id: 42
id: 0
id: 66
key: value

array type: Array; size: 11
[array content]
1
a_0: cpp
str: cpp
str: json
num: 1
bool: 0
bool: 1
null: nullptr
num: 99
num: 5
num: 10
num: 9999
str: 1733
```

## Features

- Single-include header `cpp_json.h`
- Uses `std::variant` (no manual memory management)
- Supports the core JSON value categories:
  - `Null`
  - `Boolean`
  - `Number` (stored as `double`)
  - `String`
  - `Array`
  - `Object`
- Two-three styles of type checks:
  - By `j_value_type` enum: `is<j_value_type::String>()` or simply `is<String>()`
  - By stored C++ type: `is<std::string>()`
  - By named function like `is_str()` if `INCLUDE_NAMED_CASTS` is defined as `1`
- Typed accessors:
  - `as<T>()` throws on mismatch
  - `try_as<T>()` returns pointer (or `nullptr`)
 
## TODO
- [x] ~~Add clamp idx or throw an error in `insert(idx)` on attempting to insert out of range~~
- [ ] Add `const` overload for `try_as<T>()`
- [x] ~~Add `const` overloads for named getters (`as_str`, ...)~~
- [x] ~~Consider addding `as_copy<T>()` helper that returns a value copy / conversion~~
- [ ] Consider storing different numerical values not as `double` but in native (`int64_t`, ...)
- [ ] Decide whether `json(const char*)` should treat `nullptr` as `null` instead of empty string
- [ ] Add `contains(key)` helper for objects
- [ ] Add `erase(key)` ~~and `erase(idx)` helpers~~
- [ ] Add iterators for ~~arrays~~ and objects
- [ ] **~~Add JSON serialization (`dump()`)~~, add "pretty" format**
- [ ] **Add JSON parsing (`parse()` from string / stream)**
- [ ] Add benchmarks/tests
- [ ] Documentation?
