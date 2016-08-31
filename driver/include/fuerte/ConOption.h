#ifndef FUERTE_CONOPTION
#define FUERTE_CONOPTION

#include <vector>
#include <velocypack/Value.h>

namespace arangodb
{

namespace dbinterface
{

class ConOption
{
private:
    typedef arangodb::velocypack::Value Value;
    typedef arangodb::velocypack::Value::CType CType;
public:
    typedef std::vector<ConOption>  vector;
    ConOption() = delete;
    ConOption ( const ConOption &inp );
    template <typename T>
    explicit ConOption ( const std::string &name, const T &value );
    ConOption &operator = ( const ConOption &inp );
    const std::string headerString() const;
    const std::string queryString() const;
    const std::string &name() const;
    const Value &value() const;
    void setName ( const std::string &val );
    template <typename T>
    void setValue ( const T &val );
    std::string valueString() const;

private:
    std::string _name;
    //  Need to hold string value because _value only heeps a pointer to the value
    std::string _str;
    Value _value;
};

template <typename T>
ConOption::ConOption ( const std::string &name, const T &value )
    : _name {name},_value {value} {
}

inline const std::string &ConOption::name() const
{
    return _name;
}

inline const ConOption::Value &ConOption::value() const
{
    return _value;
}

inline void ConOption::setName ( const std::string &val )
{
    _name = val;
}

template <typename T>
inline void ConOption::setValue ( const T &val )
{
    new ( &_value ) Value ( val );
}


}
}

#endif
