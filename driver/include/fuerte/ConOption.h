////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author John Bufton
////////////////////////////////////////////////////////////////////////////////

#ifndef FUERTE_CONOPTION
#define FUERTE_CONOPTION

namespace arangodb {
namespace dbinterface {
class ConOption {
 public:
  ConOption() = delete;
  explicit ConOption(const std::string& name, const std::string& value);
  const std::string headerString() const;
  const std::string queryString() const;
  const std::string& name() const;
  const std::string& value() const;
  void setName(const std::string& val);
  void setValue(const std::string& val);

 private:
  std::string _name;
  std::string _value;
};

inline ConOption::ConOption(const std::string& name, const std::string& value)
    : _name(name), _value(value) {}

inline const std::string ConOption::headerString() const {
  return std::string{_name + ':' + _value};
}

inline const std::string ConOption::queryString() const {
  return std::string{_name + '=' + _value};
}

inline const std::string& ConOption::name() const { return _name; }

inline const std::string& ConOption::value() const { return _value; }

inline void ConOption::setName(const std::string& val) { _name = val; }

inline void ConOption::setValue(const std::string& val) { _value = val; }
}
}

#endif
