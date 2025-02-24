//===- subzero/src/IceRangeSpec.cpp - Include/exclude specification -------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements a class for specifying sets of names and number ranges to
/// match against.  This is specified as a comma-separated list of clauses.
/// Each clause optionally starts with '-' to indicate exclusion instead of
/// inclusion.  A clause can be a name, or a numeric range X:Y, or a single
/// number X.  The X:Y form indicates a range of numbers greater than or equal
/// to X and strictly less than Y.  A missing "X" is taken to be 0, and a
/// missing "Y" is taken to be infinite.  E.g., "0:" and ":" specify the entire
/// set.
///
/// This is essentially the same implementation as in szbuild.py, except that
/// regular expressions are not used for the names.
///
//===----------------------------------------------------------------------===//

#include "IceRangeSpec.h"
#include "IceStringPool.h"

#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>

namespace Ice {

bool RangeSpec::HasNames = false;

namespace {

/// Helper function to parse "X" or "X:Y" into First and Last.
/// - "X" is treated as "X:X+1".
/// - ":Y" is treated as "0:Y".
/// - "X:" is treated as "X:inf"
///
/// Behavior is undefined if "X" or "Y" is not a proper number (since std::stoul
/// throws an exception).
///
/// If the string doesn't contain 1 or 2 ':' delimiters, or X>=Y,
/// report_fatal_error is called.
void getRange(const std::string &Token, uint32_t *First, uint32_t *Last) {
  bool Error = false;
  auto Tokens = RangeSpec::tokenize(Token, RangeSpec::DELIM_RANGE);
  if (Tokens.size() == 1) {
    *First = std::stoul(Tokens[0]);
    *Last = *First + 1;
  } else if (Tokens.size() == 2) {
    *First = Tokens[0].empty() ? 0 : std::stoul(Tokens[0]);
    *Last = Tokens[1].empty() ? RangeSpec::RangeMax : std::stoul(Tokens[1]);
  } else {
    Error = true;
  }
  if (*First >= *Last) {
    Error = true;
  }
  if (Error) {
    llvm::report_fatal_error("Invalid range " + Token);
  }
}

/// Helper function to add one token to the include or exclude set.  The token
/// is examined and then treated as either a numeric range or a single name.
void record(const std::string &Token, RangeSpec::Desc *D) {
  if (Token.empty())
    return;
  // Mark that an include or exclude was explicitly given.  This affects the
  // default decision when matching a value that wasn't explicitly provided in
  // the include or exclude list.
  D->IsExplicit = true;
  // A range is identified by starting with a digit or a ':'.
  if (Token[0] == RangeSpec::DELIM_RANGE || std::isdigit(Token[0])) {
    uint32_t First = 0, Last = 0;
    getRange(Token, &First, &Last);
    if (Last == RangeSpec::RangeMax) {
      D->AllFrom = std::min(D->AllFrom, First);
    } else {
      if (Last >= D->Numbers.size())
        D->Numbers.resize(Last + 1);
      D->Numbers.set(First, Last);
    }
  } else {
    // Otherwise treat it as a single name.
    D->Names.insert(Token);
  }
}

} // end of anonymous namespace

std::vector<std::string> RangeSpec::tokenize(const std::string &Spec,
                                             char Delimiter) {
  std::vector<std::string> Tokens;
  if (!Spec.empty()) {
    std::string::size_type StartPos = 0;
    std::string::size_type DelimPos = 0;
    while (DelimPos != std::string::npos) {
      DelimPos = Spec.find(Delimiter, StartPos);
      Tokens.emplace_back(Spec.substr(StartPos, DelimPos - StartPos));
      StartPos = DelimPos + 1;
    }
  }
  return Tokens;
}

/// Initialize the RangeSpec with the given string.  Calling init multiple times
/// (e.g. init("A");init("B");) is equivalent to init("A,B"); .
void RangeSpec::init(const std::string &Spec) {
  auto Tokens = tokenize(Spec, DELIM_LIST);
  for (const auto &Token : Tokens) {
    if (Token[0] == '-') {
      exclude(Token.substr(1));
    } else {
      include(Token);
    }
  }
  if (!Includes.Names.empty() || !Excludes.Names.empty())
    HasNames = true;
}

/// Determine whether the given Name/Number combo match the specification given
/// to the init() method.  Explicit excludes take precedence over explicit
/// includes.  If the combo doesn't match any explicit include or exclude:
/// - false if the init() string is empty (no explicit includes or excludes)
/// - true if there is at least one explicit exclude and no explicit includes
/// - false otherwise (at least one explicit include)
bool RangeSpec::match(const std::string &Name, uint32_t Number) const {
  // No match if it is explicitly excluded by name or number.
  if (Excludes.Names.find(Name) != Excludes.Names.end())
    return false;
  if (Number >= Excludes.AllFrom)
    return false;
  if (Number < Excludes.Numbers.size() && Excludes.Numbers[Number])
    return false;

  // Positive match if it is explicitly included by name or number.
  if (Includes.Names.find(Name) != Includes.Names.end())
    return true;
  if (Number >= Includes.AllFrom)
    return true;
  if (Number < Includes.Numbers.size() && Includes.Numbers[Number])
    return true;

  // Otherwise use the default decision.
  return Excludes.IsExplicit && !Includes.IsExplicit;
}

void RangeSpec::include(const std::string &Token) { record(Token, &Includes); }

void RangeSpec::exclude(const std::string &Token) { record(Token, &Excludes); }

} // end of namespace Ice
