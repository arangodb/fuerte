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
 public:
  typedef uint16_t Flags;
  enum Flag : Flags {
    Opt_Defaults = 0,
    Opt_NoneMatchRev = 1,
    Opt_MatchRev = 2,
    Opt_MatchMask = Opt_NoneMatchRev | Opt_MatchRev,
    Opt_PolicyError = 4,
    Opt_PolicyLast = 8,
    Opt_PolicyMask = Opt_PolicyError | Opt_PolicyLast,
    Opt_WaitForSync = 16,
    Opt_NoWaitForSync = 32,
    Opt_SyncMask = Opt_NoWaitForSync | Opt_WaitForSync,
    Opt_CreateCollection = 64  // Default is collection must exist
    ,
    Opt_Merge = 0  // Default is to merge
    ,
    Opt_NoMerge = 128,
    Opt_RunSync = 0  // Default to run synchronusly
    ,
    Opt_RunAsync = 256,
    Opt_RemoveNull = 512
  };
  DocOptions(const Flags flags, const std::string& tag = "");
  DocOptions(const Flags flags = Opt_Defaults, std::string&& tag = "");
  const std::string& eTag() const;
  Flags flags() const;
  void selectOpts(Flags);
  void clearOpts(Flags);
  DocOptions& operator=(const Flags inp);
  DocOptions& operator=(const Flag inp);
  DocOptions& operator=(const std::string& inp);
  DocOptions& operator=(std::string&& inp);

 private:
  std::string _eTag;
  Flags _flgs;
};

inline DocOptions& DocOptions::operator=(const Flags inp) {
  _flgs = inp;
  return *this;
}

inline DocOptions& DocOptions::operator=(const Flag inp) {
  _flgs = inp;
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

inline const std::string& DocOptions::eTag() const { return _eTag; }

inline DocOptions::Flags DocOptions::flags() const { return _flgs; }

inline void DocOptions::selectOpts(Flags inp) { _flgs |= inp; }

inline void DocOptions::clearOpts(Flags inp) { _flgs &= ~inp; }
}
}

#endif  // FUERTE_DOCOPTIONS_H
