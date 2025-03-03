//===-- UnresolvedSet.h - Unresolved sets of declarations  ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the UnresolvedSet class, which is used to store
//  collections of declarations in the AST.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_UNRESOLVEDSET_H
#define LLVM_CLANG_AST_UNRESOLVEDSET_H

#include "clang/AST/DeclAccessPair.h"
#include "clang/Basic/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/iterator.h"

namespace clang {

/// The iterator over UnresolvedSets.  Serves as both the const and
/// non-const iterator.
class UnresolvedSetIterator : public llvm::iterator_adaptor_base<
                                  UnresolvedSetIterator, DeclAccessPair *,
                                  std::random_access_iterator_tag, NamedDecl *,
                                  std::ptrdiff_t, NamedDecl *, NamedDecl *> {
  friend class UnresolvedSetImpl;
  friend class ASTUnresolvedSet;
  friend class OverloadExpr;

  explicit UnresolvedSetIterator(DeclAccessPair *Iter)
      : iterator_adaptor_base(Iter) {}
  explicit UnresolvedSetIterator(const DeclAccessPair *Iter)
      : iterator_adaptor_base(const_cast<DeclAccessPair *>(Iter)) {}

public:
  UnresolvedSetIterator() {}

  NamedDecl *getDecl() const { return I->getDecl(); }
  void setDecl(NamedDecl *ND) const { return I->setDecl(ND); }
  AccessSpecifier getAccess() const { return I->getAccess(); }
  void setAccess(AccessSpecifier AS) { I->setAccess(AS); }
  const DeclAccessPair &getPair() const { return *I; }

  NamedDecl *operator*() const { return getDecl(); }
  NamedDecl *operator->() const { return **this; }
};

/// \brief A set of unresolved declarations.
class UnresolvedSetImpl {
  typedef SmallVectorImpl<DeclAccessPair> DeclsTy;

  // Don't allow direct construction, and only permit subclassing by
  // UnresolvedSet.
private:
  template <unsigned N> friend class UnresolvedSet;
  UnresolvedSetImpl() {}
  UnresolvedSetImpl(const UnresolvedSetImpl &) {}

public:
  // We don't currently support assignment through this iterator, so we might
  // as well use the same implementation twice.
  typedef UnresolvedSetIterator iterator;
  typedef UnresolvedSetIterator const_iterator;

  iterator begin() { return iterator(decls().begin()); }
  iterator end() { return iterator(decls().end()); }

  const_iterator begin() const { return const_iterator(decls().begin()); }
  const_iterator end() const { return const_iterator(decls().end()); }

  void addDecl(NamedDecl *D) {
    addDecl(D, AS_none);
  }

  void addDecl(NamedDecl *D, AccessSpecifier AS) {
    decls().push_back(DeclAccessPair::make(D, AS));
  }

  /// Replaces the given declaration with the new one, once.
  ///
  /// \return true if the set changed
  bool replace(const NamedDecl* Old, NamedDecl *New) {
    for (DeclsTy::iterator I = decls().begin(), E = decls().end(); I != E; ++I)
      if (I->getDecl() == Old)
        return (I->setDecl(New), true);
    return false;
  }

  /// Replaces the declaration at the given iterator with the new one,
  /// preserving the original access bits.
  void replace(iterator I, NamedDecl *New) { I.I->setDecl(New); }

  void replace(iterator I, NamedDecl *New, AccessSpecifier AS) {
    I.I->set(New, AS);
  }

  void erase(unsigned I) { decls()[I] = decls().pop_back_val(); }

  void erase(iterator I) { *I.I = decls().pop_back_val(); }

  void setAccess(iterator I, AccessSpecifier AS) { I.I->setAccess(AS); }

  void clear() { decls().clear(); }
  void set_size(unsigned N) { decls().set_size(N); }

  bool empty() const { return decls().empty(); }
  unsigned size() const { return decls().size(); }

  void append(iterator I, iterator E) { decls().append(I.I, E.I); }

  DeclAccessPair &operator[](unsigned I) { return decls()[I]; }
  const DeclAccessPair &operator[](unsigned I) const { return decls()[I]; }

private:
  // These work because the only permitted subclass is UnresolvedSetImpl

  DeclsTy &decls() {
    return *reinterpret_cast<DeclsTy*>(this);
  }
  const DeclsTy &decls() const {
    return *reinterpret_cast<const DeclsTy*>(this);
  }
};

/// \brief A set of unresolved declarations.
template <unsigned InlineCapacity> class UnresolvedSet :
    public UnresolvedSetImpl {
  SmallVector<DeclAccessPair, InlineCapacity> Decls;
};

  
} // namespace clang

#endif
