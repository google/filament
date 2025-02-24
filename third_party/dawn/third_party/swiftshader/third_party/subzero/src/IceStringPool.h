//===- subzero/src/IceStringPool.h - String pooling -------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines unique pooled strings with short unique IDs.  This makes
/// hashing, equality testing, and ordered comparison faster, and avoids a lot
/// of memory allocation compared to directly using std::string.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICESTRINGPOOL_H
#define SUBZERO_SRC_ICESTRINGPOOL_H

#include "IceDefs.h" // Ostream

#include "llvm/Support/ErrorHandling.h"

#include <cstdint> // uintptr_t
#include <string>

namespace Ice {

class StringPool {
  StringPool(const StringPool &) = delete;
  StringPool &operator=(const StringPool &) = delete;

public:
  using IDType = uintptr_t;

  StringPool() = default;
  ~StringPool() = default;
  IDType getNewID() {
    // TODO(stichnot): Make it so that the GlobalString ctor doesn't have to
    // grab the lock, and instead does an atomic increment of NextID.
    auto NewID = NextID;
    NextID += IDIncrement;
    return NewID;
  }
  IDType getOrAddString(const std::string &Value) {
    auto Iter = StringToId.find(Value);
    if (Iter == StringToId.end()) {
      auto *NewStr = new std::string(Value);
      auto ID = reinterpret_cast<IDType>(NewStr);
      StringToId[Value].reset(NewStr);
      return ID;
    }
    return reinterpret_cast<IDType>(Iter->second.get());
  }
  void dump(Ostream &Str) const {
    if (StringToId.empty())
      return;
    Str << "String pool (NumStrings=" << StringToId.size()
        << " NumIDs=" << ((NextID - FirstID) / IDIncrement) << "):";
    for (const auto &Tuple : StringToId) {
      Str << " " << Tuple.first;
    }
    Str << "\n";
  }

private:
  static constexpr IDType FirstID = 1;
  static constexpr IDType IDIncrement = 2;
  IDType NextID = FirstID;
  std::unordered_map<std::string, std::unique_ptr<std::string>> StringToId;
};

template <typename Traits> class StringID {
public:
  using IDType = StringPool::IDType;

  StringID() = default; // Create a default, invalid StringID.
  StringID(const StringID &) = default;
  StringID &operator=(const StringID &) = default;

  /// Create a unique StringID without an actual string, by grabbing the next
  /// unique integral ID from the Owner.
  static StringID createWithoutString(const typename Traits::OwnerType *Owner) {
    return StringID(Owner);
  }
  /// Create a unique StringID that holds an actual string, by fetching or
  /// adding the string from the Owner's pool.
  static StringID createWithString(const typename Traits::OwnerType *Owner,
                                   const std::string &Value) {
    return StringID(Owner, Value);
  }

  /// Tests whether the StringID was initialized with respect to an actual
  /// std::string value, i.e. via StringID::createWithString().
  bool hasStdString() const { return isValid() && ((ID & 0x1) == 0); }

  IDType getID() const {
    assert(isValid());
    return ID;
  }
  const std::string &toString() const {
    if (!hasStdString())
      llvm::report_fatal_error(
          "toString() called when hasStdString() is false");
    return *reinterpret_cast<std::string *>(ID);
  }
  std::string toStringOrEmpty() const {
    if (hasStdString())
      return toString();
    return "";
  }

  bool operator==(const StringID &Other) const { return ID == Other.ID; }
  bool operator!=(const StringID &Other) const { return !(*this == Other); }
  bool operator<(const StringID &Other) const {
    const bool ThisHasString = hasStdString();
    const bool OtherHasString = Other.hasStdString();
    // Do a normal string comparison if both have strings.
    if (ThisHasString && OtherHasString)
      return this->toString() < Other.toString();
    // Use the ID as a tiebreaker if neither has a string.
    if (!ThisHasString && !OtherHasString)
      return ID < Other.ID;
    // If exactly one has a string, then that one comes first.
    assert(ThisHasString != OtherHasString);
    return ThisHasString;
  }

private:
  static constexpr IDType InvalidID = 0;
  IDType ID = InvalidID;

  explicit StringID(const typename Traits::OwnerType *Owner)
      : ID(Traits::getStrings(Owner)->getNewID()) {}
  StringID(const typename Traits::OwnerType *Owner, const std::string &Value)
      : ID(Traits::getStrings(Owner)->getOrAddString(Value)) {
    assert(hasStdString());
  }
  bool isValid() const { return ID != InvalidID; }
};

// TODO(stichnot, jpp): Move GlobalStringPoolTraits definition into
// IceGlobalContext.h, once the include order issues are solved.
struct GlobalStringPoolTraits {
  using OwnerType = GlobalContext;
  static LockedPtr<StringPool> getStrings(const OwnerType *Owner);
};

using GlobalString = StringID<struct GlobalStringPoolTraits>;

template <typename T>
Ostream &operator<<(Ostream &Str, const StringID<T> &Name) {
  return Str << Name.toString();
}

template <typename T>
std::string operator+(const std::string &A, const StringID<T> &B) {
  return A + B.toString();
}

template <typename T>
std::string operator+(const StringID<T> &A, const std::string &B) {
  return A.toString() + B;
}

} // end of namespace Ice

namespace std {
template <typename T> struct hash<Ice::StringID<T>> {
  size_t operator()(const Ice::StringID<T> &Key) const {
    if (Key.hasStdString())
      return hash<std::string>()(Key.toString());
    return hash<Ice::StringPool::IDType>()(Key.getID());
  }
};
} // end of namespace std

#endif // SUBZERO_SRC_ICESTRINGPOOL_H
