#pragma once

/*
  cpp_json
*/

/*
  Config
*/
#define CPP_JSON_INCLUDE_NAMED_CASTS 1
#define CPP_JSON_USE_CHAR_CONVERT_ON_DOUBLES 1
#define CPP_JSON_THROW_ON_BAD_COPY_CAST 0
#define CPP_JSON_DEFAULT_DUMP_FORMAT j_dump_format::Pretty

/*
  Includes
*/
#include <cstddef>
#include <ostream>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <limits>
#include <concepts>
#include <cmath>

#if CPP_JSON_USE_CHAR_CONVERT_ON_DOUBLES == 1
#include <charconv>
#endif

namespace cpp_json {
  enum class j_value_type : uint8_t
  {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
  };

  enum class j_dump_format : uint8_t {
    Compress,
    Compact,
    Pretty
  };

#define CPP_JSON_ASSERT_TYPE(type) \
  do { if (!is<type>()) { throw type_error((std::string)__func__ + " on non-" + #type); } } while(0);

  class json {
  public:
    using array = std::vector<json>;
    using object = std::vector<std::pair<std::string, json>>;

  private:
    using storage = std::variant<std::nullptr_t, bool, double, std::string, array, object>;
    storage v_{nullptr};

    static std::runtime_error type_error(const std::string& msg) {
      return std::runtime_error(std::string("cpp_json error: ") + msg);
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

    template <class V> requires std::is_arithmetic_v<std::remove_cvref_t<V>>
    json(V v) {
      using T = std::remove_cvref_t<V>;

      if constexpr (std::same_as<T, bool>) {
        v_ = static_cast<bool>(v);
      } else {
        v_ = static_cast<double>(v);
      }
    }

    json(std::nullptr_t) {}

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
#if CPP_JSON_INCLUDE_NAMED_CASTS == 1
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

    [[nodiscard]] j_value_type type() const noexcept {
      return std::visit([]<typename T0>(const T0&) -> j_value_type {
        using T = std::decay_t<T0>;
        if constexpr (std::is_same_v<T, std::nullptr_t>) return j_value_type::Null;
        else if constexpr (std::is_same_v<T, bool>) return j_value_type::Boolean;
        else if constexpr (std::is_same_v<T, double>) return j_value_type::Number;
        else if constexpr (std::is_same_v<T, std::string>) return j_value_type::String;
        else if constexpr (std::is_same_v<T, array>) return j_value_type::Array;
        else return j_value_type::Object;
      }, v_);
    }

    [[nodiscard]] const char* type_str() const noexcept {
      switch (type()) {
        default: return "Null";
        case j_value_type::Boolean: return "Boolean";
        case j_value_type::Number: return "Number";
        case j_value_type::String: return "String";
        case j_value_type::Array: return "Array";
        case j_value_type::Object: return "Object";
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
#if CPP_JSON_INCLUDE_NAMED_CASTS == 1
    std::string& as_str() {
      CPP_JSON_ASSERT_TYPE(std::string)

      return std::get<std::string>(v_);
    }

    double& as_num() {
      CPP_JSON_ASSERT_TYPE(double)

      return std::get<double>(v_);
    }

    object& as_obj() {
      CPP_JSON_ASSERT_TYPE(object)

      return std::get<object>(v_);
    }

    array& as_arr() {
      CPP_JSON_ASSERT_TYPE(array)

      return std::get<array>(v_);
    }

    [[nodiscard]] const std::string& as_str() const {
      CPP_JSON_ASSERT_TYPE(std::string)

      return std::get<std::string>(v_);
    }

    [[nodiscard]] const double& as_num() const {
      CPP_JSON_ASSERT_TYPE(double)

      return std::get<double>(v_);
    }

    [[nodiscard]] const object& as_obj() const {
      CPP_JSON_ASSERT_TYPE(object)

      return std::get<object>(v_);
    }

    [[nodiscard]] const array& as_arr() const {
      CPP_JSON_ASSERT_TYPE(array)

      return std::get<array>(v_);
    }
#endif
    template<typename T> requires std::is_arithmetic_v<T>
    T as_copy() const {
      switch (type()) {
        default: {
#if CPP_JSON_THROW_ON_BAD_COPY_CAST == 1
          throw type_error("invalid type cast (copy)");
#else
          break;
#endif
        }
        case j_value_type::Number: return static_cast<T>(as<double>());
        case j_value_type::Boolean: return static_cast<T>(as<bool>());
        case j_value_type::Null: return static_cast<T>(0);
      }

      return T{};
    }

    template<typename T> requires std::same_as<T, std::string>
    T as_copy() const {
      std::string out;
      switch (type()) {
        default: {
#if CPP_JSON_THROW_ON_BAD_COPY_CAST == 1
          throw type_error("invalid type cast (copy)");
#endif
          break;
        }
        case j_value_type::Number: {
          this->dump_double(this->as<double>(), out);
          return out;
        }
        case j_value_type::Boolean: return as<bool>() ? "true" : "false";
        case j_value_type::Null: return "null";
        case j_value_type::String: {
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
    [[nodiscard]] inline T* try_as() noexcept {
      return std::get_if<T>(&v_);
    }

    template<class T> requires allowed_v<T>
    [[nodiscard]] inline const T* try_as() const noexcept {
      return std::get_if<T>(&v_);
    }
#pragma endregion

#pragma region operators
    // Objects
    json& operator[](const char* key) {
      CPP_JSON_ASSERT_TYPE(object)

      auto& o = as<object>();
      for (auto& [k, v] : o) {
        if (k == key) {
          return v;
        }
      }

      o.emplace_back(std::string(key), json(nullptr));
      return o.back().second;
    }

    const json& operator[](const char* key) const {
      CPP_JSON_ASSERT_TYPE(object)

      for (auto& [k, v] : as<object>()) {
        if (k == key) {
          return v;
        }
      }

      throw type_error("operator[](key) -> key does not exists");
    }

    // json
    inline friend bool operator==(const json& a, const json& b) noexcept {
      return a.v_ == b.v_;
    }

    // Arrays
    template<std::integral T>
    json& operator[](const T idx) {
      return this->at(static_cast<size_t>(idx));
    }

    template<std::integral T>
    [[nodiscard]] const json& operator[](const T idx) const {
      return this->at(static_cast<size_t>(idx));
    }

    // ostream
    friend std::ostream& operator<<(std::ostream& os, const json& j) {
      return os << j.dump();
    }

    // String
    json& operator+=(const std::string& rhs) {
      CPP_JSON_ASSERT_TYPE(std::string)

      auto& str = as<std::string>();
      str += rhs;

      return *this;
    }

    // Number
    template<typename T> requires std::is_arithmetic_v<T>
    json& operator+=(const T& rhs) {
      CPP_JSON_ASSERT_TYPE(double)

      as<double>() += static_cast<double>(rhs);
      return *this;
    }

    template<typename T> requires std::is_arithmetic_v<T>
    json& operator-=(const T& rhs) {
      CPP_JSON_ASSERT_TYPE(double)

      as<double>() -= static_cast<double>(rhs);
      return *this;
    }

    json& operator++() {
      CPP_JSON_ASSERT_TYPE(double)

      ++as<double>();
      return *this;
    }

    json& operator--() {
      CPP_JSON_ASSERT_TYPE(double)

      --as<double>();
      return *this;
    }

    template<typename T> requires std::is_arithmetic_v<T>
    json operator+(const T& rhs) const {
      CPP_JSON_ASSERT_TYPE(double)

      return as<double>() + static_cast<double>(rhs);
    }

    template<typename T> requires std::is_arithmetic_v<T>
    json operator-(const T& rhs) const {
      CPP_JSON_ASSERT_TYPE(double)

      return as<double>() - static_cast<double>(rhs);
    }
#pragma endregion

#pragma region arrays
    // Iteration
    [[nodiscard]] array::iterator begin() {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().begin();
    }

    [[nodiscard]] array::iterator end() {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().end();
    }

    [[nodiscard]] array::const_iterator begin() const {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().begin();
    }

    [[nodiscard]] array::const_iterator end() const {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().end();
    }

    [[nodiscard]] json& front() {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().front();
    }

    [[nodiscard]] json& back() {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().back();
    }

    json& at(const size_t idx) {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().at(idx);
    }

    [[nodiscard]] const json& at(const size_t idx) const {
      CPP_JSON_ASSERT_TYPE(array)

      return as<array>().at(idx);
    }

    // Inserting
    void push_back(json value) {
      CPP_JSON_ASSERT_TYPE(array)

      as<array>().push_back(std::move(value));
    }

    void insert(const size_t idx, json value) {
      CPP_JSON_ASSERT_TYPE(array)

      auto& a = as<array>();
      if (idx > a.size()) {
        throw type_error("index out of range");
      }

      a.insert(a.begin() + static_cast<std::ptrdiff_t>(idx), std::move(value));
    }

    // Erasing
    void erase(const size_t idx) {
      CPP_JSON_ASSERT_TYPE(array)

      auto& a = as<array>();
      if (idx >= a.size()) {
        throw type_error("index out of range");
      }

      a.erase(a.begin() + static_cast<std::ptrdiff_t>(idx));
    }
#pragma endregion

#pragma region objects
    [[nodiscard]] bool contains_key(const char* key) const {
      CPP_JSON_ASSERT_TYPE(object)

      for (const auto &p: as<object>()) {
        if (p.first == key) {
          return true;
        }
      }

      return false;
    }

    bool erase_by_key(const char* key) {
      if (!is<object>()) {
        throw type_error("erase_by_key(key) on non-object");
      }

      auto& o = as<object>();
      for (auto i = 0llu; i < o.size(); i++) {
        if (o[i].first == key) {
          o.erase(o.begin() + static_cast<std::ptrdiff_t>(i));
          return true;
        }
      }

      return false;
    }
#pragma endregion

#pragma region size
    [[nodiscard]] size_t size() const noexcept {
      switch (type()) {
        default: break;
        case j_value_type::Array: return as<array>().size();
        case j_value_type::Object: return as<object>().size();
        case j_value_type::String: return as<std::string>().size();
      }

      return 0llu;
    }

    [[nodiscard]] inline bool empty() const noexcept {
      return size() == 0llu;
    }
#pragma endregion

    inline void make_null() noexcept {
      v_ = nullptr;
    }
  private:
    struct dump_data {
      j_dump_format format = CPP_JSON_DEFAULT_DUMP_FORMAT;
      size_t tab = 0llu;

      [[nodiscard]] inline bool pretty() const {
        return format == j_dump_format::Pretty;
      }

      [[nodiscard]] inline bool minimal() const {
        return format == j_dump_format::Compress;
      }

      [[nodiscard]] inline bool compact() const {
        return format == j_dump_format::Compact;
      }

      dump_data& operator++(int) {
        if (pretty()) {
          tab++;
        }

        return *this;
      }

      dump_data& operator--(int) {
        if (pretty() && tab > 0llu) {
          tab--;
        }

        return *this;
      }
    };

    void dump_object(const json& j, std::string& out, dump_data& data) const {
      const auto& o = j.as<object>();
      const auto sz = o.size();

      if (sz == 0llu) {
        out += "{}";
        return;
      }

      out += (data.pretty() || data.minimal()) ? "{" : "{ ";
      data++;

      for (auto idx = 0llu; idx < sz; idx++) {
        const auto& [k, v] = o[idx];
        if (data.pretty()) {
          out.push_back('\n');
          out.insert(out.end(), data.tab, '\t');
        }

        out.push_back('"');
        dump_string(k, out);
        out += data.minimal() ? "\":" : "\": ";
        dump_internal(v, out, data);

        if (idx != sz - 1llu) {
          out += ((data.pretty() || data.minimal()) ? "," : ", ");
        }
      }

      data--;

      if (data.pretty()) {
        out += "\n";
        out.insert(out.end(), data.tab, '\t');
        out += "}";
      } else {
        out += data.minimal() ? "}" : " }";
      }
    }

    void dump_array(const json& j, std::string& out, dump_data& data) const {
      const auto& a = j.as<array>();
      const auto sz = a.size();
      if (sz == 0llu) {
        out += "[]";
        return;
      }

      switch (data.format) {
        default: out += "[ "; break;
        case j_dump_format::Pretty: out += "[\n"; break;
        case j_dump_format::Compress: out += "["; break;
      }
      data++;

      for (auto idx = 0llu; idx < sz; idx++) {
        const auto& v = a.at(idx);
        if (data.pretty()) {
          out.insert(out.end(), data.tab, '\t');
        }
        dump_internal(v, out, data);

        if (idx != sz - 1llu) {
          switch (data.format) {
            default: out += ","; break;
            case j_dump_format::Compact: out += ", "; break;
            case j_dump_format::Pretty: out += ",\n"; break;
          }
        }
      }

      data--;

      if (data.pretty()) {
        out.push_back('\n');
        out.insert(out.end(), data.tab, '\t');
      }

      out += data.compact() ? " ]" : "]";
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void dump_string(const std::string& s, std::string& out) const {
      static constexpr char hex[] = "0123456789abcdef";
      for (unsigned char c : s) {
        switch (c) {
          default: {
            if (c < 0x20) { //uXXXX, control chars
              out += "\\u00";
              out.push_back(hex[c / 16]);
              out.push_back(hex[c % 16]);
              break;
            }

            out.push_back(static_cast<char>(c));
            break;
          }
            //\ + "\/bfnrt
          case '"': out += "\\\""; break;
          case '\b': out += "\\b"; break;
          case '\f': out += "\\f"; break;
          case '\n': out += "\\n"; break;
          case '\r': out += "\\r"; break;
          case '\t': out += "\\t"; break;
          case '\\': out += "\\\\"; break;
        }
      }
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void dump_double(const double& d, std::string& out) const {
      if (!std::isfinite(d)) {
        out += "null";
        return;
      }

      if (d == 0.0) { //-0.0
        out += "0";
        return;
      }

#if CPP_JSON_USE_CHAR_CONVERT_ON_DOUBLES == 1
      char buf[64];
      auto [ptr, ec] = std::to_chars(
          buf,
          buf + sizeof(buf),
          d,
          std::chars_format::general,
          std::numeric_limits<double>::max_digits10
      );

      if (ec == std::errc{}) {
        out.append(buf, ptr);
      } else {
        out += "null";
      }
#else
      out += std::to_string(d);
#endif
    }

    void dump_internal(const json& j, std::string& out, dump_data& data) const {
      const auto type_ = j.type();
      switch (type_) {
        default: break;
        case j_value_type::Object: dump_object(j, out, data); break;
        case j_value_type::Array: dump_array(j, out, data); break;
        case j_value_type::String: {
          out.push_back('"');
          dump_string(j.as<std::string>(), out);
          out.push_back('"');
          break;
        }
        case j_value_type::Number: dump_double(j.as<double>(), out); break;
        case j_value_type::Boolean: out += j.as<bool>() ? "true" : "false"; break;
        case j_value_type::Null: out += "null"; break;
      }

    }
  public:
    [[nodiscard]] std::string dump(const j_dump_format dump_format = CPP_JSON_DEFAULT_DUMP_FORMAT) const {
      std::string out;

      dump_data data{.format = dump_format};
      dump_internal(*this, out, data);

      return out;
    }
  };
}