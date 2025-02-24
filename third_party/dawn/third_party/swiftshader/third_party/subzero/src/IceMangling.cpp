//===- subzero/src/IceMangling.cpp - Cross test name mangling --*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines utility functions for name mangling for cross tests.
///
//===----------------------------------------------------------------------===//

#include "IceMangling.h"
#include "IceDefs.h"
#include "IceGlobalContext.h"

#include <cctype> // isdigit(), isupper()
#include <locale> // locale

namespace Ice {

using ManglerVector = llvm::SmallVector<char, 32>;

namespace {

// Scan a string for S[0-9A-Z]*_ patterns and replace them with
// S<num>_ where <num> is the next base-36 value.  If a type name
// legitimately contains that pattern, then the substitution will be
// made in error and most likely the link will fail.  In this case,
// the test classes can be rewritten not to use that pattern, which is
// much simpler and more reliable than implementing a full demangling
// parser.  Another substitution-in-error may occur if a type
// identifier ends with the pattern S[0-9A-Z]*, because an immediately
// following substitution string like "S1_" or "PS1_" may be combined
// with the previous type.
void incrementSubstitutions(ManglerVector &OldName) {
  const std::locale CLocale("C");
  // Provide extra space in case the length of <num> increases.
  ManglerVector NewName(OldName.size() * 2);
  size_t OldPos = 0;
  size_t NewPos = 0;
  const size_t OldLen = OldName.size();
  for (; OldPos < OldLen; ++OldPos, ++NewPos) {
    if (OldName[OldPos] == '\0')
      break;
    if (OldName[OldPos] == 'S') {
      // Search forward until we find _ or invalid character (including \0).
      bool AllZs = true;
      bool Found = false;
      size_t Last;
      for (Last = OldPos + 1; Last < OldLen; ++Last) {
        char Ch = OldName[Last];
        if (Ch == '_') {
          Found = true;
          break;
        } else if (std::isdigit(Ch) || std::isupper(Ch, CLocale)) {
          if (Ch != 'Z')
            AllZs = false;
        } else {
          // Invalid character, stop searching.
          break;
        }
      }
      if (Found) {
        NewName[NewPos++] = OldName[OldPos++]; // 'S'
        size_t Length = Last - OldPos;
        // NewPos and OldPos point just past the 'S'.
        assert(NewName[NewPos - 1] == 'S');
        assert(OldName[OldPos - 1] == 'S');
        assert(OldName[OldPos + Length] == '_');
        if (AllZs) {
          // Replace N 'Z' characters with a '0' (if N=0) or '1' (if N>0)
          // followed by N '0' characters.
          NewName[NewPos++] = (Length ? '1' : '0');
          for (size_t i = 0; i < Length; ++i) {
            NewName[NewPos++] = '0';
          }
        } else {
          // Iterate right-to-left and increment the base-36 number.
          bool Carry = true;
          for (size_t i = 0; i < Length; ++i) {
            size_t Offset = Length - 1 - i;
            char Ch = OldName[OldPos + Offset];
            if (Carry) {
              Carry = false;
              switch (Ch) {
              case '9':
                Ch = 'A';
                break;
              case 'Z':
                Ch = '0';
                Carry = true;
                break;
              default:
                ++Ch;
                break;
              }
            }
            NewName[NewPos + Offset] = Ch;
          }
          NewPos += Length;
        }
        OldPos = Last;
        // Fall through and let the '_' be copied across.
      }
    }
    NewName[NewPos] = OldName[OldPos];
  }
  assert(NewName[NewPos] == '\0');
  OldName = NewName;
}

} // end of anonymous namespace

// In this context, name mangling means to rewrite a symbol using a given
// prefix. For a C++ symbol, nest the original symbol inside the "prefix"
// namespace. For other symbols, just prepend the prefix.
std::string mangleName(const std::string &Name) {
  // An already-nested name like foo::bar() gets pushed down one level, making
  // it equivalent to Prefix::foo::bar().
  //   _ZN3foo3barExyz ==> _ZN6Prefix3foo3barExyz
  // A non-nested but mangled name like bar() gets nested, making it equivalent
  // to Prefix::bar().
  //   _Z3barxyz ==> ZN6Prefix3barExyz
  // An unmangled, extern "C" style name, gets a simple prefix:
  //   bar ==> Prefixbar
  if (!BuildDefs::dump() || getFlags().getTestPrefix().empty())
    return Name;

  const std::string TestPrefix = getFlags().getTestPrefix();
  unsigned PrefixLength = TestPrefix.length();
  ManglerVector NameBase(1 + Name.length());
  const size_t BufLen = 30 + Name.length() + PrefixLength;
  ManglerVector NewName(BufLen);
  uint32_t BaseLength = 0; // using uint32_t due to sscanf format string

  int ItemsParsed = sscanf(Name.c_str(), "_ZN%s", NameBase.data());
  if (ItemsParsed == 1) {
    // Transform _ZN3foo3barExyz ==> _ZN6Prefix3foo3barExyz
    //   (splice in "6Prefix")          ^^^^^^^
    snprintf(NewName.data(), BufLen, "_ZN%u%s%s", PrefixLength,
             TestPrefix.c_str(), NameBase.data());
    // We ignore the snprintf return value (here and below). If we somehow
    // miscalculated the output buffer length, the output will be truncated,
    // but it will be truncated consistently for all mangleName() calls on the
    // same input string.
    incrementSubstitutions(NewName);
    return NewName.data();
  }

  // Artificially limit BaseLength to 9 digits (less than 1 billion) because
  // sscanf behavior is undefined on integer overflow. If there are more than 9
  // digits (which we test by looking at the beginning of NameBase), then we
  // consider this a failure to parse a namespace mangling, and fall back to
  // the simple prefixing.
  ItemsParsed = sscanf(Name.c_str(), "_Z%9u%s", &BaseLength, NameBase.data());
  if (ItemsParsed == 2 && BaseLength <= strlen(NameBase.data()) &&
      !isdigit(NameBase[0])) {
    // Transform _Z3barxyz ==> _ZN6Prefix3barExyz
    //                           ^^^^^^^^    ^
    // (splice in "N6Prefix", and insert "E" after "3bar") But an "I" after the
    // identifier indicates a template argument list terminated with "E";
    // insert the new "E" before/after the old "E".  E.g.:
    // Transform _Z3barIabcExyz ==> _ZN6Prefix3barIabcEExyz
    //                                ^^^^^^^^         ^
    // (splice in "N6Prefix", and insert "E" after "3barIabcE")
    ManglerVector OrigName(Name.length());
    ManglerVector OrigSuffix(Name.length());
    uint32_t ActualBaseLength = BaseLength;
    if (NameBase[ActualBaseLength] == 'I') {
      ++ActualBaseLength;
      while (NameBase[ActualBaseLength] != 'E' &&
             NameBase[ActualBaseLength] != '\0')
        ++ActualBaseLength;
    }
    strncpy(OrigName.data(), NameBase.data(), ActualBaseLength);
    OrigName[ActualBaseLength] = '\0';
    strcpy(OrigSuffix.data(), NameBase.data() + ActualBaseLength);
    snprintf(NewName.data(), BufLen, "_ZN%u%s%u%sE%s", PrefixLength,
             TestPrefix.c_str(), BaseLength, OrigName.data(),
             OrigSuffix.data());
    incrementSubstitutions(NewName);
    return NewName.data();
  }

  // Transform bar ==> Prefixbar
  //                   ^^^^^^
  return TestPrefix + Name;
}

} // end of namespace Ice
