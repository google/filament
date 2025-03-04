//===--- AttributeList.h - Parsed attribute sets ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the AttributeList class, which is used to collect
// parsed attributes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_ATTRIBUTELIST_H
#define LLVM_CLANG_SEMA_ATTRIBUTELIST_H

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/VersionTuple.h"
#include "clang/Sema/Ownership.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Allocator.h"
#include <cassert>

namespace clang {
  class ASTContext;
  class IdentifierInfo;
  class Expr;

/// \brief Represents information about a change in availability for
/// an entity, which is part of the encoding of the 'availability'
/// attribute.
struct AvailabilityChange {
  /// \brief The location of the keyword indicating the kind of change.
  SourceLocation KeywordLoc;

  /// \brief The version number at which the change occurred.
  VersionTuple Version;

  /// \brief The source range covering the version number.
  SourceRange VersionRange;

  /// \brief Determine whether this availability change is valid.
  bool isValid() const { return !Version.empty(); }
};

/// \brief Wraps an identifier and optional source location for the identifier.
struct IdentifierLoc {
  SourceLocation Loc;
  IdentifierInfo *Ident;

  static IdentifierLoc *create(ASTContext &Ctx, SourceLocation Loc,
                               IdentifierInfo *Ident);
};

/// \brief A union of the various pointer types that can be passed to an
/// AttributeList as an argument.
typedef llvm::PointerUnion<Expr*, IdentifierLoc*> ArgsUnion;
typedef llvm::SmallVector<ArgsUnion, 12U> ArgsVector;

/// AttributeList - Represents a syntactic attribute.
///
/// For a GNU attribute, there are four forms of this construct:
///
/// 1: __attribute__(( const )). ParmName/Args/NumArgs will all be unused.
/// 2: __attribute__(( mode(byte) )). ParmName used, Args/NumArgs unused.
/// 3: __attribute__(( format(printf, 1, 2) )). ParmName/Args/NumArgs all used.
/// 4: __attribute__(( aligned(16) )). ParmName is unused, Args/Num used.
///
class AttributeList { // TODO: This should really be called ParsedAttribute
public:
  /// The style used to specify an attribute.
  enum Syntax {
    /// __attribute__((...))
    AS_GNU,
    /// [[...]]
    AS_CXX11,
    /// __declspec(...)
    AS_Declspec,
    /// __ptr16, alignas(...), etc.
    AS_Keyword,
    /// Context-sensitive version of a keyword attribute.
    AS_ContextSensitiveKeyword,
    /// #pragma ...
    AS_Pragma
  };

private:
  IdentifierInfo *AttrName;
  IdentifierInfo *ScopeName;
  SourceRange AttrRange;
  SourceLocation ScopeLoc;
  SourceLocation EllipsisLoc;

  /// The number of expression arguments this attribute has.
  /// The expressions themselves are stored after the object.
  unsigned NumArgs : 15;

  /// Corresponds to the Syntax enum.
  unsigned SyntaxUsed : 3;

  /// True if already diagnosed as invalid.
  mutable unsigned Invalid : 1;

  /// True if this attribute was used as a type attribute.
  mutable unsigned UsedAsTypeAttr : 1;

  /// True if this has the extra information associated with an
  /// availability attribute.
  unsigned IsAvailability : 1;

  /// True if this has extra information associated with a
  /// type_tag_for_datatype attribute.
  unsigned IsTypeTagForDatatype : 1;

  /// True if this has extra information associated with a
  /// Microsoft __delcspec(property) attribute.
  unsigned IsProperty : 1;

  /// True if this has a ParsedType
  unsigned HasParsedType : 1;
  
  unsigned padding : 8;

  unsigned AttrKind;

  /// \brief The location of the 'unavailable' keyword in an
  /// availability attribute.
  SourceLocation UnavailableLoc;
  
  const Expr *MessageExpr;

  /// The next attribute in the current position.
  AttributeList *NextInPosition;

  /// The next attribute allocated in the current Pool.
  AttributeList *NextInPool;

  /// Arguments, if any, are stored immediately following the object.
  ArgsUnion *getArgsBuffer() {
    return reinterpret_cast<ArgsUnion*>(this+1);
  }
  ArgsUnion const *getArgsBuffer() const {
    return reinterpret_cast<ArgsUnion const *>(this+1);
  }

  enum AvailabilitySlot {
    IntroducedSlot, DeprecatedSlot, ObsoletedSlot
  };

  /// Availability information is stored immediately following the arguments,
  /// if any, at the end of the object.
  AvailabilityChange &getAvailabilitySlot(AvailabilitySlot index) {    
    return reinterpret_cast<AvailabilityChange*>(getArgsBuffer()
                                                 + NumArgs)[index];
  }
  const AvailabilityChange &getAvailabilitySlot(AvailabilitySlot index) const {
    return reinterpret_cast<const AvailabilityChange*>(getArgsBuffer()
                                                       + NumArgs)[index];
  }

public:
  struct TypeTagForDatatypeData {
    ParsedType *MatchingCType;
    unsigned LayoutCompatible : 1;
    unsigned MustBeNull : 1;
  };
  struct PropertyData {
    IdentifierInfo *GetterId, *SetterId;
    PropertyData(IdentifierInfo *getterId, IdentifierInfo *setterId)
    : GetterId(getterId), SetterId(setterId) {}
  };

private:
  /// Type tag information is stored immediately following the arguments, if
  /// any, at the end of the object.  They are mutually exlusive with
  /// availability slots.
  TypeTagForDatatypeData &getTypeTagForDatatypeDataSlot() {
    return *reinterpret_cast<TypeTagForDatatypeData*>(getArgsBuffer()+NumArgs);
  }

  const TypeTagForDatatypeData &getTypeTagForDatatypeDataSlot() const {
    return *reinterpret_cast<const TypeTagForDatatypeData*>(getArgsBuffer()
                                                            + NumArgs);
  }

  /// The type buffer immediately follows the object and are mutually exclusive
  /// with arguments.
  ParsedType &getTypeBuffer() {
    return *reinterpret_cast<ParsedType *>(this + 1);
  }

  const ParsedType &getTypeBuffer() const {
    return *reinterpret_cast<const ParsedType *>(this + 1);
  }

  /// The property data immediately follows the object is is mutually exclusive
  /// with arguments.
  PropertyData &getPropertyDataBuffer() {
    assert(IsProperty);
    return *reinterpret_cast<PropertyData*>(this + 1);
  }

  const PropertyData &getPropertyDataBuffer() const {
    assert(IsProperty);
    return *reinterpret_cast<const PropertyData*>(this + 1);
  }

  AttributeList(const AttributeList &) = delete;
  void operator=(const AttributeList &) = delete;
  void operator delete(void *) = delete;
  ~AttributeList() = delete;

  size_t allocated_size() const;

  /// Constructor for attributes with expression arguments.
  AttributeList(IdentifierInfo *attrName, SourceRange attrRange,
                IdentifierInfo *scopeName, SourceLocation scopeLoc,
                ArgsUnion *args, unsigned numArgs,
                Syntax syntaxUsed, SourceLocation ellipsisLoc)
    : AttrName(attrName), ScopeName(scopeName), AttrRange(attrRange),
      ScopeLoc(scopeLoc), EllipsisLoc(ellipsisLoc), NumArgs(numArgs),
      SyntaxUsed(syntaxUsed), Invalid(false), UsedAsTypeAttr(false),
      IsAvailability(false), IsTypeTagForDatatype(false), IsProperty(false),
      HasParsedType(false), NextInPosition(nullptr), NextInPool(nullptr) {
    if (numArgs) memcpy(getArgsBuffer(), args, numArgs * sizeof(ArgsUnion));
    AttrKind = getKind(getName(), getScopeName(), syntaxUsed);
  }

  /// Constructor for availability attributes.
  AttributeList(IdentifierInfo *attrName, SourceRange attrRange,
                IdentifierInfo *scopeName, SourceLocation scopeLoc,
                IdentifierLoc *Parm, const AvailabilityChange &introduced,
                const AvailabilityChange &deprecated,
                const AvailabilityChange &obsoleted,
                SourceLocation unavailable, 
                const Expr *messageExpr,
                Syntax syntaxUsed)
    : AttrName(attrName), ScopeName(scopeName), AttrRange(attrRange),
      ScopeLoc(scopeLoc), EllipsisLoc(), NumArgs(1), SyntaxUsed(syntaxUsed),
      Invalid(false), UsedAsTypeAttr(false), IsAvailability(true),
      IsTypeTagForDatatype(false), IsProperty(false), HasParsedType(false),
      UnavailableLoc(unavailable), MessageExpr(messageExpr),
      NextInPosition(nullptr), NextInPool(nullptr) {
    ArgsUnion PVal(Parm);
    memcpy(getArgsBuffer(), &PVal, sizeof(ArgsUnion));
    new (&getAvailabilitySlot(IntroducedSlot)) AvailabilityChange(introduced);
    new (&getAvailabilitySlot(DeprecatedSlot)) AvailabilityChange(deprecated);
    new (&getAvailabilitySlot(ObsoletedSlot)) AvailabilityChange(obsoleted);
    AttrKind = getKind(getName(), getScopeName(), syntaxUsed);
  }

  /// Constructor for objc_bridge_related attributes.
  AttributeList(IdentifierInfo *attrName, SourceRange attrRange,
                IdentifierInfo *scopeName, SourceLocation scopeLoc,
                IdentifierLoc *Parm1,
                IdentifierLoc *Parm2,
                IdentifierLoc *Parm3,
                Syntax syntaxUsed)
  : AttrName(attrName), ScopeName(scopeName), AttrRange(attrRange),
    ScopeLoc(scopeLoc), EllipsisLoc(), NumArgs(3), SyntaxUsed(syntaxUsed),
    Invalid(false), UsedAsTypeAttr(false), IsAvailability(false),
    IsTypeTagForDatatype(false), IsProperty(false), HasParsedType(false),
    NextInPosition(nullptr), NextInPool(nullptr) {
    ArgsVector Args;
    Args.push_back(Parm1);
    Args.push_back(Parm2);
    Args.push_back(Parm3);
    memcpy(getArgsBuffer(), &Args[0], 3 * sizeof(ArgsUnion));
    AttrKind = getKind(getName(), getScopeName(), syntaxUsed);
  }
  
  /// Constructor for type_tag_for_datatype attribute.
  AttributeList(IdentifierInfo *attrName, SourceRange attrRange,
                IdentifierInfo *scopeName, SourceLocation scopeLoc,
                IdentifierLoc *ArgKind, ParsedType matchingCType,
                bool layoutCompatible, bool mustBeNull, Syntax syntaxUsed)
    : AttrName(attrName), ScopeName(scopeName), AttrRange(attrRange),
      ScopeLoc(scopeLoc), EllipsisLoc(), NumArgs(1), SyntaxUsed(syntaxUsed),
      Invalid(false), UsedAsTypeAttr(false), IsAvailability(false),
      IsTypeTagForDatatype(true), IsProperty(false), HasParsedType(false),
      NextInPosition(nullptr), NextInPool(nullptr) {
    ArgsUnion PVal(ArgKind);
    memcpy(getArgsBuffer(), &PVal, sizeof(ArgsUnion));
    TypeTagForDatatypeData &ExtraData = getTypeTagForDatatypeDataSlot();
    new (&ExtraData.MatchingCType) ParsedType(matchingCType);
    ExtraData.LayoutCompatible = layoutCompatible;
    ExtraData.MustBeNull = mustBeNull;
    AttrKind = getKind(getName(), getScopeName(), syntaxUsed);
  }

  /// Constructor for attributes with a single type argument.
  AttributeList(IdentifierInfo *attrName, SourceRange attrRange,
                IdentifierInfo *scopeName, SourceLocation scopeLoc,
                ParsedType typeArg, Syntax syntaxUsed)
      : AttrName(attrName), ScopeName(scopeName), AttrRange(attrRange),
        ScopeLoc(scopeLoc), EllipsisLoc(), NumArgs(0), SyntaxUsed(syntaxUsed),
        Invalid(false), UsedAsTypeAttr(false), IsAvailability(false),
        IsTypeTagForDatatype(false), IsProperty(false), HasParsedType(true),
        NextInPosition(nullptr), NextInPool(nullptr) {
    new (&getTypeBuffer()) ParsedType(typeArg);
    AttrKind = getKind(getName(), getScopeName(), syntaxUsed);
  }

  /// Constructor for microsoft __declspec(property) attribute.
  AttributeList(IdentifierInfo *attrName, SourceRange attrRange,
                IdentifierInfo *scopeName, SourceLocation scopeLoc,
                IdentifierInfo *getterId, IdentifierInfo *setterId,
                Syntax syntaxUsed)
    : AttrName(attrName), ScopeName(scopeName), AttrRange(attrRange),
      ScopeLoc(scopeLoc), EllipsisLoc(), NumArgs(0), SyntaxUsed(syntaxUsed),
      Invalid(false), UsedAsTypeAttr(false), IsAvailability(false),
      IsTypeTagForDatatype(false), IsProperty(true), HasParsedType(false),
      NextInPosition(nullptr), NextInPool(nullptr) {
    new (&getPropertyDataBuffer()) PropertyData(getterId, setterId);
    AttrKind = getKind(getName(), getScopeName(), syntaxUsed);
  }

  friend class AttributePool;
  friend class AttributeFactory;

public:
  enum Kind {           
    #define PARSED_ATTR(NAME) AT_##NAME,
    #include "clang/Sema/AttrParsedAttrList.inc"
    #undef PARSED_ATTR
    IgnoredAttribute,
    UnknownAttribute
  };

  IdentifierInfo *getName() const { return AttrName; }
  SourceLocation getLoc() const { return AttrRange.getBegin(); }
  SourceRange getRange() const { return AttrRange; }
  
  bool hasScope() const { return ScopeName; }
  IdentifierInfo *getScopeName() const { return ScopeName; }
  SourceLocation getScopeLoc() const { return ScopeLoc; }
  
  bool hasParsedType() const { return HasParsedType; }

  /// Is this the Microsoft __declspec(property) attribute?
  bool isDeclspecPropertyAttribute() const  {
    return IsProperty;
  }

  bool isAlignasAttribute() const {
    // FIXME: Use a better mechanism to determine this.
    return getKind() == AT_Aligned && isKeywordAttribute();
  }

  bool isDeclspecAttribute() const { return SyntaxUsed == AS_Declspec; }
  bool isCXX11Attribute() const {
    return SyntaxUsed == AS_CXX11 || isAlignasAttribute();
  }
  bool isKeywordAttribute() const {
    return SyntaxUsed == AS_Keyword || SyntaxUsed == AS_ContextSensitiveKeyword;
  }

  bool isContextSensitiveKeywordAttribute() const {
    return SyntaxUsed == AS_ContextSensitiveKeyword;
  }

  bool isInvalid() const { return Invalid; }
  void setInvalid(bool b = true) const { Invalid = b; }

  bool isUsedAsTypeAttr() const { return UsedAsTypeAttr; }
  void setUsedAsTypeAttr() { UsedAsTypeAttr = true; }

  bool isPackExpansion() const { return EllipsisLoc.isValid(); }
  SourceLocation getEllipsisLoc() const { return EllipsisLoc; }

  Kind getKind() const { return Kind(AttrKind); }
  static Kind getKind(const IdentifierInfo *Name, const IdentifierInfo *Scope,
                      Syntax SyntaxUsed);

  AttributeList *getNext() const { return NextInPosition; }
  void setNext(AttributeList *N) { NextInPosition = N; }

  /// getNumArgs - Return the number of actual arguments to this attribute.
  unsigned getNumArgs() const { return NumArgs; }

  /// getArg - Return the specified argument.
  ArgsUnion getArg(unsigned Arg) const {
    assert(Arg < NumArgs && "Arg access out of range!");
    return getArgsBuffer()[Arg];
  }

  bool isArgExpr(unsigned Arg) const {
    return Arg < NumArgs && getArg(Arg).is<Expr*>();
  }
  Expr *getArgAsExpr(unsigned Arg) const {
    return getArg(Arg).get<Expr*>();
  }

  bool isArgIdent(unsigned Arg) const {
    return Arg < NumArgs && getArg(Arg).is<IdentifierLoc*>();
  }
  IdentifierLoc *getArgAsIdent(unsigned Arg) const {
    return getArg(Arg).get<IdentifierLoc*>();
  }

  const AvailabilityChange &getAvailabilityIntroduced() const {
    assert(getKind() == AT_Availability && "Not an availability attribute");
    return getAvailabilitySlot(IntroducedSlot);
  }

  const AvailabilityChange &getAvailabilityDeprecated() const {
    assert(getKind() == AT_Availability && "Not an availability attribute");
    return getAvailabilitySlot(DeprecatedSlot);
  }

  const AvailabilityChange &getAvailabilityObsoleted() const {
    assert(getKind() == AT_Availability && "Not an availability attribute");
    return getAvailabilitySlot(ObsoletedSlot);
  }

  SourceLocation getUnavailableLoc() const {
    assert(getKind() == AT_Availability && "Not an availability attribute");
    return UnavailableLoc;
  }
  
  const Expr * getMessageExpr() const {
    assert(getKind() == AT_Availability && "Not an availability attribute");
    return MessageExpr;
  }

  const ParsedType &getMatchingCType() const {
    assert(getKind() == AT_TypeTagForDatatype &&
           "Not a type_tag_for_datatype attribute");
    return *getTypeTagForDatatypeDataSlot().MatchingCType;
  }

  bool getLayoutCompatible() const {
    assert(getKind() == AT_TypeTagForDatatype &&
           "Not a type_tag_for_datatype attribute");
    return getTypeTagForDatatypeDataSlot().LayoutCompatible;
  }

  bool getMustBeNull() const {
    assert(getKind() == AT_TypeTagForDatatype &&
           "Not a type_tag_for_datatype attribute");
    return getTypeTagForDatatypeDataSlot().MustBeNull;
  }

  const ParsedType &getTypeArg() const {
    assert(HasParsedType && "Not a type attribute");
    return getTypeBuffer();
  }

  const PropertyData &getPropertyData() const {
    assert(isDeclspecPropertyAttribute() && "Not a __delcspec(property) attribute");
    return getPropertyDataBuffer();
  }

  /// \brief Get an index into the attribute spelling list
  /// defined in Attr.td. This index is used by an attribute
  /// to pretty print itself.
  unsigned getAttributeSpellingListIndex() const;

  bool isTargetSpecificAttr() const;
  bool isTypeAttr() const;

  bool hasCustomParsing() const;
  unsigned getMinArgs() const;
  unsigned getMaxArgs() const;
  bool hasVariadicArg() const;
  bool diagnoseAppertainsTo(class Sema &S, const Decl *D) const;
  bool diagnoseLangOpts(class Sema &S) const;
  bool existsInTarget(const llvm::Triple &T) const;
  bool isKnownToGCC() const;

  /// \brief If the parsed attribute has a semantic equivalent, and it would
  /// have a semantic Spelling enumeration (due to having semantically-distinct
  /// spelling variations), return the value of that semantic spelling. If the
  /// parsed attribute does not have a semantic equivalent, or would not have
  /// a Spelling enumeration, the value UINT_MAX is returned.
  unsigned getSemanticSpelling() const;
};

/// A factory, from which one makes pools, from which one creates
/// individual attributes which are deallocated with the pool.
///
/// Note that it's tolerably cheap to create and destroy one of
/// these as long as you don't actually allocate anything in it.
class AttributeFactory {
public:
  enum {
    /// The required allocation size of an availability attribute,
    /// which we want to ensure is a multiple of sizeof(void*).
    AvailabilityAllocSize =
      sizeof(AttributeList)
      + ((3 * sizeof(AvailabilityChange) + sizeof(void*) +
         sizeof(ArgsUnion) - 1)
         / sizeof(void*) * sizeof(void*)),
    TypeTagForDatatypeAllocSize =
      sizeof(AttributeList)
      + (sizeof(AttributeList::TypeTagForDatatypeData) + sizeof(void *) +
         sizeof(ArgsUnion) - 1)
        / sizeof(void*) * sizeof(void*),
    PropertyAllocSize =
      sizeof(AttributeList)
      + (sizeof(AttributeList::PropertyData) + sizeof(void *) - 1)
        / sizeof(void*) * sizeof(void*)
  };

private:
  enum {
    /// The number of free lists we want to be sure to support
    /// inline.  This is just enough that availability attributes
    /// don't surpass it.  It's actually very unlikely we'll see an
    /// attribute that needs more than that; on x86-64 you'd need 10
    /// expression arguments, and on i386 you'd need 19.
    InlineFreeListsCapacity =
      1 + (AvailabilityAllocSize - sizeof(AttributeList)) / sizeof(void*)
  };

  llvm::BumpPtrAllocator Alloc;

  /// Free lists.  The index is determined by the following formula:
  ///   (size - sizeof(AttributeList)) / sizeof(void*)
  SmallVector<AttributeList*, InlineFreeListsCapacity> FreeLists;

  // The following are the private interface used by AttributePool.
  friend class AttributePool;

  /// Allocate an attribute of the given size.
  void *allocate(size_t size);

  /// Reclaim all the attributes in the given pool chain, which is
  /// non-empty.  Note that the current implementation is safe
  /// against reclaiming things which were not actually allocated
  /// with the allocator, although of course it's important to make
  /// sure that their allocator lives at least as long as this one.
  void reclaimPool(AttributeList *head);

public:
  AttributeFactory();
  ~AttributeFactory();
};

class AttributePool {
  AttributeFactory &Factory;
  AttributeList *Head;

  void *allocate(size_t size) {
    return Factory.allocate(size);
  }

  AttributeList *add(AttributeList *attr) {
    // We don't care about the order of the pool.
    attr->NextInPool = Head;
    Head = attr;
    return attr;
  }

  void takePool(AttributeList *pool);

public:
  /// Create a new pool for a factory.
  AttributePool(AttributeFactory &factory) : Factory(factory), Head(nullptr) {}

  /// Move the given pool's allocations to this pool.
  AttributePool(AttributePool &pool) : Factory(pool.Factory), Head(pool.Head) {
    pool.Head = nullptr;
  }

  AttributeFactory &getFactory() const { return Factory; }

  void clear() {
    if (Head) {
      Factory.reclaimPool(Head);
      Head = nullptr;
    }
  }

  /// Take the given pool's allocations and add them to this pool.
  void takeAllFrom(AttributePool &pool) {
    if (pool.Head) {
      takePool(pool.Head);
      pool.Head = nullptr;
    }
  }

  ~AttributePool() {
    if (Head) Factory.reclaimPool(Head);
  }

  AttributeList *create(IdentifierInfo *attrName, SourceRange attrRange,
                        IdentifierInfo *scopeName, SourceLocation scopeLoc,
                        ArgsUnion *args, unsigned numArgs,
                        AttributeList::Syntax syntax,
                        SourceLocation ellipsisLoc = SourceLocation()) {
    void *memory = allocate(sizeof(AttributeList)
                            + numArgs * sizeof(ArgsUnion));
    return add(new (memory) AttributeList(attrName, attrRange,
                                          scopeName, scopeLoc,
                                          args, numArgs, syntax,
                                          ellipsisLoc));
  }

  AttributeList *create(IdentifierInfo *attrName, SourceRange attrRange,
                        IdentifierInfo *scopeName, SourceLocation scopeLoc,
                        IdentifierLoc *Param,
                        const AvailabilityChange &introduced,
                        const AvailabilityChange &deprecated,
                        const AvailabilityChange &obsoleted,
                        SourceLocation unavailable,
                        const Expr *MessageExpr,
                        AttributeList::Syntax syntax) {
    void *memory = allocate(AttributeFactory::AvailabilityAllocSize);
    return add(new (memory) AttributeList(attrName, attrRange,
                                          scopeName, scopeLoc,
                                          Param, introduced, deprecated,
                                          obsoleted, unavailable, MessageExpr,
                                          syntax));
  }

  AttributeList *create(IdentifierInfo *attrName, SourceRange attrRange,
                        IdentifierInfo *scopeName, SourceLocation scopeLoc,
                        IdentifierLoc *Param1,
                        IdentifierLoc *Param2,
                        IdentifierLoc *Param3,
                        AttributeList::Syntax syntax) {
    size_t size = sizeof(AttributeList) + 3 * sizeof(ArgsUnion);
    void *memory = allocate(size);
    return add(new (memory) AttributeList(attrName, attrRange,
                                          scopeName, scopeLoc,
                                          Param1, Param2, Param3,
                                          syntax));
  }

  AttributeList *createTypeTagForDatatype(
                    IdentifierInfo *attrName, SourceRange attrRange,
                    IdentifierInfo *scopeName, SourceLocation scopeLoc,
                    IdentifierLoc *argumentKind, ParsedType matchingCType,
                    bool layoutCompatible, bool mustBeNull,
                    AttributeList::Syntax syntax) {
    void *memory = allocate(AttributeFactory::TypeTagForDatatypeAllocSize);
    return add(new (memory) AttributeList(attrName, attrRange,
                                          scopeName, scopeLoc,
                                          argumentKind, matchingCType,
                                          layoutCompatible, mustBeNull,
                                          syntax));
  }

  AttributeList *createTypeAttribute(
                    IdentifierInfo *attrName, SourceRange attrRange,
                    IdentifierInfo *scopeName, SourceLocation scopeLoc,
                    ParsedType typeArg, AttributeList::Syntax syntaxUsed) {
    void *memory = allocate(sizeof(AttributeList) + sizeof(void *));
    return add(new (memory) AttributeList(attrName, attrRange,
                                          scopeName, scopeLoc,
                                          typeArg, syntaxUsed));
  }

  AttributeList *createPropertyAttribute(
                    IdentifierInfo *attrName, SourceRange attrRange,
                    IdentifierInfo *scopeName, SourceLocation scopeLoc,
                    IdentifierInfo *getterId, IdentifierInfo *setterId,
                    AttributeList::Syntax syntaxUsed) {
    void *memory = allocate(AttributeFactory::PropertyAllocSize);
    return add(new (memory) AttributeList(attrName, attrRange,
                                          scopeName, scopeLoc,
                                          getterId, setterId,
                                          syntaxUsed));
  }
};

/// ParsedAttributes - A collection of parsed attributes.  Currently
/// we don't differentiate between the various attribute syntaxes,
/// which is basically silly.
///
/// Right now this is a very lightweight container, but the expectation
/// is that this will become significantly more serious.
class ParsedAttributes {
public:
  ParsedAttributes(AttributeFactory &factory)
    : pool(factory), list(nullptr) {
  }

  ParsedAttributes(const ParsedAttributes &) = delete;

  AttributePool &getPool() const { return pool; }

  bool empty() const { return list == nullptr; }

  void add(AttributeList *newAttr) {
    assert(newAttr);
    assert(newAttr->getNext() == nullptr);
    newAttr->setNext(list);
    list = newAttr;
  }

  void addAll(AttributeList *newList) {
    if (!newList) return;

    AttributeList *lastInNewList = newList;
    while (AttributeList *next = lastInNewList->getNext())
      lastInNewList = next;

    lastInNewList->setNext(list);
    list = newList;
  }

  void set(AttributeList *newList) {
    list = newList;
  }

  void takeAllFrom(ParsedAttributes &attrs) {
    addAll(attrs.list);
    attrs.list = nullptr;
    pool.takeAllFrom(attrs.pool);
  }

  void clear() { list = nullptr; pool.clear(); }
  AttributeList *getList() const { return list; }

  /// Returns a reference to the attribute list.  Try not to introduce
  /// dependencies on this method, it may not be long-lived.
  AttributeList *&getListRef() { return list; }

  /// Add attribute with expression arguments.
  AttributeList *addNew(IdentifierInfo *attrName, SourceRange attrRange,
                        IdentifierInfo *scopeName, SourceLocation scopeLoc,
                        ArgsUnion *args, unsigned numArgs,
                        AttributeList::Syntax syntax,
                        SourceLocation ellipsisLoc = SourceLocation()) {
    AttributeList *attr =
      pool.create(attrName, attrRange, scopeName, scopeLoc, args, numArgs,
                  syntax, ellipsisLoc);
    add(attr);
    return attr;
  }

  /// Add availability attribute.
  AttributeList *addNew(IdentifierInfo *attrName, SourceRange attrRange,
                        IdentifierInfo *scopeName, SourceLocation scopeLoc,
                        IdentifierLoc *Param,
                        const AvailabilityChange &introduced,
                        const AvailabilityChange &deprecated,
                        const AvailabilityChange &obsoleted,
                        SourceLocation unavailable,
                        const Expr *MessageExpr,
                        AttributeList::Syntax syntax) {
    AttributeList *attr =
      pool.create(attrName, attrRange, scopeName, scopeLoc, Param, introduced,
                  deprecated, obsoleted, unavailable, MessageExpr, syntax);
    add(attr);
    return attr;
  }

  /// Add objc_bridge_related attribute.
  AttributeList *addNew(IdentifierInfo *attrName, SourceRange attrRange,
                        IdentifierInfo *scopeName, SourceLocation scopeLoc,
                        IdentifierLoc *Param1,
                        IdentifierLoc *Param2,
                        IdentifierLoc *Param3,
                        AttributeList::Syntax syntax) {
    AttributeList *attr =
      pool.create(attrName, attrRange, scopeName, scopeLoc,
                  Param1, Param2, Param3, syntax);
    add(attr);
    return attr;
  }

  /// Add type_tag_for_datatype attribute.
  AttributeList *addNewTypeTagForDatatype(
                        IdentifierInfo *attrName, SourceRange attrRange,
                        IdentifierInfo *scopeName, SourceLocation scopeLoc,
                        IdentifierLoc *argumentKind, ParsedType matchingCType,
                        bool layoutCompatible, bool mustBeNull,
                        AttributeList::Syntax syntax) {
    AttributeList *attr =
      pool.createTypeTagForDatatype(attrName, attrRange,
                                    scopeName, scopeLoc,
                                    argumentKind, matchingCType,
                                    layoutCompatible, mustBeNull, syntax);
    add(attr);
    return attr;
  }

  /// Add an attribute with a single type argument.
  AttributeList *
  addNewTypeAttr(IdentifierInfo *attrName, SourceRange attrRange,
                 IdentifierInfo *scopeName, SourceLocation scopeLoc,
                 ParsedType typeArg, AttributeList::Syntax syntaxUsed) {
    AttributeList *attr =
        pool.createTypeAttribute(attrName, attrRange, scopeName, scopeLoc,
                                 typeArg, syntaxUsed);
    add(attr);
    return attr;
  }

  /// Add microsoft __delspec(property) attribute.
  AttributeList *
  addNewPropertyAttr(IdentifierInfo *attrName, SourceRange attrRange,
                 IdentifierInfo *scopeName, SourceLocation scopeLoc,
                 IdentifierInfo *getterId, IdentifierInfo *setterId,
                 AttributeList::Syntax syntaxUsed) {
    AttributeList *attr =
        pool.createPropertyAttribute(attrName, attrRange, scopeName, scopeLoc,
                                     getterId, setterId, syntaxUsed);
    add(attr);
    return attr;
  }

private:
  mutable AttributePool pool;
  AttributeList *list;
};

/// These constants match the enumerated choices of
/// err_attribute_argument_n_type and err_attribute_argument_type.
enum AttributeArgumentNType {
  AANT_ArgumentIntOrBool,
  AANT_ArgumentIntegerConstant,
  AANT_ArgumentString,
  AANT_ArgumentIdentifier
};

/// These constants match the enumerated choices of
/// warn_attribute_wrong_decl_type and err_attribute_wrong_decl_type.
enum AttributeDeclKind {
  ExpectedFunction,
  ExpectedUnion,
  ExpectedVariableOrFunction,
  ExpectedFunctionOrMethod,
  ExpectedParameter,
  ExpectedFunctionMethodOrBlock,
  ExpectedFunctionMethodOrClass,
  ExpectedFunctionMethodOrParameter,
  ExpectedClass,
  ExpectedEnum,
  ExpectedVariable,
  ExpectedMethod,
  ExpectedVariableFunctionOrLabel,
  ExpectedFieldOrGlobalVar,
  ExpectedStruct,
  ExpectedVariableOrTypedef,
  ExpectedTLSVar,
  ExpectedVariableOrField,
  ExpectedVariableFieldOrTag,
  ExpectedTypeOrNamespace,
  ExpectedObjectiveCInterface,
  ExpectedMethodOrProperty,
  ExpectedStructOrUnion,
  ExpectedStructOrUnionOrClass,
  ExpectedType,
  ExpectedObjCInstanceMethod,
  ExpectedObjCInterfaceDeclInitMethod,
  ExpectedFunctionVariableOrClass,
  ExpectedObjectiveCProtocol,
  ExpectedFunctionGlobalVarMethodOrProperty,
  ExpectedStructOrUnionOrTypedef,
  ExpectedStructOrTypedef,
  ExpectedObjectiveCInterfaceOrProtocol,
  ExpectedKernelFunction,
  // SPIRV Change Begins
  ExpectedField,
  ExpectedScalarGlobalVar,
  ExpectedStructGlobalVar,
  ExpectedGlobalVarOrCTBuffer,
  ExpectedTextureOrSamplerState,
  ExpectedRWTextureOrBuffer,
  ExpectedCounterStructuredBuffer,
  ExpectedSubpassInput,
  ExpectedCTBuffer,
  // SPIRV Change Ends
  // HLSL Change Begins - add attribute decl combinations
  ExpectedVariableOrParam,
  ExpectedFunctionOrParamOrField,
  ExpectedFunctionOrVariableOrParamOrFieldOrType
  // HLSL Change Ends
};

}  // end namespace clang

#endif
