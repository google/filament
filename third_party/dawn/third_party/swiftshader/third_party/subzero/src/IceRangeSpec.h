//===- subzero/src/IceRangeSpec.h - Include/exclude specs -------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares a class for specifying inclusion/exclusion of values, such
/// as functions to match.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICERANGESPEC_H
#define SUBZERO_SRC_ICERANGESPEC_H

#include "IceStringPool.h"

#include <string>
#include <unordered_set>
#include <vector>

#include "llvm/ADT/BitVector.h"

namespace Ice {

class RangeSpec {
  RangeSpec(const RangeSpec &) = delete;
  RangeSpec &operator=(const RangeSpec &) = delete;

public:
  static constexpr char DELIM_LIST = ',';
  static constexpr char DELIM_RANGE = ':';
  static constexpr uint32_t RangeMax = std::numeric_limits<uint32_t>::max();
  RangeSpec() = default;
  struct Desc {
    // Set of names explicitly provided.
    std::unordered_set<std::string> Names;
    // Set of numbers explicitly provided.
    llvm::BitVector Numbers;
    // The smallest X for which the open-ended interval "X:" was provided.  This
    // is needed because the intervals are parsed before we know the largest
    // number that might be matched against, and we can't make the Numbers
    // bitvector infinitely long.
    uint32_t AllFrom = RangeMax;
    // Whether a clause was explicitly provided.
    bool IsExplicit = false;
  };
  void init(const std::string &Spec);
  bool match(const std::string &Name, uint32_t Number) const;
  bool match(GlobalString Name, uint32_t Number) const {
    return match(Name.toStringOrEmpty(), Number);
  }
  // Returns true if any RangeSpec object has had init() called with an explicit
  // name rather than (or in addition to) a numeric range.  If so, we want to
  // construct explicit names for functions even in a non-DUMP build so that
  // matching on function name works correctly.  Note that this is not
  // thread-safe, so we count on all this being handled by the startup thread.
  static bool hasNames() { return HasNames; }
  // Helper function to tokenize a string into a vector of string tokens, given
  // a single delimiter character.  An empty string produces an empty token
  // vector.  Zero-length tokens are allowed, e.g. ",a,,,b," may tokenize to
  // {"","a","","","b",""}.
  static std::vector<std::string> tokenize(const std::string &Spec,
                                           char Delimiter);

private:
  void include(const std::string &Token);
  void exclude(const std::string &Token);
  Desc Includes;
  Desc Excludes;
  static bool HasNames;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICERANGESPEC_H
