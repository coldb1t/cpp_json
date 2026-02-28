# cpp_json

A small, single-header mini JSON library implemented in modern C++ (**C++20**).  
It focuses on a clean, type-safe API using `std::variant`, with convenient helpers for runtime type checks and typed access.
Tested on macOS, built with CLion.
Example usage in `main.cpp`

## Example

```c++
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
```

## Output

```
JSON dump: 
{
	"id": 9,
	"bool": true,
	"null_value": null,
	"array": [
		"cpp",
		99,
		1.05,
		0,
		false,
		null,
		10
	],
	"users": [
		{},
		{
			"id": 1,
			"data": {
				"name": "Joe Griffin",
				"phone": "+1-207-883-5350"
			}
		},
		{
			"id": 2,
			"data": {
				"name": "John Clark",
				"phone": "+1-817-473-1454"
			}
		}
	],
	"empty_array": [],
	"array_with_empty_objects": [
		{},
		{}
	]
}
Root type: Object; Size: 7
id (as uint16_t): 1	id: 2.0499999999999998	
=========[ARRAY]=========
Type: Array; Size: 7
element[0] ("cpp") equals to element[1] (99): false
Array iteration: [ "cpp" (String) 99 (Number) 1.05 (Number) 0 (Number) false (Boolean) null (Null) 10 (Number) ]
Array dump: [ "cpp", 99, 1.05, 0, false, null, 10 ]
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
