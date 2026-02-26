#pragma once

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#define INCLUDE_NAMED_CASTS 1

namespace cpp_json {
  enum class j_value_type : std::uint8_t
  {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
  };

  using enum j_value_type;

  class json {
  public:
    using array = std::vector<json>;
    using object = std::vector<std::pair<std::string, json>>;

  private:
    using storage = std::variant<std::nullptr_t, bool, double, std::string, array, object>;
    storage v_{nullptr};

    static std::runtime_error type_error(const char* msg) {
      return std::runtime_error(std::string("cpp_json error: ") + std::string(msg));
    }

    template<class TT>
    static constexpr bool allowed_v =
        std::same_as<TT, bool> ||
        std::same_as<TT, double> ||
        std::same_as<TT, std::string> ||
        std::same_as<TT, array> ||
        std::same_as<TT, object> ||
        std::same_as<TT, std::nullptr_t>;
  public:
    json() noexcept = default;

    template <class V>
    requires std::is_arithmetic_v<std::remove_cvref_t<V>>
    json(V v) {
      using T = std::remove_cvref_t<V>;

      if constexpr (std::same_as<T, bool>) {
        v_ = static_cast<bool>(v);
      } else {
        v_ = static_cast<double>(v);
      }
    }

    template<typename V>
    requires std::same_as<V, std::nullptr_t>
    json(const V v) {
      v_ = nullptr;
    };

    json(const char* s) : v_(std::string(s ? s : "")) {}
    json(std::string s) : v_(std::move(s)) {}

    json(array a) : v_(std::move(a)) {}
    json(object o) : v_(std::move(o)) {}

    static json arr(const std::initializer_list<json> items = {}) {
      return {array(items)};
    }

    static json obj(const std::initializer_list<std::pair<const char*, json>> items = {}) {
      object o;
      o.reserve(items.size());

      for (auto const& [k, v] : items) {
        o.emplace_back(std::string(k), v);
      }

      return json{std::move(o)};
    }

    json(const json&) = default;
    json(json&&) noexcept = default;
    json& operator=(const json&) = default;
    json& operator=(json&&) noexcept = default;
    ~json() = default;

#pragma region type checks
#if INCLUDE_NAMED_CASTS == 1
    [[nodiscard]] inline bool is_str() const noexcept {
      return is<String>();
    }

    [[nodiscard]] inline bool is_arr() const noexcept {
      return is<array>();
    }

    [[nodiscard]] inline bool is_obj() const noexcept {
      return is<object>();
    }

    [[nodiscard]] inline bool is_null() const noexcept {
      return is<Null>();
    }

    [[nodiscard]] inline bool is_num() const noexcept {
      return is<Number>();
    }

    [[nodiscard]] inline bool is_bool() const noexcept {
      return is<Boolean>();
    }
#endif

    [[nodiscard]] inline j_value_type type() const noexcept {
      return static_cast<j_value_type>(v_.index());
    }

    [[nodiscard]] const char* type_str() const noexcept {
      switch (type()) {
        default:
          return "Null";

        case Boolean:
          return "Boolean";
        case Number:
          return "Number";
        case String:
          return "String";
        case Array:
          return "Array";
        case Object:
          return "Object";
      }
    }

    template <j_value_type V>
    [[nodiscard]] inline bool is() const {
      return type() == V;
    }

    template<class T> requires allowed_v<T>
    [[nodiscard]] inline bool is() const {
      return std::holds_alternative<T>(v_);
    }
#pragma endregion

#pragma region type casts
#if INCLUDE_NAMED_CASTS == 1
    std::string& as_str() {
      if (!is<String>()) {
        throw type_error("invalid type cast");
      }

      return std::get<std::string>(v_);
    }

    double& as_num() {
      if (!is<Number>()) {
        throw type_error("invalid type cast");
      }

      return std::get<double>(v_);
    }

    object& as_obj() {
      if (!is<Object>()) {
        throw type_error("invalid type cast");
      }

      return std::get<object>(v_);
    }

    array& as_arr() {
      if (!is<Array>()) {
        throw type_error("invalid type cast");
      }

      return std::get<array>(v_);
    }
#endif
    template<class T> requires allowed_v<T>
    T& as() {
      if (!is<T>()) {
        throw type_error("invalid type cast");
      }

      return std::get<T>(v_);
    }

    template<class T> requires allowed_v<T>
    [[nodiscard]] const T& as() const {
      if (!is<T>()) {
        throw type_error("invalid type cast");
      }

      return std::get<T>(v_);
    }

    template<class T> requires allowed_v<T>
    T* try_as() {
      return std::get_if<T>(&v_);
    }
#pragma endregion

#pragma region operators
    // Objects
    json& operator[](const char* key) {
      if (!is<object>()) {
        throw type_error("operator[](key) on a non-object");
      }

      auto& o = as<object>();
      for (auto& [k, v] : o) {
        if (k == key) {
          return v;
        }
      }

      o.emplace_back(std::string(key), json(nullptr));
      return o.back().second;
    }

    // json
    friend bool operator==(const json& a, const json& b) noexcept {
      return a.v_ == b.v_;
    }

    // Arrays
    json& operator[](const size_t key) {
      if (!is<Array>()) {
        throw type_error("invalid type cast");
      }

      return as<array>().at(key);
    }
#pragma endregion

#pragma region arrays
    json& at(const size_t idx) {
      if (!is<array>()) {
        throw type_error("at(idx) on non-array");
      }

      return as<array>().at(idx);
    }

    [[nodiscard]] const json& at(const size_t idx) const {
      if (!is<array>()) {
        throw type_error("at(idx) on non-array");
      }

      return as<array>().at(idx);
    }

    void push_back(json value) {
      if (!is<array>()) {
        throw type_error("push_back() on non-array");
      }

      as<array>().push_back(std::move(value));
    }

    void insert(const size_t idx, json value) {
      if (!is<array>()) {
        throw type_error("insert(idx) on non-array");
      }

      auto& a = as<array>();
      if (idx > a.size()) {
        throw type_error("index out of range");
      }

      a.insert(a.begin() + idx, std::move(value));
    }

    [[nodiscard]] json& front() {
      if (!is<array>()) {
        throw type_error("front() on non-array");
      }

      return as<array>().front();
    }

    [[nodiscard]] json& back() {
      if (!is<array>()) {
        throw type_error("back() on non-array");
      }

      return as<array>().back();
    }
#pragma endregion

#pragma region size
    [[nodiscard]] size_t size() const noexcept {
      switch (type()) {
        default:
          break;

        case Array:
          return as<array>().size();

        case Object:
          return as<object>().size();

        case String:
          return as<std::string>().size();
      }

      return 0llu;
    }

    [[nodiscard]] bool empty() const noexcept {
      return size() == 0llu;
    }
#pragma endregion

    void make_null() noexcept {
      v_ = nullptr;
    }

  private:
    void dump_object(const json& j, std::string& out) const {
      const auto& o = j.as<object>();
      if (o.empty()) {
        out += "{}";
        return;
      }

      out += "{ ";

      for (auto idx = 0llu; idx < o.size(); idx++) {
        const auto& [k, v] = o[idx];

        dump_string(k, out);
        out += ": ";
        dump_internal(v, out);

        if (idx != o.size() - 1llu) {
          out += ", ";
        }
      }
      out += " }";
    }

    void dump_array(const json& j, std::string& out) const {
      const auto& a = j.as<array>();
      if (a.empty()) {
        out += "[]";
        return;
      }

      out.push_back('[');
      for (auto idx = 0llu; idx < a.size(); idx++) {
        const auto& v = a.at(idx);
        dump_internal(v, out);
        if (idx != a.size() - 1llu) {
          out += ", ";
        }
      }

      out.push_back(']');
    }

    void dump_string(const std::string& s, std::string& out) const {
      out.push_back('"');
      for (const auto& c : s) {
        switch (c) {
          default:
            //\uXXXX
            out.push_back(c);
            break;
            //\ + "\/bfnrt
          case '"':
            out += "\\\"";
            break;
          case '\b':
            out += "\\b";
            break;
          case '\f':
            out += "\\f";
            break;
          case '\n':
            out += "\\n";
            break;
          case '\r':
            out += "\\r";
            break;
          case '\t':
            out += "\\t";
            break;
          case '\\':
            out += "\\\\";
            break;
        }
      }
      out.push_back('"');
    }

    void dump_double(const double& d, std::string& out) const {
      if (const auto d_i64 = static_cast<int64_t>(d); d == static_cast<double>(d_i64)) {
        out += std::to_string(d_i64);
        return;
      }

      out += std::to_string(d);
    }

    void dump_internal(const json& j, std::string& out) const {
      switch (j.type()) {
        case Object:
          dump_object(j, out);
          break;
        case Array:
          dump_array(j, out);
          break;
        case String:
          dump_string(j.as<std::string>(), out);
          break;
        case Number: {
          dump_double(j.as<double>(), out);
          break;
        }
        case Boolean:
          out += j.as<bool>() ? "true" : "false";
          break;
        case Null:
          out += "null";
          break;
      }
    }

  public:
    [[nodiscard]] std::string dump() const {
      std::string out;

      dump_internal(*this, out);

      return out;
    }
  };
}