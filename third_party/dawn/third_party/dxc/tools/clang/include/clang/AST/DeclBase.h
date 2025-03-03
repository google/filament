//===-- DeclBase.h - Base Classes for representing declarations -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Decl and DeclContext interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_DECLBASE_H
#define LLVM_CLANG_AST_DECLBASE_H

#include "clang/AST/AttrIterator.h"
#include "clang/AST/DeclarationName.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/iterator.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/PrettyStackTrace.h"

namespace clang {
class ASTMutationListener;
class BlockDecl;
class CXXRecordDecl;
class CompoundStmt;
class DeclContext;
class DeclarationName;
class DependentDiagnostic;
class EnumDecl;
class FunctionDecl;
class FunctionType;
enum Linkage : unsigned char;
class LinkageComputer;
class LinkageSpecDecl;
class Module;
class NamedDecl;
class NamespaceDecl;
class ObjCCategoryDecl;
class ObjCCategoryImplDecl;
class ObjCContainerDecl;
class ObjCImplDecl;
class ObjCImplementationDecl;
class ObjCInterfaceDecl;
class ObjCMethodDecl;
class ObjCProtocolDecl;
struct PrintingPolicy;
class RecordDecl;
class Stmt;
class StoredDeclsMap;
class TranslationUnitDecl;
class UsingDirectiveDecl;
}

namespace clang {

  /// \brief Captures the result of checking the availability of a
  /// declaration.
  enum AvailabilityResult {
    AR_Available = 0,
    AR_NotYetIntroduced,
    AR_Deprecated,
    AR_Unavailable
  };

/// Decl - This represents one declaration (or definition), e.g. a variable,
/// typedef, function, struct, etc.
///
class Decl {
public:
  /// \brief Lists the kind of concrete classes of Decl.
  enum Kind {
#define DECL(DERIVED, BASE) DERIVED,
#define ABSTRACT_DECL(DECL)
#define DECL_RANGE(BASE, START, END) \
        first##BASE = START, last##BASE = END,
#define LAST_DECL_RANGE(BASE, START, END) \
        first##BASE = START, last##BASE = END
#include "clang/AST/DeclNodes.inc"
  };

  /// \brief A placeholder type used to construct an empty shell of a
  /// decl-derived type that will be filled in later (e.g., by some
  /// deserialization method).
  struct EmptyShell { };

  /// IdentifierNamespace - The different namespaces in which
  /// declarations may appear.  According to C99 6.2.3, there are
  /// four namespaces, labels, tags, members and ordinary
  /// identifiers.  C++ describes lookup completely differently:
  /// certain lookups merely "ignore" certain kinds of declarations,
  /// usually based on whether the declaration is of a type, etc.
  ///
  /// These are meant as bitmasks, so that searches in
  /// C++ can look into the "tag" namespace during ordinary lookup.
  ///
  /// Decl currently provides 15 bits of IDNS bits.
  enum IdentifierNamespace {
    /// Labels, declared with 'x:' and referenced with 'goto x'.
    IDNS_Label               = 0x0001,

    /// Tags, declared with 'struct foo;' and referenced with
    /// 'struct foo'.  All tags are also types.  This is what
    /// elaborated-type-specifiers look for in C.
    IDNS_Tag                 = 0x0002,

    /// Types, declared with 'struct foo', typedefs, etc.
    /// This is what elaborated-type-specifiers look for in C++,
    /// but note that it's ill-formed to find a non-tag.
    IDNS_Type                = 0x0004,

    /// Members, declared with object declarations within tag
    /// definitions.  In C, these can only be found by "qualified"
    /// lookup in member expressions.  In C++, they're found by
    /// normal lookup.
    IDNS_Member              = 0x0008,

    /// Namespaces, declared with 'namespace foo {}'.
    /// Lookup for nested-name-specifiers find these.
    IDNS_Namespace           = 0x0010,

    /// Ordinary names.  In C, everything that's not a label, tag,
    /// or member ends up here.
    IDNS_Ordinary            = 0x0020,

    /// Objective C \@protocol.
    IDNS_ObjCProtocol        = 0x0040,

    /// This declaration is a friend function.  A friend function
    /// declaration is always in this namespace but may also be in
    /// IDNS_Ordinary if it was previously declared.
    IDNS_OrdinaryFriend      = 0x0080,

    /// This declaration is a friend class.  A friend class
    /// declaration is always in this namespace but may also be in
    /// IDNS_Tag|IDNS_Type if it was previously declared.
    IDNS_TagFriend           = 0x0100,

    /// This declaration is a using declaration.  A using declaration
    /// *introduces* a number of other declarations into the current
    /// scope, and those declarations use the IDNS of their targets,
    /// but the actual using declarations go in this namespace.
    IDNS_Using               = 0x0200,

    /// This declaration is a C++ operator declared in a non-class
    /// context.  All such operators are also in IDNS_Ordinary.
    /// C++ lexical operator lookup looks for these.
    IDNS_NonMemberOperator   = 0x0400,

    /// This declaration is a function-local extern declaration of a
    /// variable or function. This may also be IDNS_Ordinary if it
    /// has been declared outside any function.
    IDNS_LocalExtern         = 0x0800
  };

  /// ObjCDeclQualifier - 'Qualifiers' written next to the return and
  /// parameter types in method declarations.  Other than remembering
  /// them and mangling them into the method's signature string, these
  /// are ignored by the compiler; they are consumed by certain
  /// remote-messaging frameworks.
  ///
  /// in, inout, and out are mutually exclusive and apply only to
  /// method parameters.  bycopy and byref are mutually exclusive and
  /// apply only to method parameters (?).  oneway applies only to
  /// results.  All of these expect their corresponding parameter to
  /// have a particular type.  None of this is currently enforced by
  /// clang.
  ///
  /// This should be kept in sync with ObjCDeclSpec::ObjCDeclQualifier.
  enum ObjCDeclQualifier {
    OBJC_TQ_None = 0x0,
    OBJC_TQ_In = 0x1,
    OBJC_TQ_Inout = 0x2,
    OBJC_TQ_Out = 0x4,
    OBJC_TQ_Bycopy = 0x8,
    OBJC_TQ_Byref = 0x10,
    OBJC_TQ_Oneway = 0x20,

    /// The nullability qualifier is set when the nullability of the
    /// result or parameter was expressed via a context-sensitive
    /// keyword.
    OBJC_TQ_CSNullability = 0x40
  };

protected:
  // Enumeration values used in the bits stored in NextInContextAndBits.
  enum {
    /// \brief Whether this declaration is a top-level declaration (function,
    /// global variable, etc.) that is lexically inside an objc container
    /// definition.
    TopLevelDeclInObjCContainerFlag = 0x01,
    
    /// \brief Whether this declaration is private to the module in which it was
    /// defined.
    ModulePrivateFlag = 0x02
  };
  
  /// \brief The next declaration within the same lexical
  /// DeclContext. These pointers form the linked list that is
  /// traversed via DeclContext's decls_begin()/decls_end().
  ///
  /// The extra two bits are used for the TopLevelDeclInObjCContainer and
  /// ModulePrivate bits.
  llvm::PointerIntPair<Decl *, 2, unsigned> NextInContextAndBits;

private:
  friend class DeclContext;

  struct MultipleDC {
    DeclContext *SemanticDC;
    DeclContext *LexicalDC;
  };


  /// DeclCtx - Holds either a DeclContext* or a MultipleDC*.
  /// For declarations that don't contain C++ scope specifiers, it contains
  /// the DeclContext where the Decl was declared.
  /// For declarations with C++ scope specifiers, it contains a MultipleDC*
  /// with the context where it semantically belongs (SemanticDC) and the
  /// context where it was lexically declared (LexicalDC).
  /// e.g.:
  ///
  ///   namespace A {
  ///      void f(); // SemanticDC == LexicalDC == 'namespace A'
  ///   }
  ///   void A::f(); // SemanticDC == namespace 'A'
  ///                // LexicalDC == global namespace
  llvm::PointerUnion<DeclContext*, MultipleDC*> DeclCtx;

  inline bool isInSemaDC() const    { return DeclCtx.is<DeclContext*>(); }
  inline bool isOutOfSemaDC() const { return DeclCtx.is<MultipleDC*>(); }
  inline MultipleDC *getMultipleDC() const {
    return DeclCtx.get<MultipleDC*>();
  }
  inline DeclContext *getSemanticDC() const {
    return DeclCtx.get<DeclContext*>();
  }

  /// Loc - The location of this decl.
  SourceLocation Loc;

  /// DeclKind - This indicates which class this is.
  unsigned DeclKind : 8;

  /// InvalidDecl - This indicates a semantic error occurred.
  unsigned InvalidDecl :  1;

  /// HasAttrs - This indicates whether the decl has attributes or not.
  unsigned HasAttrs : 1;

  /// Implicit - Whether this declaration was implicitly generated by
  /// the implementation rather than explicitly written by the user.
  unsigned Implicit : 1;

  /// \brief Whether this declaration was "used", meaning that a definition is
  /// required.
  unsigned Used : 1;

  /// \brief Whether this declaration was "referenced".
  /// The difference with 'Used' is whether the reference appears in a
  /// evaluated context or not, e.g. functions used in uninstantiated templates
  /// are regarded as "referenced" but not "used".
  unsigned Referenced : 1;

  /// \brief Whether statistic collection is enabled.
  static bool StatisticsEnabled;

protected:
  /// Access - Used by C++ decls for the access specifier.
  // NOTE: VC++ treats enums as signed, avoid using the AccessSpecifier enum
  unsigned Access : 2;
  friend class CXXClassMemberWrapper;

  /// \brief Whether this declaration was loaded from an AST file.
  unsigned FromASTFile : 1;

  /// \brief Whether this declaration is hidden from normal name lookup, e.g.,
  /// because it is was loaded from an AST file is either module-private or
  /// because its submodule has not been made visible.
  unsigned Hidden : 1;
  
  /// IdentifierNamespace - This specifies what IDNS_* namespace this lives in.
  unsigned IdentifierNamespace : 12;

  /// \brief If 0, we have not computed the linkage of this declaration.
  /// Otherwise, it is the linkage + 1.
  mutable unsigned CacheValidAndLinkage : 3;

  friend class ASTDeclWriter;
  friend class ASTDeclReader;
  friend class ASTReader;
  friend class LinkageComputer;

  template<typename decl_type> friend class Redeclarable;

  /// \brief Allocate memory for a deserialized declaration.
  ///
  /// This routine must be used to allocate memory for any declaration that is
  /// deserialized from a module file.
  ///
  /// \param Size The size of the allocated object.
  /// \param Ctx The context in which we will allocate memory.
  /// \param ID The global ID of the deserialized declaration.
  /// \param Extra The amount of extra space to allocate after the object.
  void *operator new(std::size_t Size, const ASTContext &Ctx, unsigned ID,
                     std::size_t Extra = 0);

  /// \brief Allocate memory for a non-deserialized declaration.
  void *operator new(std::size_t Size, const ASTContext &Ctx,
                     DeclContext *Parent, std::size_t Extra = 0);

private:
  bool AccessDeclContextSanity() const;

protected:

  Decl(Kind DK, DeclContext *DC, SourceLocation L)
    : NextInContextAndBits(), DeclCtx(DC),
      Loc(L), DeclKind(DK), InvalidDecl(0),
      HasAttrs(false), Implicit(false), Used(false), Referenced(false),
      Access(AS_none), FromASTFile(0), Hidden(DC && cast<Decl>(DC)->Hidden),
      IdentifierNamespace(getIdentifierNamespaceForKind(DK)),
      CacheValidAndLinkage(0)
  {
    if (StatisticsEnabled) add(DK);
  }

  Decl(Kind DK, EmptyShell Empty)
    : NextInContextAndBits(), DeclKind(DK), InvalidDecl(0),
      HasAttrs(false), Implicit(false), Used(false), Referenced(false),
      Access(AS_none), FromASTFile(0), Hidden(0),
      IdentifierNamespace(getIdentifierNamespaceForKind(DK)),
      CacheValidAndLinkage(0)
  {
    if (StatisticsEnabled) add(DK);
  }

  virtual ~Decl();

  /// \brief Update a potentially out-of-date declaration.
  void updateOutOfDate(IdentifierInfo &II) const;

  Linkage getCachedLinkage() const {
    return Linkage(CacheValidAndLinkage - 1);
  }

  void setCachedLinkage(Linkage L) const {
    CacheValidAndLinkage = L + 1;
  }

  bool hasCachedLinkage() const {
    return CacheValidAndLinkage;
  }

public:

  /// \brief Source range that this declaration covers.
  virtual SourceRange getSourceRange() const LLVM_READONLY {
    return SourceRange(getLocation(), getLocation());
  }
  SourceLocation getLocStart() const LLVM_READONLY {
    return getSourceRange().getBegin();
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getSourceRange().getEnd();
  }

  SourceLocation getLocation() const { return Loc; }
  void setLocation(SourceLocation L) { Loc = L; }

  Kind getKind() const { return static_cast<Kind>(DeclKind); }
  const char *getDeclKindName() const;

  Decl *getNextDeclInContext() { return NextInContextAndBits.getPointer(); }
  const Decl *getNextDeclInContext() const {return NextInContextAndBits.getPointer();}

  DeclContext *getDeclContext() {
    if (isInSemaDC())
      return getSemanticDC();
    return getMultipleDC()->SemanticDC;
  }
  const DeclContext *getDeclContext() const {
    return const_cast<Decl*>(this)->getDeclContext();
  }

  /// Find the innermost non-closure ancestor of this declaration,
  /// walking up through blocks, lambdas, etc.  If that ancestor is
  /// not a code context (!isFunctionOrMethod()), returns null.
  ///
  /// A declaration may be its own non-closure context.
  Decl *getNonClosureContext();
  const Decl *getNonClosureContext() const {
    return const_cast<Decl*>(this)->getNonClosureContext();
  }

  TranslationUnitDecl *getTranslationUnitDecl();
  const TranslationUnitDecl *getTranslationUnitDecl() const {
    return const_cast<Decl*>(this)->getTranslationUnitDecl();
  }

  bool isInAnonymousNamespace() const;

  bool isInStdNamespace() const;

  ASTContext &getASTContext() const LLVM_READONLY;

  void setAccess(AccessSpecifier AS) {
    Access = AS;
    assert(AccessDeclContextSanity());
  }

  AccessSpecifier getAccess() const {
    assert(AccessDeclContextSanity());
    return AccessSpecifier(Access);
  }

  /// \brief Retrieve the access specifier for this declaration, even though
  /// it may not yet have been properly set.
  AccessSpecifier getAccessUnsafe() const {
    return AccessSpecifier(Access);
  }

  bool hasAttrs() const { return HasAttrs; }
  void setAttrs(const AttrVec& Attrs) {
    return setAttrsImpl(Attrs, getASTContext());
  }
  AttrVec &getAttrs() {
    return const_cast<AttrVec&>(const_cast<const Decl*>(this)->getAttrs());
  }
  const AttrVec &getAttrs() const;
  void dropAttrs();

  void addAttr(Attr *A) {
    if (hasAttrs())
      getAttrs().push_back(A);
    else
      setAttrs(AttrVec(1, A));
  }

  typedef AttrVec::const_iterator attr_iterator;
  typedef llvm::iterator_range<attr_iterator> attr_range;

  attr_range attrs() const {
    return attr_range(attr_begin(), attr_end());
  }

  attr_iterator attr_begin() const {
    return hasAttrs() ? getAttrs().begin() : nullptr;
  }
  attr_iterator attr_end() const {
    return hasAttrs() ? getAttrs().end() : nullptr;
  }

  template <typename T>
  void dropAttr() {
    if (!HasAttrs) return;

    AttrVec &Vec = getAttrs();
    Vec.erase(std::remove_if(Vec.begin(), Vec.end(), isa<T, Attr*>), Vec.end());

    if (Vec.empty())
      HasAttrs = false;
  }

  template <typename T>
  llvm::iterator_range<specific_attr_iterator<T>> specific_attrs() const {
    return llvm::iterator_range<specific_attr_iterator<T>>(
        specific_attr_begin<T>(), specific_attr_end<T>());
  }

  template <typename T>
  specific_attr_iterator<T> specific_attr_begin() const {
    return specific_attr_iterator<T>(attr_begin());
  }
  template <typename T>
  specific_attr_iterator<T> specific_attr_end() const {
    return specific_attr_iterator<T>(attr_end());
  }

  template<typename T> T *getAttr() const {
    return hasAttrs() ? getSpecificAttr<T>(getAttrs()) : nullptr;
  }
  template<typename T> bool hasAttr() const {
    return hasAttrs() && hasSpecificAttr<T>(getAttrs());
  }

  /// getMaxAlignment - return the maximum alignment specified by attributes
  /// on this decl, 0 if there are none.
  unsigned getMaxAlignment() const;

  /// setInvalidDecl - Indicates the Decl had a semantic error. This
  /// allows for graceful error recovery.
  void setInvalidDecl(bool Invalid = true);
  bool isInvalidDecl() const { return (bool) InvalidDecl; }

  /// isImplicit - Indicates whether the declaration was implicitly
  /// generated by the implementation. If false, this declaration
  /// was written explicitly in the source code.
  bool isImplicit() const { return Implicit; }
  void setImplicit(bool I = true) { Implicit = I; }

  /// \brief Whether this declaration was used, meaning that a definition
  /// is required.
  ///
  /// \param CheckUsedAttr When true, also consider the "used" attribute
  /// (in addition to the "used" bit set by \c setUsed()) when determining
  /// whether the function is used.
  bool isUsed(bool CheckUsedAttr = true) const;

  /// \brief Set whether the declaration is used, in the sense of odr-use.
  ///
  /// This should only be used immediately after creating a declaration.
  void setIsUsed() { Used = true; }

  /// \brief Mark the declaration used, in the sense of odr-use.
  ///
  /// This notifies any mutation listeners in addition to setting a bit
  /// indicating the declaration is used.
  void markUsed(ASTContext &C);

  /// \brief Whether any declaration of this entity was referenced.
  bool isReferenced() const;

  /// \brief Whether this declaration was referenced. This should not be relied
  /// upon for anything other than debugging.
  bool isThisDeclarationReferenced() const { return Referenced; }

  void setReferenced(bool R = true) { Referenced = R; }

  /// \brief Whether this declaration is a top-level declaration (function,
  /// global variable, etc.) that is lexically inside an objc container
  /// definition.
  bool isTopLevelDeclInObjCContainer() const {
    return NextInContextAndBits.getInt() & TopLevelDeclInObjCContainerFlag;
  }

  void setTopLevelDeclInObjCContainer(bool V = true) {
    unsigned Bits = NextInContextAndBits.getInt();
    if (V)
      Bits |= TopLevelDeclInObjCContainerFlag;
    else
      Bits &= ~TopLevelDeclInObjCContainerFlag;
    NextInContextAndBits.setInt(Bits);
  }

  /// \brief Whether this declaration was marked as being private to the
  /// module in which it was defined.
  bool isModulePrivate() const {
    return NextInContextAndBits.getInt() & ModulePrivateFlag;
  }

protected:
  /// \brief Specify whether this declaration was marked as being private
  /// to the module in which it was defined.
  void setModulePrivate(bool MP = true) {
    unsigned Bits = NextInContextAndBits.getInt();
    if (MP)
      Bits |= ModulePrivateFlag;
    else
      Bits &= ~ModulePrivateFlag;
    NextInContextAndBits.setInt(Bits);
  }

  /// \brief Set the owning module ID.
  void setOwningModuleID(unsigned ID) {
    assert(isFromASTFile() && "Only works on a deserialized declaration");
    *((unsigned*)this - 2) = ID;
  }
  
public:
  
  /// \brief Determine the availability of the given declaration.
  ///
  /// This routine will determine the most restrictive availability of
  /// the given declaration (e.g., preferring 'unavailable' to
  /// 'deprecated').
  ///
  /// \param Message If non-NULL and the result is not \c
  /// AR_Available, will be set to a (possibly empty) message
  /// describing why the declaration has not been introduced, is
  /// deprecated, or is unavailable.
  AvailabilityResult getAvailability(std::string *Message = nullptr) const;

  /// \brief Determine whether this declaration is marked 'deprecated'.
  ///
  /// \param Message If non-NULL and the declaration is deprecated,
  /// this will be set to the message describing why the declaration
  /// was deprecated (which may be empty).
  bool isDeprecated(std::string *Message = nullptr) const {
    return getAvailability(Message) == AR_Deprecated;
  }

  /// \brief Determine whether this declaration is marked 'unavailable'.
  ///
  /// \param Message If non-NULL and the declaration is unavailable,
  /// this will be set to the message describing why the declaration
  /// was made unavailable (which may be empty).
  bool isUnavailable(std::string *Message = nullptr) const {
    return getAvailability(Message) == AR_Unavailable;
  }

  /// \brief Determine whether this is a weak-imported symbol.
  ///
  /// Weak-imported symbols are typically marked with the
  /// 'weak_import' attribute, but may also be marked with an
  /// 'availability' attribute where we're targing a platform prior to
  /// the introduction of this feature.
  bool isWeakImported() const;

  /// \brief Determines whether this symbol can be weak-imported,
  /// e.g., whether it would be well-formed to add the weak_import
  /// attribute.
  ///
  /// \param IsDefinition Set to \c true to indicate that this
  /// declaration cannot be weak-imported because it has a definition.
  bool canBeWeakImported(bool &IsDefinition) const;

  /// \brief Determine whether this declaration came from an AST file (such as
  /// a precompiled header or module) rather than having been parsed.
  bool isFromASTFile() const { return FromASTFile; }

  /// \brief Retrieve the global declaration ID associated with this 
  /// declaration, which specifies where in the 
  unsigned getGlobalID() const { 
    if (isFromASTFile())
      return *((const unsigned*)this - 1);
    return 0;
  }
  
  /// \brief Retrieve the global ID of the module that owns this particular
  /// declaration.
  unsigned getOwningModuleID() const {
    if (isFromASTFile())
      return *((const unsigned*)this - 2);
    
    return 0;
  }

private:
  Module *getOwningModuleSlow() const;
protected:
  bool hasLocalOwningModuleStorage() const;

public:
  /// \brief Get the imported owning module, if this decl is from an imported
  /// (non-local) module.
  Module *getImportedOwningModule() const {
    if (!isFromASTFile())
      return nullptr;

    return getOwningModuleSlow();
  }

  /// \brief Get the local owning module, if known. Returns nullptr if owner is
  /// not yet known or declaration is not from a module.
  Module *getLocalOwningModule() const {
    if (isFromASTFile() || !Hidden)
      return nullptr;
    return reinterpret_cast<Module *const *>(this)[-1];
  }
  void setLocalOwningModule(Module *M) {
    assert(!isFromASTFile() && Hidden && hasLocalOwningModuleStorage() &&
           "should not have a cached owning module");
    reinterpret_cast<Module **>(this)[-1] = M;
  }

  unsigned getIdentifierNamespace() const {
    return IdentifierNamespace;
  }
  bool isInIdentifierNamespace(unsigned NS) const {
    return getIdentifierNamespace() & NS;
  }
  static unsigned getIdentifierNamespaceForKind(Kind DK);

  bool hasTagIdentifierNamespace() const {
    return isTagIdentifierNamespace(getIdentifierNamespace());
  }
  static bool isTagIdentifierNamespace(unsigned NS) {
    // TagDecls have Tag and Type set and may also have TagFriend.
    return (NS & ~IDNS_TagFriend) == (IDNS_Tag | IDNS_Type);
  }

  /// getLexicalDeclContext - The declaration context where this Decl was
  /// lexically declared (LexicalDC). May be different from
  /// getDeclContext() (SemanticDC).
  /// e.g.:
  ///
  ///   namespace A {
  ///      void f(); // SemanticDC == LexicalDC == 'namespace A'
  ///   }
  ///   void A::f(); // SemanticDC == namespace 'A'
  ///                // LexicalDC == global namespace
  DeclContext *getLexicalDeclContext() {
    if (isInSemaDC())
      return getSemanticDC();
    return getMultipleDC()->LexicalDC;
  }
  const DeclContext *getLexicalDeclContext() const {
    return const_cast<Decl*>(this)->getLexicalDeclContext();
  }

  /// Determine whether this declaration is declared out of line (outside its
  /// semantic context).
  virtual bool isOutOfLine() const;

  /// setDeclContext - Set both the semantic and lexical DeclContext
  /// to DC.
  void setDeclContext(DeclContext *DC);

  void setLexicalDeclContext(DeclContext *DC);

  /// isDefinedOutsideFunctionOrMethod - This predicate returns true if this
  /// scoped decl is defined outside the current function or method.  This is
  /// roughly global variables and functions, but also handles enums (which
  /// could be defined inside or outside a function etc).
  bool isDefinedOutsideFunctionOrMethod() const {
    return getParentFunctionOrMethod() == nullptr;
  }

  /// HLSL Change Begin - back port from llvm-project/73c6a2448f24 &
  /// f721e0582b15. Determine whether a substitution into this declaration would
  /// occur as part of a substitution into a dependent local scope. Such a
  /// substitution transitively substitutes into all constructs nested within
  /// this declaration.
  ///
  /// This recognizes non-defining declarations as well as members of local
  /// classes and lambdas:
  /// \code
  ///     template<typename T> void foo() { void bar(); }
  ///     template<typename T> void foo2() { class ABC { void bar(); }; }
  ///     template<typename T> inline int x = [](){ return 0; }();
  /// \endcode
  bool isInLocalScopeForInstantiation() const;
  /// HLSL Change End - back port from llvm-project/73c6a2448f24 & f721e0582b15.

  /// \brief If this decl is defined inside a function/method/block it returns
  /// the corresponding DeclContext, otherwise it returns null.
  const DeclContext *getParentFunctionOrMethod() const;
  DeclContext *getParentFunctionOrMethod() {
    return const_cast<DeclContext*>(
                    const_cast<const Decl*>(this)->getParentFunctionOrMethod());
  }

  /// \brief Retrieves the "canonical" declaration of the given declaration.
  virtual Decl *getCanonicalDecl() { return this; }
  const Decl *getCanonicalDecl() const {
    return const_cast<Decl*>(this)->getCanonicalDecl();
  }

  /// \brief Whether this particular Decl is a canonical one.
  bool isCanonicalDecl() const { return getCanonicalDecl() == this; }
  
protected:
  /// \brief Returns the next redeclaration or itself if this is the only decl.
  ///
  /// Decl subclasses that can be redeclared should override this method so that
  /// Decl::redecl_iterator can iterate over them.
  virtual Decl *getNextRedeclarationImpl() { return this; }

  /// \brief Implementation of getPreviousDecl(), to be overridden by any
  /// subclass that has a redeclaration chain.
  virtual Decl *getPreviousDeclImpl() { return nullptr; }

  /// \brief Implementation of getMostRecentDecl(), to be overridden by any
  /// subclass that has a redeclaration chain.
  virtual Decl *getMostRecentDeclImpl() { return this; }

public:
  /// \brief Iterates through all the redeclarations of the same decl.
  class redecl_iterator {
    /// Current - The current declaration.
    Decl *Current;
    Decl *Starter;

  public:
    typedef Decl *value_type;
    typedef const value_type &reference;
    typedef const value_type *pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef std::ptrdiff_t difference_type;

    redecl_iterator() : Current(nullptr) { }
    explicit redecl_iterator(Decl *C) : Current(C), Starter(C) { }

    reference operator*() const { return Current; }
    value_type operator->() const { return Current; }

    redecl_iterator& operator++() {
      assert(Current && "Advancing while iterator has reached end");
      // Get either previous decl or latest decl.
      Decl *Next = Current->getNextRedeclarationImpl();
      assert(Next && "Should return next redeclaration or itself, never null!");
      Current = (Next != Starter) ? Next : nullptr;
      return *this;
    }

    redecl_iterator operator++(int) {
      redecl_iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    friend bool operator==(redecl_iterator x, redecl_iterator y) {
      return x.Current == y.Current;
    }
    friend bool operator!=(redecl_iterator x, redecl_iterator y) {
      return x.Current != y.Current;
    }
  };

  typedef llvm::iterator_range<redecl_iterator> redecl_range;

  /// \brief Returns an iterator range for all the redeclarations of the same
  /// decl. It will iterate at least once (when this decl is the only one).
  redecl_range redecls() const {
    return redecl_range(redecls_begin(), redecls_end());
  }

  redecl_iterator redecls_begin() const {
    return redecl_iterator(const_cast<Decl *>(this));
  }
  redecl_iterator redecls_end() const { return redecl_iterator(); }

  /// \brief Retrieve the previous declaration that declares the same entity
  /// as this declaration, or NULL if there is no previous declaration.
  Decl *getPreviousDecl() { return getPreviousDeclImpl(); }
  
  /// \brief Retrieve the most recent declaration that declares the same entity
  /// as this declaration, or NULL if there is no previous declaration.
  const Decl *getPreviousDecl() const { 
    return const_cast<Decl *>(this)->getPreviousDeclImpl();
  }

  /// \brief True if this is the first declaration in its redeclaration chain.
  bool isFirstDecl() const {
    return getPreviousDecl() == nullptr;
  }

  /// \brief Retrieve the most recent declaration that declares the same entity
  /// as this declaration (which may be this declaration).
  Decl *getMostRecentDecl() { return getMostRecentDeclImpl(); }

  /// \brief Retrieve the most recent declaration that declares the same entity
  /// as this declaration (which may be this declaration).
  const Decl *getMostRecentDecl() const { 
    return const_cast<Decl *>(this)->getMostRecentDeclImpl();
  }

  /// getBody - If this Decl represents a declaration for a body of code,
  ///  such as a function or method definition, this method returns the
  ///  top-level Stmt* of that body.  Otherwise this method returns null.
  virtual Stmt* getBody() const { return nullptr; }

  /// \brief Returns true if this \c Decl represents a declaration for a body of
  /// code, such as a function or method definition.
  /// Note that \c hasBody can also return true if any redeclaration of this
  /// \c Decl represents a declaration for a body of code.
  virtual bool hasBody() const { return getBody() != nullptr; }

  /// getBodyRBrace - Gets the right brace of the body, if a body exists.
  /// This works whether the body is a CompoundStmt or a CXXTryStmt.
  SourceLocation getBodyRBrace() const;

  // global temp stats (until we have a per-module visitor)
  static void add(Kind k);
  static void EnableStatistics();
  static void PrintStats();

  /// isTemplateParameter - Determines whether this declaration is a
  /// template parameter.
  bool isTemplateParameter() const;

  /// isTemplateParameter - Determines whether this declaration is a
  /// template parameter pack.
  bool isTemplateParameterPack() const;

  /// \brief Whether this declaration is a parameter pack.
  bool isParameterPack() const;

  /// \brief returns true if this declaration is a template
  bool isTemplateDecl() const;

  /// \brief Whether this declaration is a function or function template.
  bool isFunctionOrFunctionTemplate() const {
    return (DeclKind >= Decl::firstFunction &&
            DeclKind <= Decl::lastFunction) ||
           DeclKind == FunctionTemplate;
  }

  /// \brief Returns the function itself, or the templated function if this is a
  /// function template.
  FunctionDecl *getAsFunction() LLVM_READONLY;

  const FunctionDecl *getAsFunction() const {
    return const_cast<Decl *>(this)->getAsFunction();
  }

  /// \brief Changes the namespace of this declaration to reflect that it's
  /// a function-local extern declaration.
  ///
  /// These declarations appear in the lexical context of the extern
  /// declaration, but in the semantic context of the enclosing namespace
  /// scope.
  void setLocalExternDecl() {
    assert((IdentifierNamespace == IDNS_Ordinary ||
            IdentifierNamespace == IDNS_OrdinaryFriend) &&
           "namespace is not ordinary");

    Decl *Prev = getPreviousDecl();
    IdentifierNamespace &= ~IDNS_Ordinary;

    IdentifierNamespace |= IDNS_LocalExtern;
    if (Prev && Prev->getIdentifierNamespace() & IDNS_Ordinary)
      IdentifierNamespace |= IDNS_Ordinary;
  }

  /// \brief Determine whether this is a block-scope declaration with linkage.
  /// This will either be a local variable declaration declared 'extern', or a
  /// local function declaration.
  bool isLocalExternDecl() {
    return IdentifierNamespace & IDNS_LocalExtern;
  }

  /// \brief Changes the namespace of this declaration to reflect that it's
  /// the object of a friend declaration.
  ///
  /// These declarations appear in the lexical context of the friending
  /// class, but in the semantic context of the actual entity.  This property
  /// applies only to a specific decl object;  other redeclarations of the
  /// same entity may not (and probably don't) share this property.
  void setObjectOfFriendDecl(bool PerformFriendInjection = false) {
    unsigned OldNS = IdentifierNamespace;
    assert((OldNS & (IDNS_Tag | IDNS_Ordinary |
                     IDNS_TagFriend | IDNS_OrdinaryFriend |
                     IDNS_LocalExtern)) &&
           "namespace includes neither ordinary nor tag");
    assert(!(OldNS & ~(IDNS_Tag | IDNS_Ordinary | IDNS_Type |
                       IDNS_TagFriend | IDNS_OrdinaryFriend |
                       IDNS_LocalExtern)) &&
           "namespace includes other than ordinary or tag");

    Decl *Prev = getPreviousDecl();
    IdentifierNamespace &= ~(IDNS_Ordinary | IDNS_Tag | IDNS_Type);

    if (OldNS & (IDNS_Tag | IDNS_TagFriend)) {
      IdentifierNamespace |= IDNS_TagFriend;
      if (PerformFriendInjection ||
          (Prev && Prev->getIdentifierNamespace() & IDNS_Tag))
        IdentifierNamespace |= IDNS_Tag | IDNS_Type;
    }

    if (OldNS & (IDNS_Ordinary | IDNS_OrdinaryFriend | IDNS_LocalExtern)) {
      IdentifierNamespace |= IDNS_OrdinaryFriend;
      if (PerformFriendInjection ||
          (Prev && Prev->getIdentifierNamespace() & IDNS_Ordinary))
        IdentifierNamespace |= IDNS_Ordinary;
    }
  }

  enum FriendObjectKind {
    FOK_None,      ///< Not a friend object.
    FOK_Declared,  ///< A friend of a previously-declared entity.
    FOK_Undeclared ///< A friend of a previously-undeclared entity.
  };

  /// \brief Determines whether this declaration is the object of a
  /// friend declaration and, if so, what kind.
  ///
  /// There is currently no direct way to find the associated FriendDecl.
  FriendObjectKind getFriendObjectKind() const {
    unsigned mask =
        (IdentifierNamespace & (IDNS_TagFriend | IDNS_OrdinaryFriend));
    if (!mask) return FOK_None;
    return (IdentifierNamespace & (IDNS_Tag | IDNS_Ordinary) ? FOK_Declared
                                                             : FOK_Undeclared);
  }

  /// Specifies that this declaration is a C++ overloaded non-member.
  void setNonMemberOperator() {
    assert(getKind() == Function || getKind() == FunctionTemplate);
    assert((IdentifierNamespace & IDNS_Ordinary) &&
           "visible non-member operators should be in ordinary namespace");
    IdentifierNamespace |= IDNS_NonMemberOperator;
  }

  static bool classofKind(Kind K) { return true; }
  static DeclContext *castToDeclContext(const Decl *);
  static Decl *castFromDeclContext(const DeclContext *);

  void print(raw_ostream &Out, unsigned Indentation = 0,
             bool PrintInstantiation = false) const;
  void print(raw_ostream &Out, const PrintingPolicy &Policy,
             unsigned Indentation = 0, bool PrintInstantiation = false) const;
  static void printGroup(Decl** Begin, unsigned NumDecls,
                         raw_ostream &Out, const PrintingPolicy &Policy,
                         unsigned Indentation = 0);
  // Debuggers don't usually respect default arguments.
  void dump() const;
  // Same as dump(), but forces color printing.
  void dumpColor() const;
  void dump(raw_ostream &Out) const;

  /// \brief Looks through the Decl's underlying type to extract a FunctionType
  /// when possible. Will return null if the type underlying the Decl does not
  /// have a FunctionType.
  const FunctionType *getFunctionType(bool BlocksToo = true) const;

private:
  void setAttrsImpl(const AttrVec& Attrs, ASTContext &Ctx);
  void setDeclContextsImpl(DeclContext *SemaDC, DeclContext *LexicalDC,
                           ASTContext &Ctx);

protected:
  ASTMutationListener *getASTMutationListener() const;
};

/// \brief Determine whether two declarations declare the same entity.
inline bool declaresSameEntity(const Decl *D1, const Decl *D2) {
  if (!D1 || !D2)
    return false;
  
  if (D1 == D2)
    return true;
  
  return D1->getCanonicalDecl() == D2->getCanonicalDecl();
}
  
/// PrettyStackTraceDecl - If a crash occurs, indicate that it happened when
/// doing something to a specific decl.
class PrettyStackTraceDecl : public llvm::PrettyStackTraceEntry {
  const Decl *TheDecl;
  SourceLocation Loc;
  SourceManager &SM;
  const char *Message;
public:
  PrettyStackTraceDecl(const Decl *theDecl, SourceLocation L,
                       SourceManager &sm, const char *Msg)
  : TheDecl(theDecl), Loc(L), SM(sm), Message(Msg) {}

  void print(raw_ostream &OS) const override;
};

/// \brief The results of name lookup within a DeclContext. This is either a
/// single result (with no stable storage) or a collection of results (with
/// stable storage provided by the lookup table).
class DeclContextLookupResult {
  typedef ArrayRef<NamedDecl *> ResultTy;
  ResultTy Result;
  // If there is only one lookup result, it would be invalidated by
  // reallocations of the name table, so store it separately.
  NamedDecl *Single;

  static NamedDecl *const SingleElementDummyList;

public:
  DeclContextLookupResult() : Result(), Single() {}
  DeclContextLookupResult(ArrayRef<NamedDecl *> Result)
      : Result(Result), Single() {}
  DeclContextLookupResult(NamedDecl *Single)
      : Result(SingleElementDummyList), Single(Single) {}

  class iterator;
  typedef llvm::iterator_adaptor_base<iterator, ResultTy::iterator,
                                      std::random_access_iterator_tag,
                                      NamedDecl *const> IteratorBase;
  class iterator : public IteratorBase {
    value_type SingleElement;

  public:
    iterator() : IteratorBase(), SingleElement() {}
    explicit iterator(pointer Pos, value_type Single = nullptr)
        : IteratorBase(Pos), SingleElement(Single) {}

    reference operator*() const {
      return SingleElement ? SingleElement : IteratorBase::operator*();
    }
  };
  typedef iterator const_iterator;
  typedef iterator::pointer pointer;
  typedef iterator::reference reference;

  iterator begin() const { return iterator(Result.begin(), Single); }
  iterator end() const { return iterator(Result.end(), Single); }

  bool empty() const { return Result.empty(); }
  pointer data() const { return Single ? &Single : Result.data(); }
  size_t size() const { return Single ? 1 : Result.size(); }
  reference front() const { return Single ? Single : Result.front(); }
  reference back() const { return Single ? Single : Result.back(); }
  reference operator[](size_t N) const { return Single ? Single : Result[N]; }

  // FIXME: Remove this from the interface
  DeclContextLookupResult slice(size_t N) const {
    DeclContextLookupResult Sliced = Result.slice(N);
    Sliced.Single = Single;
    return Sliced;
  }
};

/// DeclContext - This is used only as base class of specific decl types that
/// can act as declaration contexts. These decls are (only the top classes
/// that directly derive from DeclContext are mentioned, not their subclasses):
///
///   TranslationUnitDecl
///   NamespaceDecl
///   FunctionDecl
///   TagDecl
///   ObjCMethodDecl
///   ObjCContainerDecl
///   LinkageSpecDecl
///   BlockDecl
///
class DeclContext {
  /// DeclKind - This indicates which class this is.
  unsigned DeclKind : 8;

  /// \brief Whether this declaration context also has some external
  /// storage that contains additional declarations that are lexically
  /// part of this context.
  mutable bool ExternalLexicalStorage : 1;

  /// \brief Whether this declaration context also has some external
  /// storage that contains additional declarations that are visible
  /// in this context.
  mutable bool ExternalVisibleStorage : 1;

  /// \brief Whether this declaration context has had external visible
  /// storage added since the last lookup. In this case, \c LookupPtr's
  /// invariant may not hold and needs to be fixed before we perform
  /// another lookup.
  mutable bool NeedToReconcileExternalVisibleStorage : 1;

  /// \brief If \c true, this context may have local lexical declarations
  /// that are missing from the lookup table.
  mutable bool HasLazyLocalLexicalLookups : 1;

  /// \brief If \c true, the external source may have lexical declarations
  /// that are missing from the lookup table.
  mutable bool HasLazyExternalLexicalLookups : 1;

  /// \brief Pointer to the data structure used to lookup declarations
  /// within this context (or a DependentStoredDeclsMap if this is a
  /// dependent context). We maintain the invariant that, if the map
  /// contains an entry for a DeclarationName (and we haven't lazily
  /// omitted anything), then it contains all relevant entries for that
  /// name (modulo the hasExternalDecls() flag).
  mutable StoredDeclsMap *LookupPtr;

protected:
  /// FirstDecl - The first declaration stored within this declaration
  /// context.
  mutable Decl *FirstDecl;

  /// LastDecl - The last declaration stored within this declaration
  /// context. FIXME: We could probably cache this value somewhere
  /// outside of the DeclContext, to reduce the size of DeclContext by
  /// another pointer.
  mutable Decl *LastDecl;

  friend class ExternalASTSource;
  friend class ASTDeclReader;
  friend class ASTWriter;

  /// \brief Build up a chain of declarations.
  ///
  /// \returns the first/last pair of declarations.
  static std::pair<Decl *, Decl *>
  BuildDeclChain(ArrayRef<Decl*> Decls, bool FieldsAlreadyLoaded);

  DeclContext(Decl::Kind K)
      : DeclKind(K), ExternalLexicalStorage(false),
        ExternalVisibleStorage(false),
        NeedToReconcileExternalVisibleStorage(false),
        HasLazyLocalLexicalLookups(false), HasLazyExternalLexicalLookups(false),
        LookupPtr(nullptr), FirstDecl(nullptr), LastDecl(nullptr) {}

public:
  ~DeclContext();

  Decl::Kind getDeclKind() const {
    return static_cast<Decl::Kind>(DeclKind);
  }
  const char *getDeclKindName() const;

  /// getParent - Returns the containing DeclContext.
  DeclContext *getParent() {
    return cast<Decl>(this)->getDeclContext();
  }
  const DeclContext *getParent() const {
    return const_cast<DeclContext*>(this)->getParent();
  }

  /// getLexicalParent - Returns the containing lexical DeclContext. May be
  /// different from getParent, e.g.:
  ///
  ///   namespace A {
  ///      struct S;
  ///   }
  ///   struct A::S {}; // getParent() == namespace 'A'
  ///                   // getLexicalParent() == translation unit
  ///
  DeclContext *getLexicalParent() {
    return cast<Decl>(this)->getLexicalDeclContext();
  }
  const DeclContext *getLexicalParent() const {
    return const_cast<DeclContext*>(this)->getLexicalParent();
  }

  DeclContext *getLookupParent();

  const DeclContext *getLookupParent() const {
    return const_cast<DeclContext*>(this)->getLookupParent();
  }

  ASTContext &getParentASTContext() const {
    return cast<Decl>(this)->getASTContext();
  }

  bool isClosure() const {
    return DeclKind == Decl::Block;
  }

  bool isObjCContainer() const {
    switch (DeclKind) {
        case Decl::ObjCCategory:
        case Decl::ObjCCategoryImpl:
        case Decl::ObjCImplementation:
        case Decl::ObjCInterface:
        case Decl::ObjCProtocol:
            return true;
    }
    return false;
  }

  bool isFunctionOrMethod() const {
    switch (DeclKind) {
    case Decl::Block:
    case Decl::Captured:
    case Decl::ObjCMethod:
      return true;
    default:
      return DeclKind >= Decl::firstFunction && DeclKind <= Decl::lastFunction;
    }
  }

  /// \brief Test whether the context supports looking up names.
  bool isLookupContext() const {
    return !isFunctionOrMethod() && DeclKind != Decl::LinkageSpec;
  }

  bool isFileContext() const {
    return DeclKind == Decl::TranslationUnit || DeclKind == Decl::Namespace;
  }

  bool isTranslationUnit() const {
    return DeclKind == Decl::TranslationUnit;
  }

  bool isRecord() const {
    return DeclKind >= Decl::firstRecord && DeclKind <= Decl::lastRecord;
  }

  bool isNamespace() const {
    return DeclKind == Decl::Namespace;
  }

  bool isStdNamespace() const;

  bool isInlineNamespace() const;

  /// \brief Determines whether this context is dependent on a
  /// template parameter.
  bool isDependentContext() const;

  /// isTransparentContext - Determines whether this context is a
  /// "transparent" context, meaning that the members declared in this
  /// context are semantically declared in the nearest enclosing
  /// non-transparent (opaque) context but are lexically declared in
  /// this context. For example, consider the enumerators of an
  /// enumeration type:
  /// @code
  /// enum E {
  ///   Val1
  /// };
  /// @endcode
  /// Here, E is a transparent context, so its enumerator (Val1) will
  /// appear (semantically) that it is in the same context of E.
  /// Examples of transparent contexts include: enumerations (except for
  /// C++0x scoped enums), and C++ linkage specifications.
  bool isTransparentContext() const;

  /// \brief Determines whether this context or some of its ancestors is a
  /// linkage specification context that specifies C linkage.
  bool isExternCContext() const;

  /// \brief Determines whether this context or some of its ancestors is a
  /// linkage specification context that specifies C++ linkage.
  bool isExternCXXContext() const;

  /// \brief Determine whether this declaration context is equivalent
  /// to the declaration context DC.
  bool Equals(const DeclContext *DC) const {
    return DC && this->getPrimaryContext() == DC->getPrimaryContext();
  }

  /// \brief Determine whether this declaration context encloses the
  /// declaration context DC.
  bool Encloses(const DeclContext *DC) const;

  /// \brief Find the nearest non-closure ancestor of this context,
  /// i.e. the innermost semantic parent of this context which is not
  /// a closure.  A context may be its own non-closure ancestor.
  Decl *getNonClosureAncestor();
  const Decl *getNonClosureAncestor() const {
    return const_cast<DeclContext*>(this)->getNonClosureAncestor();
  }

  /// getPrimaryContext - There may be many different
  /// declarations of the same entity (including forward declarations
  /// of classes, multiple definitions of namespaces, etc.), each with
  /// a different set of declarations. This routine returns the
  /// "primary" DeclContext structure, which will contain the
  /// information needed to perform name lookup into this context.
  DeclContext *getPrimaryContext();
  const DeclContext *getPrimaryContext() const {
    return const_cast<DeclContext*>(this)->getPrimaryContext();
  }

  /// getRedeclContext - Retrieve the context in which an entity conflicts with
  /// other entities of the same name, or where it is a redeclaration if the
  /// two entities are compatible. This skips through transparent contexts.
  DeclContext *getRedeclContext();
  const DeclContext *getRedeclContext() const {
    return const_cast<DeclContext *>(this)->getRedeclContext();
  }

  /// \brief Retrieve the nearest enclosing namespace context.
  DeclContext *getEnclosingNamespaceContext();
  const DeclContext *getEnclosingNamespaceContext() const {
    return const_cast<DeclContext *>(this)->getEnclosingNamespaceContext();
  }

  /// \brief Retrieve the outermost lexically enclosing record context.
  RecordDecl *getOuterLexicalRecordContext();
  const RecordDecl *getOuterLexicalRecordContext() const {
    return const_cast<DeclContext *>(this)->getOuterLexicalRecordContext();
  }

  /// \brief Test if this context is part of the enclosing namespace set of
  /// the context NS, as defined in C++0x [namespace.def]p9. If either context
  /// isn't a namespace, this is equivalent to Equals().
  ///
  /// The enclosing namespace set of a namespace is the namespace and, if it is
  /// inline, its enclosing namespace, recursively.
  bool InEnclosingNamespaceSetOf(const DeclContext *NS) const;

  /// \brief Collects all of the declaration contexts that are semantically
  /// connected to this declaration context.
  ///
  /// For declaration contexts that have multiple semantically connected but
  /// syntactically distinct contexts, such as C++ namespaces, this routine 
  /// retrieves the complete set of such declaration contexts in source order.
  /// For example, given:
  ///
  /// \code
  /// namespace N {
  ///   int x;
  /// }
  /// namespace N {
  ///   int y;
  /// }
  /// \endcode
  ///
  /// The \c Contexts parameter will contain both definitions of N.
  ///
  /// \param Contexts Will be cleared and set to the set of declaration
  /// contexts that are semanticaly connected to this declaration context,
  /// in source order, including this context (which may be the only result,
  /// for non-namespace contexts).
  void collectAllContexts(SmallVectorImpl<DeclContext *> &Contexts);

  /// decl_iterator - Iterates through the declarations stored
  /// within this context.
  class decl_iterator {
    /// Current - The current declaration.
    Decl *Current;

  public:
    typedef Decl *value_type;
    typedef const value_type &reference;
    typedef const value_type *pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef std::ptrdiff_t            difference_type;

    decl_iterator() : Current(nullptr) { }
    explicit decl_iterator(Decl *C) : Current(C) { }

    reference operator*() const { return Current; }
    // This doesn't meet the iterator requirements, but it's convenient
    value_type operator->() const { return Current; }

    decl_iterator& operator++() {
      Current = Current->getNextDeclInContext();
      return *this;
    }

    decl_iterator operator++(int) {
      decl_iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    friend bool operator==(decl_iterator x, decl_iterator y) {
      return x.Current == y.Current;
    }
    friend bool operator!=(decl_iterator x, decl_iterator y) {
      return x.Current != y.Current;
    }
  };

  typedef llvm::iterator_range<decl_iterator> decl_range;

  /// decls_begin/decls_end - Iterate over the declarations stored in
  /// this context.
  decl_range decls() const { return decl_range(decls_begin(), decls_end()); }
  decl_iterator decls_begin() const;
  decl_iterator decls_end() const { return decl_iterator(); }
  bool decls_empty() const;

  /// noload_decls_begin/end - Iterate over the declarations stored in this
  /// context that are currently loaded; don't attempt to retrieve anything
  /// from an external source.
  decl_range noload_decls() const {
    return decl_range(noload_decls_begin(), noload_decls_end());
  }
  decl_iterator noload_decls_begin() const { return decl_iterator(FirstDecl); }
  decl_iterator noload_decls_end() const { return decl_iterator(); }

  /// specific_decl_iterator - Iterates over a subrange of
  /// declarations stored in a DeclContext, providing only those that
  /// are of type SpecificDecl (or a class derived from it). This
  /// iterator is used, for example, to provide iteration over just
  /// the fields within a RecordDecl (with SpecificDecl = FieldDecl).
  template<typename SpecificDecl>
  class specific_decl_iterator {
    /// Current - The current, underlying declaration iterator, which
    /// will either be NULL or will point to a declaration of
    /// type SpecificDecl.
    DeclContext::decl_iterator Current;

    /// SkipToNextDecl - Advances the current position up to the next
    /// declaration of type SpecificDecl that also meets the criteria
    /// required by Acceptable.
    void SkipToNextDecl() {
      while (*Current && !isa<SpecificDecl>(*Current))
        ++Current;
    }

  public:
    typedef SpecificDecl *value_type;
    // TODO: Add reference and pointer typedefs (with some appropriate proxy
    // type) if we ever have a need for them.
    typedef void reference;
    typedef void pointer;
    typedef std::iterator_traits<DeclContext::decl_iterator>::difference_type
      difference_type;
    typedef std::forward_iterator_tag iterator_category;

    specific_decl_iterator() : Current() { }

    /// specific_decl_iterator - Construct a new iterator over a
    /// subset of the declarations the range [C,
    /// end-of-declarations). If A is non-NULL, it is a pointer to a
    /// member function of SpecificDecl that should return true for
    /// all of the SpecificDecl instances that will be in the subset
    /// of iterators. For example, if you want Objective-C instance
    /// methods, SpecificDecl will be ObjCMethodDecl and A will be
    /// &ObjCMethodDecl::isInstanceMethod.
    explicit specific_decl_iterator(DeclContext::decl_iterator C) : Current(C) {
      SkipToNextDecl();
    }

    value_type operator*() const { return cast<SpecificDecl>(*Current); }
    // This doesn't meet the iterator requirements, but it's convenient
    value_type operator->() const { return **this; }

    specific_decl_iterator& operator++() {
      ++Current;
      SkipToNextDecl();
      return *this;
    }

    specific_decl_iterator operator++(int) {
      specific_decl_iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    friend bool operator==(const specific_decl_iterator& x,
                           const specific_decl_iterator& y) {
      return x.Current == y.Current;
    }

    friend bool operator!=(const specific_decl_iterator& x,
                           const specific_decl_iterator& y) {
      return x.Current != y.Current;
    }
  };

  /// \brief Iterates over a filtered subrange of declarations stored
  /// in a DeclContext.
  ///
  /// This iterator visits only those declarations that are of type
  /// SpecificDecl (or a class derived from it) and that meet some
  /// additional run-time criteria. This iterator is used, for
  /// example, to provide access to the instance methods within an
  /// Objective-C interface (with SpecificDecl = ObjCMethodDecl and
  /// Acceptable = ObjCMethodDecl::isInstanceMethod).
  template<typename SpecificDecl, bool (SpecificDecl::*Acceptable)() const>
  class filtered_decl_iterator {
    /// Current - The current, underlying declaration iterator, which
    /// will either be NULL or will point to a declaration of
    /// type SpecificDecl.
    DeclContext::decl_iterator Current;

    /// SkipToNextDecl - Advances the current position up to the next
    /// declaration of type SpecificDecl that also meets the criteria
    /// required by Acceptable.
    void SkipToNextDecl() {
      while (*Current &&
             (!isa<SpecificDecl>(*Current) ||
              (Acceptable && !(cast<SpecificDecl>(*Current)->*Acceptable)())))
        ++Current;
    }

  public:
    typedef SpecificDecl *value_type;
    // TODO: Add reference and pointer typedefs (with some appropriate proxy
    // type) if we ever have a need for them.
    typedef void reference;
    typedef void pointer;
    typedef std::iterator_traits<DeclContext::decl_iterator>::difference_type
      difference_type;
    typedef std::forward_iterator_tag iterator_category;

    filtered_decl_iterator() : Current() { }

    /// filtered_decl_iterator - Construct a new iterator over a
    /// subset of the declarations the range [C,
    /// end-of-declarations). If A is non-NULL, it is a pointer to a
    /// member function of SpecificDecl that should return true for
    /// all of the SpecificDecl instances that will be in the subset
    /// of iterators. For example, if you want Objective-C instance
    /// methods, SpecificDecl will be ObjCMethodDecl and A will be
    /// &ObjCMethodDecl::isInstanceMethod.
    explicit filtered_decl_iterator(DeclContext::decl_iterator C) : Current(C) {
      SkipToNextDecl();
    }

    value_type operator*() const { return cast<SpecificDecl>(*Current); }
    value_type operator->() const { return cast<SpecificDecl>(*Current); }

    filtered_decl_iterator& operator++() {
      ++Current;
      SkipToNextDecl();
      return *this;
    }

    filtered_decl_iterator operator++(int) {
      filtered_decl_iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    friend bool operator==(const filtered_decl_iterator& x,
                           const filtered_decl_iterator& y) {
      return x.Current == y.Current;
    }

    friend bool operator!=(const filtered_decl_iterator& x,
                           const filtered_decl_iterator& y) {
      return x.Current != y.Current;
    }
  };

  /// @brief Add the declaration D into this context.
  ///
  /// This routine should be invoked when the declaration D has first
  /// been declared, to place D into the context where it was
  /// (lexically) defined. Every declaration must be added to one
  /// (and only one!) context, where it can be visited via
  /// [decls_begin(), decls_end()). Once a declaration has been added
  /// to its lexical context, the corresponding DeclContext owns the
  /// declaration.
  ///
  /// If D is also a NamedDecl, it will be made visible within its
  /// semantic context via makeDeclVisibleInContext.
  void addDecl(Decl *D);

  /// @brief Add the declaration D into this context, but suppress
  /// searches for external declarations with the same name.
  ///
  /// Although analogous in function to addDecl, this removes an
  /// important check.  This is only useful if the Decl is being
  /// added in response to an external search; in all other cases,
  /// addDecl() is the right function to use.
  /// See the ASTImporter for use cases.
  void addDeclInternal(Decl *D);

  /// @brief Add the declaration D to this context without modifying
  /// any lookup tables.
  ///
  /// This is useful for some operations in dependent contexts where
  /// the semantic context might not be dependent;  this basically
  /// only happens with friends.
  void addHiddenDecl(Decl *D);

  /// @brief Removes a declaration from this context.
  void removeDecl(Decl *D);
    
  /// @brief Checks whether a declaration is in this context.
  bool containsDecl(Decl *D) const;

  typedef DeclContextLookupResult lookup_result;
  typedef lookup_result::iterator lookup_iterator;

  /// lookup - Find the declarations (if any) with the given Name in
  /// this context. Returns a range of iterators that contains all of
  /// the declarations with this name, with object, function, member,
  /// and enumerator names preceding any tag name. Note that this
  /// routine will not look into parent contexts.
  lookup_result lookup(DeclarationName Name) const;

  /// \brief Find the declarations with the given name that are visible
  /// within this context; don't attempt to retrieve anything from an
  /// external source.
  lookup_result noload_lookup(DeclarationName Name);

  /// \brief A simplistic name lookup mechanism that performs name lookup
  /// into this declaration context without consulting the external source.
  ///
  /// This function should almost never be used, because it subverts the
  /// usual relationship between a DeclContext and the external source.
  /// See the ASTImporter for the (few, but important) use cases.
  ///
  /// FIXME: This is very inefficient; replace uses of it with uses of
  /// noload_lookup.
  void localUncachedLookup(DeclarationName Name,
                           SmallVectorImpl<NamedDecl *> &Results);

  /// @brief Makes a declaration visible within this context.
  ///
  /// This routine makes the declaration D visible to name lookup
  /// within this context and, if this is a transparent context,
  /// within its parent contexts up to the first enclosing
  /// non-transparent context. Making a declaration visible within a
  /// context does not transfer ownership of a declaration, and a
  /// declaration can be visible in many contexts that aren't its
  /// lexical context.
  ///
  /// If D is a redeclaration of an existing declaration that is
  /// visible from this context, as determined by
  /// NamedDecl::declarationReplaces, the previous declaration will be
  /// replaced with D.
  void makeDeclVisibleInContext(NamedDecl *D);

  /// all_lookups_iterator - An iterator that provides a view over the results
  /// of looking up every possible name.
  class all_lookups_iterator;

  typedef llvm::iterator_range<all_lookups_iterator> lookups_range;

  lookups_range lookups() const;
  lookups_range noload_lookups() const;

  /// \brief Iterators over all possible lookups within this context.
  all_lookups_iterator lookups_begin() const;
  all_lookups_iterator lookups_end() const;

  /// \brief Iterators over all possible lookups within this context that are
  /// currently loaded; don't attempt to retrieve anything from an external
  /// source.
  all_lookups_iterator noload_lookups_begin() const;
  all_lookups_iterator noload_lookups_end() const;

  struct udir_iterator;
  typedef llvm::iterator_adaptor_base<udir_iterator, lookup_iterator,
                                      std::random_access_iterator_tag,
                                      UsingDirectiveDecl *> udir_iterator_base;
  struct udir_iterator : udir_iterator_base {
    udir_iterator(lookup_iterator I) : udir_iterator_base(I) {}
    UsingDirectiveDecl *operator*() const;
  };

  typedef llvm::iterator_range<udir_iterator> udir_range;

  udir_range using_directives() const;

  // These are all defined in DependentDiagnostic.h.
  class ddiag_iterator;
  typedef llvm::iterator_range<DeclContext::ddiag_iterator> ddiag_range;

  inline ddiag_range ddiags() const;

  // Low-level accessors

  /// \brief Mark that there are external lexical declarations that we need
  /// to include in our lookup table (and that are not available as external
  /// visible lookups). These extra lookup results will be found by walking
  /// the lexical declarations of this context. This should be used only if
  /// setHasExternalLexicalStorage() has been called on any decl context for
  /// which this is the primary context.
  void setMustBuildLookupTable() {
    assert(this == getPrimaryContext() &&
           "should only be called on primary context");
    HasLazyExternalLexicalLookups = true;
  }

  /// \brief Retrieve the internal representation of the lookup structure.
  /// This may omit some names if we are lazily building the structure.
  StoredDeclsMap *getLookupPtr() const { return LookupPtr; }

  /// \brief Ensure the lookup structure is fully-built and return it.
  StoredDeclsMap *buildLookup();

  /// \brief Whether this DeclContext has external storage containing
  /// additional declarations that are lexically in this context.
  bool hasExternalLexicalStorage() const { return ExternalLexicalStorage; }

  /// \brief State whether this DeclContext has external storage for
  /// declarations lexically in this context.
  void setHasExternalLexicalStorage(bool ES = true) {
    ExternalLexicalStorage = ES;
  }

  /// \brief Whether this DeclContext has external storage containing
  /// additional declarations that are visible in this context.
  bool hasExternalVisibleStorage() const { return ExternalVisibleStorage; }

  /// \brief State whether this DeclContext has external storage for
  /// declarations visible in this context.
  void setHasExternalVisibleStorage(bool ES = true) {
    ExternalVisibleStorage = ES;
    if (ES && LookupPtr)
      NeedToReconcileExternalVisibleStorage = true;
  }

  /// \brief Determine whether the given declaration is stored in the list of
  /// declarations lexically within this context.
  bool isDeclInLexicalTraversal(const Decl *D) const {
    return D && (D->NextInContextAndBits.getPointer() || D == FirstDecl || 
                 D == LastDecl);
  }

  static bool classof(const Decl *D);
  static bool classof(const DeclContext *D) { return true; }

  void dumpDeclContext() const;
  void dumpLookups() const;
  void dumpLookups(llvm::raw_ostream &OS, bool DumpDecls = false) const;

private:
  void reconcileExternalVisibleStorage() const;
  bool LoadLexicalDeclsFromExternalStorage() const;

  /// @brief Makes a declaration visible within this context, but
  /// suppresses searches for external declarations with the same
  /// name.
  ///
  /// Analogous to makeDeclVisibleInContext, but for the exclusive
  /// use of addDeclInternal().
  void makeDeclVisibleInContextInternal(NamedDecl *D);

  friend class DependentDiagnostic;
  StoredDeclsMap *CreateStoredDeclsMap(ASTContext &C) const;

  void buildLookupImpl(DeclContext *DCtx, bool Internal);
  void makeDeclVisibleInContextWithFlags(NamedDecl *D, bool Internal,
                                         bool Rediscoverable);
  void makeDeclVisibleInContextImpl(NamedDecl *D, bool Internal);
};

inline bool Decl::isTemplateParameter() const {
  return getKind() == TemplateTypeParm || getKind() == NonTypeTemplateParm ||
         getKind() == TemplateTemplateParm;
}

// Specialization selected when ToTy is not a known subclass of DeclContext.
template <class ToTy,
          bool IsKnownSubtype = ::std::is_base_of<DeclContext, ToTy>::value>
struct cast_convert_decl_context {
  static const ToTy *doit(const DeclContext *Val) {
    return static_cast<const ToTy*>(Decl::castFromDeclContext(Val));
  }

  static ToTy *doit(DeclContext *Val) {
    return static_cast<ToTy*>(Decl::castFromDeclContext(Val));
  }
};

// Specialization selected when ToTy is a known subclass of DeclContext.
template <class ToTy>
struct cast_convert_decl_context<ToTy, true> {
  static const ToTy *doit(const DeclContext *Val) {
    return static_cast<const ToTy*>(Val);
  }

  static ToTy *doit(DeclContext *Val) {
    return static_cast<ToTy*>(Val);
  }
};


} // end clang.

namespace llvm {

/// isa<T>(DeclContext*)
template <typename To>
struct isa_impl<To, ::clang::DeclContext> {
  static bool doit(const ::clang::DeclContext &Val) {
    return To::classofKind(Val.getDeclKind());
  }
};

/// cast<T>(DeclContext*)
template<class ToTy>
struct cast_convert_val<ToTy,
                        const ::clang::DeclContext,const ::clang::DeclContext> {
  static const ToTy &doit(const ::clang::DeclContext &Val) {
    return *::clang::cast_convert_decl_context<ToTy>::doit(&Val);
  }
};
template<class ToTy>
struct cast_convert_val<ToTy, ::clang::DeclContext, ::clang::DeclContext> {
  static ToTy &doit(::clang::DeclContext &Val) {
    return *::clang::cast_convert_decl_context<ToTy>::doit(&Val);
  }
};
template<class ToTy>
struct cast_convert_val<ToTy,
                     const ::clang::DeclContext*, const ::clang::DeclContext*> {
  static const ToTy *doit(const ::clang::DeclContext *Val) {
    return ::clang::cast_convert_decl_context<ToTy>::doit(Val);
  }
};
template<class ToTy>
struct cast_convert_val<ToTy, ::clang::DeclContext*, ::clang::DeclContext*> {
  static ToTy *doit(::clang::DeclContext *Val) {
    return ::clang::cast_convert_decl_context<ToTy>::doit(Val);
  }
};

/// Implement cast_convert_val for Decl -> DeclContext conversions.
template<class FromTy>
struct cast_convert_val< ::clang::DeclContext, FromTy, FromTy> {
  static ::clang::DeclContext &doit(const FromTy &Val) {
    return *FromTy::castToDeclContext(&Val);
  }
};

template<class FromTy>
struct cast_convert_val< ::clang::DeclContext, FromTy*, FromTy*> {
  static ::clang::DeclContext *doit(const FromTy *Val) {
    return FromTy::castToDeclContext(Val);
  }
};

template<class FromTy>
struct cast_convert_val< const ::clang::DeclContext, FromTy, FromTy> {
  static const ::clang::DeclContext &doit(const FromTy &Val) {
    return *FromTy::castToDeclContext(&Val);
  }
};

template<class FromTy>
struct cast_convert_val< const ::clang::DeclContext, FromTy*, FromTy*> {
  static const ::clang::DeclContext *doit(const FromTy *Val) {
    return FromTy::castToDeclContext(Val);
  }
};

} // end namespace llvm

#endif
