////////////////////////////////////////////////////////////////////////////////
/// @brief C++ Library to interface to Arangodb.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
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
/// @author Copyright 2016, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////
#ifndef FUERTE_DOCOPTIONS_H
#define FUERTE_DOCOPTIONS_H

#include <string>

namespace arangodb {

namespace dbinterface {

class DocOptions {
 private:
  typedef uint16_t Flags;
  std::string _eTag;
  Flags _flgs;

 public:
  enum class Rev : Flags {
    Reset = 0,
    Match = Reset + 1,
    NoMatch = Match + 1,
    Mask = 3
  };

  enum class Policy : Flags {
    Reset = 0,
    Error = Reset,
    Last = Error + 4,
    Mask = 4
  };

  enum class Sync : Flags {
    Reset = 0,
    Wait = Reset + 8,
    NoWait = Wait + 8,
    Mask = 0x18
  };

  enum class CreateCol : Flags {
    Reset = 0,
    No = Reset,
    Yes = No + 0x20,
    Mask = 0x20
  };

  enum class Merge : Flags {
    Reset = 0,
    Yes = Reset,
    No = Yes + 0x40,
    Mask = 0x40
  };

  enum class Run : Flags {
    Reset = 0,
    Sync = Reset,
    Async = Sync + 0x80,
    Mask = 0x80
  };

  enum class RemoveNull : Flags {
    Reset = 0,
    No = Reset,
    Yes = No + 0x100,
    Mask = 0x100
  };

  enum class CreateColType : Flags {
    Reset = 0,
    Document = Reset,
    Edge = Document + 0x200,
    Mask = 0x200
  };

  DocOptions& resetAllFlags();
  template <typename C>
  DocOptions& resetFlags();
  template <typename T>
  const T flag() const;
  template <typename T>
  bool flagged(T inp) const;
  operator const std::string&() const;
  DocOptions& operator=(const std::string& inp);
  DocOptions& operator=(std::string&& inp);

 private:
  template <typename T>
  void addFlags(T flag);
  template <typename T, typename... Args>
  void addFlags(T flag, Args... args);

 public:
  template <typename T, typename... Args>
  DocOptions& setFlags(T flag, Args... args);
  template <typename T>
  DocOptions& setFlags(T flag);
  explicit DocOptions();
  explicit DocOptions(const DocOptions& inp);
  explicit DocOptions(DocOptions&& inp);
  explicit DocOptions(const std::string& inp);
  explicit DocOptions(std::string&& inp);
  virtual ~DocOptions();
  template <typename T, typename... Args>
  explicit DocOptions(const std::string& tag, T flag, Args... args);
  template <typename T, typename... Args>
  explicit DocOptions(std::string&& tag, T flag, Args... args);
};

inline DocOptions::DocOptions(const DocOptions& inp)
    : _eTag(inp._eTag), _flgs(inp._flgs) {}

inline DocOptions::DocOptions(DocOptions&& inp)
    : _eTag(inp._eTag), _flgs(inp._flgs) {}

template <typename T, typename... Args>
DocOptions::DocOptions(const std::string& tag, T flag, Args... args)
    : _eTag(tag) {
  setFlags(flag, args...);
}

template <typename T, typename... Args>
DocOptions::DocOptions(std::string&& tag, T flag, Args... args)
    : _eTag(tag) {
  setFlags(flag, args...);
}

inline DocOptions::DocOptions(const std::string& inp) : _eTag(inp), _flgs(0) {}

inline DocOptions::DocOptions(std::string&& inp) : _eTag(inp), _flgs(0) {}

inline DocOptions::DocOptions() : _eTag("12345678"), _flgs(0) {}

inline DocOptions::~DocOptions() {}

template <typename T>
void DocOptions::addFlags(T flag) {
  _flgs &= ~static_cast<Flags>(T::Mask);
  _flgs |= static_cast<Flags>(flag);
}

template <typename T, typename... Args>
void DocOptions::addFlags(T flag, Args... args) {
  addFlags(flag);
  addFlags(args...);
}

template <typename T>
DocOptions& DocOptions::setFlags(T flag) {
  if (T::Mask == T::Mask) {
    _flgs = static_cast<Flags>(flag);
  }
  return *this;
}

template <typename T, typename... Args>
DocOptions& DocOptions::setFlags(T flag, Args... args) {
  setFlags(flag);
  addFlags(args...);
  return *this;
}

inline DocOptions& DocOptions::resetAllFlags() {
  _flgs = 0;
  return *this;
}

template <typename C>
inline DocOptions& DocOptions::resetFlags() {
  _flgs &= ~static_cast<Flags>(C::Mask);
  return *this;
}

inline DocOptions& DocOptions::operator=(const std::string& inp) {
  _eTag = inp;
  return *this;
}

inline DocOptions& DocOptions::operator=(std::string&& inp) {
  _eTag = inp;
  return *this;
}

inline DocOptions::operator const std::string&() const { return _eTag; }

template <typename T>
inline const T DocOptions::flag() const {
  Flags tmp = _flgs & static_cast<Flags>(T::Mask);
  return static_cast<T>(tmp);
}

template <typename T>
inline bool DocOptions::flagged(T inp) const {
  return flag<T>() == inp;
}
}
}

#endif  // FUERTE_DOCOPTIONS_H
