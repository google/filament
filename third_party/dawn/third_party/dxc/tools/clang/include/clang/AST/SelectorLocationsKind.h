//===--- SelectorLocationsKind.h - Kind of selector locations ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Describes whether the identifier locations for a selector are "standard"
// or not.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_SELECTORLOCATIONSKIND_H
#define LLVM_CLANG_AST_SELECTORLOCATIONSKIND_H

#include "clang/Basic/LLVM.h"

namespace clang {
  class Selector;
  class SourceLocation;
  class Expr;
  class ParmVarDecl;

/// \brief Whether all locations of the selector identifiers are in a
/// "standard" position.
enum SelectorLocationsKind {
  /// \brief Non-standard.
  SelLoc_NonStandard = 0,

  /// \brief For nullary selectors, immediately before the end:
  ///    "[foo release]" / "-(void)release;"
  /// Or immediately before the arguments:
  ///    "[foo first:1 second:2]" / "-(id)first:(int)x second:(int)y;
  SelLoc_StandardNoSpace = 1,

  /// \brief For nullary selectors, immediately before the end:
  ///    "[foo release]" / "-(void)release;"
  /// Or with a space between the arguments:
  ///    "[foo first: 1 second: 2]" / "-(id)first: (int)x second: (int)y;
  SelLoc_StandardWithSpace = 2
};

/// \brief Returns true if all \p SelLocs are in a "standard" location.
SelectorLocationsKind hasStandardSelectorLocs(Selector Sel,
                                              ArrayRef<SourceLocation> SelLocs,
                                              ArrayRef<Expr *> Args,
                                              SourceLocation EndLoc);

/// \brief Get the "standard" location of a selector identifier, e.g:
/// For nullary selectors, immediately before ']': "[foo release]"
///
/// \param WithArgSpace if true the standard location is with a space apart
/// before arguments: "[foo first: 1 second: 2]"
/// If false: "[foo first:1 second:2]"
SourceLocation getStandardSelectorLoc(unsigned Index,
                                      Selector Sel,
                                      bool WithArgSpace,
                                      ArrayRef<Expr *> Args,
                                      SourceLocation EndLoc);

/// \brief Returns true if all \p SelLocs are in a "standard" location.
SelectorLocationsKind hasStandardSelectorLocs(Selector Sel,
                                              ArrayRef<SourceLocation> SelLocs,
                                              ArrayRef<ParmVarDecl *> Args,
                                              SourceLocation EndLoc);

/// \brief Get the "standard" location of a selector identifier, e.g:
/// For nullary selectors, immediately before ']': "[foo release]"
///
/// \param WithArgSpace if true the standard location is with a space apart
/// before arguments: "-(id)first: (int)x second: (int)y;"
/// If false: "-(id)first:(int)x second:(int)y;"
SourceLocation getStandardSelectorLoc(unsigned Index,
                                      Selector Sel,
                                      bool WithArgSpace,
                                      ArrayRef<ParmVarDecl *> Args,
                                      SourceLocation EndLoc);

} // end namespace clang

#endif
