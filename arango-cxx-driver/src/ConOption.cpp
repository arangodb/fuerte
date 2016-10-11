#include "../include/fuerte/ConOption.h"

namespace arangodb {

namespace dbinterface {

ConOption::ConOption(const ConOption& inp)
    : _name{inp._name}, _value{inp._value} {
  if (_value.cType() == CType::String) {
    _str = inp._str;
    new (&_value) Value(_str);
  }
}

template <>
ConOption::ConOption<std::string>(const std::string& name,
                                  const std::string& value)
    : _name{name}, _str{value}, _value{_str} {}

//
// IMPORTANT
// Value object only holds a pointer to a std::string object,  so a copy is
// required
//
template <>
inline void ConOption::setValue<std::string>(const std::string& val) {
  _str = val;
  if (_value.cType() != CType::String) {
    new (&_value) Value{_str};
  }
}

ConOption& ConOption::operator=(const ConOption& inp) {
  _name = inp._name;
  _value = inp._value;
  if (_value.cType() == CType::String) {
    _str = inp._str;
    new (&_value) Value(_str);
  }
  return *this;
}

std::string ConOption::valueString() const {
  switch (_value.cType()) {
    case CType::UInt64: {
      return std::to_string(_value.getUInt64());
    }

    case CType::Int64: {
      return std::to_string(_value.getInt64());
    }

    case CType::Double: {
      return std::to_string(_value.getDouble());
    }

    case CType::CharPtr: {
      return std::string{_value.getCharPtr()};
    }

    case CType::String: {
      return _str;
    }

    case CType::Bool: {
      return std::string{_value.getBool() ? "true" : "false"};
    }
    default:;
  }
  return std::string{};
}

const std::string ConOption::headerString() const {
  return std::string{_name + ':' + valueString()};
}

const std::string ConOption::queryString() const {
  return std::string{_name + '=' + valueString()};
}
}
}
