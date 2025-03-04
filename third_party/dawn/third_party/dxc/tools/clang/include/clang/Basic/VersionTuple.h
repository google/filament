//===- VersionTuple.h - Version Number Handling -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the clang::VersionTuple class, which represents a version in
/// the form major[.minor[.subminor]].
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_BASIC_VERSIONTUPLE_H
#define LLVM_CLANG_BASIC_VERSIONTUPLE_H

#include "clang/Basic/LLVM.h"
#include "llvm/ADT/Optional.h"
#include <string>
#include <tuple>

namespace clang {

/// \brief Represents a version number in the form major[.minor[.subminor[.build]]].
class VersionTuple {
  unsigned Major : 31;
  unsigned Minor : 31;
  unsigned Subminor : 31;
  unsigned Build : 31;
  unsigned HasMinor : 1;
  unsigned HasSubminor : 1;
  unsigned HasBuild : 1;
  unsigned UsesUnderscores : 1;

public:
  VersionTuple()
      : Major(0), Minor(0), Subminor(0), Build(0), HasMinor(false),
        HasSubminor(false), HasBuild(false), UsesUnderscores(false) {}

  explicit VersionTuple(unsigned Major)
      : Major(Major), Minor(0), Subminor(0), Build(0), HasMinor(false),
        HasSubminor(false), HasBuild(false), UsesUnderscores(false) {}

  explicit VersionTuple(unsigned Major, unsigned Minor,
                        bool UsesUnderscores = false)
      : Major(Major), Minor(Minor), Subminor(0), Build(0), HasMinor(true),
        HasSubminor(false), HasBuild(false), UsesUnderscores(UsesUnderscores) {}

  explicit VersionTuple(unsigned Major, unsigned Minor, unsigned Subminor,
                        bool UsesUnderscores = false)
      : Major(Major), Minor(Minor), Subminor(Subminor), Build(0),
        HasMinor(true), HasSubminor(true), HasBuild(false),
        UsesUnderscores(UsesUnderscores) {}

  explicit VersionTuple(unsigned Major, unsigned Minor, unsigned Subminor,
                        unsigned Build, bool UsesUnderscores = false)
      : Major(Major), Minor(Minor), Subminor(Subminor), Build(Build),
        HasMinor(true), HasSubminor(true), HasBuild(true),
        UsesUnderscores(UsesUnderscores) {}

  /// \brief Determine whether this version information is empty
  /// (e.g., all version components are zero).
  bool empty() const {
    return Major == 0 && Minor == 0 && Subminor == 0 && Build == 0;
  }

  /// \brief Retrieve the major version number.
  unsigned getMajor() const { return Major; }

  /// \brief Retrieve the minor version number, if provided.
  Optional<unsigned> getMinor() const {
    if (!HasMinor)
      return None;
    return Minor;
  }

  /// \brief Retrieve the subminor version number, if provided.
  Optional<unsigned> getSubminor() const {
    if (!HasSubminor)
      return None;
    return Subminor;
  }

  /// \brief Retrieve the build version number, if provided.
  Optional<unsigned> getBuild() const {
    if (!HasBuild)
      return None;
    return Build;
  }

  bool usesUnderscores() const {
    return UsesUnderscores;
  }

  void UseDotAsSeparator() {
    UsesUnderscores = false;
  }
  
  /// \brief Determine if two version numbers are equivalent. If not
  /// provided, minor and subminor version numbers are considered to be zero.
  friend bool operator==(const VersionTuple& X, const VersionTuple &Y) {
    return X.Major == Y.Major && X.Minor == Y.Minor &&
           X.Subminor == Y.Subminor && X.Build == Y.Build;
  }

  /// \brief Determine if two version numbers are not equivalent.
  ///
  /// If not provided, minor and subminor version numbers are considered to be 
  /// zero.
  friend bool operator!=(const VersionTuple &X, const VersionTuple &Y) {
    return !(X == Y);
  }

  /// \brief Determine whether one version number precedes another.
  ///
  /// If not provided, minor and subminor version numbers are considered to be
  /// zero.
  friend bool operator<(const VersionTuple &X, const VersionTuple &Y) {
    return std::tie(X.Major, X.Minor, X.Subminor, X.Build) <
           std::tie(Y.Major, Y.Minor, Y.Subminor, Y.Build);
  }

  /// \brief Determine whether one version number follows another.
  ///
  /// If not provided, minor and subminor version numbers are considered to be
  /// zero.
  friend bool operator>(const VersionTuple &X, const VersionTuple &Y) {
    return Y < X;
  }

  /// \brief Determine whether one version number precedes or is
  /// equivalent to another. 
  ///
  /// If not provided, minor and subminor version numbers are considered to be
  /// zero.
  friend bool operator<=(const VersionTuple &X, const VersionTuple &Y) {
    return !(Y < X);
  }

  /// \brief Determine whether one version number follows or is
  /// equivalent to another.
  ///
  /// If not provided, minor and subminor version numbers are considered to be
  /// zero.
  friend bool operator>=(const VersionTuple &X, const VersionTuple &Y) {
    return !(X < Y);
  }

  /// \brief Retrieve a string representation of the version number.
  std::string getAsString() const;

  /// \brief Try to parse the given string as a version number.
  /// \returns \c true if the string does not match the regular expression
  ///   [0-9]+(\.[0-9]+){0,3}
  bool tryParse(StringRef string);
};

/// \brief Print a version number.
raw_ostream& operator<<(raw_ostream &Out, const VersionTuple &V);

} // end namespace clang
#endif // LLVM_CLANG_BASIC_VERSIONTUPLE_H
