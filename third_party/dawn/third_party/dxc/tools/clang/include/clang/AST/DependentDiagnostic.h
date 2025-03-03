//===-- DependentDiagnostic.h - Dependently-generated diagnostics -*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines interfaces for diagnostics which may or may
//  fire based on how a template is instantiated.
//
//  At the moment, the only consumer of this interface is access
//  control.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_DEPENDENTDIAGNOSTIC_H
#define LLVM_CLANG_AST_DEPENDENTDIAGNOSTIC_H

#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclContextInternals.h"
#include "clang/AST/Type.h"
#include "clang/Basic/PartialDiagnostic.h"
#include "clang/Basic/SourceLocation.h"

namespace clang {

class ASTContext;
class CXXRecordDecl;
class NamedDecl;

/// A dependently-generated diagnostic.
class DependentDiagnostic {
public:
  enum AccessNonce { Access = 0 };

  static DependentDiagnostic *Create(ASTContext &Context,
                                     DeclContext *Parent,
                                     AccessNonce _,
                                     SourceLocation Loc,
                                     bool IsMemberAccess,
                                     AccessSpecifier AS,
                                     NamedDecl *TargetDecl,
                                     CXXRecordDecl *NamingClass,
                                     QualType BaseObjectType,
                                     const PartialDiagnostic &PDiag) {
    DependentDiagnostic *DD = Create(Context, Parent, PDiag);
    DD->AccessData.Loc = Loc.getRawEncoding();
    DD->AccessData.IsMember = IsMemberAccess;
    DD->AccessData.Access = AS;
    DD->AccessData.TargetDecl = TargetDecl;
    DD->AccessData.NamingClass = NamingClass;
    DD->AccessData.BaseObjectType = BaseObjectType.getAsOpaquePtr();
    return DD;
  }

  unsigned getKind() const {
    return Access;
  }

  bool isAccessToMember() const {
    assert(getKind() == Access);
    return AccessData.IsMember;
  }

  AccessSpecifier getAccess() const {
    assert(getKind() == Access);
    return AccessSpecifier(AccessData.Access);
  }

  SourceLocation getAccessLoc() const {
    assert(getKind() == Access);
    return SourceLocation::getFromRawEncoding(AccessData.Loc);
  }

  NamedDecl *getAccessTarget() const {
    assert(getKind() == Access);
    return AccessData.TargetDecl;
  }

  NamedDecl *getAccessNamingClass() const {
    assert(getKind() == Access);
    return AccessData.NamingClass;
  }

  QualType getAccessBaseObjectType() const {
    assert(getKind() == Access);
    return QualType::getFromOpaquePtr(AccessData.BaseObjectType);
  }

  const PartialDiagnostic &getDiagnostic() const {
    return Diag;
  }

private:
  DependentDiagnostic(const PartialDiagnostic &PDiag,
                      PartialDiagnostic::Storage *Storage) 
    : Diag(PDiag, Storage) {}
  
  static DependentDiagnostic *Create(ASTContext &Context,
                                     DeclContext *Parent,
                                     const PartialDiagnostic &PDiag);

  friend class DependentStoredDeclsMap;
  friend class DeclContext::ddiag_iterator;
  DependentDiagnostic *NextDiagnostic;

  PartialDiagnostic Diag;

  struct {
    unsigned Loc;
    unsigned Access : 2;
    unsigned IsMember : 1;
    NamedDecl *TargetDecl;
    CXXRecordDecl *NamingClass;
    void *BaseObjectType;
  } AccessData;
};

/// 

/// An iterator over the dependent diagnostics in a dependent context.
class DeclContext::ddiag_iterator {
public:
  ddiag_iterator() : Ptr(nullptr) {}
  explicit ddiag_iterator(DependentDiagnostic *Ptr) : Ptr(Ptr) {}

  typedef DependentDiagnostic *value_type;
  typedef DependentDiagnostic *reference;
  typedef DependentDiagnostic *pointer;
  typedef int difference_type;
  typedef std::forward_iterator_tag iterator_category;

  reference operator*() const { return Ptr; }

  ddiag_iterator &operator++() {
    assert(Ptr && "attempt to increment past end of diag list");
    Ptr = Ptr->NextDiagnostic;
    return *this;
  }

  ddiag_iterator operator++(int) {
    ddiag_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(ddiag_iterator Other) const {
    return Ptr == Other.Ptr;
  }

  bool operator!=(ddiag_iterator Other) const {
    return Ptr != Other.Ptr;
  }

  ddiag_iterator &operator+=(difference_type N) {
    assert(N >= 0 && "cannot rewind a DeclContext::ddiag_iterator");
    while (N--)
      ++*this;
    return *this;
  }

  ddiag_iterator operator+(difference_type N) const {
    ddiag_iterator tmp = *this;
    tmp += N;
    return tmp;
  }

private:
  DependentDiagnostic *Ptr;
};

inline DeclContext::ddiag_range DeclContext::ddiags() const {
  assert(isDependentContext()
         && "cannot iterate dependent diagnostics of non-dependent context");
  const DependentStoredDeclsMap *Map
    = static_cast<DependentStoredDeclsMap*>(getPrimaryContext()->getLookupPtr());

  if (!Map)
    // Return an empty range using the always-end default constructor.
    return ddiag_range(ddiag_iterator(), ddiag_iterator());

  return ddiag_range(ddiag_iterator(Map->FirstDiagnostic), ddiag_iterator());
}

}

#endif
