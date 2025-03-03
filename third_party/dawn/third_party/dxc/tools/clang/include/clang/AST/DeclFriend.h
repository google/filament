//===-- DeclFriend.h - Classes for C++ friend declarations -*- C++ -*------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the section of the AST representing C++ friend
// declarations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_DECLFRIEND_H
#define LLVM_CLANG_AST_DECLFRIEND_H

#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TypeLoc.h"
#include "llvm/Support/Compiler.h"

namespace clang {

/// FriendDecl - Represents the declaration of a friend entity,
/// which can be a function, a type, or a templated function or type.
//  For example:
///
/// @code
/// template <typename T> class A {
///   friend int foo(T);
///   friend class B;
///   friend T; // only in C++0x
///   template <typename U> friend class C;
///   template <typename U> friend A& operator+=(A&, const U&) { ... }
/// };
/// @endcode
///
/// The semantic context of a friend decl is its declaring class.
class FriendDecl : public Decl {
  virtual void anchor();
public:
  typedef llvm::PointerUnion<NamedDecl*,TypeSourceInfo*> FriendUnion;

private:
  // The declaration that's a friend of this class.
  FriendUnion Friend;

  // A pointer to the next friend in the sequence.
  LazyDeclPtr NextFriend;

  // Location of the 'friend' specifier.
  SourceLocation FriendLoc;

  /// True if this 'friend' declaration is unsupported.  Eventually we
  /// will support every possible friend declaration, but for now we
  /// silently ignore some and set this flag to authorize all access.
  bool UnsupportedFriend : 1;

  // The number of "outer" template parameter lists in non-templatic
  // (currently unsupported) friend type declarations, such as
  //     template <class T> friend class A<T>::B;
  unsigned NumTPLists : 31;

  // The tail-allocated friend type template parameter lists (if any).
  TemplateParameterList* const *getTPLists() const {
    return reinterpret_cast<TemplateParameterList* const *>(this + 1);
  }
  TemplateParameterList **getTPLists() {
    return reinterpret_cast<TemplateParameterList**>(this + 1);
  }

  friend class CXXRecordDecl::friend_iterator;
  friend class CXXRecordDecl;

  FriendDecl(DeclContext *DC, SourceLocation L, FriendUnion Friend,
             SourceLocation FriendL,
             ArrayRef<TemplateParameterList*> FriendTypeTPLists)
    : Decl(Decl::Friend, DC, L),
      Friend(Friend),
      NextFriend(),
      FriendLoc(FriendL),
      UnsupportedFriend(false),
      NumTPLists(FriendTypeTPLists.size()) {
    for (unsigned i = 0; i < NumTPLists; ++i)
      getTPLists()[i] = FriendTypeTPLists[i];
  }

  FriendDecl(EmptyShell Empty, unsigned NumFriendTypeTPLists)
    : Decl(Decl::Friend, Empty), NextFriend(),
      NumTPLists(NumFriendTypeTPLists) { }

  FriendDecl *getNextFriend() {
    if (!NextFriend.isOffset())
      return cast_or_null<FriendDecl>(NextFriend.get(nullptr));
    return getNextFriendSlowCase();
  }
  FriendDecl *getNextFriendSlowCase();

public:
  static FriendDecl *Create(ASTContext &C, DeclContext *DC,
                            SourceLocation L, FriendUnion Friend_,
                            SourceLocation FriendL,
                            ArrayRef<TemplateParameterList*> FriendTypeTPLists
                            = None);
  static FriendDecl *CreateDeserialized(ASTContext &C, unsigned ID,
                                        unsigned FriendTypeNumTPLists);

  /// If this friend declaration names an (untemplated but possibly
  /// dependent) type, return the type; otherwise return null.  This
  /// is used for elaborated-type-specifiers and, in C++0x, for
  /// arbitrary friend type declarations.
  TypeSourceInfo *getFriendType() const {
    return Friend.dyn_cast<TypeSourceInfo*>();
  }
  unsigned getFriendTypeNumTemplateParameterLists() const {
    return NumTPLists;
  }
  TemplateParameterList *getFriendTypeTemplateParameterList(unsigned N) const {
    assert(N < NumTPLists);
    return getTPLists()[N];
  }

  /// If this friend declaration doesn't name a type, return the inner
  /// declaration.
  NamedDecl *getFriendDecl() const {
    return Friend.dyn_cast<NamedDecl*>();
  }

  /// Retrieves the location of the 'friend' keyword.
  SourceLocation getFriendLoc() const {
    return FriendLoc;
  }

  /// Retrieves the source range for the friend declaration.
  SourceRange getSourceRange() const override LLVM_READONLY {
    if (NamedDecl *ND = getFriendDecl()) {
      if (FunctionDecl *FD = dyn_cast<FunctionDecl>(ND))
        return FD->getSourceRange();
      if (FunctionTemplateDecl *FTD = dyn_cast<FunctionTemplateDecl>(ND))
        return FTD->getSourceRange();
      if (ClassTemplateDecl *CTD = dyn_cast<ClassTemplateDecl>(ND))
        return CTD->getSourceRange();
      if (DeclaratorDecl *DD = dyn_cast<DeclaratorDecl>(ND)) {
        if (DD->getOuterLocStart() != DD->getInnerLocStart())
          return DD->getSourceRange();
      }
      return SourceRange(getFriendLoc(), ND->getLocEnd());
    }
    else if (TypeSourceInfo *TInfo = getFriendType()) {
      SourceLocation StartL = (NumTPLists == 0)
        ? getFriendLoc()
        : getTPLists()[0]->getTemplateLoc();
      return SourceRange(StartL, TInfo->getTypeLoc().getEndLoc());
    }
    else
      return SourceRange(getFriendLoc(), getLocation());
  }

  /// Determines if this friend kind is unsupported.
  bool isUnsupportedFriend() const {
    return UnsupportedFriend;
  }
  void setUnsupportedFriend(bool Unsupported) {
    UnsupportedFriend = Unsupported;
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Decl::Friend; }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
};

/// An iterator over the friend declarations of a class.
class CXXRecordDecl::friend_iterator {
  FriendDecl *Ptr;

  friend class CXXRecordDecl;
  explicit friend_iterator(FriendDecl *Ptr) : Ptr(Ptr) {}
public:
  friend_iterator() {}

  typedef FriendDecl *value_type;
  typedef FriendDecl *reference;
  typedef FriendDecl *pointer;
  typedef int difference_type;
  typedef std::forward_iterator_tag iterator_category;

  reference operator*() const { return Ptr; }

  friend_iterator &operator++() {
    assert(Ptr && "attempt to increment past end of friend list");
    Ptr = Ptr->getNextFriend();
    return *this;
  }

  friend_iterator operator++(int) {
    friend_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  bool operator==(const friend_iterator &Other) const {
    return Ptr == Other.Ptr;
  }

  bool operator!=(const friend_iterator &Other) const {
    return Ptr != Other.Ptr;
  }

  friend_iterator &operator+=(difference_type N) {
    assert(N >= 0 && "cannot rewind a CXXRecordDecl::friend_iterator");
    while (N--)
      ++*this;
    return *this;
  }

  friend_iterator operator+(difference_type N) const {
    friend_iterator tmp = *this;
    tmp += N;
    return tmp;
  }
};

inline CXXRecordDecl::friend_iterator CXXRecordDecl::friend_begin() const {
  return friend_iterator(getFirstFriend());
}

inline CXXRecordDecl::friend_iterator CXXRecordDecl::friend_end() const {
  return friend_iterator(nullptr);
}

inline CXXRecordDecl::friend_range CXXRecordDecl::friends() const {
  return friend_range(friend_begin(), friend_end());
}

inline void CXXRecordDecl::pushFriendDecl(FriendDecl *FD) {
  assert(!FD->NextFriend && "friend already has next friend?");
  FD->NextFriend = data().FirstFriend;
  data().FirstFriend = FD;
}
  
}

#endif
