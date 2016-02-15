#ifndef DOCOPTIONS_H
#define DOCOPTIONS_H

#include <string>

namespace arangodb {

namespace dbinterface {

class DocOptions {
 public:
  enum {
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
    Opt_NoMerge = 128  // Default is to merge
    ,
    Opt_TypePath = 0  // Default when listing documents
    //,Opt_TypeId = 512
    //,Opt_TypeKey = 256
    //,Opt_TypeMask = Opt_TypeId | Opt_TypeKey
    ,
    Opt_RunAsync = 256  // Default to run synchronusly
  };
  DocOptions(const uint16_t opts, const std::string& tag = "");
  DocOptions(const uint16_t opts = Opt_Defaults, std::string&& tag = "");
  const std::string& eTag() const;
  uint16_t opts() const;
  void selectOpts(uint16_t);
  void clearOpts(uint16_t);
  DocOptions& operator=(uint16_t inp);
  DocOptions& operator=(const std::string& inp);
  DocOptions& operator=(std::string&& inp);

 private:
  std::string _eTag;
  uint16_t _opts;
};

inline DocOptions& DocOptions::operator=(uint16_t inp) {
  _opts = inp;
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

inline uint16_t DocOptions::opts() const { return _opts; }

inline void DocOptions::selectOpts(uint16_t inp) { _opts |= inp; }

inline void DocOptions::clearOpts(uint16_t inp) { _opts &= ~inp; }
}
}

#endif  // DOCOPTIONS_H
