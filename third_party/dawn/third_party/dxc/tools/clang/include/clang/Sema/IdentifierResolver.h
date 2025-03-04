//===- IdentifierResolver.h - Lexical Scope Name lookup ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the IdentifierResolver class, which is used for lexical
// scoped lookup, based on declaration names.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_IDENTIFIERRESOLVER_H
#define LLVM_CLANG_SEMA_IDENTIFIERRESOLVER_H

#include "clang/Basic/IdentifierTable.h"
#include "llvm/ADT/SmallVector.h"

namespace clang {

class ASTContext;
class Decl;
class DeclContext;
class DeclarationName;
class ExternalPreprocessorSource;
class NamedDecl;
class Preprocessor;
class Scope;
  
/// IdentifierResolver - Keeps track of shadowed decls on enclosing
/// scopes.  It manages the shadowing chains of declaration names and
/// implements efficient decl lookup based on a declaration name.
class IdentifierResolver {

  /// IdDeclInfo - Keeps track of information about decls associated
  /// to a particular declaration name. IdDeclInfos are lazily
  /// constructed and assigned to a declaration name the first time a
  /// decl with that declaration name is shadowed in some scope.
  class IdDeclInfo {
  public:
    typedef SmallVector<NamedDecl*, 2> DeclsTy;

    inline DeclsTy::iterator decls_begin() { return Decls.begin(); }
    inline DeclsTy::iterator decls_end() { return Decls.end(); }

    void AddDecl(NamedDecl *D) { Decls.push_back(D); }

    /// RemoveDecl - Remove the decl from the scope chain.
    /// The decl must already be part of the decl chain.
    void RemoveDecl(NamedDecl *D);

    /// \brief Insert the given declaration at the given position in the list.
    void InsertDecl(DeclsTy::iterator Pos, NamedDecl *D) {
      Decls.insert(Pos, D);
    }
                    
  private:
    DeclsTy Decls;
  };

public:

  /// iterator - Iterate over the decls of a specified declaration name.
  /// It will walk or not the parent declaration contexts depending on how
  /// it was instantiated.
  class iterator {
  public:
    typedef NamedDecl *             value_type;
    typedef NamedDecl *             reference;
    typedef NamedDecl *             pointer;
    typedef std::input_iterator_tag iterator_category;
    typedef std::ptrdiff_t          difference_type;

    /// Ptr - There are 3 forms that 'Ptr' represents:
    /// 1) A single NamedDecl. (Ptr & 0x1 == 0)
    /// 2) A IdDeclInfo::DeclsTy::iterator that traverses only the decls of the
    ///    same declaration context. (Ptr & 0x3 == 0x1)
    /// 3) A IdDeclInfo::DeclsTy::iterator that traverses the decls of parent
    ///    declaration contexts too. (Ptr & 0x3 == 0x3)
    uintptr_t Ptr;
    typedef IdDeclInfo::DeclsTy::iterator BaseIter;

    /// A single NamedDecl. (Ptr & 0x1 == 0)
    iterator(NamedDecl *D) {
      Ptr = reinterpret_cast<uintptr_t>(D);
      assert((Ptr & 0x1) == 0 && "Invalid Ptr!");
    }
    /// A IdDeclInfo::DeclsTy::iterator that walks or not the parent declaration
    /// contexts depending on 'LookInParentCtx'.
    iterator(BaseIter I) {
      Ptr = reinterpret_cast<uintptr_t>(I) | 0x1;
    }

    bool isIterator() const { return (Ptr & 0x1); }

    BaseIter getIterator() const {
      assert(isIterator() && "Ptr not an iterator!");
      return reinterpret_cast<BaseIter>(Ptr & ~0x3);
    }

    friend class IdentifierResolver;

    void incrementSlowCase();
  public:
    iterator() : Ptr(0) {}

    NamedDecl *operator*() const {
      if (isIterator())
        return *getIterator();
      else
        return reinterpret_cast<NamedDecl*>(Ptr);
    }

    bool operator==(const iterator &RHS) const {
      return Ptr == RHS.Ptr;
    }
    bool operator!=(const iterator &RHS) const {
      return Ptr != RHS.Ptr;
    }

    // Preincrement.
    iterator& operator++() {
      if (!isIterator()) // common case.
        Ptr = 0;
      else
        incrementSlowCase();
      return *this;
    }

    uintptr_t getAsOpaqueValue() const { return Ptr; }

    static iterator getFromOpaqueValue(uintptr_t P) {
      iterator Result;
      Result.Ptr = P;
      return Result;
    }
  };

  /// begin - Returns an iterator for decls with the name 'Name'.
  iterator begin(DeclarationName Name);

  /// end - Returns an iterator that has 'finished'.
  iterator end() {
    return iterator();
  }

  /// isDeclInScope - If 'Ctx' is a function/method, isDeclInScope returns true
  /// if 'D' is in Scope 'S', otherwise 'S' is ignored and isDeclInScope returns
  /// true if 'D' belongs to the given declaration context.
  ///
  /// \param AllowInlineNamespace If \c true, we are checking whether a prior
  ///        declaration is in scope in a declaration that requires a prior
  ///        declaration (because it is either explicitly qualified or is a
  ///        template instantiation or specialization). In this case, a
  ///        declaration is in scope if it's in the inline namespace set of the
  ///        context.
  bool isDeclInScope(Decl *D, DeclContext *Ctx, Scope *S = nullptr,
                     bool AllowInlineNamespace = false) const;

  /// AddDecl - Link the decl to its shadowed decl chain.
  void AddDecl(NamedDecl *D);

  /// RemoveDecl - Unlink the decl from its shadowed decl chain.
  /// The decl must already be part of the decl chain.
  void RemoveDecl(NamedDecl *D);

  /// \brief Insert the given declaration after the given iterator
  /// position.
  void InsertDeclAfter(iterator Pos, NamedDecl *D);

  /// \brief Try to add the given declaration to the top level scope, if it
  /// (or a redeclaration of it) hasn't already been added.
  ///
  /// \param D The externally-produced declaration to add.
  ///
  /// \param Name The name of the externally-produced declaration.
  ///
  /// \returns true if the declaration was added, false otherwise.
  bool tryAddTopLevelDecl(NamedDecl *D, DeclarationName Name);
  
  explicit IdentifierResolver(Preprocessor &PP);
  ~IdentifierResolver();

private:
  const LangOptions &LangOpt;
  Preprocessor &PP;
  
  class IdDeclInfoMap;
  IdDeclInfoMap *IdDeclInfos;

  void updatingIdentifier(IdentifierInfo &II);
  void readingIdentifier(IdentifierInfo &II);
  
  /// FETokenInfo contains a Decl pointer if lower bit == 0.
  static inline bool isDeclPtr(void *Ptr) {
    return (reinterpret_cast<uintptr_t>(Ptr) & 0x1) == 0;
  }

  /// FETokenInfo contains a IdDeclInfo pointer if lower bit == 1.
  static inline IdDeclInfo *toIdDeclInfo(void *Ptr) {
    assert((reinterpret_cast<uintptr_t>(Ptr) & 0x1) == 1
          && "Ptr not a IdDeclInfo* !");
    return reinterpret_cast<IdDeclInfo*>(
                    reinterpret_cast<uintptr_t>(Ptr) & ~0x1
                                                            );
  }
};

} // end namespace clang

#endif
