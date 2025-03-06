//===-- CanonicalType.h - C Language Family Type Representation -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the CanQual class template, which provides access to
//  canonical types.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_CANONICALTYPE_H
#define LLVM_CLANG_AST_CANONICALTYPE_H

#include "clang/AST/Type.h"
#include "llvm/ADT/iterator.h"
#include "llvm/Support/Casting.h"

namespace clang {

template<typename T> class CanProxy;
template<typename T> struct CanProxyAdaptor;

//----------------------------------------------------------------------------//
// Canonical, qualified type template
//----------------------------------------------------------------------------//

/// \brief Represents a canonical, potentially-qualified type.
///
/// The CanQual template is a lightweight smart pointer that provides access
/// to the canonical representation of a type, where all typedefs and other
/// syntactic sugar has been eliminated. A CanQualType may also have various
/// qualifiers (const, volatile, restrict) attached to it.
///
/// The template type parameter @p T is one of the Type classes (PointerType,
/// BuiltinType, etc.). The type stored within @c CanQual<T> will be of that
/// type (or some subclass of that type). The typedef @c CanQualType is just
/// a shorthand for @c CanQual<Type>.
///
/// An instance of @c CanQual<T> can be implicitly converted to a
/// @c CanQual<U> when T is derived from U, which essentially provides an
/// implicit upcast. For example, @c CanQual<LValueReferenceType> can be
/// converted to @c CanQual<ReferenceType>. Note that any @c CanQual type can
/// be implicitly converted to a QualType, but the reverse operation requires
/// a call to ASTContext::getCanonicalType().
///
///
template<typename T = Type>
class CanQual {
  /// \brief The actual, canonical type.
  QualType Stored;

public:
  /// \brief Constructs a NULL canonical type.
  CanQual() : Stored() { }

  /// \brief Converting constructor that permits implicit upcasting of
  /// canonical type pointers.
  template <typename U>
  CanQual(const CanQual<U> &Other,
          typename std::enable_if<std::is_base_of<T, U>::value, int>::type = 0);

  /// \brief Retrieve the underlying type pointer, which refers to a
  /// canonical type.
  ///
  /// The underlying pointer must not be NULL.
  const T *getTypePtr() const { return cast<T>(Stored.getTypePtr()); }

  /// \brief Retrieve the underlying type pointer, which refers to a
  /// canonical type, or NULL.
  ///
  const T *getTypePtrOrNull() const { 
    return cast_or_null<T>(Stored.getTypePtrOrNull()); 
  }

  /// \brief Implicit conversion to a qualified type.
  operator QualType() const { return Stored; }

  /// \brief Implicit conversion to bool.
  explicit operator bool() const { return !isNull(); }
  
  bool isNull() const {
    return Stored.isNull();
  }

  SplitQualType split() const { return Stored.split(); }

  /// \brief Retrieve a canonical type pointer with a different static type,
  /// upcasting or downcasting as needed.
  ///
  /// The getAs() function is typically used to try to downcast to a
  /// more specific (canonical) type in the type system. For example:
  ///
  /// @code
  /// void f(CanQual<Type> T) {
  ///   if (CanQual<PointerType> Ptr = T->getAs<PointerType>()) {
  ///     // look at Ptr's pointee type
  ///   }
  /// }
  /// @endcode
  ///
  /// \returns A proxy pointer to the same type, but with the specified
  /// static type (@p U). If the dynamic type is not the specified static type
  /// or a derived class thereof, a NULL canonical type.
  template<typename U> CanProxy<U> getAs() const;

  template<typename U> CanProxy<U> castAs() const;

  /// \brief Overloaded arrow operator that produces a canonical type
  /// proxy.
  CanProxy<T> operator->() const;

  /// \brief Retrieve all qualifiers.
  Qualifiers getQualifiers() const { return Stored.getLocalQualifiers(); }

  /// \brief Retrieve the const/volatile/restrict qualifiers.
  unsigned getCVRQualifiers() const { return Stored.getLocalCVRQualifiers(); }

  /// \brief Determines whether this type has any qualifiers
  bool hasQualifiers() const { return Stored.hasLocalQualifiers(); }

  bool isConstQualified() const {
    return Stored.isLocalConstQualified();
  }
  bool isVolatileQualified() const {
    return Stored.isLocalVolatileQualified();
  }
  bool isRestrictQualified() const {
    return Stored.isLocalRestrictQualified();
  }

  /// \brief Determines if this canonical type is furthermore
  /// canonical as a parameter.  The parameter-canonicalization
  /// process decays arrays to pointers and drops top-level qualifiers.
  bool isCanonicalAsParam() const {
    return Stored.isCanonicalAsParam();
  }

  /// \brief Retrieve the unqualified form of this type.
  CanQual<T> getUnqualifiedType() const;

  /// \brief Retrieves a version of this type with const applied.
  /// Note that this does not always yield a canonical type.
  QualType withConst() const {
    return Stored.withConst();
  }

  /// \brief Determines whether this canonical type is more qualified than
  /// the @p Other canonical type.
  bool isMoreQualifiedThan(CanQual<T> Other) const {
    return Stored.isMoreQualifiedThan(Other.Stored);
  }

  /// \brief Determines whether this canonical type is at least as qualified as
  /// the @p Other canonical type.
  bool isAtLeastAsQualifiedAs(CanQual<T> Other) const {
    return Stored.isAtLeastAsQualifiedAs(Other.Stored);
  }

  /// \brief If the canonical type is a reference type, returns the type that
  /// it refers to; otherwise, returns the type itself.
  CanQual<Type> getNonReferenceType() const;

  /// \brief Retrieve the internal representation of this canonical type.
  void *getAsOpaquePtr() const { return Stored.getAsOpaquePtr(); }

  /// \brief Construct a canonical type from its internal representation.
  static CanQual<T> getFromOpaquePtr(void *Ptr);

  /// \brief Builds a canonical type from a QualType.
  ///
  /// This routine is inherently unsafe, because it requires the user to
  /// ensure that the given type is a canonical type with the correct
  // (dynamic) type.
  static CanQual<T> CreateUnsafe(QualType Other);

  void dump() const { Stored.dump(); }

  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddPointer(getAsOpaquePtr());
  }
};

template<typename T, typename U>
inline bool operator==(CanQual<T> x, CanQual<U> y) {
  return x.getAsOpaquePtr() == y.getAsOpaquePtr();
}

template<typename T, typename U>
inline bool operator!=(CanQual<T> x, CanQual<U> y) {
  return x.getAsOpaquePtr() != y.getAsOpaquePtr();
}

/// \brief Represents a canonical, potentially-qualified type.
typedef CanQual<Type> CanQualType;

inline CanQualType Type::getCanonicalTypeUnqualified() const {
  return CanQualType::CreateUnsafe(getCanonicalTypeInternal());
}

inline const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                           CanQualType T) {
  DB << static_cast<QualType>(T);
  return DB;
}

//----------------------------------------------------------------------------//
// Internal proxy classes used by canonical types
//----------------------------------------------------------------------------//

#define LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(Accessor)                    \
CanQualType Accessor() const {                                           \
return CanQualType::CreateUnsafe(this->getTypePtr()->Accessor());      \
}

#define LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(Type, Accessor)             \
Type Accessor() const { return this->getTypePtr()->Accessor(); }

/// \brief Base class of all canonical proxy types, which is responsible for
/// storing the underlying canonical type and providing basic conversions.
template<typename T>
class CanProxyBase {
protected:
  CanQual<T> Stored;

public:
  /// \brief Retrieve the pointer to the underlying Type
  const T *getTypePtr() const { return Stored.getTypePtr(); }

  /// \brief Implicit conversion to the underlying pointer.
  ///
  /// Also provides the ability to use canonical type proxies in a Boolean
  // context,e.g.,
  /// @code
  ///   if (CanQual<PointerType> Ptr = T->getAs<PointerType>()) { ... }
  /// @endcode
  operator const T*() const { return this->Stored.getTypePtrOrNull(); }

  /// \brief Try to convert the given canonical type to a specific structural
  /// type.
  template<typename U> CanProxy<U> getAs() const {
    return this->Stored.template getAs<U>();
  }

  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(Type::TypeClass, getTypeClass)

  // Type predicates
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjectType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIncompleteType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIncompleteOrObjectType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isVariablyModifiedType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isEnumeralType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isBooleanType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isCharType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isWideCharType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegralType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegralOrEnumerationType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isRealFloatingType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isComplexType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isAnyComplexType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isFloatingType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isRealType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isArithmeticType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isVoidType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isDerivedType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isScalarType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isAggregateType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isAnyPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isVoidPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isFunctionPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isMemberFunctionPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isClassType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isStructureType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isInterfaceType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isStructureOrClassType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isUnionType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isComplexIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isNullPtrType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isDependentType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isOverloadableType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isArrayType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasPointerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasObjCPointerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasIntegerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasSignedIntegerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasUnsignedIntegerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasFloatingRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isPromotableIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isSignedIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isUnsignedIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isSignedIntegerOrEnumerationType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isUnsignedIntegerOrEnumerationType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isConstantSizeType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isSpecifierType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(CXXRecordDecl*, getAsCXXRecordDecl)

  /// \brief Retrieve the proxy-adaptor type.
  ///
  /// This arrow operator is used when CanProxyAdaptor has been specialized
  /// for the given type T. In that case, we reference members of the
  /// CanProxyAdaptor specialization. Otherwise, this operator will be hidden
  /// by the arrow operator in the primary CanProxyAdaptor template.
  const CanProxyAdaptor<T> *operator->() const {
    return static_cast<const CanProxyAdaptor<T> *>(this);
  }
};

/// \brief Replacable canonical proxy adaptor class that provides the link
/// between a canonical type and the accessors of the type.
///
/// The CanProxyAdaptor is a replaceable class template that is instantiated
/// as part of each canonical proxy type. The primary template merely provides
/// redirection to the underlying type (T), e.g., @c PointerType. One can
/// provide specializations of this class template for each underlying type
/// that provide accessors returning canonical types (@c CanQualType) rather
/// than the more typical @c QualType, to propagate the notion of "canonical"
/// through the system.
template<typename T>
struct CanProxyAdaptor : CanProxyBase<T> { };

/// \brief Canonical proxy type returned when retrieving the members of a
/// canonical type or as the result of the @c CanQual<T>::getAs member
/// function.
///
/// The CanProxy type mainly exists as a proxy through which operator-> will
/// look to either map down to a raw T* (e.g., PointerType*) or to a proxy
/// type that provides canonical-type access to the fields of the type.
template<typename T>
class CanProxy : public CanProxyAdaptor<T> {
public:
  /// \brief Build a NULL proxy.
  CanProxy() { }

  /// \brief Build a proxy to the given canonical type.
  CanProxy(CanQual<T> Stored) { this->Stored = Stored; }

  /// \brief Implicit conversion to the stored canonical type.
  operator CanQual<T>() const { return this->Stored; }
};

} // end namespace clang

namespace llvm {

/// Implement simplify_type for CanQual<T>, so that we can dyn_cast from
/// CanQual<T> to a specific Type class. We're prefer isa/dyn_cast/cast/etc.
/// to return smart pointer (proxies?).
template<typename T>
struct simplify_type< ::clang::CanQual<T> > {
  typedef const T *SimpleType;
  static SimpleType getSimplifiedValue(::clang::CanQual<T> Val) {
    return Val.getTypePtr();
  }
};

// Teach SmallPtrSet that CanQual<T> is "basically a pointer".
template<typename T>
class PointerLikeTypeTraits<clang::CanQual<T> > {
public:
  static inline void *getAsVoidPointer(clang::CanQual<T> P) {
    return P.getAsOpaquePtr();
  }
  static inline clang::CanQual<T> getFromVoidPointer(void *P) {
    return clang::CanQual<T>::getFromOpaquePtr(P);
  }
  // qualifier information is encoded in the low bits.
  enum { NumLowBitsAvailable = 0 };
};

} // end namespace llvm

namespace clang {

//----------------------------------------------------------------------------//
// Canonical proxy adaptors for canonical type nodes.
//----------------------------------------------------------------------------//

/// \brief Iterator adaptor that turns an iterator over canonical QualTypes
/// into an iterator over CanQualTypes.
template <typename InputIterator>
struct CanTypeIterator
    : llvm::iterator_adaptor_base<
          CanTypeIterator<InputIterator>, InputIterator,
          typename std::iterator_traits<InputIterator>::iterator_category,
          CanQualType,
          typename std::iterator_traits<InputIterator>::difference_type,
          CanProxy<Type>, CanQualType> {
  CanTypeIterator() {}
  explicit CanTypeIterator(InputIterator Iter)
      : CanTypeIterator::iterator_adaptor_base(std::move(Iter)) {}

  CanQualType operator*() const { return CanQualType::CreateUnsafe(*this->I); }
  CanProxy<Type> operator->() const;
};

template<>
struct CanProxyAdaptor<ComplexType> : public CanProxyBase<ComplexType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getElementType)
};

template<>
struct CanProxyAdaptor<PointerType> : public CanProxyBase<PointerType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<BlockPointerType>
  : public CanProxyBase<BlockPointerType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<ReferenceType> : public CanProxyBase<ReferenceType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<LValueReferenceType>
  : public CanProxyBase<LValueReferenceType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<RValueReferenceType>
  : public CanProxyBase<RValueReferenceType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<MemberPointerType>
  : public CanProxyBase<MemberPointerType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getPointeeType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(const Type *, getClass)
};

// CanProxyAdaptors for arrays are intentionally unimplemented because
// they are not safe.
template<> struct CanProxyAdaptor<ArrayType>;
template<> struct CanProxyAdaptor<ConstantArrayType>;
template<> struct CanProxyAdaptor<IncompleteArrayType>;
template<> struct CanProxyAdaptor<VariableArrayType>;
template<> struct CanProxyAdaptor<DependentSizedArrayType>;

template<>
struct CanProxyAdaptor<DependentSizedExtVectorType>
  : public CanProxyBase<DependentSizedExtVectorType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getElementType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(const Expr *, getSizeExpr)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(SourceLocation, getAttributeLoc)
};

template<>
struct CanProxyAdaptor<VectorType> : public CanProxyBase<VectorType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getElementType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumElements)
};

template<>
struct CanProxyAdaptor<ExtVectorType> : public CanProxyBase<ExtVectorType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getElementType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumElements)
};

template<>
struct CanProxyAdaptor<FunctionType> : public CanProxyBase<FunctionType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getReturnType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(FunctionType::ExtInfo, getExtInfo)
};

template<>
struct CanProxyAdaptor<FunctionNoProtoType>
  : public CanProxyBase<FunctionNoProtoType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getReturnType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(FunctionType::ExtInfo, getExtInfo)
};

template<>
struct CanProxyAdaptor<FunctionProtoType>
  : public CanProxyBase<FunctionProtoType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getReturnType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(FunctionType::ExtInfo, getExtInfo)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumParams)
  CanQualType getParamType(unsigned i) const {
    return CanQualType::CreateUnsafe(this->getTypePtr()->getParamType(i));
  }

  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isVariadic)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getTypeQuals)

  typedef CanTypeIterator<FunctionProtoType::param_type_iterator>
  param_type_iterator;

  param_type_iterator param_type_begin() const {
    return param_type_iterator(this->getTypePtr()->param_type_begin());
  }

  param_type_iterator param_type_end() const {
    return param_type_iterator(this->getTypePtr()->param_type_end());
  }

  // Note: canonical function types never have exception specifications
};

template<>
struct CanProxyAdaptor<TypeOfType> : public CanProxyBase<TypeOfType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getUnderlyingType)
};

template<>
struct CanProxyAdaptor<DecltypeType> : public CanProxyBase<DecltypeType> {
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(Expr *, getUnderlyingExpr)
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getUnderlyingType)
};

template <>
struct CanProxyAdaptor<UnaryTransformType>
    : public CanProxyBase<UnaryTransformType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getBaseType)
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getUnderlyingType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(UnaryTransformType::UTTKind, getUTTKind)
};

template<>
struct CanProxyAdaptor<TagType> : public CanProxyBase<TagType> {
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(TagDecl *, getDecl)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isBeingDefined)
};

template<>
struct CanProxyAdaptor<RecordType> : public CanProxyBase<RecordType> {
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(RecordDecl *, getDecl)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isBeingDefined)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasConstFields)
};

template<>
struct CanProxyAdaptor<EnumType> : public CanProxyBase<EnumType> {
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(EnumDecl *, getDecl)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isBeingDefined)
};

template<>
struct CanProxyAdaptor<TemplateTypeParmType>
  : public CanProxyBase<TemplateTypeParmType> {
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getDepth)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getIndex)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isParameterPack)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(TemplateTypeParmDecl *, getDecl)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(IdentifierInfo *, getIdentifier)
};

template<>
struct CanProxyAdaptor<ObjCObjectType>
  : public CanProxyBase<ObjCObjectType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getBaseType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(const ObjCInterfaceDecl *,
                                      getInterface)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCUnqualifiedId)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCUnqualifiedClass)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedId)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedClass)

  typedef ObjCObjectPointerType::qual_iterator qual_iterator;
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_begin)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_end)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, qual_empty)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumProtocols)
};

template<>
struct CanProxyAdaptor<ObjCObjectPointerType>
  : public CanProxyBase<ObjCObjectPointerType> {
  LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(getPointeeType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(const ObjCInterfaceType *,
                                      getInterfaceType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCIdType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCClassType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedIdType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedClassType)

  typedef ObjCObjectPointerType::qual_iterator qual_iterator;
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_begin)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_end)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, qual_empty)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumProtocols)
};

//----------------------------------------------------------------------------//
// Method and function definitions
//----------------------------------------------------------------------------//
template<typename T>
inline CanQual<T> CanQual<T>::getUnqualifiedType() const {
  return CanQual<T>::CreateUnsafe(Stored.getLocalUnqualifiedType());
}

template<typename T>
inline CanQual<Type> CanQual<T>::getNonReferenceType() const {
  if (CanQual<ReferenceType> RefType = getAs<ReferenceType>())
    return RefType->getPointeeType();
  else
    return *this;
}

template<typename T>
CanQual<T> CanQual<T>::getFromOpaquePtr(void *Ptr) {
  CanQual<T> Result;
  Result.Stored = QualType::getFromOpaquePtr(Ptr);
  assert((!Result || Result.Stored.getAsOpaquePtr() == (void*)-1 ||
          Result.Stored.isCanonical()) && "Type is not canonical!");
  return Result;
}

template<typename T>
CanQual<T> CanQual<T>::CreateUnsafe(QualType Other) {
  assert((Other.isNull() || Other.isCanonical()) && "Type is not canonical!");
  assert((Other.isNull() || isa<T>(Other.getTypePtr())) &&
         "Dynamic type does not meet the static type's requires");
  CanQual<T> Result;
  Result.Stored = Other;
  return Result;
}

template<typename T>
template<typename U>
CanProxy<U> CanQual<T>::getAs() const {
  ArrayType_cannot_be_used_with_getAs<U> at;
  (void)at;

  if (Stored.isNull())
    return CanProxy<U>();

  if (isa<U>(Stored.getTypePtr()))
    return CanQual<U>::CreateUnsafe(Stored);

  return CanProxy<U>();
}

template<typename T>
template<typename U>
CanProxy<U> CanQual<T>::castAs() const {
  ArrayType_cannot_be_used_with_getAs<U> at;
  (void)at;

  assert(!Stored.isNull() && isa<U>(Stored.getTypePtr()));
  return CanQual<U>::CreateUnsafe(Stored);
}

template<typename T>
CanProxy<T> CanQual<T>::operator->() const {
  return CanProxy<T>(*this);
}

template <typename InputIterator>
CanProxy<Type> CanTypeIterator<InputIterator>::operator->() const {
  return CanProxy<Type>(*this);
}

}


#endif
