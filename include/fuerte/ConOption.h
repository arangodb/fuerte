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
