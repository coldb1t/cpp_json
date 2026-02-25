# cpp_json

A small, single-header mini JSON library implemented in modern C++ (**C++20**).  
It focuses on a clean, type-safe API using `std::variant`, with convenient helpers for runtime type checks and typed access.
Tested on Mac OS, built with CLion.
Example usage in `main.cpp`

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
- [ ] Add `const` overload for `try_as<T>()`
- [ ] Consider addding `as_copy<T>()` helper that returns a value copy / conversion
- [ ] Consider storing different numerical values not as `double` but in native (`int64_t`, ...)
- [ ] Decide whether `json(const char*)` should treat `nullptr` as `null` instead of empty string
- [ ] Add `contains(key)` helper for objects
- [ ] Add `erase(key)` and `erase(idx)` helpers
- [ ] Add iterators for arrays and objects
- [ ] **Add JSON serialization (`dump()` / `to_string()`)**
- [ ] **Add JSON parsing (`parse()` from string / stream)**
- [ ] Add benchmarks/tests
- [ ] Documentation?
