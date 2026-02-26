#pragma once

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// Config
#define INCLUDE_NAMED_CASTS 1
#define THROW_ON_BAD_COPY_CAST 0
#define USE_CHAR_CONVERT_ON_DOUBLES 1

//
#if USE_CHAR_CONVERT_ON_DOUBLES == 1
#include <charconv>
#endif

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
      return is<std::string>();
    }

    [[nodiscard]] inline bool is_arr() const noexcept {
      return is<array>();
    }

    [[nodiscard]] inline bool is_obj() const noexcept {
      return is<object>();
    }

    [[nodiscard]] inline bool is_null() const noexcept {
      return is<std::nullptr_t>();
    }

    [[nodiscard]] inline bool is_num() const noexcept {
      return is<double>();
    }

    [[nodiscard]] inline bool is_bool() const noexcept {
      return is<bool>();
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

    [[nodiscard]] const std::string& as_str() const {
      if (!is<String>()) {
        throw type_error("invalid type cast");
      }

      return std::get<std::string>(v_);
    }

    [[nodiscard]] const double& as_num() const {
      if (!is<Number>()) {
        throw type_error("invalid type cast");
      }

      return std::get<double>(v_);
    }

    [[nodiscard]] const object& as_obj() const {
      if (!is<Object>()) {
        throw type_error("invalid type cast");
      }

      return std::get<object>(v_);
    }

    [[nodiscard]] const array& as_arr() const {
      if (!is<Array>()) {
        throw type_error("invalid type cast");
      }

      return std::get<array>(v_);
    }
#endif
    template<typename T> requires std::is_arithmetic_v<T>
    T as_copy() const {
      switch (type()) {
        default: {
#if THROW_ON_BAD_COPY_CAST == 1
          throw type_error("invalid type cast (copy)");
#endif
          break;
        }
        case Number:
          return static_cast<T>(as<double>());
        case Boolean:
          return static_cast<T>(as<bool>());
        case Null:
          return static_cast<T>(0);
      }


      return T{};
    }

    template<typename T> requires std::same_as<T, std::string>
    T as_copy() const {
      std::string out;
      switch (type()) {
        default: {
#if THROW_ON_BAD_COPY_CAST == 1
          throw type_error("invalid type cast (copy)");
#endif
          break;
        }

        case Number: {
          this->dump_double(this->as<double>(), out);
          return out;
        }

        case Boolean:
          return as<bool>() ? "true" : "false";

        case Null:
          return "null";

        case String: {
          this->dump_string(this->as<std::string>(), out);
          return out;
        }
      }

      return out;
    }

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
    inline friend bool operator==(const json& a, const json& b) noexcept {
      return a.v_ == b.v_;
    }

    // Arrays
    json& operator[](const size_t key) {
      return this->at(key);
    }

    [[nodiscard]] const json& operator[](const size_t key) const {
      return this->at(key);
    }

    // ostream
    friend std::ostream& operator<<(std::ostream& os, const json& j) {
      return os << j.dump();
    }
#pragma endregion

#pragma region arrays
    // Iteration
    [[nodiscard]] array::iterator begin() {
      if (!is<array>()) {
        throw type_error("begin() on non-array");
      }

      return as<array>().begin();
    }

    [[nodiscard]] array::iterator end() {
      if (!is<array>()) {
        throw type_error("end() on non-array");
      }

      return as<array>().end();
    }

    [[nodiscard]] array::const_iterator begin() const {
      if (!is<array>()) {
        throw type_error("begin() on non-array");
      }

      return as<array>().begin();
    }

    [[nodiscard]] array::const_iterator end() const {
      if (!is<array>()) {
        throw type_error("end() on non-array");
      }

      return as<array>().end();
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

    // Inserting
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

    // Erasing
    void erase(const size_t idx) {
      if (!is<array>()) {
        throw type_error("erase(idx) on non-array");
      }

      auto& a = as<array>();
      if (idx > a.size()) {
        throw type_error("index out of range");
      }

      a.erase(a.begin() + idx);
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

      const auto sz = o.size();
      for (auto idx = 0llu; idx < sz; idx++) {
        const auto& [k, v] = o[idx];

        out.push_back('"');
        dump_string(k, out);
        out += "\": ";
        dump_internal(v, out);

        if (idx != sz - 1llu) {
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
      const auto sz = a.size();
      for (auto idx = 0llu; idx < sz; idx++) {
        const auto& v = a.at(idx);
        dump_internal(v, out);
        if (idx != sz - 1llu) {
          out += ", ";
        }
      }

      out.push_back(']');
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void dump_string(const std::string& s, std::string& out) const {
      static constexpr char hex[] = "0123456789abcdef";
      for (unsigned char c : s) {
        switch (c) {
          default: {
            //uXXXX, control chars
            if (c < 0x20) {
              out += "\\u00";
              out.push_back(hex[c / 16]);
              out.push_back(hex[c % 16]);
              break;
            }

            out.push_back(static_cast<char>(c));
            break;
          }
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
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void dump_double(const double& d, std::string& out) const {
      if (!std::isfinite(d)) {
        out += "null";
        return;
      }

      if (const auto d_i64 = static_cast<int64_t>(d); d == static_cast<double>(d_i64)) {
        out += std::to_string(d_i64);
        return;
      }

#if USE_CHAR_CONVERT_ON_DOUBLES == 1
      char buf[64];
      auto [ptr, ec] = std::to_chars(
          buf,
          buf + sizeof(buf),
          d,
          std::chars_format::general,
          std::numeric_limits<double>::max_digits10
      );

      out.append(buf, ptr);
#else
      out += std::to_string(d);
#endif
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
          out.push_back('"');
          dump_string(j.as<std::string>(), out);
          out.push_back('"');
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