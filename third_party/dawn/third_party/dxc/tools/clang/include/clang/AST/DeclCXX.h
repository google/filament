//===-- DeclCXX.h - Classes for representing C++ declarations -*- C++ -*-=====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the C++ Decl subclasses, other than those for templates
/// (found in DeclTemplate.h) and friends (in DeclFriend.h).
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_DECLCXX_H
#define LLVM_CLANG_AST_DECLCXX_H

#include "clang/AST/ASTUnresolvedSet.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/LambdaCapture.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/Support/Compiler.h"

namespace clang {

class ClassTemplateDecl;
class ClassTemplateSpecializationDecl;
class CXXBasePath;
class CXXBasePaths;
class CXXConstructorDecl;
class CXXConversionDecl;
class CXXDestructorDecl;
class CXXMethodDecl;
class CXXRecordDecl;
class CXXMemberLookupCriteria;
class CXXFinalOverriderMap;
class CXXIndirectPrimaryBaseSet;
class FriendDecl;
class LambdaExpr;
class UsingDecl;

/// \brief Represents any kind of function declaration, whether it is a
/// concrete function or a function template.
class AnyFunctionDecl {
  NamedDecl *Function;

  AnyFunctionDecl(NamedDecl *ND) : Function(ND) { }

public:
  AnyFunctionDecl(FunctionDecl *FD) : Function(FD) { }
  AnyFunctionDecl(FunctionTemplateDecl *FTD);

  /// \brief Implicily converts any function or function template into a
  /// named declaration.
  operator NamedDecl *() const { return Function; }

  /// \brief Retrieve the underlying function or function template.
  NamedDecl *get() const { return Function; }

  static AnyFunctionDecl getFromNamedDecl(NamedDecl *ND) {
    return AnyFunctionDecl(ND);
  }
};

} // end namespace clang

namespace llvm {
  // Provide PointerLikeTypeTraits for non-cvr pointers.
  template<>
  class PointerLikeTypeTraits< ::clang::AnyFunctionDecl> {
  public:
    static inline void *getAsVoidPointer(::clang::AnyFunctionDecl F) {
      return F.get();
    }
    static inline ::clang::AnyFunctionDecl getFromVoidPointer(void *P) {
      return ::clang::AnyFunctionDecl::getFromNamedDecl(
                                      static_cast< ::clang::NamedDecl*>(P));
    }

    enum { NumLowBitsAvailable = 2 };
  };

} // end namespace llvm

namespace clang {

/// \brief Represents an access specifier followed by colon ':'.
///
/// An objects of this class represents sugar for the syntactic occurrence
/// of an access specifier followed by a colon in the list of member
/// specifiers of a C++ class definition.
///
/// Note that they do not represent other uses of access specifiers,
/// such as those occurring in a list of base specifiers.
/// Also note that this class has nothing to do with so-called
/// "access declarations" (C++98 11.3 [class.access.dcl]).
class AccessSpecDecl : public Decl {
  virtual void anchor();
  /// \brief The location of the ':'.
  SourceLocation ColonLoc;

  AccessSpecDecl(AccessSpecifier AS, DeclContext *DC,
                 SourceLocation ASLoc, SourceLocation ColonLoc)
    : Decl(AccessSpec, DC, ASLoc), ColonLoc(ColonLoc) {
    setAccess(AS);
  }
  AccessSpecDecl(EmptyShell Empty)
    : Decl(AccessSpec, Empty) { }
public:
  /// \brief The location of the access specifier.
  SourceLocation getAccessSpecifierLoc() const { return getLocation(); }
  /// \brief Sets the location of the access specifier.
  void setAccessSpecifierLoc(SourceLocation ASLoc) { setLocation(ASLoc); }

  /// \brief The location of the colon following the access specifier.
  SourceLocation getColonLoc() const { return ColonLoc; }
  /// \brief Sets the location of the colon.
  void setColonLoc(SourceLocation CLoc) { ColonLoc = CLoc; }

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(getAccessSpecifierLoc(), getColonLoc());
  }

  static AccessSpecDecl *Create(ASTContext &C, AccessSpecifier AS,
                                DeclContext *DC, SourceLocation ASLoc,
                                SourceLocation ColonLoc) {
    return new (C, DC) AccessSpecDecl(AS, DC, ASLoc, ColonLoc);
  }
  static AccessSpecDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == AccessSpec; }
};


/// \brief Represents a base class of a C++ class.
///
/// Each CXXBaseSpecifier represents a single, direct base class (or
/// struct) of a C++ class (or struct). It specifies the type of that
/// base class, whether it is a virtual or non-virtual base, and what
/// level of access (public, protected, private) is used for the
/// derivation. For example:
///
/// \code
///   class A { };
///   class B { };
///   class C : public virtual A, protected B { };
/// \endcode
///
/// In this code, C will have two CXXBaseSpecifiers, one for "public
/// virtual A" and the other for "protected B".
class CXXBaseSpecifier {
  /// \brief The source code range that covers the full base
  /// specifier, including the "virtual" (if present) and access
  /// specifier (if present).
  SourceRange Range;

  /// \brief The source location of the ellipsis, if this is a pack
  /// expansion.
  SourceLocation EllipsisLoc;

  /// \brief Whether this is a virtual base class or not.
  bool Virtual : 1;

  /// \brief Whether this is the base of a class (true) or of a struct (false).
  ///
  /// This determines the mapping from the access specifier as written in the
  /// source code to the access specifier used for semantic analysis.
  bool BaseOfClass : 1;

  /// \brief Access specifier as written in the source code (may be AS_none).
  ///
  /// The actual type of data stored here is an AccessSpecifier, but we use
  /// "unsigned" here to work around a VC++ bug.
  unsigned Access : 2;

  /// \brief Whether the class contains a using declaration
  /// to inherit the named class's constructors.
  bool InheritConstructors : 1;

  /// \brief The type of the base class.
  ///
  /// This will be a class or struct (or a typedef of such). The source code
  /// range does not include the \c virtual or the access specifier.
  TypeSourceInfo *BaseTypeInfo;

public:
  CXXBaseSpecifier() { }

  CXXBaseSpecifier(SourceRange R, bool V, bool BC, AccessSpecifier A,
                   TypeSourceInfo *TInfo, SourceLocation EllipsisLoc)
    : Range(R), EllipsisLoc(EllipsisLoc), Virtual(V), BaseOfClass(BC),
      Access(A), InheritConstructors(false), BaseTypeInfo(TInfo) { }

  /// \brief Retrieves the source range that contains the entire base specifier.
  SourceRange getSourceRange() const LLVM_READONLY { return Range; }
  SourceLocation getLocStart() const LLVM_READONLY { return Range.getBegin(); }
  SourceLocation getLocEnd() const LLVM_READONLY { return Range.getEnd(); }

  /// \brief Determines whether the base class is a virtual base class (or not).
  bool isVirtual() const { return Virtual; }

  /// \brief Determine whether this base class is a base of a class declared
  /// with the 'class' keyword (vs. one declared with the 'struct' keyword).
  bool isBaseOfClass() const { return BaseOfClass; }

  /// \brief Determine whether this base specifier is a pack expansion.
  bool isPackExpansion() const { return EllipsisLoc.isValid(); }

  /// \brief Determine whether this base class's constructors get inherited.
  bool getInheritConstructors() const { return InheritConstructors; }

  /// \brief Set that this base class's constructors should be inherited.
  void setInheritConstructors(bool Inherit = true) {
    InheritConstructors = Inherit;
  }

  /// \brief For a pack expansion, determine the location of the ellipsis.
  SourceLocation getEllipsisLoc() const {
    return EllipsisLoc;
  }

  /// \brief Returns the access specifier for this base specifier. 
  ///
  /// This is the actual base specifier as used for semantic analysis, so
  /// the result can never be AS_none. To retrieve the access specifier as
  /// written in the source code, use getAccessSpecifierAsWritten().
  AccessSpecifier getAccessSpecifier() const {
    if ((AccessSpecifier)Access == AS_none)
      return BaseOfClass? AS_private : AS_public;
    else
      return (AccessSpecifier)Access;
  }

  /// \brief Retrieves the access specifier as written in the source code
  /// (which may mean that no access specifier was explicitly written).
  ///
  /// Use getAccessSpecifier() to retrieve the access specifier for use in
  /// semantic analysis.
  AccessSpecifier getAccessSpecifierAsWritten() const {
    return (AccessSpecifier)Access;
  }

  /// \brief Retrieves the type of the base class.
  ///
  /// This type will always be an unqualified class type.
  QualType getType() const {
    return BaseTypeInfo->getType().getUnqualifiedType();
  }

  /// \brief Retrieves the type and source location of the base class.
  TypeSourceInfo *getTypeSourceInfo() const { return BaseTypeInfo; }
};

/// \brief A lazy pointer to the definition data for a declaration.
/// FIXME: This is a little CXXRecordDecl-specific that the moment.
template<typename Decl, typename T> class LazyDefinitionDataPtr {
  llvm::PointerUnion<T *, Decl *> DataOrCanonicalDecl;

  LazyDefinitionDataPtr update() {
    if (Decl *Canon = DataOrCanonicalDecl.template dyn_cast<Decl*>()) {
      if (Canon->isCanonicalDecl())
        Canon->getMostRecentDecl();
      else
        // Declaration isn't canonical any more;
        // update it and perform path compression.
        *this = Canon->getPreviousDecl()->DefinitionData.update();
    }
    return *this;
  }

public:
  LazyDefinitionDataPtr(Decl *Canon) : DataOrCanonicalDecl(Canon) {}
  LazyDefinitionDataPtr(T *Data) : DataOrCanonicalDecl(Data) {}
  T *getNotUpdated() { return DataOrCanonicalDecl.template dyn_cast<T*>(); }
  T *get() { return update().getNotUpdated(); }
};

/// \brief Represents a C++ struct/union/class.
class CXXRecordDecl : public RecordDecl {

  friend void TagDecl::startDefinition();

  /// Values used in DefinitionData fields to represent special members.
  enum SpecialMemberFlags {
    SMF_DefaultConstructor = 0x1,
    SMF_CopyConstructor = 0x2,
    SMF_MoveConstructor = 0x4,
    SMF_CopyAssignment = 0x8,
    SMF_MoveAssignment = 0x10,
    SMF_Destructor = 0x20,
    SMF_All = 0x3f
  };

  struct DefinitionData {
    DefinitionData(CXXRecordDecl *D);

    /// \brief True if this class has any user-declared constructors.
    bool UserDeclaredConstructor : 1;

    /// \brief The user-declared special members which this class has.
    unsigned UserDeclaredSpecialMembers : 6;

    /// \brief True when this class is an aggregate.
    bool Aggregate : 1;

    /// \brief True when this class is a POD-type.
    bool PlainOldData : 1;

    /// true when this class is empty for traits purposes,
    /// i.e. has no data members other than 0-width bit-fields, has no
    /// virtual function/base, and doesn't inherit from a non-empty
    /// class. Doesn't take union-ness into account.
    bool Empty : 1;

    /// \brief True when this class is polymorphic, i.e., has at
    /// least one virtual member or derives from a polymorphic class.
    bool Polymorphic : 1;

    /// \brief True when this class is abstract, i.e., has at least
    /// one pure virtual function, (that can come from a base class).
    bool Abstract : 1;

    /// \brief True when this class has standard layout.
    ///
    /// C++11 [class]p7.  A standard-layout class is a class that:
    /// * has no non-static data members of type non-standard-layout class (or
    ///   array of such types) or reference,
    /// * has no virtual functions (10.3) and no virtual base classes (10.1),
    /// * has the same access control (Clause 11) for all non-static data
    ///   members
    /// * has no non-standard-layout base classes,
    /// * either has no non-static data members in the most derived class and at
    ///   most one base class with non-static data members, or has no base
    ///   classes with non-static data members, and
    /// * has no base classes of the same type as the first non-static data
    ///   member.
    bool IsStandardLayout : 1;

    /// \brief True when there are no non-empty base classes.
    ///
    /// This is a helper bit of state used to implement IsStandardLayout more
    /// efficiently.
    bool HasNoNonEmptyBases : 1;

    /// \brief True when there are private non-static data members.
    bool HasPrivateFields : 1;

    /// \brief True when there are protected non-static data members.
    bool HasProtectedFields : 1;

    /// \brief True when there are private non-static data members.
    bool HasPublicFields : 1;

    /// \brief True if this class (or any subobject) has mutable fields.
    bool HasMutableFields : 1;

    /// \brief True if this class (or any nested anonymous struct or union)
    /// has variant members.
    bool HasVariantMembers : 1;

    /// \brief True if there no non-field members declared by the user.
    bool HasOnlyCMembers : 1;

    /// \brief True if any field has an in-class initializer, including those
    /// within anonymous unions or structs.
    bool HasInClassInitializer : 1;

    /// \brief True if any field is of reference type, and does not have an
    /// in-class initializer.
    ///
    /// In this case, value-initialization of this class is illegal in C++98
    /// even if the class has a trivial default constructor.
    bool HasUninitializedReferenceMember : 1;

    /// \brief These flags are \c true if a defaulted corresponding special
    /// member can't be fully analyzed without performing overload resolution.
    /// @{
    bool NeedOverloadResolutionForMoveConstructor : 1;
    bool NeedOverloadResolutionForMoveAssignment : 1;
    bool NeedOverloadResolutionForDestructor : 1;
    /// @}

    /// \brief These flags are \c true if an implicit defaulted corresponding
    /// special member would be defined as deleted.
    /// @{
    bool DefaultedMoveConstructorIsDeleted : 1;
    bool DefaultedMoveAssignmentIsDeleted : 1;
    bool DefaultedDestructorIsDeleted : 1;
    /// @}

    /// \brief The trivial special members which this class has, per
    /// C++11 [class.ctor]p5, C++11 [class.copy]p12, C++11 [class.copy]p25,
    /// C++11 [class.dtor]p5, or would have if the member were not suppressed.
    ///
    /// This excludes any user-declared but not user-provided special members
    /// which have been declared but not yet defined.
    unsigned HasTrivialSpecialMembers : 6;

    /// \brief The declared special members of this class which are known to be
    /// non-trivial.
    ///
    /// This excludes any user-declared but not user-provided special members
    /// which have been declared but not yet defined, and any implicit special
    /// members which have not yet been declared.
    unsigned DeclaredNonTrivialSpecialMembers : 6;

    /// \brief True when this class has a destructor with no semantic effect.
    bool HasIrrelevantDestructor : 1;

    /// \brief True when this class has at least one user-declared constexpr
    /// constructor which is neither the copy nor move constructor.
    bool HasConstexprNonCopyMoveConstructor : 1;

    /// \brief True if a defaulted default constructor for this class would
    /// be constexpr.
    bool DefaultedDefaultConstructorIsConstexpr : 1;

    /// \brief True if this class has a constexpr default constructor.
    ///
    /// This is true for either a user-declared constexpr default constructor
    /// or an implicitly declared constexpr default constructor.
    bool HasConstexprDefaultConstructor : 1;

    /// \brief True when this class contains at least one non-static data
    /// member or base class of non-literal or volatile type.
    bool HasNonLiteralTypeFieldsOrBases : 1;

    /// \brief True when visible conversion functions are already computed
    /// and are available.
    bool ComputedVisibleConversions : 1;

    /// \brief Whether we have a C++11 user-provided default constructor (not
    /// explicitly deleted or defaulted).
    bool UserProvidedDefaultConstructor : 1;

    /// \brief The special members which have been declared for this class,
    /// either by the user or implicitly.
    unsigned DeclaredSpecialMembers : 6;

    /// \brief Whether an implicit copy constructor would have a const-qualified
    /// parameter.
    bool ImplicitCopyConstructorHasConstParam : 1;

    /// \brief Whether an implicit copy assignment operator would have a
    /// const-qualified parameter.
    bool ImplicitCopyAssignmentHasConstParam : 1;

    /// \brief Whether any declared copy constructor has a const-qualified
    /// parameter.
    bool HasDeclaredCopyConstructorWithConstParam : 1;

    /// \brief Whether any declared copy assignment operator has either a
    /// const-qualified reference parameter or a non-reference parameter.
    bool HasDeclaredCopyAssignmentWithConstParam : 1;

    /// \brief Whether this class describes a C++ lambda.
    bool IsLambda : 1;

    /// \brief Whether we are currently parsing base specifiers.
    bool IsParsingBaseSpecifiers : 1;

    /// \brief The number of base class specifiers in Bases.
    unsigned NumBases;

    /// \brief The number of virtual base class specifiers in VBases.
    unsigned NumVBases;

    /// \brief Base classes of this class.
    ///
    /// FIXME: This is wasted space for a union.
    LazyCXXBaseSpecifiersPtr Bases;

    /// \brief direct and indirect virtual base classes of this class.
    LazyCXXBaseSpecifiersPtr VBases;

    /// \brief The conversion functions of this C++ class (but not its
    /// inherited conversion functions).
    ///
    /// Each of the entries in this overload set is a CXXConversionDecl.
    LazyASTUnresolvedSet Conversions;

    /// \brief The conversion functions of this C++ class and all those
    /// inherited conversion functions that are visible in this class.
    ///
    /// Each of the entries in this overload set is a CXXConversionDecl or a
    /// FunctionTemplateDecl.
    LazyASTUnresolvedSet VisibleConversions;

    /// \brief The declaration which defines this record.
    CXXRecordDecl *Definition;

    /// \brief The first friend declaration in this class, or null if there
    /// aren't any. 
    ///
    /// This is actually currently stored in reverse order.
    LazyDeclPtr FirstFriend;

    /// \brief Retrieve the set of direct base classes.
    CXXBaseSpecifier *getBases() const {
      if (!Bases.isOffset())
        return Bases.get(nullptr);
      return getBasesSlowCase();
    }

    /// \brief Retrieve the set of virtual base classes.
    CXXBaseSpecifier *getVBases() const {
      if (!VBases.isOffset())
        return VBases.get(nullptr);
      return getVBasesSlowCase();
    }

  private:
    CXXBaseSpecifier *getBasesSlowCase() const;
    CXXBaseSpecifier *getVBasesSlowCase() const;
  };

  typedef LazyDefinitionDataPtr<CXXRecordDecl, struct DefinitionData>
      DefinitionDataPtr;
  friend class LazyDefinitionDataPtr<CXXRecordDecl, struct DefinitionData>;

  mutable DefinitionDataPtr DefinitionData;

  /// \brief Describes a C++ closure type (generated by a lambda expression).
  struct LambdaDefinitionData : public DefinitionData {
    typedef LambdaCapture Capture;

    LambdaDefinitionData(CXXRecordDecl *D, TypeSourceInfo *Info, 
                         bool Dependent, bool IsGeneric, 
                         LambdaCaptureDefault CaptureDefault) 
      : DefinitionData(D), Dependent(Dependent), IsGenericLambda(IsGeneric), 
        CaptureDefault(CaptureDefault), NumCaptures(0), NumExplicitCaptures(0), 
        ManglingNumber(0), ContextDecl(nullptr), Captures(nullptr),
        MethodTyInfo(Info) {
      IsLambda = true;

      // C++11 [expr.prim.lambda]p3:
      //   This class type is neither an aggregate nor a literal type.
      Aggregate = false;
      PlainOldData = false;
      HasNonLiteralTypeFieldsOrBases = true;
    }

    /// \brief Whether this lambda is known to be dependent, even if its
    /// context isn't dependent.
    /// 
    /// A lambda with a non-dependent context can be dependent if it occurs
    /// within the default argument of a function template, because the
    /// lambda will have been created with the enclosing context as its
    /// declaration context, rather than function. This is an unfortunate
    /// artifact of having to parse the default arguments before. 
    unsigned Dependent : 1;
    
    /// \brief Whether this lambda is a generic lambda.
    unsigned IsGenericLambda : 1;

    /// \brief The Default Capture.
    unsigned CaptureDefault : 2;

    /// \brief The number of captures in this lambda is limited 2^NumCaptures.
    unsigned NumCaptures : 15;

    /// \brief The number of explicit captures in this lambda.
    unsigned NumExplicitCaptures : 13;

    /// \brief The number used to indicate this lambda expression for name 
    /// mangling in the Itanium C++ ABI.
    unsigned ManglingNumber;
    
    /// \brief The declaration that provides context for this lambda, if the
    /// actual DeclContext does not suffice. This is used for lambdas that
    /// occur within default arguments of function parameters within the class
    /// or within a data member initializer.
    Decl *ContextDecl;
    
    /// \brief The list of captures, both explicit and implicit, for this 
    /// lambda.
    Capture *Captures;

    /// \brief The type of the call method.
    TypeSourceInfo *MethodTyInfo;
       
  };

  struct DefinitionData &data() const {
    auto *DD = DefinitionData.get();
    assert(DD && "queried property of class with no definition");
    return *DD;
  }

  struct LambdaDefinitionData &getLambdaData() const {
    // No update required: a merged definition cannot change any lambda
    // properties.
    auto *DD = DefinitionData.getNotUpdated();
    assert(DD && DD->IsLambda && "queried lambda property of non-lambda class");
    return static_cast<LambdaDefinitionData&>(*DD);
  }

  /// \brief The template or declaration that this declaration
  /// describes or was instantiated from, respectively.
  ///
  /// For non-templates, this value will be null. For record
  /// declarations that describe a class template, this will be a
  /// pointer to a ClassTemplateDecl. For member
  /// classes of class template specializations, this will be the
  /// MemberSpecializationInfo referring to the member class that was
  /// instantiated or specialized.
  llvm::PointerUnion<ClassTemplateDecl*, MemberSpecializationInfo*>
    TemplateOrInstantiation;

  friend class DeclContext;
  friend class LambdaExpr;

  /// \brief Called from setBases and addedMember to notify the class that a
  /// direct or virtual base class or a member of class type has been added.
  void addedClassSubobject(CXXRecordDecl *Base);

  /// \brief Notify the class that member has been added.
  ///
  /// This routine helps maintain information about the class based on which
  /// members have been added. It will be invoked by DeclContext::addDecl()
  /// whenever a member is added to this record.
  void addedMember(Decl *D);

  void markedVirtualFunctionPure();
  friend void FunctionDecl::setPure(bool);

  friend class ASTNodeImporter;

  /// \brief Get the head of our list of friend declarations, possibly
  /// deserializing the friends from an external AST source.
  FriendDecl *getFirstFriend() const;

protected:
  CXXRecordDecl(Kind K, TagKind TK, const ASTContext &C, DeclContext *DC,
                SourceLocation StartLoc, SourceLocation IdLoc,
                IdentifierInfo *Id, CXXRecordDecl *PrevDecl);

public:
  /// \brief Iterator that traverses the base classes of a class.
  typedef CXXBaseSpecifier*       base_class_iterator;

  /// \brief Iterator that traverses the base classes of a class.
  typedef const CXXBaseSpecifier* base_class_const_iterator;

  CXXRecordDecl *getCanonicalDecl() override {
    return cast<CXXRecordDecl>(RecordDecl::getCanonicalDecl());
  }
  const CXXRecordDecl *getCanonicalDecl() const {
    return const_cast<CXXRecordDecl*>(this)->getCanonicalDecl();
  }

  CXXRecordDecl *getPreviousDecl() {
    return cast_or_null<CXXRecordDecl>(
            static_cast<RecordDecl *>(this)->getPreviousDecl());
  }
  const CXXRecordDecl *getPreviousDecl() const {
    return const_cast<CXXRecordDecl*>(this)->getPreviousDecl();
  }

  CXXRecordDecl *getMostRecentDecl() {
    return cast<CXXRecordDecl>(
            static_cast<RecordDecl *>(this)->getMostRecentDecl());
  }

  const CXXRecordDecl *getMostRecentDecl() const {
    return const_cast<CXXRecordDecl*>(this)->getMostRecentDecl();
  }

  CXXRecordDecl *getDefinition() const {
    auto *DD = DefinitionData.get();
    return DD ? DD->Definition : nullptr;
  }

  bool hasDefinition() const { return DefinitionData.get(); }

  static CXXRecordDecl *Create(const ASTContext &C, TagKind TK, DeclContext *DC,
                               SourceLocation StartLoc, SourceLocation IdLoc,
                               IdentifierInfo *Id,
                               CXXRecordDecl *PrevDecl = nullptr,
                               bool DelayTypeCreation = false);
  static CXXRecordDecl *CreateLambda(const ASTContext &C, DeclContext *DC,
                                     TypeSourceInfo *Info, SourceLocation Loc,
                                     bool DependentLambda, bool IsGeneric,
                                     LambdaCaptureDefault CaptureDefault);
  static CXXRecordDecl *CreateDeserialized(const ASTContext &C, unsigned ID);

  bool isDynamicClass() const {
    return data().Polymorphic || data().NumVBases != 0;
  }

  void setIsParsingBaseSpecifiers() { data().IsParsingBaseSpecifiers = true; }

  bool isParsingBaseSpecifiers() const {
    return data().IsParsingBaseSpecifiers;
  }

  /// \brief Sets the base classes of this struct or class.
  void setBases(CXXBaseSpecifier const * const *Bases, unsigned NumBases);

  /// \brief Retrieves the number of base classes of this class.
  unsigned getNumBases() const { return data().NumBases; }

  typedef llvm::iterator_range<base_class_iterator> base_class_range;
  typedef llvm::iterator_range<base_class_const_iterator>
    base_class_const_range;

  base_class_range bases() {
    return base_class_range(bases_begin(), bases_end());
  }
  base_class_const_range bases() const {
    return base_class_const_range(bases_begin(), bases_end());
  }

  base_class_iterator bases_begin() { return data().getBases(); }
  base_class_const_iterator bases_begin() const { return data().getBases(); }
  base_class_iterator bases_end() { return bases_begin() + data().NumBases; }
  base_class_const_iterator bases_end() const {
    return bases_begin() + data().NumBases;
  }

  /// \brief Retrieves the number of virtual base classes of this class.
  unsigned getNumVBases() const { return data().NumVBases; }

  base_class_range vbases() {
    return base_class_range(vbases_begin(), vbases_end());
  }
  base_class_const_range vbases() const {
    return base_class_const_range(vbases_begin(), vbases_end());
  }

  base_class_iterator vbases_begin() { return data().getVBases(); }
  base_class_const_iterator vbases_begin() const { return data().getVBases(); }
  base_class_iterator vbases_end() { return vbases_begin() + data().NumVBases; }
  base_class_const_iterator vbases_end() const {
    return vbases_begin() + data().NumVBases;
  }

  /// \brief Determine whether this class has any dependent base classes which
  /// are not the current instantiation.
  bool hasAnyDependentBases() const;

  /// Iterator access to method members.  The method iterator visits
  /// all method members of the class, including non-instance methods,
  /// special methods, etc.
  typedef specific_decl_iterator<CXXMethodDecl> method_iterator;
  typedef llvm::iterator_range<specific_decl_iterator<CXXMethodDecl>>
    method_range;

  method_range methods() const {
    return method_range(method_begin(), method_end());
  }

  /// \brief Method begin iterator.  Iterates in the order the methods
  /// were declared.
  method_iterator method_begin() const {
    return method_iterator(decls_begin());
  }
  /// \brief Method past-the-end iterator.
  method_iterator method_end() const {
    return method_iterator(decls_end());
  }

  /// Iterator access to constructor members.
  typedef specific_decl_iterator<CXXConstructorDecl> ctor_iterator;
  typedef llvm::iterator_range<specific_decl_iterator<CXXConstructorDecl>>
    ctor_range;

  ctor_range ctors() const { return ctor_range(ctor_begin(), ctor_end()); }

  ctor_iterator ctor_begin() const {
    return ctor_iterator(decls_begin());
  }
  ctor_iterator ctor_end() const {
    return ctor_iterator(decls_end());
  }

  /// An iterator over friend declarations.  All of these are defined
  /// in DeclFriend.h.
  class friend_iterator;
  typedef llvm::iterator_range<friend_iterator> friend_range;

  friend_range friends() const;
  friend_iterator friend_begin() const;
  friend_iterator friend_end() const;
  void pushFriendDecl(FriendDecl *FD);

  /// Determines whether this record has any friends.
  bool hasFriends() const {
    return data().FirstFriend.isValid();
  }

  /// \brief \c true if we know for sure that this class has a single,
  /// accessible, unambiguous move constructor that is not deleted.
  bool hasSimpleMoveConstructor() const {
    return !hasUserDeclaredMoveConstructor() && hasMoveConstructor() &&
           !data().DefaultedMoveConstructorIsDeleted;
  }
  /// \brief \c true if we know for sure that this class has a single,
  /// accessible, unambiguous move assignment operator that is not deleted.
  bool hasSimpleMoveAssignment() const {
    return !hasUserDeclaredMoveAssignment() && hasMoveAssignment() &&
           !data().DefaultedMoveAssignmentIsDeleted;
  }
  /// \brief \c true if we know for sure that this class has an accessible
  /// destructor that is not deleted.
  bool hasSimpleDestructor() const {
    return !hasUserDeclaredDestructor() &&
           !data().DefaultedDestructorIsDeleted;
  }

  /// \brief Determine whether this class has any default constructors.
  bool hasDefaultConstructor() const {
    return (data().DeclaredSpecialMembers & SMF_DefaultConstructor) ||
           needsImplicitDefaultConstructor();
  }

  /// \brief Determine if we need to declare a default constructor for
  /// this class.
  ///
  /// This value is used for lazy creation of default constructors.
  bool needsImplicitDefaultConstructor() const {
    return !data().UserDeclaredConstructor &&
           !(data().DeclaredSpecialMembers & SMF_DefaultConstructor) &&
           // C++14 [expr.prim.lambda]p20:
           //   The closure type associated with a lambda-expression has no
           //   default constructor.
           !isLambda();
  }

  /// \brief Determine whether this class has any user-declared constructors.
  ///
  /// When true, a default constructor will not be implicitly declared.
  bool hasUserDeclaredConstructor() const {
    return data().UserDeclaredConstructor;
  }

  /// \brief Whether this class has a user-provided default constructor
  /// per C++11.
  bool hasUserProvidedDefaultConstructor() const {
    return data().UserProvidedDefaultConstructor;
  }

  /// \brief Determine whether this class has a user-declared copy constructor.
  ///
  /// When false, a copy constructor will be implicitly declared.
  bool hasUserDeclaredCopyConstructor() const {
    return data().UserDeclaredSpecialMembers & SMF_CopyConstructor;
  }

  /// \brief Determine whether this class needs an implicit copy
  /// constructor to be lazily declared.
  bool needsImplicitCopyConstructor() const {
    return !(data().DeclaredSpecialMembers & SMF_CopyConstructor);
  }

  /// \brief Determine whether we need to eagerly declare a defaulted copy
  /// constructor for this class.
  bool needsOverloadResolutionForCopyConstructor() const {
    return data().HasMutableFields;
  }

  /// \brief Determine whether an implicit copy constructor for this type
  /// would have a parameter with a const-qualified reference type.
  bool implicitCopyConstructorHasConstParam() const {
    return data().ImplicitCopyConstructorHasConstParam;
  }

  /// \brief Determine whether this class has a copy constructor with
  /// a parameter type which is a reference to a const-qualified type.
  bool hasCopyConstructorWithConstParam() const {
    return data().HasDeclaredCopyConstructorWithConstParam ||
           (needsImplicitCopyConstructor() &&
            implicitCopyConstructorHasConstParam());
  }

  /// \brief Whether this class has a user-declared move constructor or
  /// assignment operator.
  ///
  /// When false, a move constructor and assignment operator may be
  /// implicitly declared.
  bool hasUserDeclaredMoveOperation() const {
    return data().UserDeclaredSpecialMembers &
             (SMF_MoveConstructor | SMF_MoveAssignment);
  }

  /// \brief Determine whether this class has had a move constructor
  /// declared by the user.
  bool hasUserDeclaredMoveConstructor() const {
    return data().UserDeclaredSpecialMembers & SMF_MoveConstructor;
  }

  /// \brief Determine whether this class has a move constructor.
  bool hasMoveConstructor() const {
    return (data().DeclaredSpecialMembers & SMF_MoveConstructor) ||
           needsImplicitMoveConstructor();
  }

  /// \brief Set that we attempted to declare an implicitly move
  /// constructor, but overload resolution failed so we deleted it.
  void setImplicitMoveConstructorIsDeleted() {
    assert((data().DefaultedMoveConstructorIsDeleted ||
            needsOverloadResolutionForMoveConstructor()) &&
           "move constructor should not be deleted");
    data().DefaultedMoveConstructorIsDeleted = true;
  }

  /// \brief Determine whether this class should get an implicit move
  /// constructor or if any existing special member function inhibits this.
  bool needsImplicitMoveConstructor() const {
    return !(data().DeclaredSpecialMembers & SMF_MoveConstructor) &&
           !hasUserDeclaredCopyConstructor() &&
           !hasUserDeclaredCopyAssignment() &&
           !hasUserDeclaredMoveAssignment() &&
           !hasUserDeclaredDestructor();
  }

  /// \brief Determine whether we need to eagerly declare a defaulted move
  /// constructor for this class.
  bool needsOverloadResolutionForMoveConstructor() const {
    return data().NeedOverloadResolutionForMoveConstructor;
  }

  /// \brief Determine whether this class has a user-declared copy assignment
  /// operator.
  ///
  /// When false, a copy assigment operator will be implicitly declared.
  bool hasUserDeclaredCopyAssignment() const {
    return data().UserDeclaredSpecialMembers & SMF_CopyAssignment;
  }

  /// \brief Determine whether this class needs an implicit copy
  /// assignment operator to be lazily declared.
  bool needsImplicitCopyAssignment() const {
    return !(data().DeclaredSpecialMembers & SMF_CopyAssignment);
  }

  /// \brief Determine whether we need to eagerly declare a defaulted copy
  /// assignment operator for this class.
  bool needsOverloadResolutionForCopyAssignment() const {
    return data().HasMutableFields;
  }

  /// \brief Determine whether an implicit copy assignment operator for this
  /// type would have a parameter with a const-qualified reference type.
  bool implicitCopyAssignmentHasConstParam() const {
    return data().ImplicitCopyAssignmentHasConstParam;
  }

  /// \brief Determine whether this class has a copy assignment operator with
  /// a parameter type which is a reference to a const-qualified type or is not
  /// a reference.
  bool hasCopyAssignmentWithConstParam() const {
    return data().HasDeclaredCopyAssignmentWithConstParam ||
           (needsImplicitCopyAssignment() &&
            implicitCopyAssignmentHasConstParam());
  }

  /// \brief Determine whether this class has had a move assignment
  /// declared by the user.
  bool hasUserDeclaredMoveAssignment() const {
    return data().UserDeclaredSpecialMembers & SMF_MoveAssignment;
  }

  /// \brief Determine whether this class has a move assignment operator.
  bool hasMoveAssignment() const {
    return (data().DeclaredSpecialMembers & SMF_MoveAssignment) ||
           needsImplicitMoveAssignment();
  }

  /// \brief Set that we attempted to declare an implicit move assignment
  /// operator, but overload resolution failed so we deleted it.
  void setImplicitMoveAssignmentIsDeleted() {
    assert((data().DefaultedMoveAssignmentIsDeleted ||
            needsOverloadResolutionForMoveAssignment()) &&
           "move assignment should not be deleted");
    data().DefaultedMoveAssignmentIsDeleted = true;
  }

  /// \brief Determine whether this class should get an implicit move
  /// assignment operator or if any existing special member function inhibits
  /// this.
  bool needsImplicitMoveAssignment() const {
    return !(data().DeclaredSpecialMembers & SMF_MoveAssignment) &&
           !hasUserDeclaredCopyConstructor() &&
           !hasUserDeclaredCopyAssignment() &&
           !hasUserDeclaredMoveConstructor() &&
           !hasUserDeclaredDestructor();
  }

  /// \brief Determine whether we need to eagerly declare a move assignment
  /// operator for this class.
  bool needsOverloadResolutionForMoveAssignment() const {
    return data().NeedOverloadResolutionForMoveAssignment;
  }

  /// \brief Determine whether this class has a user-declared destructor.
  ///
  /// When false, a destructor will be implicitly declared.
  bool hasUserDeclaredDestructor() const {
    return data().UserDeclaredSpecialMembers & SMF_Destructor;
  }

  /// \brief Determine whether this class needs an implicit destructor to
  /// be lazily declared.
  bool needsImplicitDestructor() const {
    return !(data().DeclaredSpecialMembers & SMF_Destructor);
  }

  /// \brief Determine whether we need to eagerly declare a destructor for this
  /// class.
  bool needsOverloadResolutionForDestructor() const {
    return data().NeedOverloadResolutionForDestructor;
  }

  /// \brief Determine whether this class describes a lambda function object.
  bool isLambda() const {
    // An update record can't turn a non-lambda into a lambda.
    auto *DD = DefinitionData.getNotUpdated();
    return DD && DD->IsLambda;
  }

  /// \brief Determine whether this class describes a generic 
  /// lambda function object (i.e. function call operator is
  /// a template). 
  bool isGenericLambda() const; 

  /// \brief Retrieve the lambda call operator of the closure type
  /// if this is a closure type.
  CXXMethodDecl *getLambdaCallOperator() const; 

  /// \brief Retrieve the lambda static invoker, the address of which
  /// is returned by the conversion operator, and the body of which
  /// is forwarded to the lambda call operator. 
  CXXMethodDecl *getLambdaStaticInvoker() const; 

  /// \brief Retrieve the generic lambda's template parameter list.
  /// Returns null if the class does not represent a lambda or a generic 
  /// lambda.
  TemplateParameterList *getGenericLambdaTemplateParameterList() const;

  LambdaCaptureDefault getLambdaCaptureDefault() const {
    assert(isLambda());
    return static_cast<LambdaCaptureDefault>(getLambdaData().CaptureDefault);
  }

  /// \brief For a closure type, retrieve the mapping from captured
  /// variables and \c this to the non-static data members that store the
  /// values or references of the captures.
  ///
  /// \param Captures Will be populated with the mapping from captured
  /// variables to the corresponding fields.
  ///
  /// \param ThisCapture Will be set to the field declaration for the
  /// \c this capture.
  ///
  /// \note No entries will be added for init-captures, as they do not capture
  /// variables.
  void getCaptureFields(llvm::DenseMap<const VarDecl *, FieldDecl *> &Captures,
                        FieldDecl *&ThisCapture) const;

  typedef const LambdaCapture *capture_const_iterator;
  typedef llvm::iterator_range<capture_const_iterator> capture_const_range;

  capture_const_range captures() const {
    return capture_const_range(captures_begin(), captures_end());
  }
  capture_const_iterator captures_begin() const {
    return isLambda() ? getLambdaData().Captures : nullptr;
  }
  capture_const_iterator captures_end() const {
    return isLambda() ? captures_begin() + getLambdaData().NumCaptures
                      : nullptr;
  }

  typedef UnresolvedSetIterator conversion_iterator;
  conversion_iterator conversion_begin() const {
    return data().Conversions.get(getASTContext()).begin();
  }
  conversion_iterator conversion_end() const {
    return data().Conversions.get(getASTContext()).end();
  }

  /// Removes a conversion function from this class.  The conversion
  /// function must currently be a member of this class.  Furthermore,
  /// this class must currently be in the process of being defined.
  void removeConversion(const NamedDecl *Old);

  /// \brief Get all conversion functions visible in current class,
  /// including conversion function templates.
  llvm::iterator_range<conversion_iterator> getVisibleConversionFunctions();

  /// Determine whether this class is an aggregate (C++ [dcl.init.aggr]),
  /// which is a class with no user-declared constructors, no private
  /// or protected non-static data members, no base classes, and no virtual
  /// functions (C++ [dcl.init.aggr]p1).
  bool isAggregate() const { return data().Aggregate; }

  /// \brief Whether this class has any in-class initializers
  /// for non-static data members (including those in anonymous unions or
  /// structs).
  bool hasInClassInitializer() const { return data().HasInClassInitializer; }

  /// \brief Whether this class or any of its subobjects has any members of
  /// reference type which would make value-initialization ill-formed.
  ///
  /// Per C++03 [dcl.init]p5:
  ///  - if T is a non-union class type without a user-declared constructor,
  ///    then every non-static data member and base-class component of T is
  ///    value-initialized [...] A program that calls for [...]
  ///    value-initialization of an entity of reference type is ill-formed.
  bool hasUninitializedReferenceMember() const {
    return !isUnion() && !hasUserDeclaredConstructor() &&
           data().HasUninitializedReferenceMember;
  }

  /// \brief Whether this class is a POD-type (C++ [class]p4)
  ///
  /// For purposes of this function a class is POD if it is an aggregate
  /// that has no non-static non-POD data members, no reference data
  /// members, no user-defined copy assignment operator and no
  /// user-defined destructor.
  ///
  /// Note that this is the C++ TR1 definition of POD.
  bool isPOD() const { return data().PlainOldData; }

  /// \brief True if this class is C-like, without C++-specific features, e.g.
  /// it contains only public fields, no bases, tag kind is not 'class', etc.
  bool isCLike() const;

  /// \brief Determine whether this is an empty class in the sense of
  /// (C++11 [meta.unary.prop]).
  ///
  /// A non-union class is empty iff it has a virtual function, virtual base,
  /// data member (other than 0-width bit-field) or inherits from a non-empty
  /// class.
  ///
  /// \note This does NOT include a check for union-ness.
  bool isEmpty() const { return data().Empty; }

  /// Whether this class is polymorphic (C++ [class.virtual]),
  /// which means that the class contains or inherits a virtual function.
  bool isPolymorphic() const { return data().Polymorphic; }

  /// \brief Determine whether this class has a pure virtual function.
  ///
  /// The class is is abstract per (C++ [class.abstract]p2) if it declares
  /// a pure virtual function or inherits a pure virtual function that is
  /// not overridden.
  bool isAbstract() const { return data().Abstract; }

  /// \brief Determine whether this class has standard layout per 
  /// (C++ [class]p7)
  bool isStandardLayout() const { return data().IsStandardLayout; }

  /// \brief Determine whether this class, or any of its class subobjects,
  /// contains a mutable field.
  bool hasMutableFields() const { return data().HasMutableFields; }

  /// \brief Determine whether this class has any variant members.
  bool hasVariantMembers() const { return data().HasVariantMembers; }

  /// \brief Determine whether this class has a trivial default constructor
  /// (C++11 [class.ctor]p5).
  bool hasTrivialDefaultConstructor() const {
    return hasDefaultConstructor() &&
           (data().HasTrivialSpecialMembers & SMF_DefaultConstructor);
  }

  /// \brief Determine whether this class has a non-trivial default constructor
  /// (C++11 [class.ctor]p5).
  bool hasNonTrivialDefaultConstructor() const {
    return (data().DeclaredNonTrivialSpecialMembers & SMF_DefaultConstructor) ||
           (needsImplicitDefaultConstructor() &&
            !(data().HasTrivialSpecialMembers & SMF_DefaultConstructor));
  }

  /// \brief Determine whether this class has at least one constexpr constructor
  /// other than the copy or move constructors.
  bool hasConstexprNonCopyMoveConstructor() const {
    return data().HasConstexprNonCopyMoveConstructor ||
           (needsImplicitDefaultConstructor() &&
            defaultedDefaultConstructorIsConstexpr());
  }

  /// \brief Determine whether a defaulted default constructor for this class
  /// would be constexpr.
  bool defaultedDefaultConstructorIsConstexpr() const {
    return data().DefaultedDefaultConstructorIsConstexpr &&
           (!isUnion() || hasInClassInitializer() || !hasVariantMembers());
  }

  /// \brief Determine whether this class has a constexpr default constructor.
  bool hasConstexprDefaultConstructor() const {
    return data().HasConstexprDefaultConstructor ||
           (needsImplicitDefaultConstructor() &&
            defaultedDefaultConstructorIsConstexpr());
  }

  /// \brief Determine whether this class has a trivial copy constructor
  /// (C++ [class.copy]p6, C++11 [class.copy]p12)
  bool hasTrivialCopyConstructor() const {
    return data().HasTrivialSpecialMembers & SMF_CopyConstructor;
  }

  /// \brief Determine whether this class has a non-trivial copy constructor
  /// (C++ [class.copy]p6, C++11 [class.copy]p12)
  bool hasNonTrivialCopyConstructor() const {
    return data().DeclaredNonTrivialSpecialMembers & SMF_CopyConstructor ||
           !hasTrivialCopyConstructor();
  }

  /// \brief Determine whether this class has a trivial move constructor
  /// (C++11 [class.copy]p12)
  bool hasTrivialMoveConstructor() const {
    return hasMoveConstructor() &&
           (data().HasTrivialSpecialMembers & SMF_MoveConstructor);
  }

  /// \brief Determine whether this class has a non-trivial move constructor
  /// (C++11 [class.copy]p12)
  bool hasNonTrivialMoveConstructor() const {
    return (data().DeclaredNonTrivialSpecialMembers & SMF_MoveConstructor) ||
           (needsImplicitMoveConstructor() &&
            !(data().HasTrivialSpecialMembers & SMF_MoveConstructor));
  }

  /// \brief Determine whether this class has a trivial copy assignment operator
  /// (C++ [class.copy]p11, C++11 [class.copy]p25)
  bool hasTrivialCopyAssignment() const {
    return data().HasTrivialSpecialMembers & SMF_CopyAssignment;
  }

  /// \brief Determine whether this class has a non-trivial copy assignment
  /// operator (C++ [class.copy]p11, C++11 [class.copy]p25)
  bool hasNonTrivialCopyAssignment() const {
    return data().DeclaredNonTrivialSpecialMembers & SMF_CopyAssignment ||
           !hasTrivialCopyAssignment();
  }

  /// \brief Determine whether this class has a trivial move assignment operator
  /// (C++11 [class.copy]p25)
  bool hasTrivialMoveAssignment() const {
    return hasMoveAssignment() &&
           (data().HasTrivialSpecialMembers & SMF_MoveAssignment);
  }

  /// \brief Determine whether this class has a non-trivial move assignment
  /// operator (C++11 [class.copy]p25)
  bool hasNonTrivialMoveAssignment() const {
    return (data().DeclaredNonTrivialSpecialMembers & SMF_MoveAssignment) ||
           (needsImplicitMoveAssignment() &&
            !(data().HasTrivialSpecialMembers & SMF_MoveAssignment));
  }

  /// \brief Determine whether this class has a trivial destructor
  /// (C++ [class.dtor]p3)
  bool hasTrivialDestructor() const {
    return data().HasTrivialSpecialMembers & SMF_Destructor;
  }

  /// \brief Determine whether this class has a non-trivial destructor
  /// (C++ [class.dtor]p3)
  bool hasNonTrivialDestructor() const {
    return !(data().HasTrivialSpecialMembers & SMF_Destructor);
  }

  /// \brief Determine whether this class has a destructor which has no
  /// semantic effect.
  ///
  /// Any such destructor will be trivial, public, defaulted and not deleted,
  /// and will call only irrelevant destructors.
  bool hasIrrelevantDestructor() const {
    return data().HasIrrelevantDestructor;
  }

  /// \brief Determine whether this class has a non-literal or/ volatile type
  /// non-static data member or base class.
  bool hasNonLiteralTypeFieldsOrBases() const {
    return data().HasNonLiteralTypeFieldsOrBases;
  }

  /// \brief Determine whether this class is considered trivially copyable per
  /// (C++11 [class]p6).
  bool isTriviallyCopyable() const;

  /// \brief Determine whether this class is considered trivial.
  ///
  /// C++11 [class]p6:
  ///    "A trivial class is a class that has a trivial default constructor and
  ///    is trivially copiable."
  bool isTrivial() const {
    return isTriviallyCopyable() && hasTrivialDefaultConstructor();
  }

  /// \brief Determine whether this class is a literal type.
  ///
  /// C++11 [basic.types]p10:
  ///   A class type that has all the following properties:
  ///     - it has a trivial destructor
  ///     - every constructor call and full-expression in the
  ///       brace-or-equal-intializers for non-static data members (if any) is
  ///       a constant expression.
  ///     - it is an aggregate type or has at least one constexpr constructor
  ///       or constructor template that is not a copy or move constructor, and
  ///     - all of its non-static data members and base classes are of literal
  ///       types
  ///
  /// We resolve DR1361 by ignoring the second bullet. We resolve DR1452 by
  /// treating types with trivial default constructors as literal types.
  bool isLiteral() const {
    return hasTrivialDestructor() &&
           (isAggregate() || hasConstexprNonCopyMoveConstructor() ||
            hasTrivialDefaultConstructor()) &&
           !hasNonLiteralTypeFieldsOrBases();
  }

  /// \brief If this record is an instantiation of a member class,
  /// retrieves the member class from which it was instantiated.
  ///
  /// This routine will return non-null for (non-templated) member
  /// classes of class templates. For example, given:
  ///
  /// \code
  /// template<typename T>
  /// struct X {
  ///   struct A { };
  /// };
  /// \endcode
  ///
  /// The declaration for X<int>::A is a (non-templated) CXXRecordDecl
  /// whose parent is the class template specialization X<int>. For
  /// this declaration, getInstantiatedFromMemberClass() will return
  /// the CXXRecordDecl X<T>::A. When a complete definition of
  /// X<int>::A is required, it will be instantiated from the
  /// declaration returned by getInstantiatedFromMemberClass().
  CXXRecordDecl *getInstantiatedFromMemberClass() const;

  /// \brief If this class is an instantiation of a member class of a
  /// class template specialization, retrieves the member specialization
  /// information.
  MemberSpecializationInfo *getMemberSpecializationInfo() const {
    return TemplateOrInstantiation.dyn_cast<MemberSpecializationInfo *>();
  }

  /// \brief Specify that this record is an instantiation of the
  /// member class \p RD.
  void setInstantiationOfMemberClass(CXXRecordDecl *RD,
                                     TemplateSpecializationKind TSK);

  /// \brief Retrieves the class template that is described by this
  /// class declaration.
  ///
  /// Every class template is represented as a ClassTemplateDecl and a
  /// CXXRecordDecl. The former contains template properties (such as
  /// the template parameter lists) while the latter contains the
  /// actual description of the template's
  /// contents. ClassTemplateDecl::getTemplatedDecl() retrieves the
  /// CXXRecordDecl that from a ClassTemplateDecl, while
  /// getDescribedClassTemplate() retrieves the ClassTemplateDecl from
  /// a CXXRecordDecl.
  ClassTemplateDecl *getDescribedClassTemplate() const {
    return TemplateOrInstantiation.dyn_cast<ClassTemplateDecl*>();
  }

  void setDescribedClassTemplate(ClassTemplateDecl *Template) {
    TemplateOrInstantiation = Template;
  }

  /// \brief Determine whether this particular class is a specialization or
  /// instantiation of a class template or member class of a class template,
  /// and how it was instantiated or specialized.
  TemplateSpecializationKind getTemplateSpecializationKind() const;

  /// \brief Set the kind of specialization or template instantiation this is.
  void setTemplateSpecializationKind(TemplateSpecializationKind TSK);

  /// \brief Retrieve the record declaration from which this record could be
  /// instantiated. Returns null if this class is not a template instantiation.
  const CXXRecordDecl *getTemplateInstantiationPattern() const;

  CXXRecordDecl *getTemplateInstantiationPattern() {
    return const_cast<CXXRecordDecl *>(const_cast<const CXXRecordDecl *>(this)
                                           ->getTemplateInstantiationPattern());
  }

  /// \brief Returns the destructor decl for this class.
  CXXDestructorDecl *getDestructor() const;

  /// \brief Returns true if the class destructor, or any implicitly invoked
  /// destructors are marked noreturn.
  bool isAnyDestructorNoReturn() const;

  /// \brief If the class is a local class [class.local], returns
  /// the enclosing function declaration.
  const FunctionDecl *isLocalClass() const {
    if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(getDeclContext()))
      return RD->isLocalClass();

    return dyn_cast<FunctionDecl>(getDeclContext());
  }

  FunctionDecl *isLocalClass() {
    return const_cast<FunctionDecl*>(
        const_cast<const CXXRecordDecl*>(this)->isLocalClass());
  }

  /// \brief Determine whether this dependent class is a current instantiation,
  /// when viewed from within the given context.
  bool isCurrentInstantiation(const DeclContext *CurContext) const;

  /// \brief Determine whether this class is derived from the class \p Base.
  ///
  /// This routine only determines whether this class is derived from \p Base,
  /// but does not account for factors that may make a Derived -> Base class
  /// ill-formed, such as private/protected inheritance or multiple, ambiguous
  /// base class subobjects.
  ///
  /// \param Base the base class we are searching for.
  ///
  /// \returns true if this class is derived from Base, false otherwise.
  bool isDerivedFrom(const CXXRecordDecl *Base) const;

  /// \brief Determine whether this class is derived from the type \p Base.
  ///
  /// This routine only determines whether this class is derived from \p Base,
  /// but does not account for factors that may make a Derived -> Base class
  /// ill-formed, such as private/protected inheritance or multiple, ambiguous
  /// base class subobjects.
  ///
  /// \param Base the base class we are searching for.
  ///
  /// \param Paths will contain the paths taken from the current class to the
  /// given \p Base class.
  ///
  /// \returns true if this class is derived from \p Base, false otherwise.
  ///
  /// \todo add a separate parameter to configure IsDerivedFrom, rather than
  /// tangling input and output in \p Paths
  bool isDerivedFrom(const CXXRecordDecl *Base, CXXBasePaths &Paths) const;

  /// \brief Determine whether this class is virtually derived from
  /// the class \p Base.
  ///
  /// This routine only determines whether this class is virtually
  /// derived from \p Base, but does not account for factors that may
  /// make a Derived -> Base class ill-formed, such as
  /// private/protected inheritance or multiple, ambiguous base class
  /// subobjects.
  ///
  /// \param Base the base class we are searching for.
  ///
  /// \returns true if this class is virtually derived from Base,
  /// false otherwise.
  bool isVirtuallyDerivedFrom(const CXXRecordDecl *Base) const;

  /// \brief Determine whether this class is provably not derived from
  /// the type \p Base.
  bool isProvablyNotDerivedFrom(const CXXRecordDecl *Base) const;

  /// \brief Function type used by forallBases() as a callback.
  ///
  /// \param BaseDefinition the definition of the base class
  ///
  /// \returns true if this base matched the search criteria
  typedef bool ForallBasesCallback(const CXXRecordDecl *BaseDefinition,
                                   void *UserData);

  /// \brief Determines if the given callback holds for all the direct
  /// or indirect base classes of this type.
  ///
  /// The class itself does not count as a base class.  This routine
  /// returns false if the class has non-computable base classes.
  ///
  /// \param BaseMatches Callback invoked for each (direct or indirect) base
  /// class of this type, or if \p AllowShortCircuit is true then until a call
  /// returns false.
  ///
  /// \param UserData Passed as the second argument of every call to
  /// \p BaseMatches.
  ///
  /// \param AllowShortCircuit if false, forces the callback to be called
  /// for every base class, even if a dependent or non-matching base was
  /// found.
  bool forallBases(ForallBasesCallback *BaseMatches, void *UserData,
                   bool AllowShortCircuit = true) const;

  /// \brief Function type used by lookupInBases() to determine whether a
  /// specific base class subobject matches the lookup criteria.
  ///
  /// \param Specifier the base-class specifier that describes the inheritance
  /// from the base class we are trying to match.
  ///
  /// \param Path the current path, from the most-derived class down to the
  /// base named by the \p Specifier.
  ///
  /// \param UserData a single pointer to user-specified data, provided to
  /// lookupInBases().
  ///
  /// \returns true if this base matched the search criteria, false otherwise.
  typedef bool BaseMatchesCallback(const CXXBaseSpecifier *Specifier,
                                   CXXBasePath &Path,
                                   void *UserData);

  /// \brief Look for entities within the base classes of this C++ class,
  /// transitively searching all base class subobjects.
  ///
  /// This routine uses the callback function \p BaseMatches to find base
  /// classes meeting some search criteria, walking all base class subobjects
  /// and populating the given \p Paths structure with the paths through the
  /// inheritance hierarchy that resulted in a match. On a successful search,
  /// the \p Paths structure can be queried to retrieve the matching paths and
  /// to determine if there were any ambiguities.
  ///
  /// \param BaseMatches callback function used to determine whether a given
  /// base matches the user-defined search criteria.
  ///
  /// \param UserData user data pointer that will be provided to \p BaseMatches.
  ///
  /// \param Paths used to record the paths from this class to its base class
  /// subobjects that match the search criteria.
  ///
  /// \returns true if there exists any path from this class to a base class
  /// subobject that matches the search criteria.
  bool lookupInBases(BaseMatchesCallback *BaseMatches, void *UserData,
                     CXXBasePaths &Paths) const;

  /// \brief Base-class lookup callback that determines whether the given
  /// base class specifier refers to a specific class declaration.
  ///
  /// This callback can be used with \c lookupInBases() to determine whether
  /// a given derived class has is a base class subobject of a particular type.
  /// The user data pointer should refer to the canonical CXXRecordDecl of the
  /// base class that we are searching for.
  static bool FindBaseClass(const CXXBaseSpecifier *Specifier,
                            CXXBasePath &Path, void *BaseRecord);

  /// \brief Base-class lookup callback that determines whether the
  /// given base class specifier refers to a specific class
  /// declaration and describes virtual derivation.
  ///
  /// This callback can be used with \c lookupInBases() to determine
  /// whether a given derived class has is a virtual base class
  /// subobject of a particular type.  The user data pointer should
  /// refer to the canonical CXXRecordDecl of the base class that we
  /// are searching for.
  static bool FindVirtualBaseClass(const CXXBaseSpecifier *Specifier,
                                   CXXBasePath &Path, void *BaseRecord);

  /// \brief Base-class lookup callback that determines whether there exists
  /// a tag with the given name.
  ///
  /// This callback can be used with \c lookupInBases() to find tag members
  /// of the given name within a C++ class hierarchy. The user data pointer
  /// is an opaque \c DeclarationName pointer.
  static bool FindTagMember(const CXXBaseSpecifier *Specifier,
                            CXXBasePath &Path, void *Name);

  /// \brief Base-class lookup callback that determines whether there exists
  /// a member with the given name.
  ///
  /// This callback can be used with \c lookupInBases() to find members
  /// of the given name within a C++ class hierarchy. The user data pointer
  /// is an opaque \c DeclarationName pointer.
  static bool FindOrdinaryMember(const CXXBaseSpecifier *Specifier,
                                 CXXBasePath &Path, void *Name);

  /// \brief Base-class lookup callback that determines whether there exists
  /// a member with the given name that can be used in a nested-name-specifier.
  ///
  /// This callback can be used with \c lookupInBases() to find membes of
  /// the given name within a C++ class hierarchy that can occur within
  /// nested-name-specifiers.
  static bool FindNestedNameSpecifierMember(const CXXBaseSpecifier *Specifier,
                                            CXXBasePath &Path,
                                            void *UserData);

  /// \brief Retrieve the final overriders for each virtual member
  /// function in the class hierarchy where this class is the
  /// most-derived class in the class hierarchy.
  void getFinalOverriders(CXXFinalOverriderMap &FinaOverriders) const;

  /// \brief Get the indirect primary bases for this class.
  void getIndirectPrimaryBases(CXXIndirectPrimaryBaseSet& Bases) const;

  /// Renders and displays an inheritance diagram
  /// for this C++ class and all of its base classes (transitively) using
  /// GraphViz.
  void viewInheritance(ASTContext& Context) const;

  /// \brief Calculates the access of a decl that is reached
  /// along a path.
  static AccessSpecifier MergeAccess(AccessSpecifier PathAccess,
                                     AccessSpecifier DeclAccess) {
    assert(DeclAccess != AS_none);
    if (DeclAccess == AS_private) return AS_none;
    return (PathAccess > DeclAccess ? PathAccess : DeclAccess);
  }

  /// \brief Indicates that the declaration of a defaulted or deleted special
  /// member function is now complete.
  void finishedDefaultedOrDeletedMember(CXXMethodDecl *MD);

  /// \brief Indicates that the definition of this class is now complete.
  void completeDefinition() override;

  /// \brief Indicates that the definition of this class is now complete,
  /// and provides a final overrider map to help determine
  ///
  /// \param FinalOverriders The final overrider map for this class, which can
  /// be provided as an optimization for abstract-class checking. If NULL,
  /// final overriders will be computed if they are needed to complete the
  /// definition.
  void completeDefinition(CXXFinalOverriderMap *FinalOverriders);

  /// \brief Determine whether this class may end up being abstract, even though
  /// it is not yet known to be abstract.
  ///
  /// \returns true if this class is not known to be abstract but has any
  /// base classes that are abstract. In this case, \c completeDefinition()
  /// will need to compute final overriders to determine whether the class is
  /// actually abstract.
  bool mayBeAbstract() const;

  /// \brief If this is the closure type of a lambda expression, retrieve the
  /// number to be used for name mangling in the Itanium C++ ABI.
  ///
  /// Zero indicates that this closure type has internal linkage, so the 
  /// mangling number does not matter, while a non-zero value indicates which
  /// lambda expression this is in this particular context.
  unsigned getLambdaManglingNumber() const {
    assert(isLambda() && "Not a lambda closure type!");
    return getLambdaData().ManglingNumber;
  }
  
  /// \brief Retrieve the declaration that provides additional context for a 
  /// lambda, when the normal declaration context is not specific enough.
  ///
  /// Certain contexts (default arguments of in-class function parameters and 
  /// the initializers of data members) have separate name mangling rules for
  /// lambdas within the Itanium C++ ABI. For these cases, this routine provides
  /// the declaration in which the lambda occurs, e.g., the function parameter 
  /// or the non-static data member. Otherwise, it returns NULL to imply that
  /// the declaration context suffices.
  Decl *getLambdaContextDecl() const {
    assert(isLambda() && "Not a lambda closure type!");
    return getLambdaData().ContextDecl;    
  }
  
  /// \brief Set the mangling number and context declaration for a lambda
  /// class.
  void setLambdaMangling(unsigned ManglingNumber, Decl *ContextDecl) {
    getLambdaData().ManglingNumber = ManglingNumber;
    getLambdaData().ContextDecl = ContextDecl;
  }

  /// \brief Returns the inheritance model used for this record.
  MSInheritanceAttr::Spelling getMSInheritanceModel() const;
  /// \brief Calculate what the inheritance model would be for this class.
  MSInheritanceAttr::Spelling calculateInheritanceModel() const;

  /// In the Microsoft C++ ABI, use zero for the field offset of a null data
  /// member pointer if we can guarantee that zero is not a valid field offset,
  /// or if the member pointer has multiple fields.  Polymorphic classes have a
  /// vfptr at offset zero, so we can use zero for null.  If there are multiple
  /// fields, we can use zero even if it is a valid field offset because
  /// null-ness testing will check the other fields.
  bool nullFieldOffsetIsZero() const {
    return !MSInheritanceAttr::hasOnlyOneField(/*IsMemberFunction=*/false,
                                               getMSInheritanceModel()) ||
           (hasDefinition() && isPolymorphic());
  }

  /// \brief Controls when vtordisps will be emitted if this record is used as a
  /// virtual base.
  MSVtorDispAttr::Mode getMSVtorDispMode() const;

  /// \brief Determine whether this lambda expression was known to be dependent
  /// at the time it was created, even if its context does not appear to be
  /// dependent.
  ///
  /// This flag is a workaround for an issue with parsing, where default
  /// arguments are parsed before their enclosing function declarations have
  /// been created. This means that any lambda expressions within those
  /// default arguments will have as their DeclContext the context enclosing
  /// the function declaration, which may be non-dependent even when the
  /// function declaration itself is dependent. This flag indicates when we
  /// know that the lambda is dependent despite that.
  bool isDependentLambda() const {
    return isLambda() && getLambdaData().Dependent;
  }

  TypeSourceInfo *getLambdaTypeInfo() const {
    return getLambdaData().MethodTyInfo;
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) {
    return K >= firstCXXRecord && K <= lastCXXRecord;
  }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
  friend class ASTReader;
  friend class ASTWriter;
};

/// \brief Represents a static or instance method of a struct/union/class.
///
/// In the terminology of the C++ Standard, these are the (static and
/// non-static) member functions, whether virtual or not.
class CXXMethodDecl : public FunctionDecl {
  void anchor() override;
protected:
  CXXMethodDecl(Kind DK, ASTContext &C, CXXRecordDecl *RD,
                SourceLocation StartLoc, const DeclarationNameInfo &NameInfo,
                QualType T, TypeSourceInfo *TInfo,
                StorageClass SC, bool isInline,
                bool isConstexpr, SourceLocation EndLocation)
    : FunctionDecl(DK, C, RD, StartLoc, NameInfo, T, TInfo,
                   SC, isInline, isConstexpr) {
    if (EndLocation.isValid())
      setRangeEnd(EndLocation);
  }

public:
  static CXXMethodDecl *Create(ASTContext &C, CXXRecordDecl *RD,
                               SourceLocation StartLoc,
                               const DeclarationNameInfo &NameInfo,
                               QualType T, TypeSourceInfo *TInfo,
                               StorageClass SC,
                               bool isInline,
                               bool isConstexpr,
                               SourceLocation EndLocation);

  static CXXMethodDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  bool isStatic() const;
  bool isInstance() const { return !isStatic(); }

  /// Returns true if the given operator is implicitly static in a record
  /// context.
  static bool isStaticOverloadedOperator(OverloadedOperatorKind OOK) {
    // [class.free]p1:
    // Any allocation function for a class T is a static member
    // (even if not explicitly declared static).
    // [class.free]p6 Any deallocation function for a class X is a static member
    // (even if not explicitly declared static).
    return OOK == OO_New || OOK == OO_Array_New || OOK == OO_Delete ||
           OOK == OO_Array_Delete;
  }

  bool isConst() const { return getType()->castAs<FunctionType>()->isConst(); }
  bool isVolatile() const { return getType()->castAs<FunctionType>()->isVolatile(); }

  bool isVirtual() const {
    CXXMethodDecl *CD =
      cast<CXXMethodDecl>(const_cast<CXXMethodDecl*>(this)->getCanonicalDecl());

    // Member function is virtual if it is marked explicitly so, or if it is
    // declared in __interface -- then it is automatically pure virtual.
    if (CD->isVirtualAsWritten() || CD->isPure())
      return true;

    return (CD->begin_overridden_methods() != CD->end_overridden_methods());
  }

  /// \brief Determine whether this is a usual deallocation function
  /// (C++ [basic.stc.dynamic.deallocation]p2), which is an overloaded
  /// delete or delete[] operator with a particular signature.
  bool isUsualDeallocationFunction() const;

  /// \brief Determine whether this is a copy-assignment operator, regardless
  /// of whether it was declared implicitly or explicitly.
  bool isCopyAssignmentOperator() const;

  /// \brief Determine whether this is a move assignment operator.
  bool isMoveAssignmentOperator() const;

  CXXMethodDecl *getCanonicalDecl() override {
    return cast<CXXMethodDecl>(FunctionDecl::getCanonicalDecl());
  }
  const CXXMethodDecl *getCanonicalDecl() const {
    return const_cast<CXXMethodDecl*>(this)->getCanonicalDecl();
  }

  CXXMethodDecl *getMostRecentDecl() {
    return cast<CXXMethodDecl>(
            static_cast<FunctionDecl *>(this)->getMostRecentDecl());
  }
  const CXXMethodDecl *getMostRecentDecl() const {
    return const_cast<CXXMethodDecl*>(this)->getMostRecentDecl();
  }

  /// True if this method is user-declared and was not
  /// deleted or defaulted on its first declaration.
  bool isUserProvided() const {
    return !(isDeleted() || getCanonicalDecl()->isDefaulted());
  }

  ///
  void addOverriddenMethod(const CXXMethodDecl *MD);

  typedef const CXXMethodDecl *const* method_iterator;

  method_iterator begin_overridden_methods() const;
  method_iterator end_overridden_methods() const;
  unsigned size_overridden_methods() const;

  /// Returns the parent of this method declaration, which
  /// is the class in which this method is defined.
  const CXXRecordDecl *getParent() const {
    return cast<CXXRecordDecl>(FunctionDecl::getParent());
  }

  /// Returns the parent of this method declaration, which
  /// is the class in which this method is defined.
  CXXRecordDecl *getParent() {
    return const_cast<CXXRecordDecl *>(
             cast<CXXRecordDecl>(FunctionDecl::getParent()));
  }

  /// \brief Returns the type of the \c this pointer.
  ///
  /// Should only be called for instance (i.e., non-static) methods.
  QualType getThisType(ASTContext &C) const;

  // HLSL Change Begin - This is a reference.
  /// \brief Returns the type of the \c this object looking through the pointer.
  ///
  /// Should only be called for instance (i.e., non-static) methods.
  QualType getThisObjectType(ASTContext &C) const;
  // HLSL Change End - This is a reference.

  unsigned getTypeQualifiers() const {
    return getType()->getAs<FunctionProtoType>()->getTypeQuals();
  }

  /// \brief Retrieve the ref-qualifier associated with this method.
  ///
  /// In the following example, \c f() has an lvalue ref-qualifier, \c g()
  /// has an rvalue ref-qualifier, and \c h() has no ref-qualifier.
  /// @code
  /// struct X {
  ///   void f() &;
  ///   void g() &&;
  ///   void h();
  /// };
  /// @endcode
  RefQualifierKind getRefQualifier() const {
    return getType()->getAs<FunctionProtoType>()->getRefQualifier();
  }

  bool hasInlineBody() const;

  /// \brief Determine whether this is a lambda closure type's static member
  /// function that is used for the result of the lambda's conversion to
  /// function pointer (for a lambda with no captures).
  ///
  /// The function itself, if used, will have a placeholder body that will be
  /// supplied by IR generation to either forward to the function call operator
  /// or clone the function call operator.
  bool isLambdaStaticInvoker() const;

  /// \brief Find the method in \p RD that corresponds to this one.
  ///
  /// Find if \p RD or one of the classes it inherits from override this method.
  /// If so, return it. \p RD is assumed to be a subclass of the class defining
  /// this method (or be the class itself), unless \p MayBeBase is set to true.
  CXXMethodDecl *
  getCorrespondingMethodInClass(const CXXRecordDecl *RD,
                                bool MayBeBase = false);

  const CXXMethodDecl *
  getCorrespondingMethodInClass(const CXXRecordDecl *RD,
                                bool MayBeBase = false) const {
    return const_cast<CXXMethodDecl *>(this)
              ->getCorrespondingMethodInClass(RD, MayBeBase);
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) {
    return K >= firstCXXMethod && K <= lastCXXMethod;
  }
};

/// \brief Represents a C++ base or member initializer.
///
/// This is part of a constructor initializer that
/// initializes one non-static member variable or one base class. For
/// example, in the following, both 'A(a)' and 'f(3.14159)' are member
/// initializers:
///
/// \code
/// class A { };
/// class B : public A {
///   float f;
/// public:
///   B(A& a) : A(a), f(3.14159) { }
/// };
/// \endcode
class CXXCtorInitializer {
  /// \brief Either the base class name/delegating constructor type (stored as
  /// a TypeSourceInfo*), an normal field (FieldDecl), or an anonymous field
  /// (IndirectFieldDecl*) being initialized.
  llvm::PointerUnion3<TypeSourceInfo *, FieldDecl *, IndirectFieldDecl *>
    Initializee;

  /// \brief The source location for the field name or, for a base initializer
  /// pack expansion, the location of the ellipsis.
  ///
  /// In the case of a delegating
  /// constructor, it will still include the type's source location as the
  /// Initializee points to the CXXConstructorDecl (to allow loop detection).
  SourceLocation MemberOrEllipsisLocation;

  /// \brief The argument used to initialize the base or member, which may
  /// end up constructing an object (when multiple arguments are involved).
  Stmt *Init;

  /// \brief Location of the left paren of the ctor-initializer.
  SourceLocation LParenLoc;

  /// \brief Location of the right paren of the ctor-initializer.
  SourceLocation RParenLoc;

  /// \brief If the initializee is a type, whether that type makes this
  /// a delegating initialization.
  bool IsDelegating : 1;

  /// \brief If the initializer is a base initializer, this keeps track
  /// of whether the base is virtual or not.
  bool IsVirtual : 1;

  /// \brief Whether or not the initializer is explicitly written
  /// in the sources.
  bool IsWritten : 1;

  /// If IsWritten is true, then this number keeps track of the textual order
  /// of this initializer in the original sources, counting from 0; otherwise,
  /// it stores the number of array index variables stored after this object
  /// in memory.
  unsigned SourceOrderOrNumArrayIndices : 13;

  CXXCtorInitializer(ASTContext &Context, FieldDecl *Member,
                     SourceLocation MemberLoc, SourceLocation L, Expr *Init,
                     SourceLocation R, VarDecl **Indices, unsigned NumIndices);

public:
  /// \brief Creates a new base-class initializer.
  explicit
  CXXCtorInitializer(ASTContext &Context, TypeSourceInfo *TInfo, bool IsVirtual,
                     SourceLocation L, Expr *Init, SourceLocation R,
                     SourceLocation EllipsisLoc);

  /// \brief Creates a new member initializer.
  explicit
  CXXCtorInitializer(ASTContext &Context, FieldDecl *Member,
                     SourceLocation MemberLoc, SourceLocation L, Expr *Init,
                     SourceLocation R);

  /// \brief Creates a new anonymous field initializer.
  explicit
  CXXCtorInitializer(ASTContext &Context, IndirectFieldDecl *Member,
                     SourceLocation MemberLoc, SourceLocation L, Expr *Init,
                     SourceLocation R);

  /// \brief Creates a new delegating initializer.
  explicit
  CXXCtorInitializer(ASTContext &Context, TypeSourceInfo *TInfo,
                     SourceLocation L, Expr *Init, SourceLocation R);

  /// \brief Creates a new member initializer that optionally contains
  /// array indices used to describe an elementwise initialization.
  static CXXCtorInitializer *Create(ASTContext &Context, FieldDecl *Member,
                                    SourceLocation MemberLoc, SourceLocation L,
                                    Expr *Init, SourceLocation R,
                                    VarDecl **Indices, unsigned NumIndices);

  /// \brief Determine whether this initializer is initializing a base class.
  bool isBaseInitializer() const {
    return Initializee.is<TypeSourceInfo*>() && !IsDelegating;
  }

  /// \brief Determine whether this initializer is initializing a non-static
  /// data member.
  bool isMemberInitializer() const { return Initializee.is<FieldDecl*>(); }

  bool isAnyMemberInitializer() const {
    return isMemberInitializer() || isIndirectMemberInitializer();
  }

  bool isIndirectMemberInitializer() const {
    return Initializee.is<IndirectFieldDecl*>();
  }

  /// \brief Determine whether this initializer is an implicit initializer
  /// generated for a field with an initializer defined on the member
  /// declaration.
  ///
  /// In-class member initializers (also known as "non-static data member
  /// initializations", NSDMIs) were introduced in C++11.
  bool isInClassMemberInitializer() const {
    return Init->getStmtClass() == Stmt::CXXDefaultInitExprClass;
  }

  /// \brief Determine whether this initializer is creating a delegating
  /// constructor.
  bool isDelegatingInitializer() const {
    return Initializee.is<TypeSourceInfo*>() && IsDelegating;
  }

  /// \brief Determine whether this initializer is a pack expansion.
  bool isPackExpansion() const {
    return isBaseInitializer() && MemberOrEllipsisLocation.isValid();
  }

  // \brief For a pack expansion, returns the location of the ellipsis.
  SourceLocation getEllipsisLoc() const {
    assert(isPackExpansion() && "Initializer is not a pack expansion");
    return MemberOrEllipsisLocation;
  }

  /// If this is a base class initializer, returns the type of the
  /// base class with location information. Otherwise, returns an NULL
  /// type location.
  TypeLoc getBaseClassLoc() const;

  /// If this is a base class initializer, returns the type of the base class.
  /// Otherwise, returns null.
  const Type *getBaseClass() const;

  /// Returns whether the base is virtual or not.
  bool isBaseVirtual() const {
    assert(isBaseInitializer() && "Must call this on base initializer!");

    return IsVirtual;
  }

  /// \brief Returns the declarator information for a base class or delegating
  /// initializer.
  TypeSourceInfo *getTypeSourceInfo() const {
    return Initializee.dyn_cast<TypeSourceInfo *>();
  }

  /// \brief If this is a member initializer, returns the declaration of the
  /// non-static data member being initialized. Otherwise, returns null.
  FieldDecl *getMember() const {
    if (isMemberInitializer())
      return Initializee.get<FieldDecl*>();
    return nullptr;
  }
  FieldDecl *getAnyMember() const {
    if (isMemberInitializer())
      return Initializee.get<FieldDecl*>();
    if (isIndirectMemberInitializer())
      return Initializee.get<IndirectFieldDecl*>()->getAnonField();
    return nullptr;
  }

  IndirectFieldDecl *getIndirectMember() const {
    if (isIndirectMemberInitializer())
      return Initializee.get<IndirectFieldDecl*>();
    return nullptr;
  }

  SourceLocation getMemberLocation() const {
    return MemberOrEllipsisLocation;
  }

  /// \brief Determine the source location of the initializer.
  SourceLocation getSourceLocation() const;

  /// \brief Determine the source range covering the entire initializer.
  SourceRange getSourceRange() const LLVM_READONLY;

  /// \brief Determine whether this initializer is explicitly written
  /// in the source code.
  bool isWritten() const { return IsWritten; }

  /// \brief Return the source position of the initializer, counting from 0.
  /// If the initializer was implicit, -1 is returned.
  int getSourceOrder() const {
    return IsWritten ? static_cast<int>(SourceOrderOrNumArrayIndices) : -1;
  }

  /// \brief Set the source order of this initializer.
  ///
  /// This can only be called once for each initializer; it cannot be called
  /// on an initializer having a positive number of (implicit) array indices.
  ///
  /// This assumes that the initializer was written in the source code, and
  /// ensures that isWritten() returns true.
  void setSourceOrder(int pos) {
    assert(!IsWritten &&
           "calling twice setSourceOrder() on the same initializer");
    assert(SourceOrderOrNumArrayIndices == 0 &&
           "setSourceOrder() used when there are implicit array indices");
    assert(pos >= 0 &&
           "setSourceOrder() used to make an initializer implicit");
    IsWritten = true;
    SourceOrderOrNumArrayIndices = static_cast<unsigned>(pos);
  }

  SourceLocation getLParenLoc() const { return LParenLoc; }
  SourceLocation getRParenLoc() const { return RParenLoc; }

  /// \brief Determine the number of implicit array indices used while
  /// described an array member initialization.
  unsigned getNumArrayIndices() const {
    return IsWritten ? 0 : SourceOrderOrNumArrayIndices;
  }

  /// \brief Retrieve a particular array index variable used to
  /// describe an array member initialization.
  VarDecl *getArrayIndex(unsigned I) {
    assert(I < getNumArrayIndices() && "Out of bounds member array index");
    return reinterpret_cast<VarDecl **>(this + 1)[I];
  }
  const VarDecl *getArrayIndex(unsigned I) const {
    assert(I < getNumArrayIndices() && "Out of bounds member array index");
    return reinterpret_cast<const VarDecl * const *>(this + 1)[I];
  }
  void setArrayIndex(unsigned I, VarDecl *Index) {
    assert(I < getNumArrayIndices() && "Out of bounds member array index");
    reinterpret_cast<VarDecl **>(this + 1)[I] = Index;
  }
  ArrayRef<VarDecl *> getArrayIndexes() {
    assert(getNumArrayIndices() != 0 && "Getting indexes for non-array init");
    return llvm::makeArrayRef(reinterpret_cast<VarDecl **>(this + 1),
                              getNumArrayIndices());
  }

  /// \brief Get the initializer.
  Expr *getInit() const { return static_cast<Expr*>(Init); }
};

/// \brief Represents a C++ constructor within a class.
///
/// For example:
///
/// \code
/// class X {
/// public:
///   explicit X(int); // represented by a CXXConstructorDecl.
/// };
/// \endcode
class CXXConstructorDecl : public CXXMethodDecl {
  void anchor() override;
  /// \brief Whether this constructor declaration has the \c explicit keyword
  /// specified.
  bool IsExplicitSpecified : 1;

  /// \name Support for base and member initializers.
  /// \{
  /// \brief The arguments used to initialize the base or member.
  LazyCXXCtorInitializersPtr CtorInitializers;
  unsigned NumCtorInitializers;
  /// \}

  CXXConstructorDecl(ASTContext &C, CXXRecordDecl *RD, SourceLocation StartLoc,
                     const DeclarationNameInfo &NameInfo,
                     QualType T, TypeSourceInfo *TInfo,
                     bool isExplicitSpecified, bool isInline,
                     bool isImplicitlyDeclared, bool isConstexpr)
    : CXXMethodDecl(CXXConstructor, C, RD, StartLoc, NameInfo, T, TInfo,
                    SC_None, isInline, isConstexpr, SourceLocation()),
      IsExplicitSpecified(isExplicitSpecified), CtorInitializers(nullptr),
      NumCtorInitializers(0) {
    setImplicit(isImplicitlyDeclared);
  }

public:
  static CXXConstructorDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  static CXXConstructorDecl *Create(ASTContext &C, CXXRecordDecl *RD,
                                    SourceLocation StartLoc,
                                    const DeclarationNameInfo &NameInfo,
                                    QualType T, TypeSourceInfo *TInfo,
                                    bool isExplicit,
                                    bool isInline, bool isImplicitlyDeclared,
                                    bool isConstexpr);

  /// \brief Determine whether this constructor declaration has the
  /// \c explicit keyword specified.
  bool isExplicitSpecified() const { return IsExplicitSpecified; }

  /// \brief Determine whether this constructor was marked "explicit" or not.
  bool isExplicit() const {
    return cast<CXXConstructorDecl>(getFirstDecl())->isExplicitSpecified();
  }

  /// \brief Iterates through the member/base initializer list.
  typedef CXXCtorInitializer **init_iterator;

  /// \brief Iterates through the member/base initializer list.
  typedef CXXCtorInitializer *const *init_const_iterator;

  typedef llvm::iterator_range<init_iterator> init_range;
  typedef llvm::iterator_range<init_const_iterator> init_const_range;

  init_range inits() { return init_range(init_begin(), init_end()); }
  init_const_range inits() const {
    return init_const_range(init_begin(), init_end());
  }

  /// \brief Retrieve an iterator to the first initializer.
  init_iterator init_begin() {
    const auto *ConstThis = this;
    return const_cast<init_iterator>(ConstThis->init_begin());
  }
  /// \brief Retrieve an iterator to the first initializer.
  init_const_iterator init_begin() const;

  /// \brief Retrieve an iterator past the last initializer.
  init_iterator       init_end()       {
    return init_begin() + NumCtorInitializers;
  }
  /// \brief Retrieve an iterator past the last initializer.
  init_const_iterator init_end() const {
    return init_begin() + NumCtorInitializers;
  }

  typedef std::reverse_iterator<init_iterator> init_reverse_iterator;
  typedef std::reverse_iterator<init_const_iterator>
          init_const_reverse_iterator;

  init_reverse_iterator init_rbegin() {
    return init_reverse_iterator(init_end());
  }
  init_const_reverse_iterator init_rbegin() const {
    return init_const_reverse_iterator(init_end());
  }

  init_reverse_iterator init_rend() {
    return init_reverse_iterator(init_begin());
  }
  init_const_reverse_iterator init_rend() const {
    return init_const_reverse_iterator(init_begin());
  }

  /// \brief Determine the number of arguments used to initialize the member
  /// or base.
  unsigned getNumCtorInitializers() const {
      return NumCtorInitializers;
  }

  void setNumCtorInitializers(unsigned numCtorInitializers) {
    NumCtorInitializers = numCtorInitializers;
  }

  void setCtorInitializers(CXXCtorInitializer **Initializers) {
    CtorInitializers = Initializers;
  }

  /// \brief Determine whether this constructor is a delegating constructor.
  bool isDelegatingConstructor() const {
    return (getNumCtorInitializers() == 1) &&
           init_begin()[0]->isDelegatingInitializer();
  }

  /// \brief When this constructor delegates to another, retrieve the target.
  CXXConstructorDecl *getTargetConstructor() const;

  /// Whether this constructor is a default
  /// constructor (C++ [class.ctor]p5), which can be used to
  /// default-initialize a class of this type.
  bool isDefaultConstructor() const;

  /// \brief Whether this constructor is a copy constructor (C++ [class.copy]p2,
  /// which can be used to copy the class.
  ///
  /// \p TypeQuals will be set to the qualifiers on the
  /// argument type. For example, \p TypeQuals would be set to \c
  /// Qualifiers::Const for the following copy constructor:
  ///
  /// \code
  /// class X {
  /// public:
  ///   X(const X&);
  /// };
  /// \endcode
  bool isCopyConstructor(unsigned &TypeQuals) const;

  /// Whether this constructor is a copy
  /// constructor (C++ [class.copy]p2, which can be used to copy the
  /// class.
  bool isCopyConstructor() const {
    unsigned TypeQuals = 0;
    return isCopyConstructor(TypeQuals);
  }

  /// \brief Determine whether this constructor is a move constructor
  /// (C++0x [class.copy]p3), which can be used to move values of the class.
  ///
  /// \param TypeQuals If this constructor is a move constructor, will be set
  /// to the type qualifiers on the referent of the first parameter's type.
  bool isMoveConstructor(unsigned &TypeQuals) const;

  /// \brief Determine whether this constructor is a move constructor
  /// (C++0x [class.copy]p3), which can be used to move values of the class.
  bool isMoveConstructor() const {
    unsigned TypeQuals = 0;
    return isMoveConstructor(TypeQuals);
  }

  /// \brief Determine whether this is a copy or move constructor.
  ///
  /// \param TypeQuals Will be set to the type qualifiers on the reference
  /// parameter, if in fact this is a copy or move constructor.
  bool isCopyOrMoveConstructor(unsigned &TypeQuals) const;

  /// \brief Determine whether this a copy or move constructor.
  bool isCopyOrMoveConstructor() const {
    unsigned Quals;
    return isCopyOrMoveConstructor(Quals);
  }

  /// Whether this constructor is a
  /// converting constructor (C++ [class.conv.ctor]), which can be
  /// used for user-defined conversions.
  bool isConvertingConstructor(bool AllowExplicit) const;

  /// \brief Determine whether this is a member template specialization that
  /// would copy the object to itself. Such constructors are never used to copy
  /// an object.
  bool isSpecializationCopyingObject() const;

  /// \brief Get the constructor that this inheriting constructor is based on.
  const CXXConstructorDecl *getInheritedConstructor() const;

  /// \brief Set the constructor that this inheriting constructor is based on.
  void setInheritedConstructor(const CXXConstructorDecl *BaseCtor);

  CXXConstructorDecl *getCanonicalDecl() override {
    return cast<CXXConstructorDecl>(FunctionDecl::getCanonicalDecl());
  }
  const CXXConstructorDecl *getCanonicalDecl() const {
    return const_cast<CXXConstructorDecl*>(this)->getCanonicalDecl();
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == CXXConstructor; }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
};

/// \brief Represents a C++ destructor within a class.
///
/// For example:
///
/// \code
/// class X {
/// public:
///   ~X(); // represented by a CXXDestructorDecl.
/// };
/// \endcode
class CXXDestructorDecl : public CXXMethodDecl {
  void anchor() override;

  FunctionDecl *OperatorDelete;

  CXXDestructorDecl(ASTContext &C, CXXRecordDecl *RD, SourceLocation StartLoc,
                    const DeclarationNameInfo &NameInfo,
                    QualType T, TypeSourceInfo *TInfo,
                    bool isInline, bool isImplicitlyDeclared)
    : CXXMethodDecl(CXXDestructor, C, RD, StartLoc, NameInfo, T, TInfo,
                    SC_None, isInline, /*isConstexpr=*/false, SourceLocation()),
      OperatorDelete(nullptr) {
    setImplicit(isImplicitlyDeclared);
  }

public:
  static CXXDestructorDecl *Create(ASTContext &C, CXXRecordDecl *RD,
                                   SourceLocation StartLoc,
                                   const DeclarationNameInfo &NameInfo,
                                   QualType T, TypeSourceInfo* TInfo,
                                   bool isInline,
                                   bool isImplicitlyDeclared);
  static CXXDestructorDecl *CreateDeserialized(ASTContext & C, unsigned ID);

  void setOperatorDelete(FunctionDecl *OD);
  const FunctionDecl *getOperatorDelete() const {
    return cast<CXXDestructorDecl>(getFirstDecl())->OperatorDelete;
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == CXXDestructor; }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
};

/// \brief Represents a C++ conversion function within a class.
///
/// For example:
///
/// \code
/// class X {
/// public:
///   operator bool();
/// };
/// \endcode
class CXXConversionDecl : public CXXMethodDecl {
  void anchor() override;
  /// Whether this conversion function declaration is marked
  /// "explicit", meaning that it can only be applied when the user
  /// explicitly wrote a cast. This is a C++0x feature.
  bool IsExplicitSpecified : 1;

  CXXConversionDecl(ASTContext &C, CXXRecordDecl *RD, SourceLocation StartLoc,
                    const DeclarationNameInfo &NameInfo,
                    QualType T, TypeSourceInfo *TInfo,
                    bool isInline, bool isExplicitSpecified,
                    bool isConstexpr, SourceLocation EndLocation)
    : CXXMethodDecl(CXXConversion, C, RD, StartLoc, NameInfo, T, TInfo,
                    SC_None, isInline, isConstexpr, EndLocation),
      IsExplicitSpecified(isExplicitSpecified) { }

public:
  static CXXConversionDecl *Create(ASTContext &C, CXXRecordDecl *RD,
                                   SourceLocation StartLoc,
                                   const DeclarationNameInfo &NameInfo,
                                   QualType T, TypeSourceInfo *TInfo,
                                   bool isInline, bool isExplicit,
                                   bool isConstexpr,
                                   SourceLocation EndLocation);
  static CXXConversionDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  /// Whether this conversion function declaration is marked
  /// "explicit", meaning that it can only be used for direct initialization
  /// (including explitly written casts).  This is a C++11 feature.
  bool isExplicitSpecified() const { return IsExplicitSpecified; }

  /// \brief Whether this is an explicit conversion operator (C++11 and later).
  ///
  /// Explicit conversion operators are only considered for direct
  /// initialization, e.g., when the user has explicitly written a cast.
  bool isExplicit() const {
    return cast<CXXConversionDecl>(getFirstDecl())->isExplicitSpecified();
  }

  /// \brief Returns the type that this conversion function is converting to.
  QualType getConversionType() const {
    return getType()->getAs<FunctionType>()->getReturnType();
  }

  /// \brief Determine whether this conversion function is a conversion from
  /// a lambda closure type to a block pointer.
  bool isLambdaToBlockPointerConversion() const;
  
  // Implement isa/cast/dyncast/etc.
  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == CXXConversion; }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
};

/// \brief Represents a linkage specification. 
///
/// For example:
/// \code
///   extern "C" void foo();
/// \endcode
class LinkageSpecDecl : public Decl, public DeclContext {
  virtual void anchor();
public:
  /// \brief Represents the language in a linkage specification.
  ///
  /// The values are part of the serialization ABI for
  /// ASTs and cannot be changed without altering that ABI.  To help
  /// ensure a stable ABI for this, we choose the DW_LANG_ encodings
  /// from the dwarf standard.
  enum LanguageIDs {
    lang_c = /* DW_LANG_C */ 0x0002,
    lang_cxx = /* DW_LANG_C_plus_plus */ 0x0004
  };
private:
  /// \brief The language for this linkage specification.
  unsigned Language : 3;
  /// \brief True if this linkage spec has braces.
  ///
  /// This is needed so that hasBraces() returns the correct result while the
  /// linkage spec body is being parsed.  Once RBraceLoc has been set this is
  /// not used, so it doesn't need to be serialized.
  unsigned HasBraces : 1;
  /// \brief The source location for the extern keyword.
  SourceLocation ExternLoc;
  /// \brief The source location for the right brace (if valid).
  SourceLocation RBraceLoc;

  LinkageSpecDecl(DeclContext *DC, SourceLocation ExternLoc,
                  SourceLocation LangLoc, LanguageIDs lang, bool HasBraces)
    : Decl(LinkageSpec, DC, LangLoc), DeclContext(LinkageSpec),
      Language(lang), HasBraces(HasBraces), ExternLoc(ExternLoc),
      RBraceLoc(SourceLocation()) { }

public:
  static LinkageSpecDecl *Create(ASTContext &C, DeclContext *DC,
                                 SourceLocation ExternLoc,
                                 SourceLocation LangLoc, LanguageIDs Lang,
                                 bool HasBraces);
  static LinkageSpecDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  /// \brief Return the language specified by this linkage specification.
  LanguageIDs getLanguage() const { return LanguageIDs(Language); }
  /// \brief Set the language specified by this linkage specification.
  void setLanguage(LanguageIDs L) { Language = L; }

  /// \brief Determines whether this linkage specification had braces in
  /// its syntactic form.
  bool hasBraces() const {
    assert(!RBraceLoc.isValid() || HasBraces);
    return HasBraces;
  }

  SourceLocation getExternLoc() const { return ExternLoc; }
  SourceLocation getRBraceLoc() const { return RBraceLoc; }
  void setExternLoc(SourceLocation L) { ExternLoc = L; }
  void setRBraceLoc(SourceLocation L) {
    RBraceLoc = L;
    HasBraces = RBraceLoc.isValid();
  }

  SourceLocation getLocEnd() const LLVM_READONLY {
    if (hasBraces())
      return getRBraceLoc();
    // No braces: get the end location of the (only) declaration in context
    // (if present).
    return decls_empty() ? getLocation() : decls_begin()->getLocEnd();
  }

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(ExternLoc, getLocEnd());
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == LinkageSpec; }
  static DeclContext *castToDeclContext(const LinkageSpecDecl *D) {
    return static_cast<DeclContext *>(const_cast<LinkageSpecDecl*>(D));
  }
  static LinkageSpecDecl *castFromDeclContext(const DeclContext *DC) {
    return static_cast<LinkageSpecDecl *>(const_cast<DeclContext*>(DC));
  }
};

/// \brief Represents C++ using-directive.
///
/// For example:
/// \code
///    using namespace std;
/// \endcode
///
/// \note UsingDirectiveDecl should be Decl not NamedDecl, but we provide
/// artificial names for all using-directives in order to store
/// them in DeclContext effectively.
class UsingDirectiveDecl : public NamedDecl {
  void anchor() override;
  /// \brief The location of the \c using keyword.
  SourceLocation UsingLoc;

  /// \brief The location of the \c namespace keyword.
  SourceLocation NamespaceLoc;

  /// \brief The nested-name-specifier that precedes the namespace.
  NestedNameSpecifierLoc QualifierLoc;

  /// \brief The namespace nominated by this using-directive.
  NamedDecl *NominatedNamespace;

  /// Enclosing context containing both using-directive and nominated
  /// namespace.
  DeclContext *CommonAncestor;

  /// \brief Returns special DeclarationName used by using-directives.
  ///
  /// This is only used by DeclContext for storing UsingDirectiveDecls in
  /// its lookup structure.
  static DeclarationName getName() {
    return DeclarationName::getUsingDirectiveName();
  }

  UsingDirectiveDecl(DeclContext *DC, SourceLocation UsingLoc,
                     SourceLocation NamespcLoc,
                     NestedNameSpecifierLoc QualifierLoc,
                     SourceLocation IdentLoc,
                     NamedDecl *Nominated,
                     DeclContext *CommonAncestor)
    : NamedDecl(UsingDirective, DC, IdentLoc, getName()), UsingLoc(UsingLoc),
      NamespaceLoc(NamespcLoc), QualifierLoc(QualifierLoc),
      NominatedNamespace(Nominated), CommonAncestor(CommonAncestor) { }

public:
  /// \brief Retrieve the nested-name-specifier that qualifies the
  /// name of the namespace, with source-location information.
  NestedNameSpecifierLoc getQualifierLoc() const { return QualifierLoc; }

  /// \brief Retrieve the nested-name-specifier that qualifies the
  /// name of the namespace.
  NestedNameSpecifier *getQualifier() const {
    return QualifierLoc.getNestedNameSpecifier();
  }

  NamedDecl *getNominatedNamespaceAsWritten() { return NominatedNamespace; }
  const NamedDecl *getNominatedNamespaceAsWritten() const {
    return NominatedNamespace;
  }

  /// \brief Returns the namespace nominated by this using-directive.
  NamespaceDecl *getNominatedNamespace();

  const NamespaceDecl *getNominatedNamespace() const {
    return const_cast<UsingDirectiveDecl*>(this)->getNominatedNamespace();
  }

  /// \brief Returns the common ancestor context of this using-directive and
  /// its nominated namespace.
  DeclContext *getCommonAncestor() { return CommonAncestor; }
  const DeclContext *getCommonAncestor() const { return CommonAncestor; }

  /// \brief Return the location of the \c using keyword.
  SourceLocation getUsingLoc() const { return UsingLoc; }

  // FIXME: Could omit 'Key' in name.
  /// \brief Returns the location of the \c namespace keyword.
  SourceLocation getNamespaceKeyLocation() const { return NamespaceLoc; }

  /// \brief Returns the location of this using declaration's identifier.
  SourceLocation getIdentLocation() const { return getLocation(); }

  static UsingDirectiveDecl *Create(ASTContext &C, DeclContext *DC,
                                    SourceLocation UsingLoc,
                                    SourceLocation NamespaceLoc,
                                    NestedNameSpecifierLoc QualifierLoc,
                                    SourceLocation IdentLoc,
                                    NamedDecl *Nominated,
                                    DeclContext *CommonAncestor);
  static UsingDirectiveDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(UsingLoc, getLocation());
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == UsingDirective; }

  // Friend for getUsingDirectiveName.
  friend class DeclContext;

  friend class ASTDeclReader;
};

/// \brief Represents a C++ namespace alias.
///
/// For example:
///
/// \code
/// namespace Foo = Bar;
/// \endcode
class NamespaceAliasDecl : public NamedDecl,
                           public Redeclarable<NamespaceAliasDecl> {
  void anchor() override;

  /// \brief The location of the \c namespace keyword.
  SourceLocation NamespaceLoc;

  /// \brief The location of the namespace's identifier.
  ///
  /// This is accessed by TargetNameLoc.
  SourceLocation IdentLoc;

  /// \brief The nested-name-specifier that precedes the namespace.
  NestedNameSpecifierLoc QualifierLoc;

  /// \brief The Decl that this alias points to, either a NamespaceDecl or
  /// a NamespaceAliasDecl.
  NamedDecl *Namespace;

  NamespaceAliasDecl(ASTContext &C, DeclContext *DC,
                     SourceLocation NamespaceLoc, SourceLocation AliasLoc,
                     IdentifierInfo *Alias, NestedNameSpecifierLoc QualifierLoc,
                     SourceLocation IdentLoc, NamedDecl *Namespace)
      : NamedDecl(NamespaceAlias, DC, AliasLoc, Alias), redeclarable_base(C),
        NamespaceLoc(NamespaceLoc), IdentLoc(IdentLoc),
        QualifierLoc(QualifierLoc), Namespace(Namespace) {}

  typedef Redeclarable<NamespaceAliasDecl> redeclarable_base;
  NamespaceAliasDecl *getNextRedeclarationImpl() override;
  NamespaceAliasDecl *getPreviousDeclImpl() override;
  NamespaceAliasDecl *getMostRecentDeclImpl() override;

  friend class ASTDeclReader;

public:
  static NamespaceAliasDecl *Create(ASTContext &C, DeclContext *DC,
                                    SourceLocation NamespaceLoc,
                                    SourceLocation AliasLoc,
                                    IdentifierInfo *Alias,
                                    NestedNameSpecifierLoc QualifierLoc,
                                    SourceLocation IdentLoc,
                                    NamedDecl *Namespace);

  static NamespaceAliasDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  typedef redeclarable_base::redecl_range redecl_range;
  typedef redeclarable_base::redecl_iterator redecl_iterator;
  using redeclarable_base::redecls_begin;
  using redeclarable_base::redecls_end;
  using redeclarable_base::redecls;
  using redeclarable_base::getPreviousDecl;
  using redeclarable_base::getMostRecentDecl;

  NamespaceAliasDecl *getCanonicalDecl() override {
    return getFirstDecl();
  }
  const NamespaceAliasDecl *getCanonicalDecl() const {
    return getFirstDecl();
  }

  /// \brief Retrieve the nested-name-specifier that qualifies the
  /// name of the namespace, with source-location information.
  NestedNameSpecifierLoc getQualifierLoc() const { return QualifierLoc; }

  /// \brief Retrieve the nested-name-specifier that qualifies the
  /// name of the namespace.
  NestedNameSpecifier *getQualifier() const {
    return QualifierLoc.getNestedNameSpecifier();
  }

  /// \brief Retrieve the namespace declaration aliased by this directive.
  NamespaceDecl *getNamespace() {
    if (NamespaceAliasDecl *AD = dyn_cast<NamespaceAliasDecl>(Namespace))
      return AD->getNamespace();

    return cast<NamespaceDecl>(Namespace);
  }

  const NamespaceDecl *getNamespace() const {
    return const_cast<NamespaceAliasDecl*>(this)->getNamespace();
  }

  /// Returns the location of the alias name, i.e. 'foo' in
  /// "namespace foo = ns::bar;".
  SourceLocation getAliasLoc() const { return getLocation(); }

  /// Returns the location of the \c namespace keyword.
  SourceLocation getNamespaceLoc() const { return NamespaceLoc; }

  /// Returns the location of the identifier in the named namespace.
  SourceLocation getTargetNameLoc() const { return IdentLoc; }

  /// \brief Retrieve the namespace that this alias refers to, which
  /// may either be a NamespaceDecl or a NamespaceAliasDecl.
  NamedDecl *getAliasedNamespace() const { return Namespace; }

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(NamespaceLoc, IdentLoc);
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == NamespaceAlias; }
};

/// \brief Represents a shadow declaration introduced into a scope by a
/// (resolved) using declaration.
///
/// For example,
/// \code
/// namespace A {
///   void foo();
/// }
/// namespace B {
///   using A::foo; // <- a UsingDecl
///                 // Also creates a UsingShadowDecl for A::foo() in B
/// }
/// \endcode
class UsingShadowDecl : public NamedDecl, public Redeclarable<UsingShadowDecl> {
  void anchor() override;

  /// The referenced declaration.
  NamedDecl *Underlying;

  /// \brief The using declaration which introduced this decl or the next using
  /// shadow declaration contained in the aforementioned using declaration.
  NamedDecl *UsingOrNextShadow;
  friend class UsingDecl;

  UsingShadowDecl(ASTContext &C, DeclContext *DC, SourceLocation Loc,
                  UsingDecl *Using, NamedDecl *Target)
    : NamedDecl(UsingShadow, DC, Loc, DeclarationName()),
      redeclarable_base(C), Underlying(Target),
      UsingOrNextShadow(reinterpret_cast<NamedDecl *>(Using)) {
    if (Target) {
      setDeclName(Target->getDeclName());
      IdentifierNamespace = Target->getIdentifierNamespace();
    }
    setImplicit();
  }

  typedef Redeclarable<UsingShadowDecl> redeclarable_base;
  UsingShadowDecl *getNextRedeclarationImpl() override {
    return getNextRedeclaration();
  }
  UsingShadowDecl *getPreviousDeclImpl() override {
    return getPreviousDecl();
  }
  UsingShadowDecl *getMostRecentDeclImpl() override {
    return getMostRecentDecl();
  }

public:
  static UsingShadowDecl *Create(ASTContext &C, DeclContext *DC,
                                 SourceLocation Loc, UsingDecl *Using,
                                 NamedDecl *Target) {
    return new (C, DC) UsingShadowDecl(C, DC, Loc, Using, Target);
  }

  static UsingShadowDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  typedef redeclarable_base::redecl_range redecl_range;
  typedef redeclarable_base::redecl_iterator redecl_iterator;
  using redeclarable_base::redecls_begin;
  using redeclarable_base::redecls_end;
  using redeclarable_base::redecls;
  using redeclarable_base::getPreviousDecl;
  using redeclarable_base::getMostRecentDecl;

  UsingShadowDecl *getCanonicalDecl() override {
    return getFirstDecl();
  }
  const UsingShadowDecl *getCanonicalDecl() const {
    return getFirstDecl();
  }

  /// \brief Gets the underlying declaration which has been brought into the
  /// local scope.
  NamedDecl *getTargetDecl() const { return Underlying; }

  /// \brief Sets the underlying declaration which has been brought into the
  /// local scope.
  void setTargetDecl(NamedDecl* ND) {
    assert(ND && "Target decl is null!");
    Underlying = ND;
    IdentifierNamespace = ND->getIdentifierNamespace();
  }

  /// \brief Gets the using declaration to which this declaration is tied.
  UsingDecl *getUsingDecl() const;

  /// \brief The next using shadow declaration contained in the shadow decl
  /// chain of the using declaration which introduced this decl.
  UsingShadowDecl *getNextUsingShadowDecl() const {
    return dyn_cast_or_null<UsingShadowDecl>(UsingOrNextShadow);
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Decl::UsingShadow; }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
};

/// \brief Represents a C++ using-declaration.
///
/// For example:
/// \code
///    using someNameSpace::someIdentifier;
/// \endcode
class UsingDecl : public NamedDecl, public Mergeable<UsingDecl> {
  void anchor() override;

  /// \brief The source location of the 'using' keyword itself.
  SourceLocation UsingLocation;

  /// \brief The nested-name-specifier that precedes the name.
  NestedNameSpecifierLoc QualifierLoc;

  /// \brief Provides source/type location info for the declaration name
  /// embedded in the ValueDecl base class.
  DeclarationNameLoc DNLoc;

  /// \brief The first shadow declaration of the shadow decl chain associated
  /// with this using declaration.
  ///
  /// The bool member of the pair store whether this decl has the \c typename
  /// keyword.
  llvm::PointerIntPair<UsingShadowDecl *, 1, bool> FirstUsingShadow;

  UsingDecl(DeclContext *DC, SourceLocation UL,
            NestedNameSpecifierLoc QualifierLoc,
            const DeclarationNameInfo &NameInfo, bool HasTypenameKeyword)
    : NamedDecl(Using, DC, NameInfo.getLoc(), NameInfo.getName()),
      UsingLocation(UL), QualifierLoc(QualifierLoc),
      DNLoc(NameInfo.getInfo()), FirstUsingShadow(nullptr, HasTypenameKeyword) {
  }

public:
  /// \brief Return the source location of the 'using' keyword.
  SourceLocation getUsingLoc() const { return UsingLocation; }

  /// \brief Set the source location of the 'using' keyword.
  void setUsingLoc(SourceLocation L) { UsingLocation = L; }

  /// \brief Retrieve the nested-name-specifier that qualifies the name,
  /// with source-location information.
  NestedNameSpecifierLoc getQualifierLoc() const { return QualifierLoc; }

  /// \brief Retrieve the nested-name-specifier that qualifies the name.
  NestedNameSpecifier *getQualifier() const {
    return QualifierLoc.getNestedNameSpecifier();
  }

  DeclarationNameInfo getNameInfo() const {
    return DeclarationNameInfo(getDeclName(), getLocation(), DNLoc);
  }

  /// \brief Return true if it is a C++03 access declaration (no 'using').
  bool isAccessDeclaration() const { return UsingLocation.isInvalid(); }

  /// \brief Return true if the using declaration has 'typename'.
  bool hasTypename() const { return FirstUsingShadow.getInt(); }

  /// \brief Sets whether the using declaration has 'typename'.
  void setTypename(bool TN) { FirstUsingShadow.setInt(TN); }

  /// \brief Iterates through the using shadow declarations associated with
  /// this using declaration.
  class shadow_iterator {
    /// \brief The current using shadow declaration.
    UsingShadowDecl *Current;

  public:
    typedef UsingShadowDecl*          value_type;
    typedef UsingShadowDecl*          reference;
    typedef UsingShadowDecl*          pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef std::ptrdiff_t            difference_type;

    shadow_iterator() : Current(nullptr) { }
    explicit shadow_iterator(UsingShadowDecl *C) : Current(C) { }

    reference operator*() const { return Current; }
    pointer operator->() const { return Current; }

    shadow_iterator& operator++() {
      Current = Current->getNextUsingShadowDecl();
      return *this;
    }

    shadow_iterator operator++(int) {
      shadow_iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    friend bool operator==(shadow_iterator x, shadow_iterator y) {
      return x.Current == y.Current;
    }
    friend bool operator!=(shadow_iterator x, shadow_iterator y) {
      return x.Current != y.Current;
    }
  };

  typedef llvm::iterator_range<shadow_iterator> shadow_range;

  shadow_range shadows() const {
    return shadow_range(shadow_begin(), shadow_end());
  }
  shadow_iterator shadow_begin() const {
    return shadow_iterator(FirstUsingShadow.getPointer());
  }
  shadow_iterator shadow_end() const { return shadow_iterator(); }

  /// \brief Return the number of shadowed declarations associated with this
  /// using declaration.
  unsigned shadow_size() const {
    return std::distance(shadow_begin(), shadow_end());
  }

  void addShadowDecl(UsingShadowDecl *S);
  void removeShadowDecl(UsingShadowDecl *S);

  static UsingDecl *Create(ASTContext &C, DeclContext *DC,
                           SourceLocation UsingL,
                           NestedNameSpecifierLoc QualifierLoc,
                           const DeclarationNameInfo &NameInfo,
                           bool HasTypenameKeyword);

  static UsingDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  SourceRange getSourceRange() const override LLVM_READONLY;

  /// Retrieves the canonical declaration of this declaration.
  UsingDecl *getCanonicalDecl() override { return getFirstDecl(); }
  const UsingDecl *getCanonicalDecl() const { return getFirstDecl(); }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == Using; }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
};

/// \brief Represents a dependent using declaration which was not marked with
/// \c typename.
///
/// Unlike non-dependent using declarations, these *only* bring through
/// non-types; otherwise they would break two-phase lookup.
///
/// \code
/// template \<class T> class A : public Base<T> {
///   using Base<T>::foo;
/// };
/// \endcode
class UnresolvedUsingValueDecl : public ValueDecl,
                                 public Mergeable<UnresolvedUsingValueDecl> {
  void anchor() override;

  /// \brief The source location of the 'using' keyword
  SourceLocation UsingLocation;

  /// \brief The nested-name-specifier that precedes the name.
  NestedNameSpecifierLoc QualifierLoc;

  /// \brief Provides source/type location info for the declaration name
  /// embedded in the ValueDecl base class.
  DeclarationNameLoc DNLoc;

  UnresolvedUsingValueDecl(DeclContext *DC, QualType Ty,
                           SourceLocation UsingLoc,
                           NestedNameSpecifierLoc QualifierLoc,
                           const DeclarationNameInfo &NameInfo)
    : ValueDecl(UnresolvedUsingValue, DC,
                NameInfo.getLoc(), NameInfo.getName(), Ty),
      UsingLocation(UsingLoc), QualifierLoc(QualifierLoc),
      DNLoc(NameInfo.getInfo())
  { }

public:
  /// \brief Returns the source location of the 'using' keyword.
  SourceLocation getUsingLoc() const { return UsingLocation; }

  /// \brief Set the source location of the 'using' keyword.
  void setUsingLoc(SourceLocation L) { UsingLocation = L; }

  /// \brief Return true if it is a C++03 access declaration (no 'using').
  bool isAccessDeclaration() const { return UsingLocation.isInvalid(); }

  /// \brief Retrieve the nested-name-specifier that qualifies the name,
  /// with source-location information.
  NestedNameSpecifierLoc getQualifierLoc() const { return QualifierLoc; }

  /// \brief Retrieve the nested-name-specifier that qualifies the name.
  NestedNameSpecifier *getQualifier() const {
    return QualifierLoc.getNestedNameSpecifier();
  }

  DeclarationNameInfo getNameInfo() const {
    return DeclarationNameInfo(getDeclName(), getLocation(), DNLoc);
  }

  static UnresolvedUsingValueDecl *
    Create(ASTContext &C, DeclContext *DC, SourceLocation UsingLoc,
           NestedNameSpecifierLoc QualifierLoc,
           const DeclarationNameInfo &NameInfo);

  static UnresolvedUsingValueDecl *
  CreateDeserialized(ASTContext &C, unsigned ID);

  SourceRange getSourceRange() const override LLVM_READONLY;

  /// Retrieves the canonical declaration of this declaration.
  UnresolvedUsingValueDecl *getCanonicalDecl() override {
    return getFirstDecl();
  }
  const UnresolvedUsingValueDecl *getCanonicalDecl() const {
    return getFirstDecl();
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == UnresolvedUsingValue; }

  friend class ASTDeclReader;
  friend class ASTDeclWriter;
};

/// \brief Represents a dependent using declaration which was marked with
/// \c typename.
///
/// \code
/// template \<class T> class A : public Base<T> {
///   using typename Base<T>::foo;
/// };
/// \endcode
///
/// The type associated with an unresolved using typename decl is
/// currently always a typename type.
class UnresolvedUsingTypenameDecl
    : public TypeDecl,
      public Mergeable<UnresolvedUsingTypenameDecl> {
  void anchor() override;

  /// \brief The source location of the 'typename' keyword
  SourceLocation TypenameLocation;

  /// \brief The nested-name-specifier that precedes the name.
  NestedNameSpecifierLoc QualifierLoc;

  UnresolvedUsingTypenameDecl(DeclContext *DC, SourceLocation UsingLoc,
                              SourceLocation TypenameLoc,
                              NestedNameSpecifierLoc QualifierLoc,
                              SourceLocation TargetNameLoc,
                              IdentifierInfo *TargetName)
    : TypeDecl(UnresolvedUsingTypename, DC, TargetNameLoc, TargetName,
               UsingLoc),
      TypenameLocation(TypenameLoc), QualifierLoc(QualifierLoc) { }

  friend class ASTDeclReader;

public:
  /// \brief Returns the source location of the 'using' keyword.
  SourceLocation getUsingLoc() const { return getLocStart(); }

  /// \brief Returns the source location of the 'typename' keyword.
  SourceLocation getTypenameLoc() const { return TypenameLocation; }

  /// \brief Retrieve the nested-name-specifier that qualifies the name,
  /// with source-location information.
  NestedNameSpecifierLoc getQualifierLoc() const { return QualifierLoc; }

  /// \brief Retrieve the nested-name-specifier that qualifies the name.
  NestedNameSpecifier *getQualifier() const {
    return QualifierLoc.getNestedNameSpecifier();
  }

  static UnresolvedUsingTypenameDecl *
    Create(ASTContext &C, DeclContext *DC, SourceLocation UsingLoc,
           SourceLocation TypenameLoc, NestedNameSpecifierLoc QualifierLoc,
           SourceLocation TargetNameLoc, DeclarationName TargetName);

  static UnresolvedUsingTypenameDecl *
  CreateDeserialized(ASTContext &C, unsigned ID);

  /// Retrieves the canonical declaration of this declaration.
  UnresolvedUsingTypenameDecl *getCanonicalDecl() override {
    return getFirstDecl();
  }
  const UnresolvedUsingTypenameDecl *getCanonicalDecl() const {
    return getFirstDecl();
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == UnresolvedUsingTypename; }
};

/// \brief Represents a C++11 static_assert declaration.
class StaticAssertDecl : public Decl {
  virtual void anchor();
  llvm::PointerIntPair<Expr *, 1, bool> AssertExprAndFailed;
  StringLiteral *Message;
  SourceLocation RParenLoc;

  StaticAssertDecl(DeclContext *DC, SourceLocation StaticAssertLoc,
                   Expr *AssertExpr, StringLiteral *Message,
                   SourceLocation RParenLoc, bool Failed)
    : Decl(StaticAssert, DC, StaticAssertLoc),
      AssertExprAndFailed(AssertExpr, Failed), Message(Message),
      RParenLoc(RParenLoc) { }

public:
  static StaticAssertDecl *Create(ASTContext &C, DeclContext *DC,
                                  SourceLocation StaticAssertLoc,
                                  Expr *AssertExpr, StringLiteral *Message,
                                  SourceLocation RParenLoc, bool Failed);
  static StaticAssertDecl *CreateDeserialized(ASTContext &C, unsigned ID);
  
  Expr *getAssertExpr() { return AssertExprAndFailed.getPointer(); }
  const Expr *getAssertExpr() const { return AssertExprAndFailed.getPointer(); }

  StringLiteral *getMessage() { return Message; }
  const StringLiteral *getMessage() const { return Message; }

  bool isFailed() const { return AssertExprAndFailed.getInt(); }

  SourceLocation getRParenLoc() const { return RParenLoc; }

  SourceRange getSourceRange() const override LLVM_READONLY {
    return SourceRange(getLocation(), getRParenLoc());
  }

  static bool classof(const Decl *D) { return classofKind(D->getKind()); }
  static bool classofKind(Kind K) { return K == StaticAssert; }

  friend class ASTDeclReader;
};

/// An instance of this class represents the declaration of a property
/// member.  This is a Microsoft extension to C++, first introduced in
/// Visual Studio .NET 2003 as a parallel to similar features in C#
/// and Managed C++.
///
/// A property must always be a non-static class member.
///
/// A property member superficially resembles a non-static data
/// member, except preceded by a property attribute:
///   __declspec(property(get=GetX, put=PutX)) int x;
/// Either (but not both) of the 'get' and 'put' names may be omitted.
///
/// A reference to a property is always an lvalue.  If the lvalue
/// undergoes lvalue-to-rvalue conversion, then a getter name is
/// required, and that member is called with no arguments.
/// If the lvalue is assigned into, then a setter name is required,
/// and that member is called with one argument, the value assigned.
/// Both operations are potentially overloaded.  Compound assignments
/// are permitted, as are the increment and decrement operators.
///
/// The getter and putter methods are permitted to be overloaded,
/// although their return and parameter types are subject to certain
/// restrictions according to the type of the property.
///
/// A property declared using an incomplete array type may
/// additionally be subscripted, adding extra parameters to the getter
/// and putter methods.
class MSPropertyDecl : public DeclaratorDecl {
  IdentifierInfo *GetterId, *SetterId;

  MSPropertyDecl(DeclContext *DC, SourceLocation L, DeclarationName N,
                 QualType T, TypeSourceInfo *TInfo, SourceLocation StartL,
                 IdentifierInfo *Getter, IdentifierInfo *Setter)
      : DeclaratorDecl(MSProperty, DC, L, N, T, TInfo, StartL),
        GetterId(Getter), SetterId(Setter) {}

public:
  static MSPropertyDecl *Create(ASTContext &C, DeclContext *DC,
                                SourceLocation L, DeclarationName N, QualType T,
                                TypeSourceInfo *TInfo, SourceLocation StartL,
                                IdentifierInfo *Getter, IdentifierInfo *Setter);
  static MSPropertyDecl *CreateDeserialized(ASTContext &C, unsigned ID);

  static bool classof(const Decl *D) { return D->getKind() == MSProperty; }

  bool hasGetter() const { return GetterId != nullptr; }
  IdentifierInfo* getGetterId() const { return GetterId; }
  bool hasSetter() const { return SetterId != nullptr; }
  IdentifierInfo* getSetterId() const { return SetterId; }

  friend class ASTDeclReader;
};

/// Insertion operator for diagnostics.  This allows sending an AccessSpecifier
/// into a diagnostic with <<.
const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    AccessSpecifier AS);

const PartialDiagnostic &operator<<(const PartialDiagnostic &DB,
                                    AccessSpecifier AS);

} // end namespace clang

#endif
