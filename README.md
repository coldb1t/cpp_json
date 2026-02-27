# cpp_json

A small, single-header mini JSON library implemented in modern C++ (**C++20**).  
It focuses on a clean, type-safe API using `std::variant`, with convenient helpers for runtime type checks and typed access.
Tested on macOS, built with CLion.
Example usage in `main.cpp`

## Example

```
auto j = json::obj({
  {"id", 42},
  {"key", "value"},
  {"bool", true},
  {"null_value\n", nullptr},
  {"array", json::arr({
      "cpp", "cpp", 1.05, -0.0, false, true, nullptr, 'c', 0x5, '\n'})},
  {"users", json::arr({
      json::obj({ {"id", 1} }), json::obj({ { "id", 2 } }), json::obj(), json::obj(), json::obj() })},
  {"empty_array", json::arr({})}
});

std::cout << "JSON dump: " << j << std::endl;
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
    << (array.at(0) == array[1] ? "true" : "false") << std::endl; // true

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
```

## Output

```
JSON dump: { "id": 42, "key": "value", "bool": true, "null_value\n": null, "array": ["cpp", "cpp", 1.05, 0, false, true, null, 99, 5, 10], "users": [{ "id": 1 }, { "id": 2 }, {}, {}, {}], "empty_array": [] }
{ "id": 42, "key": "value", "bool": true, "null_value\n": null, "array": ["cpp", "cpp", 1.05, 0, false, true, null, 99, 5, 10], "users": [{ "id": 1 }, { "id": 2 }, {}, {}, {}], "empty_array": [] }

Root type: Object; Count: 7
id (uint16_t): 2	id: 3	id: 3	key: value
=========[ARRAY]=========
Type: Array; Size: 12
[ARRAY CONTENT]
element [0] ("cpp") equals to [1] (true)
Array iteration: [String: cpp; String: cpp; Number: 1.05; Number: -0; Boolean: 0; Boolean: 1; Null: nullptr; Number: 99; Number: 5; Number: 10; Number: 9999; String: 1733; ]
Array dump: ["cpp", "cpp", 1.05, 0, false, true, null, 99, 5, 10, 9999, "1733"]
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

- [ ] Consider storing different numerical values not as `double` but in native (`int64_t`, ...)
- [ ] Add iterators for ~~arrays~~ and objects
- [ ] **Add JSON parsing (`parse()` from string / stream)**
- [ ] Add benchmarks/tests
- [ ] Documentation?
