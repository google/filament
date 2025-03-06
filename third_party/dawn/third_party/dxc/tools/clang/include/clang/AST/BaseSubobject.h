//===--- BaseSubobject.h - BaseSubobject class ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides a definition of the BaseSubobject class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_BASESUBOBJECT_H
#define LLVM_CLANG_AST_BASESUBOBJECT_H

#include "clang/AST/CharUnits.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/type_traits.h"

namespace clang {
  class CXXRecordDecl;

// BaseSubobject - Uniquely identifies a direct or indirect base class. 
// Stores both the base class decl and the offset from the most derived class to
// the base class. Used for vtable and VTT generation.
class BaseSubobject {
  /// Base - The base class declaration.
  const CXXRecordDecl *Base;
  
  /// BaseOffset - The offset from the most derived class to the base class.
  CharUnits BaseOffset;
  
public:
  BaseSubobject() { }
  BaseSubobject(const CXXRecordDecl *Base, CharUnits BaseOffset)
    : Base(Base), BaseOffset(BaseOffset) { }
  
  /// getBase - Returns the base class declaration.
  const CXXRecordDecl *getBase() const { return Base; }

  /// getBaseOffset - Returns the base class offset.
  CharUnits getBaseOffset() const { return BaseOffset; }

  friend bool operator==(const BaseSubobject &LHS, const BaseSubobject &RHS) {
    return LHS.Base == RHS.Base && LHS.BaseOffset == RHS.BaseOffset;
 }
};

} // end namespace clang

namespace llvm {

template<> struct DenseMapInfo<clang::BaseSubobject> {
  static clang::BaseSubobject getEmptyKey() {
    return clang::BaseSubobject(
      DenseMapInfo<const clang::CXXRecordDecl *>::getEmptyKey(),
      clang::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getEmptyKey()));
  }

  static clang::BaseSubobject getTombstoneKey() {
    return clang::BaseSubobject(
      DenseMapInfo<const clang::CXXRecordDecl *>::getTombstoneKey(),
      clang::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getTombstoneKey()));
  }

  static unsigned getHashValue(const clang::BaseSubobject &Base) {
    typedef std::pair<const clang::CXXRecordDecl *, clang::CharUnits> PairTy;
    return DenseMapInfo<PairTy>::getHashValue(PairTy(Base.getBase(),
                                                     Base.getBaseOffset()));
  }

  static bool isEqual(const clang::BaseSubobject &LHS, 
                      const clang::BaseSubobject &RHS) {
    return LHS == RHS;
  }
};

// It's OK to treat BaseSubobject as a POD type.
template <> struct isPodLike<clang::BaseSubobject> {
  static const bool value = true;
};

}

#endif
