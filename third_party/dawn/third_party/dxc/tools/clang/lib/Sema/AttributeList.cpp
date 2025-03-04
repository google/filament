//===--- AttributeList.cpp --------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the AttributeList class implementation
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/AttributeList.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Sema/SemaInternal.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSwitch.h"
using namespace clang;

IdentifierLoc *IdentifierLoc::create(ASTContext &Ctx, SourceLocation Loc,
                                     IdentifierInfo *Ident) {
  IdentifierLoc *Result = new (Ctx) IdentifierLoc;
  Result->Loc = Loc;
  Result->Ident = Ident;
  return Result;
}

size_t AttributeList::allocated_size() const {
  if (IsAvailability) return AttributeFactory::AvailabilityAllocSize;
  else if (IsTypeTagForDatatype)
    return AttributeFactory::TypeTagForDatatypeAllocSize;
  else if (IsProperty)
    return AttributeFactory::PropertyAllocSize;
  return (sizeof(AttributeList) + NumArgs * sizeof(ArgsUnion));
}

AttributeFactory::AttributeFactory() {
  // Go ahead and configure all the inline capacity.  This is just a memset.
  FreeLists.resize(InlineFreeListsCapacity);
}
AttributeFactory::~AttributeFactory() {}

static size_t getFreeListIndexForSize(size_t size) {
  assert(size >= sizeof(AttributeList));
  assert((size % sizeof(void*)) == 0);
  return ((size - sizeof(AttributeList)) / sizeof(void*));
}

void *AttributeFactory::allocate(size_t size) {
  // Check for a previously reclaimed attribute.
  size_t index = getFreeListIndexForSize(size);
  if (index < FreeLists.size()) {
    if (AttributeList *attr = FreeLists[index]) {
      FreeLists[index] = attr->NextInPool;
      return attr;
    }
  }

  // Otherwise, allocate something new.
  return Alloc.Allocate(size, llvm::AlignOf<AttributeFactory>::Alignment);
}

void AttributeFactory::reclaimPool(AttributeList *cur) {
  assert(cur && "reclaiming empty pool!");
  do {
    // Read this here, because we're going to overwrite NextInPool
    // when we toss 'cur' into the appropriate queue.
    AttributeList *next = cur->NextInPool;

    size_t size = cur->allocated_size();
    size_t freeListIndex = getFreeListIndexForSize(size);

    // Expand FreeLists to the appropriate size, if required.
    if (freeListIndex >= FreeLists.size())
      FreeLists.resize(freeListIndex+1);

    // Add 'cur' to the appropriate free-list.
    cur->NextInPool = FreeLists[freeListIndex];
    FreeLists[freeListIndex] = cur;
    
    cur = next;
  } while (cur);
}

void AttributePool::takePool(AttributeList *pool) {
  assert(pool);

  // Fast path:  this pool is empty.
  if (!Head) {
    Head = pool;
    return;
  }

  // Reverse the pool onto the current head.  This optimizes for the
  // pattern of pulling a lot of pools into a single pool.
  do {
    AttributeList *next = pool->NextInPool;
    pool->NextInPool = Head;
    Head = pool;
    pool = next;
  } while (pool);
}

#include "clang/Sema/AttrParsedAttrKinds.inc"

AttributeList::Kind AttributeList::getKind(const IdentifierInfo *Name,
                                           const IdentifierInfo *ScopeName,
                                           Syntax SyntaxUsed) {
  StringRef AttrName = Name->getName();

  SmallString<64> FullName;
  if (ScopeName)
    FullName += ScopeName->getName();

  // Normalize the attribute name, __foo__ becomes foo. This is only allowable
  // for GNU attributes.
  bool IsGNU = SyntaxUsed == AS_GNU || (SyntaxUsed == AS_CXX11 &&
                                        FullName == "gnu");
  if (IsGNU && AttrName.size() >= 4 && AttrName.startswith("__") &&
      AttrName.endswith("__"))
    AttrName = AttrName.slice(2, AttrName.size() - 2);

  // Ensure that in the case of C++11 attributes, we look for '::foo' if it is
  // unscoped.
  if (ScopeName || SyntaxUsed == AS_CXX11)
    FullName += "::";
  FullName += AttrName;

  // HLSL Change Starts: - support case-insensitive variant
#if 1
  AttributeList::Kind Result = ::getAttrKind(FullName, SyntaxUsed);
  if (Result == AttributeList::UnknownAttribute) {
    std::string lower = FullName.str().lower();
    Result = ::getAttrKind(StringRef(lower), SyntaxUsed);
  }
  return Result;
#else
  return ::getAttrKind(FullName, SyntaxUsed);
#endif
  // HLSL Change Ends
}

unsigned AttributeList::getAttributeSpellingListIndex() const {
  // Both variables will be used in tablegen generated
  // attribute spell list index matching code.
  StringRef Name = AttrName->getName();
  StringRef Scope = ScopeName ? ScopeName->getName() : "";

#include "clang/Sema/AttrSpellingListIndex.inc"

}

struct ParsedAttrInfo {
  unsigned NumArgs : 4;
  unsigned OptArgs : 4;
  unsigned HasCustomParsing : 1;
  unsigned IsTargetSpecific : 1;
  unsigned IsType : 1;
  unsigned IsKnownToGCC : 1;

  bool (*DiagAppertainsToDecl)(Sema &S, const AttributeList &Attr,
                               const Decl *);
  bool (*DiagLangOpts)(Sema &S, const AttributeList &Attr);
  bool (*ExistsInTarget)(const llvm::Triple &T);
  unsigned (*SpellingIndexToSemanticSpelling)(const AttributeList &Attr);
};

namespace {
  #include "clang/Sema/AttrParsedAttrImpl.inc"
}

static const ParsedAttrInfo &getInfo(const AttributeList &A) {
  return AttrInfoMap[A.getKind()];
}

unsigned AttributeList::getMinArgs() const {
  return getInfo(*this).NumArgs;
}

unsigned AttributeList::getMaxArgs() const {
  return getMinArgs() + getInfo(*this).OptArgs;
}

bool AttributeList::hasCustomParsing() const {
  return getInfo(*this).HasCustomParsing;
}

bool AttributeList::diagnoseAppertainsTo(Sema &S, const Decl *D) const {
  return getInfo(*this).DiagAppertainsToDecl(S, *this, D);
}

bool AttributeList::diagnoseLangOpts(Sema &S) const {
  return getInfo(*this).DiagLangOpts(S, *this);
}

bool AttributeList::isTargetSpecificAttr() const {
  return getInfo(*this).IsTargetSpecific;
}

bool AttributeList::isTypeAttr() const {
  return getInfo(*this).IsType;
}

bool AttributeList::existsInTarget(const llvm::Triple &T) const {
  return getInfo(*this).ExistsInTarget(T);
}

bool AttributeList::isKnownToGCC() const {
  return getInfo(*this).IsKnownToGCC;
}

unsigned AttributeList::getSemanticSpelling() const {
  return getInfo(*this).SpellingIndexToSemanticSpelling(*this);
}

bool AttributeList::hasVariadicArg() const {
  // If the attribute has the maximum number of optional arguments, we will
  // claim that as being variadic. If we someday get an attribute that
  // legitimately bumps up against that maximum, we can use another bit to track
  // whether it's truly variadic or not.
  return getInfo(*this).OptArgs == 15;
}
