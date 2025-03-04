//===--- Linkage.h - Linkage enumeration and utilities ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the Linkage enumeration and various utility functions.
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_BASIC_LINKAGE_H
#define LLVM_CLANG_BASIC_LINKAGE_H

#include <assert.h>
#include <stdint.h>
#include <utility>

namespace clang {

/// \brief Describes the different kinds of linkage 
/// (C++ [basic.link], C99 6.2.2) that an entity may have.
enum Linkage : unsigned char {
  /// \brief No linkage, which means that the entity is unique and
  /// can only be referred to from within its scope.
  NoLinkage = 0,

  /// \brief Internal linkage, which indicates that the entity can
  /// be referred to from within the translation unit (but not other
  /// translation units).
  InternalLinkage,

  /// \brief External linkage within a unique namespace. 
  ///
  /// From the language perspective, these entities have external
  /// linkage. However, since they reside in an anonymous namespace,
  /// their names are unique to this translation unit, which is
  /// equivalent to having internal linkage from the code-generation
  /// point of view.
  UniqueExternalLinkage,

  /// \brief No linkage according to the standard, but is visible from other
  /// translation units because of types defined in a inline function.
  VisibleNoLinkage,

  /// \brief External linkage, which indicates that the entity can
  /// be referred to from other translation units.
  ExternalLinkage
};

/// \brief Describes the different kinds of language linkage
/// (C++ [dcl.link]) that an entity may have.
enum LanguageLinkage {
  CLanguageLinkage,
  CXXLanguageLinkage,
  NoLanguageLinkage
};

/// \brief A more specific kind of linkage than enum Linkage.
///
/// This is relevant to CodeGen and AST file reading.
enum GVALinkage {
  GVA_Internal,
  GVA_AvailableExternally,
  GVA_DiscardableODR,
  GVA_StrongExternal,
  GVA_StrongODR
};

inline bool isExternallyVisible(Linkage L) {
  return L == ExternalLinkage || L == VisibleNoLinkage;
}

inline Linkage getFormalLinkage(Linkage L) {
  if (L == UniqueExternalLinkage)
    return ExternalLinkage;
  if (L == VisibleNoLinkage)
    return NoLinkage;
  return L;
}

inline bool isExternalFormalLinkage(Linkage L) {
  return getFormalLinkage(L) == ExternalLinkage;
}

/// \brief Compute the minimum linkage given two linkages.
///
/// The linkage can be interpreted as a pair formed by the formal linkage and
/// a boolean for external visibility. This is just what getFormalLinkage and
/// isExternallyVisible return. We want the minimum of both components. The
/// Linkage enum is defined in an order that makes this simple, we just need
/// special cases for when VisibleNoLinkage would lose the visible bit and
/// become NoLinkage.
inline Linkage minLinkage(Linkage L1, Linkage L2) {
  if (L2 == VisibleNoLinkage)
    std::swap(L1, L2);
  if (L1 == VisibleNoLinkage) {
    if (L2 == InternalLinkage)
      return NoLinkage;
    if (L2 == UniqueExternalLinkage)
      return NoLinkage;
  }
  return L1 < L2 ? L1 : L2;
}

} // end namespace clang

#endif // LLVM_CLANG_BASIC_LINKAGE_H
