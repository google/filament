//===--- Expr.h - Classes for representing expressions ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the Expr interface and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_EXPR_H
#define LLVM_CLANG_AST_EXPR_H

#include "clang/AST/APValue.h"
#include "clang/AST/ASTVector.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclAccessPair.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/Type.h"
#include "clang/AST/HlslTypes.h" // HLSL Change
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/TypeTraits.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Compiler.h"

namespace clang {
  class APValue;
  class ASTContext;
  class BlockDecl;
  class CXXBaseSpecifier;
  class CXXMemberCallExpr;
  class CXXOperatorCallExpr;
  class CastExpr;
  class Decl;
  class IdentifierInfo;
  class MaterializeTemporaryExpr;
  class NamedDecl;
  class ObjCPropertyRefExpr;
  class OpaqueValueExpr;
  class ParmVarDecl;
  class StringLiteral;
  class TargetInfo;
  class ValueDecl;

/// \brief A simple array of base specifiers.
typedef SmallVector<CXXBaseSpecifier*, 4> CXXCastPath;

/// \brief An adjustment to be made to the temporary created when emitting a
/// reference binding, which accesses a particular subobject of that temporary.
struct SubobjectAdjustment {
  enum {
    DerivedToBaseAdjustment,
    FieldAdjustment,
    MemberPointerAdjustment
  } Kind;


  struct DTB {
    const CastExpr *BasePath;
    const CXXRecordDecl *DerivedClass;
  };

  struct P {
    const MemberPointerType *MPT;
    Expr *RHS;
  };

  union {
    struct DTB DerivedToBase;
    FieldDecl *Field;
    struct P Ptr;
  };

  SubobjectAdjustment(const CastExpr *BasePath,
                      const CXXRecordDecl *DerivedClass)
    : Kind(DerivedToBaseAdjustment) {
    DerivedToBase.BasePath = BasePath;
    DerivedToBase.DerivedClass = DerivedClass;
  }

  SubobjectAdjustment(FieldDecl *Field)
    : Kind(FieldAdjustment) {
    this->Field = Field;
  }

  SubobjectAdjustment(const MemberPointerType *MPT, Expr *RHS)
    : Kind(MemberPointerAdjustment) {
    this->Ptr.MPT = MPT;
    this->Ptr.RHS = RHS;
  }
};

/// Expr - This represents one expression.  Note that Expr's are subclasses of
/// Stmt.  This allows an expression to be transparently used any place a Stmt
/// is required.
///
class Expr : public Stmt {
  QualType TR;

protected:
  Expr(StmtClass SC, QualType T, ExprValueKind VK, ExprObjectKind OK,
       bool TD, bool VD, bool ID, bool ContainsUnexpandedParameterPack)
    : Stmt(SC)
  {
    ExprBits.TypeDependent = TD;
    ExprBits.ValueDependent = VD;
    ExprBits.InstantiationDependent = ID;
    ExprBits.ValueKind = VK;
    ExprBits.ObjectKind = OK;
    ExprBits.ContainsUnexpandedParameterPack = ContainsUnexpandedParameterPack;
    setType(T);
  }

  /// \brief Construct an empty expression.
  explicit Expr(StmtClass SC, EmptyShell) : Stmt(SC) { }

public:
  QualType getType() const { return TR; }
  void setType(QualType t) {
    // In C++, the type of an expression is always adjusted so that it
    // will not have reference type (C++ [expr]p6). Use
    // QualType::getNonReferenceType() to retrieve the non-reference
    // type. Additionally, inspect Expr::isLvalue to determine whether
    // an expression that is adjusted in this manner should be
    // considered an lvalue.
    assert((t.isNull() || !t->isReferenceType()) &&
           "Expressions can't have reference type");

    TR = t;
  }

  /// isValueDependent - Determines whether this expression is
  /// value-dependent (C++ [temp.dep.constexpr]). For example, the
  /// array bound of "Chars" in the following example is
  /// value-dependent.
  /// @code
  /// template<int Size, char (&Chars)[Size]> struct meta_string;
  /// @endcode
  bool isValueDependent() const { return ExprBits.ValueDependent; }

  /// \brief Set whether this expression is value-dependent or not.
  void setValueDependent(bool VD) {
    ExprBits.ValueDependent = VD;
    if (VD)
      ExprBits.InstantiationDependent = true;
  }

  /// isTypeDependent - Determines whether this expression is
  /// type-dependent (C++ [temp.dep.expr]), which means that its type
  /// could change from one template instantiation to the next. For
  /// example, the expressions "x" and "x + y" are type-dependent in
  /// the following code, but "y" is not type-dependent:
  /// @code
  /// template<typename T>
  /// void add(T x, int y) {
  ///   x + y;
  /// }
  /// @endcode
  bool isTypeDependent() const { return ExprBits.TypeDependent; }

  /// \brief Set whether this expression is type-dependent or not.
  void setTypeDependent(bool TD) {
    ExprBits.TypeDependent = TD;
    if (TD)
      ExprBits.InstantiationDependent = true;
  }

  /// \brief Whether this expression is instantiation-dependent, meaning that
  /// it depends in some way on a template parameter, even if neither its type
  /// nor (constant) value can change due to the template instantiation.
  ///
  /// In the following example, the expression \c sizeof(sizeof(T() + T())) is
  /// instantiation-dependent (since it involves a template parameter \c T), but
  /// is neither type- nor value-dependent, since the type of the inner
  /// \c sizeof is known (\c std::size_t) and therefore the size of the outer
  /// \c sizeof is known.
  ///
  /// \code
  /// template<typename T>
  /// void f(T x, T y) {
  ///   sizeof(sizeof(T() + T());
  /// }
  /// \endcode
  ///
  bool isInstantiationDependent() const {
    return ExprBits.InstantiationDependent;
  }

  /// \brief Set whether this expression is instantiation-dependent or not.
  void setInstantiationDependent(bool ID) {
    ExprBits.InstantiationDependent = ID;
  }

  /// \brief Whether this expression contains an unexpanded parameter
  /// pack (for C++11 variadic templates).
  ///
  /// Given the following function template:
  ///
  /// \code
  /// template<typename F, typename ...Types>
  /// void forward(const F &f, Types &&...args) {
  ///   f(static_cast<Types&&>(args)...);
  /// }
  /// \endcode
  ///
  /// The expressions \c args and \c static_cast<Types&&>(args) both
  /// contain parameter packs.
  bool containsUnexpandedParameterPack() const {
    return ExprBits.ContainsUnexpandedParameterPack;
  }

  /// \brief Set the bit that describes whether this expression
  /// contains an unexpanded parameter pack.
  void setContainsUnexpandedParameterPack(bool PP = true) {
    ExprBits.ContainsUnexpandedParameterPack = PP;
  }

  /// getExprLoc - Return the preferred location for the arrow when diagnosing
  /// a problem with a generic expression.
  SourceLocation getExprLoc() const LLVM_READONLY;

  /// isUnusedResultAWarning - Return true if this immediate expression should
  /// be warned about if the result is unused.  If so, fill in expr, location,
  /// and ranges with expr to warn on and source locations/ranges appropriate
  /// for a warning.
  bool isUnusedResultAWarning(const Expr *&WarnExpr, SourceLocation &Loc,
                              SourceRange &R1, SourceRange &R2,
                              ASTContext &Ctx) const;

  /// isLValue - True if this expression is an "l-value" according to
  /// the rules of the current language.  C and C++ give somewhat
  /// different rules for this concept, but in general, the result of
  /// an l-value expression identifies a specific object whereas the
  /// result of an r-value expression is a value detached from any
  /// specific storage.
  ///
  /// C++11 divides the concept of "r-value" into pure r-values
  /// ("pr-values") and so-called expiring values ("x-values"), which
  /// identify specific objects that can be safely cannibalized for
  /// their resources.  This is an unfortunate abuse of terminology on
  /// the part of the C++ committee.  In Clang, when we say "r-value",
  /// we generally mean a pr-value.
  bool isLValue() const { return getValueKind() == VK_LValue; }
  bool isRValue() const { return getValueKind() == VK_RValue; }
  bool isXValue() const { return getValueKind() == VK_XValue; }
  bool isGLValue() const { return getValueKind() != VK_RValue; }

  enum LValueClassification {
    LV_Valid,
    LV_NotObjectType,
    LV_IncompleteVoidType,
    LV_DuplicateVectorComponents,
    LV_DuplicateMatrixComponents, // HLSL Change
    LV_InvalidExpression,
    LV_InvalidMessageExpression,
    LV_MemberFunction,
    LV_SubObjCPropertySetting,
    LV_ClassTemporary,
    LV_ArrayTemporary
  };
  /// Reasons why an expression might not be an l-value.
  LValueClassification ClassifyLValue(ASTContext &Ctx) const;

  enum isModifiableLvalueResult {
    MLV_Valid,
    MLV_NotObjectType,
    MLV_IncompleteVoidType,
    MLV_DuplicateVectorComponents,
    MLV_DuplicateMatrixComponents, // HLSL Change
    MLV_InvalidExpression,
    MLV_LValueCast,           // Specialized form of MLV_InvalidExpression.
    MLV_IncompleteType,
    MLV_ConstQualified,
    MLV_ConstAddrSpace,
    MLV_ArrayType,
    MLV_NoSetterProperty,
    MLV_MemberFunction,
    MLV_SubObjCPropertySetting,
    MLV_InvalidMessageExpression,
    MLV_ClassTemporary,
    MLV_ArrayTemporary
  };
  /// isModifiableLvalue - C99 6.3.2.1: an lvalue that does not have array type,
  /// does not have an incomplete type, does not have a const-qualified type,
  /// and if it is a structure or union, does not have any member (including,
  /// recursively, any member or element of all contained aggregates or unions)
  /// with a const-qualified type.
  ///
  /// \param Loc [in,out] - A source location which *may* be filled
  /// in with the location of the expression making this a
  /// non-modifiable lvalue, if specified.
  isModifiableLvalueResult
  isModifiableLvalue(ASTContext &Ctx, SourceLocation *Loc = nullptr) const;

  /// \brief The return type of classify(). Represents the C++11 expression
  ///        taxonomy.
  class Classification {
  public:
    /// \brief The various classification results. Most of these mean prvalue.
    enum Kinds {
      CL_LValue,
      CL_XValue,
      CL_Function, // Functions cannot be lvalues in C.
      CL_Void, // Void cannot be an lvalue in C.
      CL_AddressableVoid, // Void expression whose address can be taken in C.
      CL_DuplicateVectorComponents, // A vector shuffle with dupes.
      CL_DuplicateMatrixComponents, // HLSL Change: a matrix shuffle with dupes.
      CL_MemberFunction, // An expression referring to a member function
      CL_SubObjCPropertySetting,
      CL_ClassTemporary, // A temporary of class type, or subobject thereof.
      CL_ArrayTemporary, // A temporary of array type.
      CL_ObjCMessageRValue, // ObjC message is an rvalue
      CL_PRValue // A prvalue for any other reason, of any other type
    };
    /// \brief The results of modification testing.
    enum ModifiableType {
      CM_Untested, // testModifiable was false.
      CM_Modifiable,
      CM_RValue, // Not modifiable because it's an rvalue
      CM_Function, // Not modifiable because it's a function; C++ only
      CM_LValueCast, // Same as CM_RValue, but indicates GCC cast-as-lvalue ext
      CM_NoSetterProperty,// Implicit assignment to ObjC property without setter
      CM_ConstQualified,
      CM_ConstAddrSpace,
      CM_ArrayType,
      CM_IncompleteType
    };

  private:
    friend class Expr;

    unsigned short Kind;
    unsigned short Modifiable;

    explicit Classification(Kinds k, ModifiableType m)
      : Kind(k), Modifiable(m)
    {}

  public:
    Classification() {}

    Kinds getKind() const { return static_cast<Kinds>(Kind); }
    ModifiableType getModifiable() const {
      assert(Modifiable != CM_Untested && "Did not test for modifiability.");
      return static_cast<ModifiableType>(Modifiable);
    }
    bool isLValue() const { return Kind == CL_LValue; }
    bool isXValue() const { return Kind == CL_XValue; }
    bool isGLValue() const { return Kind <= CL_XValue; }
    bool isPRValue() const { return Kind >= CL_Function; }
    bool isRValue() const { return Kind >= CL_XValue; }
    bool isModifiable() const { return getModifiable() == CM_Modifiable; }

    /// \brief Create a simple, modifiably lvalue
    static Classification makeSimpleLValue() {
      return Classification(CL_LValue, CM_Modifiable);
    }

  };
  /// \brief Classify - Classify this expression according to the C++11
  ///        expression taxonomy.
  ///
  /// C++11 defines ([basic.lval]) a new taxonomy of expressions to replace the
  /// old lvalue vs rvalue. This function determines the type of expression this
  /// is. There are three expression types:
  /// - lvalues are classical lvalues as in C++03.
  /// - prvalues are equivalent to rvalues in C++03.
  /// - xvalues are expressions yielding unnamed rvalue references, e.g. a
  ///   function returning an rvalue reference.
  /// lvalues and xvalues are collectively referred to as glvalues, while
  /// prvalues and xvalues together form rvalues.
  Classification Classify(ASTContext &Ctx) const {
    return ClassifyImpl(Ctx, nullptr);
  }

  /// \brief ClassifyModifiable - Classify this expression according to the
  ///        C++11 expression taxonomy, and see if it is valid on the left side
  ///        of an assignment.
  ///
  /// This function extends classify in that it also tests whether the
  /// expression is modifiable (C99 6.3.2.1p1).
  /// \param Loc A source location that might be filled with a relevant location
  ///            if the expression is not modifiable.
  Classification ClassifyModifiable(ASTContext &Ctx, SourceLocation &Loc) const{
    return ClassifyImpl(Ctx, &Loc);
  }

  /// getValueKindForType - Given a formal return or parameter type,
  /// give its value kind.
  static ExprValueKind getValueKindForType(QualType T) {
    if (const ReferenceType *RT = T->getAs<ReferenceType>())
      return (isa<LValueReferenceType>(RT)
                ? VK_LValue
                : (RT->getPointeeType()->isFunctionType()
                     ? VK_LValue : VK_XValue));
    return VK_RValue;
  }

  /// getValueKind - The value kind that this expression produces.
  ExprValueKind getValueKind() const {
    return static_cast<ExprValueKind>(ExprBits.ValueKind);
  }

  /// getObjectKind - The object kind that this expression produces.
  /// Object kinds are meaningful only for expressions that yield an
  /// l-value or x-value.
  ExprObjectKind getObjectKind() const {
    return static_cast<ExprObjectKind>(ExprBits.ObjectKind);
  }

  bool isOrdinaryOrBitFieldObject() const {
    ExprObjectKind OK = getObjectKind();
    return (OK == OK_Ordinary || OK == OK_BitField);
  }

  /// setValueKind - Set the value kind produced by this expression.
  void setValueKind(ExprValueKind Cat) { ExprBits.ValueKind = Cat; }

  /// setObjectKind - Set the object kind produced by this expression.
  void setObjectKind(ExprObjectKind Cat) { ExprBits.ObjectKind = Cat; }

private:
  Classification ClassifyImpl(ASTContext &Ctx, SourceLocation *Loc) const;

public:

  /// \brief Returns true if this expression is a gl-value that
  /// potentially refers to a bit-field.
  ///
  /// In C++, whether a gl-value refers to a bitfield is essentially
  /// an aspect of the value-kind type system.
  bool refersToBitField() const { return getObjectKind() == OK_BitField; }

  /// \brief If this expression refers to a bit-field, retrieve the
  /// declaration of that bit-field.
  ///
  /// Note that this returns a non-null pointer in subtly different
  /// places than refersToBitField returns true.  In particular, this can
  /// return a non-null pointer even for r-values loaded from
  /// bit-fields, but it will return null for a conditional bit-field.
  FieldDecl *getSourceBitField();

  const FieldDecl *getSourceBitField() const {
    return const_cast<Expr*>(this)->getSourceBitField();
  }

  /// \brief If this expression is an l-value for an Objective C
  /// property, find the underlying property reference expression.
  const ObjCPropertyRefExpr *getObjCProperty() const;

  /// \brief Check if this expression is the ObjC 'self' implicit parameter.
  bool isObjCSelfExpr() const;

  /// \brief Returns whether this expression refers to a vector element.
  bool refersToVectorElement() const;

  /// \brief Returns whether this expression has a placeholder type.
  bool hasPlaceholderType() const {
    return getType()->isPlaceholderType();
  }

  /// \brief Returns whether this expression has a specific placeholder type.
  bool hasPlaceholderType(BuiltinType::Kind K) const {
    assert(BuiltinType::isPlaceholderTypeKind(K));
    if (const BuiltinType *BT = dyn_cast<BuiltinType>(getType()))
      return BT->getKind() == K;
    return false;
  }

  /// isKnownToHaveBooleanValue - Return true if this is an integer expression
  /// that is known to return 0 or 1.  This happens for _Bool/bool expressions
  /// but also int expressions which are produced by things like comparisons in
  /// C.
  bool isKnownToHaveBooleanValue() const;

  /// isIntegerConstantExpr - Return true if this expression is a valid integer
  /// constant expression, and, if so, return its value in Result.  If not a
  /// valid i-c-e, return false and fill in Loc (if specified) with the location
  /// of the invalid expression.
  ///
  /// Note: This does not perform the implicit conversions required by C++11
  /// [expr.const]p5.
  bool isIntegerConstantExpr(llvm::APSInt &Result, const ASTContext &Ctx,
                             SourceLocation *Loc = nullptr,
                             bool isEvaluated = true) const;
  bool isIntegerConstantExpr(const ASTContext &Ctx,
                             SourceLocation *Loc = nullptr) const;

  /// isCXX98IntegralConstantExpr - Return true if this expression is an
  /// integral constant expression in C++98. Can only be used in C++.
  bool isCXX98IntegralConstantExpr(const ASTContext &Ctx) const;

  /// isCXX11ConstantExpr - Return true if this expression is a constant
  /// expression in C++11. Can only be used in C++.
  ///
  /// Note: This does not perform the implicit conversions required by C++11
  /// [expr.const]p5.
  bool isCXX11ConstantExpr(const ASTContext &Ctx, APValue *Result = nullptr,
                           SourceLocation *Loc = nullptr) const;

  /// isPotentialConstantExpr - Return true if this function's definition
  /// might be usable in a constant expression in C++11, if it were marked
  /// constexpr. Return false if the function can never produce a constant
  /// expression, along with diagnostics describing why not.
  static bool isPotentialConstantExpr(const FunctionDecl *FD,
                                      SmallVectorImpl<
                                        PartialDiagnosticAt> &Diags);

  /// isPotentialConstantExprUnevaluted - Return true if this expression might
  /// be usable in a constant expression in C++11 in an unevaluated context, if
  /// it were in function FD marked constexpr. Return false if the function can
  /// never produce a constant expression, along with diagnostics describing
  /// why not.
  static bool isPotentialConstantExprUnevaluated(Expr *E,
                                                 const FunctionDecl *FD,
                                                 SmallVectorImpl<
                                                   PartialDiagnosticAt> &Diags);

  /// isConstantInitializer - Returns true if this expression can be emitted to
  /// IR as a constant, and thus can be used as a constant initializer in C.
  /// If this expression is not constant and Culprit is non-null,
  /// it is used to store the address of first non constant expr.
  bool isConstantInitializer(ASTContext &Ctx, bool ForRef,
                             const Expr **Culprit = nullptr) const;

  /// EvalStatus is a struct with detailed info about an evaluation in progress.
  struct EvalStatus {
    /// HasSideEffects - Whether the evaluated expression has side effects.
    /// For example, (f() && 0) can be folded, but it still has side effects.
    bool HasSideEffects;

    /// Diag - If this is non-null, it will be filled in with a stack of notes
    /// indicating why evaluation failed (or why it failed to produce a constant
    /// expression).
    /// If the expression is unfoldable, the notes will indicate why it's not
    /// foldable. If the expression is foldable, but not a constant expression,
    /// the notes will describes why it isn't a constant expression. If the
    /// expression *is* a constant expression, no notes will be produced.
    SmallVectorImpl<PartialDiagnosticAt> *Diag;

    EvalStatus() : HasSideEffects(false), Diag(nullptr) {}

    // hasSideEffects - Return true if the evaluated expression has
    // side effects.
    bool hasSideEffects() const {
      return HasSideEffects;
    }
  };

  /// EvalResult is a struct with detailed info about an evaluated expression.
  struct EvalResult : EvalStatus {
    /// Val - This is the value the expression can be folded to.
    APValue Val;

    // isGlobalLValue - Return true if the evaluated lvalue expression
    // is global.
    bool isGlobalLValue() const;
  };

  /// EvaluateAsRValue - Return true if this is a constant which we can fold to
  /// an rvalue using any crazy technique (that has nothing to do with language
  /// standards) that we want to, even if the expression has side-effects. If
  /// this function returns true, it returns the folded constant in Result. If
  /// the expression is a glvalue, an lvalue-to-rvalue conversion will be
  /// applied.
  bool EvaluateAsRValue(EvalResult &Result, const ASTContext &Ctx) const;

  /// EvaluateAsBooleanCondition - Return true if this is a constant
  /// which we we can fold and convert to a boolean condition using
  /// any crazy technique that we want to, even if the expression has
  /// side-effects.
  bool EvaluateAsBooleanCondition(bool &Result, const ASTContext &Ctx) const;

  enum SideEffectsKind { SE_NoSideEffects, SE_AllowSideEffects };

  /// EvaluateAsInt - Return true if this is a constant which we can fold and
  /// convert to an integer, using any crazy technique that we want to.
  bool EvaluateAsInt(llvm::APSInt &Result, const ASTContext &Ctx,
                     SideEffectsKind AllowSideEffects = SE_NoSideEffects) const;

  /// isEvaluatable - Call EvaluateAsRValue to see if this expression can be
  /// constant folded without side-effects, but discard the result.
  bool isEvaluatable(const ASTContext &Ctx) const;

  /// HasSideEffects - This routine returns true for all those expressions
  /// which have any effect other than producing a value. Example is a function
  /// call, volatile variable read, or throwing an exception. If
  /// IncludePossibleEffects is false, this call treats certain expressions with
  /// potential side effects (such as function call-like expressions,
  /// instantiation-dependent expressions, or invocations from a macro) as not
  /// having side effects.
  bool HasSideEffects(const ASTContext &Ctx,
                      bool IncludePossibleEffects = true) const;

  /// \brief Determine whether this expression involves a call to any function
  /// that is not trivial.
  bool hasNonTrivialCall(const ASTContext &Ctx) const;

  /// EvaluateKnownConstInt - Call EvaluateAsRValue and return the folded
  /// integer. This must be called on an expression that constant folds to an
  /// integer.
  llvm::APSInt EvaluateKnownConstInt(const ASTContext &Ctx,
                    SmallVectorImpl<PartialDiagnosticAt> *Diag = nullptr) const;

  void EvaluateForOverflow(const ASTContext &Ctx) const;

  /// EvaluateAsLValue - Evaluate an expression to see if we can fold it to an
  /// lvalue with link time known address, with no side-effects.
  bool EvaluateAsLValue(EvalResult &Result, const ASTContext &Ctx) const;

  /// EvaluateAsInitializer - Evaluate an expression as if it were the
  /// initializer of the given declaration. Returns true if the initializer
  /// can be folded to a constant, and produces any relevant notes. In C++11,
  /// notes will be produced if the expression is not a constant expression.
  bool EvaluateAsInitializer(APValue &Result, const ASTContext &Ctx,
                             const VarDecl *VD,
                             SmallVectorImpl<PartialDiagnosticAt> &Notes) const;

  /// EvaluateWithSubstitution - Evaluate an expression as if from the context
  /// of a call to the given function with the given arguments, inside an
  /// unevaluated context. Returns true if the expression could be folded to a
  /// constant.
  bool EvaluateWithSubstitution(APValue &Value, ASTContext &Ctx,
                                const FunctionDecl *Callee,
                                ArrayRef<const Expr*> Args) const;

  /// \brief Enumeration used to describe the kind of Null pointer constant
  /// returned from \c isNullPointerConstant().
  enum NullPointerConstantKind {
    /// \brief Expression is not a Null pointer constant.
    NPCK_NotNull = 0,

    /// \brief Expression is a Null pointer constant built from a zero integer
    /// expression that is not a simple, possibly parenthesized, zero literal.
    /// C++ Core Issue 903 will classify these expressions as "not pointers"
    /// once it is adopted.
    /// http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#903
    NPCK_ZeroExpression,

    /// \brief Expression is a Null pointer constant built from a literal zero.
    NPCK_ZeroLiteral,

    /// \brief Expression is a C++11 nullptr.
    NPCK_CXX11_nullptr,

    /// \brief Expression is a GNU-style __null constant.
    NPCK_GNUNull
  };

  /// \brief Enumeration used to describe how \c isNullPointerConstant()
  /// should cope with value-dependent expressions.
  enum NullPointerConstantValueDependence {
    /// \brief Specifies that the expression should never be value-dependent.
    NPC_NeverValueDependent = 0,

    /// \brief Specifies that a value-dependent expression of integral or
    /// dependent type should be considered a null pointer constant.
    NPC_ValueDependentIsNull,

    /// \brief Specifies that a value-dependent expression should be considered
    /// to never be a null pointer constant.
    NPC_ValueDependentIsNotNull
  };

  /// isNullPointerConstant - C99 6.3.2.3p3 - Test if this reduces down to
  /// a Null pointer constant. The return value can further distinguish the
  /// kind of NULL pointer constant that was detected.
  NullPointerConstantKind isNullPointerConstant(
      ASTContext &Ctx,
      NullPointerConstantValueDependence NPC) const;

  /// isOBJCGCCandidate - Return true if this expression may be used in a read/
  /// write barrier.
  bool isOBJCGCCandidate(ASTContext &Ctx) const;

  /// \brief Returns true if this expression is a bound member function.
  bool isBoundMemberFunction(ASTContext &Ctx) const;

  /// \brief Given an expression of bound-member type, find the type
  /// of the member.  Returns null if this is an *overloaded* bound
  /// member expression.
  static QualType findBoundMemberType(const Expr *expr);

  /// IgnoreImpCasts - Skip past any implicit casts which might
  /// surround this expression.  Only skips ImplicitCastExprs.
  Expr *IgnoreImpCasts() LLVM_READONLY;

  /// IgnoreImplicit - Skip past any implicit AST nodes which might
  /// surround this expression.
  Expr *IgnoreImplicit() LLVM_READONLY {
    return cast<Expr>(Stmt::IgnoreImplicit());
  }

  const Expr *IgnoreImplicit() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreImplicit();
  }

  /// IgnoreParens - Ignore parentheses.  If this Expr is a ParenExpr, return
  ///  its subexpression.  If that subexpression is also a ParenExpr,
  ///  then this method recursively returns its subexpression, and so forth.
  ///  Otherwise, the method returns the current Expr.
  Expr *IgnoreParens() LLVM_READONLY;

  /// IgnoreParenCasts - Ignore parentheses and casts.  Strip off any ParenExpr
  /// or CastExprs, returning their operand.
  Expr *IgnoreParenCasts() LLVM_READONLY;

  /// Ignore casts.  Strip off any CastExprs, returning their operand.
  Expr *IgnoreCasts() LLVM_READONLY;

  /// IgnoreParenImpCasts - Ignore parentheses and implicit casts.  Strip off
  /// any ParenExpr or ImplicitCastExprs, returning their operand.
  Expr *IgnoreParenImpCasts() LLVM_READONLY;

  /// IgnoreConversionOperator - Ignore conversion operator. If this Expr is a
  /// call to a conversion operator, return the argument.
  Expr *IgnoreConversionOperator() LLVM_READONLY;

  const Expr *IgnoreConversionOperator() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreConversionOperator();
  }

  const Expr *IgnoreParenImpCasts() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreParenImpCasts();
  }

  /// Ignore parentheses and lvalue casts.  Strip off any ParenExpr and
  /// CastExprs that represent lvalue casts, returning their operand.
  Expr *IgnoreParenLValueCasts() LLVM_READONLY;

  const Expr *IgnoreParenLValueCasts() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreParenLValueCasts();
  }

  /// IgnoreParenNoopCasts - Ignore parentheses and casts that do not change the
  /// value (including ptr->int casts of the same size).  Strip off any
  /// ParenExpr or CastExprs, returning their operand.
  Expr *IgnoreParenNoopCasts(ASTContext &Ctx) LLVM_READONLY;

  /// Ignore parentheses and derived-to-base casts.
  Expr *ignoreParenBaseCasts() LLVM_READONLY;

  const Expr *ignoreParenBaseCasts() const LLVM_READONLY {
    return const_cast<Expr*>(this)->ignoreParenBaseCasts();
  }

  /// \brief Determine whether this expression is a default function argument.
  ///
  /// Default arguments are implicitly generated in the abstract syntax tree
  /// by semantic analysis for function calls, object constructions, etc. in
  /// C++. Default arguments are represented by \c CXXDefaultArgExpr nodes;
  /// this routine also looks through any implicit casts to determine whether
  /// the expression is a default argument.
  bool isDefaultArgument() const;

  /// \brief Determine whether the result of this expression is a
  /// temporary object of the given class type.
  bool isTemporaryObject(ASTContext &Ctx, const CXXRecordDecl *TempTy) const;

  /// \brief Whether this expression is an implicit reference to 'this' in C++.
  bool isImplicitCXXThis() const;

  const Expr *IgnoreImpCasts() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreImpCasts();
  }
  const Expr *IgnoreParens() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreParens();
  }
  const Expr *IgnoreParenCasts() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreParenCasts();
  }
  /// Strip off casts, but keep parentheses.
  const Expr *IgnoreCasts() const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreCasts();
  }

  const Expr *IgnoreParenNoopCasts(ASTContext &Ctx) const LLVM_READONLY {
    return const_cast<Expr*>(this)->IgnoreParenNoopCasts(Ctx);
  }

  static bool hasAnyTypeDependentArguments(ArrayRef<Expr *> Exprs);

  /// \brief For an expression of class type or pointer to class type,
  /// return the most derived class decl the expression is known to refer to.
  ///
  /// If this expression is a cast, this method looks through it to find the
  /// most derived decl that can be inferred from the expression.
  /// This is valid because derived-to-base conversions have undefined
  /// behavior if the object isn't dynamically of the derived type.
  const CXXRecordDecl *getBestDynamicClassType() const;

  /// Walk outwards from an expression we want to bind a reference to and
  /// find the expression whose lifetime needs to be extended. Record
  /// the LHSs of comma expressions and adjustments needed along the path.
  const Expr *skipRValueSubobjectAdjustments(
      SmallVectorImpl<const Expr *> &CommaLHS,
      SmallVectorImpl<SubobjectAdjustment> &Adjustments) const;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() >= firstExprConstant &&
           T->getStmtClass() <= lastExprConstant;
  }
};


//===----------------------------------------------------------------------===//
// Primary Expressions.
//===----------------------------------------------------------------------===//

/// OpaqueValueExpr - An expression referring to an opaque object of a
/// fixed type and value class.  These don't correspond to concrete
/// syntax; instead they're used to express operations (usually copy
/// operations) on values whose source is generally obvious from
/// context.
class OpaqueValueExpr : public Expr {
  friend class ASTStmtReader;
  Expr *SourceExpr;
  SourceLocation Loc;

public:
  OpaqueValueExpr(SourceLocation Loc, QualType T, ExprValueKind VK,
                  ExprObjectKind OK = OK_Ordinary,
                  Expr *SourceExpr = nullptr)
    : Expr(OpaqueValueExprClass, T, VK, OK,
           T->isDependentType(), 
           T->isDependentType() || 
           (SourceExpr && SourceExpr->isValueDependent()),
           T->isInstantiationDependentType(),
           false),
      SourceExpr(SourceExpr), Loc(Loc) {
  }

  /// Given an expression which invokes a copy constructor --- i.e.  a
  /// CXXConstructExpr, possibly wrapped in an ExprWithCleanups ---
  /// find the OpaqueValueExpr that's the source of the construction.
  static const OpaqueValueExpr *findInCopyConstruct(const Expr *expr);

  explicit OpaqueValueExpr(EmptyShell Empty)
    : Expr(OpaqueValueExprClass, Empty) { }

  /// \brief Retrieve the location of this expression.
  SourceLocation getLocation() const { return Loc; }

  SourceLocation getLocStart() const LLVM_READONLY {
    return SourceExpr ? SourceExpr->getLocStart() : Loc;
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return SourceExpr ? SourceExpr->getLocEnd() : Loc;
  }
  SourceLocation getExprLoc() const LLVM_READONLY {
    if (SourceExpr) return SourceExpr->getExprLoc();
    return Loc;
  }

  child_range children() { return child_range(); }

  /// The source expression of an opaque value expression is the
  /// expression which originally generated the value.  This is
  /// provided as a convenience for analyses that don't wish to
  /// precisely model the execution behavior of the program.
  ///
  /// The source expression is typically set when building the
  /// expression which binds the opaque value expression in the first
  /// place.
  Expr *getSourceExpr() const { return SourceExpr; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == OpaqueValueExprClass;
  }
};

/// \brief A reference to a declared variable, function, enum, etc.
/// [C99 6.5.1p2]
///
/// This encodes all the information about how a declaration is referenced
/// within an expression.
///
/// There are several optional constructs attached to DeclRefExprs only when
/// they apply in order to conserve memory. These are laid out past the end of
/// the object, and flags in the DeclRefExprBitfield track whether they exist:
///
///   DeclRefExprBits.HasQualifier:
///       Specifies when this declaration reference expression has a C++
///       nested-name-specifier.
///   DeclRefExprBits.HasFoundDecl:
///       Specifies when this declaration reference expression has a record of
///       a NamedDecl (different from the referenced ValueDecl) which was found
///       during name lookup and/or overload resolution.
///   DeclRefExprBits.HasTemplateKWAndArgsInfo:
///       Specifies when this declaration reference expression has an explicit
///       C++ template keyword and/or template argument list.
///   DeclRefExprBits.RefersToEnclosingVariableOrCapture
///       Specifies when this declaration reference expression (validly)
///       refers to an enclosed local or a captured variable.
class DeclRefExpr : public Expr {
  /// \brief The declaration that we are referencing.
  ValueDecl *D;

  /// \brief The location of the declaration name itself.
  SourceLocation Loc;

  /// \brief Provides source/type location info for the declaration name
  /// embedded in D.
  DeclarationNameLoc DNLoc;

  /// \brief Helper to retrieve the optional NestedNameSpecifierLoc.
  NestedNameSpecifierLoc &getInternalQualifierLoc() {
    assert(hasQualifier());
    return *reinterpret_cast<NestedNameSpecifierLoc *>(this + 1);
  }

  /// \brief Helper to retrieve the optional NestedNameSpecifierLoc.
  const NestedNameSpecifierLoc &getInternalQualifierLoc() const {
    return const_cast<DeclRefExpr *>(this)->getInternalQualifierLoc();
  }

  /// \brief Test whether there is a distinct FoundDecl attached to the end of
  /// this DRE.
  bool hasFoundDecl() const { return DeclRefExprBits.HasFoundDecl; }

  /// \brief Helper to retrieve the optional NamedDecl through which this
  /// reference occurred.
  NamedDecl *&getInternalFoundDecl() {
    assert(hasFoundDecl());
    if (hasQualifier())
      return *reinterpret_cast<NamedDecl **>(&getInternalQualifierLoc() + 1);
    return *reinterpret_cast<NamedDecl **>(this + 1);
  }

  /// \brief Helper to retrieve the optional NamedDecl through which this
  /// reference occurred.
  NamedDecl *getInternalFoundDecl() const {
    return const_cast<DeclRefExpr *>(this)->getInternalFoundDecl();
  }

  DeclRefExpr(const ASTContext &Ctx,
              NestedNameSpecifierLoc QualifierLoc,
              SourceLocation TemplateKWLoc,
              ValueDecl *D, bool RefersToEnlosingVariableOrCapture,
              const DeclarationNameInfo &NameInfo,
              NamedDecl *FoundD,
              const TemplateArgumentListInfo *TemplateArgs,
              QualType T, ExprValueKind VK);

  /// \brief Construct an empty declaration reference expression.
  explicit DeclRefExpr(EmptyShell Empty)
    : Expr(DeclRefExprClass, Empty) { }

  /// \brief Computes the type- and value-dependence flags for this
  /// declaration reference expression.
  void computeDependence(const ASTContext &C);

public:
  DeclRefExpr(ValueDecl *D, bool RefersToEnclosingVariableOrCapture, QualType T,
              ExprValueKind VK, SourceLocation L,
              const DeclarationNameLoc &LocInfo = DeclarationNameLoc())
    : Expr(DeclRefExprClass, T, VK, OK_Ordinary, false, false, false, false),
      D(D), Loc(L), DNLoc(LocInfo) {
    DeclRefExprBits.HasQualifier = 0;
    DeclRefExprBits.HasTemplateKWAndArgsInfo = 0;
    DeclRefExprBits.HasFoundDecl = 0;
    DeclRefExprBits.HadMultipleCandidates = 0;
    DeclRefExprBits.RefersToEnclosingVariableOrCapture =
        RefersToEnclosingVariableOrCapture;
    computeDependence(D->getASTContext());
  }

  static DeclRefExpr *
  Create(const ASTContext &Context, NestedNameSpecifierLoc QualifierLoc,
         SourceLocation TemplateKWLoc, ValueDecl *D,
         bool RefersToEnclosingVariableOrCapture, SourceLocation NameLoc,
         QualType T, ExprValueKind VK, NamedDecl *FoundD = nullptr,
         const TemplateArgumentListInfo *TemplateArgs = nullptr);

  static DeclRefExpr *
  Create(const ASTContext &Context, NestedNameSpecifierLoc QualifierLoc,
         SourceLocation TemplateKWLoc, ValueDecl *D,
         bool RefersToEnclosingVariableOrCapture,
         const DeclarationNameInfo &NameInfo, QualType T, ExprValueKind VK,
         NamedDecl *FoundD = nullptr,
         const TemplateArgumentListInfo *TemplateArgs = nullptr);

  /// \brief Construct an empty declaration reference expression.
  static DeclRefExpr *CreateEmpty(const ASTContext &Context,
                                  bool HasQualifier,
                                  bool HasFoundDecl,
                                  bool HasTemplateKWAndArgsInfo,
                                  unsigned NumTemplateArgs);

  ValueDecl *getDecl() { return D; }
  const ValueDecl *getDecl() const { return D; }
  void setDecl(ValueDecl *NewD) { D = NewD; }

  DeclarationNameInfo getNameInfo() const {
    return DeclarationNameInfo(getDecl()->getDeclName(), Loc, DNLoc);
  }

  SourceLocation getLocation() const { return Loc; }
  void setLocation(SourceLocation L) { Loc = L; }
  SourceLocation getLocStart() const LLVM_READONLY;
  SourceLocation getLocEnd() const LLVM_READONLY;

  /// \brief Determine whether this declaration reference was preceded by a
  /// C++ nested-name-specifier, e.g., \c N::foo.
  bool hasQualifier() const { return DeclRefExprBits.HasQualifier; }

  /// \brief If the name was qualified, retrieves the nested-name-specifier
  /// that precedes the name. Otherwise, returns NULL.
  NestedNameSpecifier *getQualifier() const {
    if (!hasQualifier())
      return nullptr;

    return getInternalQualifierLoc().getNestedNameSpecifier();
  }

  /// \brief If the name was qualified, retrieves the nested-name-specifier
  /// that precedes the name, with source-location information.
  NestedNameSpecifierLoc getQualifierLoc() const {
    if (!hasQualifier())
      return NestedNameSpecifierLoc();

    return getInternalQualifierLoc();
  }

  /// \brief Get the NamedDecl through which this reference occurred.
  ///
  /// This Decl may be different from the ValueDecl actually referred to in the
  /// presence of using declarations, etc. It always returns non-NULL, and may
  /// simple return the ValueDecl when appropriate.
  NamedDecl *getFoundDecl() {
    return hasFoundDecl() ? getInternalFoundDecl() : D;
  }

  /// \brief Get the NamedDecl through which this reference occurred.
  /// See non-const variant.
  const NamedDecl *getFoundDecl() const {
    return hasFoundDecl() ? getInternalFoundDecl() : D;
  }

  bool hasTemplateKWAndArgsInfo() const {
    return DeclRefExprBits.HasTemplateKWAndArgsInfo;
  }

  /// \brief Return the optional template keyword and arguments info.
  ASTTemplateKWAndArgsInfo *getTemplateKWAndArgsInfo() {
    if (!hasTemplateKWAndArgsInfo())
      return nullptr;

    if (hasFoundDecl())
      return reinterpret_cast<ASTTemplateKWAndArgsInfo *>(
        &getInternalFoundDecl() + 1);

    if (hasQualifier())
      return reinterpret_cast<ASTTemplateKWAndArgsInfo *>(
        &getInternalQualifierLoc() + 1);

    return reinterpret_cast<ASTTemplateKWAndArgsInfo *>(this + 1);
  }

  /// \brief Return the optional template keyword and arguments info.
  const ASTTemplateKWAndArgsInfo *getTemplateKWAndArgsInfo() const {
    return const_cast<DeclRefExpr*>(this)->getTemplateKWAndArgsInfo();
  }

  /// \brief Retrieve the location of the template keyword preceding
  /// this name, if any.
  SourceLocation getTemplateKeywordLoc() const {
    if (!hasTemplateKWAndArgsInfo()) return SourceLocation();
    return getTemplateKWAndArgsInfo()->getTemplateKeywordLoc();
  }

  /// \brief Retrieve the location of the left angle bracket starting the
  /// explicit template argument list following the name, if any.
  SourceLocation getLAngleLoc() const {
    if (!hasTemplateKWAndArgsInfo()) return SourceLocation();
    return getTemplateKWAndArgsInfo()->LAngleLoc;
  }

  /// \brief Retrieve the location of the right angle bracket ending the
  /// explicit template argument list following the name, if any.
  SourceLocation getRAngleLoc() const {
    if (!hasTemplateKWAndArgsInfo()) return SourceLocation();
    return getTemplateKWAndArgsInfo()->RAngleLoc;
  }

  /// \brief Determines whether the name in this declaration reference
  /// was preceded by the template keyword.
  bool hasTemplateKeyword() const { return getTemplateKeywordLoc().isValid(); }

  /// \brief Determines whether this declaration reference was followed by an
  /// explicit template argument list.
  bool hasExplicitTemplateArgs() const { return getLAngleLoc().isValid(); }

  /// \brief Retrieve the explicit template argument list that followed the
  /// member template name.
  ASTTemplateArgumentListInfo &getExplicitTemplateArgs() {
    assert(hasExplicitTemplateArgs());
    return *getTemplateKWAndArgsInfo();
  }

  /// \brief Retrieve the explicit template argument list that followed the
  /// member template name.
  const ASTTemplateArgumentListInfo &getExplicitTemplateArgs() const {
    return const_cast<DeclRefExpr *>(this)->getExplicitTemplateArgs();
  }

  /// \brief Retrieves the optional explicit template arguments.
  /// This points to the same data as getExplicitTemplateArgs(), but
  /// returns null if there are no explicit template arguments.
  const ASTTemplateArgumentListInfo *getOptionalExplicitTemplateArgs() const {
    if (!hasExplicitTemplateArgs()) return nullptr;
    return &getExplicitTemplateArgs();
  }

  /// \brief Copies the template arguments (if present) into the given
  /// structure.
  void copyTemplateArgumentsInto(TemplateArgumentListInfo &List) const {
    if (hasExplicitTemplateArgs())
      getExplicitTemplateArgs().copyInto(List);
  }

  /// \brief Retrieve the template arguments provided as part of this
  /// template-id.
  const TemplateArgumentLoc *getTemplateArgs() const {
    if (!hasExplicitTemplateArgs())
      return nullptr;

    return getExplicitTemplateArgs().getTemplateArgs();
  }

  /// \brief Retrieve the number of template arguments provided as part of this
  /// template-id.
  unsigned getNumTemplateArgs() const {
    if (!hasExplicitTemplateArgs())
      return 0;

    return getExplicitTemplateArgs().NumTemplateArgs;
  }

  /// \brief Returns true if this expression refers to a function that
  /// was resolved from an overloaded set having size greater than 1.
  bool hadMultipleCandidates() const {
    return DeclRefExprBits.HadMultipleCandidates;
  }
  /// \brief Sets the flag telling whether this expression refers to
  /// a function that was resolved from an overloaded set having size
  /// greater than 1.
  void setHadMultipleCandidates(bool V = true) {
    DeclRefExprBits.HadMultipleCandidates = V;
  }

  /// \brief Does this DeclRefExpr refer to an enclosing local or a captured
  /// variable?
  bool refersToEnclosingVariableOrCapture() const {
    return DeclRefExprBits.RefersToEnclosingVariableOrCapture;
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == DeclRefExprClass;
  }

  // Iterators
  child_range children() { return child_range(); }

  friend class ASTStmtReader;
  friend class ASTStmtWriter;
};

/// \brief [C99 6.4.2.2] - A predefined identifier such as __func__.
class PredefinedExpr : public Expr {
public:
  enum IdentType {
    Func,
    Function,
    LFunction,  // Same as Function, but as wide string.
    FuncDName,
    FuncSig,
    PrettyFunction,
    /// \brief The same as PrettyFunction, except that the
    /// 'virtual' keyword is omitted for virtual member functions.
    PrettyFunctionNoVirtual
  };

private:
  SourceLocation Loc;
  IdentType Type;
  Stmt *FnName;

public:
  PredefinedExpr(SourceLocation L, QualType FNTy, IdentType IT,
                 StringLiteral *SL);

  /// \brief Construct an empty predefined expression.
  explicit PredefinedExpr(EmptyShell Empty)
      : Expr(PredefinedExprClass, Empty), Loc(), Type(Func), FnName(nullptr) {}

  IdentType getIdentType() const { return Type; }

  SourceLocation getLocation() const { return Loc; }
  void setLocation(SourceLocation L) { Loc = L; }

  StringLiteral *getFunctionName();
  const StringLiteral *getFunctionName() const {
    return const_cast<PredefinedExpr *>(this)->getFunctionName();
  }

  static StringRef getIdentTypeName(IdentType IT);
  static std::string ComputeName(IdentType IT, const Decl *CurrentDecl);

  SourceLocation getLocStart() const LLVM_READONLY { return Loc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return Loc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == PredefinedExprClass;
  }

  // Iterators
  child_range children() { return child_range(&FnName, &FnName + 1); }

  friend class ASTStmtReader;
};

/// \brief Used by IntegerLiteral/FloatingLiteral to store the numeric without
/// leaking memory.
///
/// For large floats/integers, APFloat/APInt will allocate memory from the heap
/// to represent these numbers.  Unfortunately, when we use a BumpPtrAllocator
/// to allocate IntegerLiteral/FloatingLiteral nodes the memory associated with
/// the APFloat/APInt values will never get freed. APNumericStorage uses
/// ASTContext's allocator for memory allocation.
class APNumericStorage {
  union {
    uint64_t VAL;    ///< Used to store the <= 64 bits integer value.
    uint64_t *pVal;  ///< Used to store the >64 bits integer value.
  };
  unsigned BitWidth;

  bool hasAllocation() const { return llvm::APInt::getNumWords(BitWidth) > 1; }

  APNumericStorage(const APNumericStorage &) = delete;
  void operator=(const APNumericStorage &) = delete;

protected:
  APNumericStorage() : VAL(0), BitWidth(0) { }

  llvm::APInt getIntValue() const {
    unsigned NumWords = llvm::APInt::getNumWords(BitWidth);
    if (NumWords > 1)
      return llvm::APInt(BitWidth, NumWords, pVal);
    else
      return llvm::APInt(BitWidth, VAL);
  }
  void setIntValue(const ASTContext &C, const llvm::APInt &Val);
};

class APIntStorage : private APNumericStorage {
public:
  llvm::APInt getValue() const { return getIntValue(); }
  void setValue(const ASTContext &C, const llvm::APInt &Val) {
    setIntValue(C, Val);
  }
};

class APFloatStorage : private APNumericStorage {
public:
  llvm::APFloat getValue(const llvm::fltSemantics &Semantics) const {
    return llvm::APFloat(Semantics, getIntValue());
  }
  void setValue(const ASTContext &C, const llvm::APFloat &Val) {
    setIntValue(C, Val.bitcastToAPInt());
  }
};

class IntegerLiteral : public Expr, public APIntStorage {
  SourceLocation Loc;

  /// \brief Construct an empty integer literal.
  explicit IntegerLiteral(EmptyShell Empty)
    : Expr(IntegerLiteralClass, Empty) { }

public:
  // type should be IntTy, LongTy, LongLongTy, UnsignedIntTy, UnsignedLongTy,
  // or UnsignedLongLongTy
  IntegerLiteral(const ASTContext &C, const llvm::APInt &V, QualType type,
                 SourceLocation l);

  /// \brief Returns a new integer literal with value 'V' and type 'type'.
  /// \param type - either IntTy, LongTy, LongLongTy, UnsignedIntTy,
  /// UnsignedLongTy, or UnsignedLongLongTy which should match the size of V
  /// \param V - the value that the returned integer literal contains.
  static IntegerLiteral *Create(const ASTContext &C, const llvm::APInt &V,
                                QualType type, SourceLocation l);
  /// \brief Returns a new empty integer literal.
  static IntegerLiteral *Create(const ASTContext &C, EmptyShell Empty);

  SourceLocation getLocStart() const LLVM_READONLY { return Loc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return Loc; }

  /// \brief Retrieve the location of the literal.
  SourceLocation getLocation() const { return Loc; }

  void setLocation(SourceLocation Location) { Loc = Location; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == IntegerLiteralClass;
  }

  // Iterators
  child_range children() { return child_range(); }
};

class CharacterLiteral : public Expr {
public:
  enum CharacterKind {
    Ascii,
    Wide,
    UTF16,
    UTF32
  };

private:
  unsigned Value;
  SourceLocation Loc;
public:
  // type should be IntTy
  CharacterLiteral(unsigned value, CharacterKind kind, QualType type,
                   SourceLocation l)
    : Expr(CharacterLiteralClass, type, VK_RValue, OK_Ordinary, false, false,
           false, false),
      Value(value), Loc(l) {
    CharacterLiteralBits.Kind = kind;
  }

  /// \brief Construct an empty character literal.
  CharacterLiteral(EmptyShell Empty) : Expr(CharacterLiteralClass, Empty) { }

  SourceLocation getLocation() const { return Loc; }
  CharacterKind getKind() const {
    return static_cast<CharacterKind>(CharacterLiteralBits.Kind);
  }

  SourceLocation getLocStart() const LLVM_READONLY { return Loc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return Loc; }

  unsigned getValue() const { return Value; }

  void setLocation(SourceLocation Location) { Loc = Location; }
  void setKind(CharacterKind kind) { CharacterLiteralBits.Kind = kind; }
  void setValue(unsigned Val) { Value = Val; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CharacterLiteralClass;
  }

  // Iterators
  child_range children() { return child_range(); }
};

class FloatingLiteral : public Expr, private APFloatStorage {
  SourceLocation Loc;

  FloatingLiteral(const ASTContext &C, const llvm::APFloat &V, bool isexact,
                  QualType Type, SourceLocation L);

  /// \brief Construct an empty floating-point literal.
  explicit FloatingLiteral(const ASTContext &C, EmptyShell Empty);

public:
  static FloatingLiteral *Create(const ASTContext &C, const llvm::APFloat &V,
                                 bool isexact, QualType Type, SourceLocation L);
  static FloatingLiteral *Create(const ASTContext &C, EmptyShell Empty);

  llvm::APFloat getValue() const {
    return APFloatStorage::getValue(getSemantics());
  }
  void setValue(const ASTContext &C, const llvm::APFloat &Val) {
    assert(&getSemantics() == &Val.getSemantics() && "Inconsistent semantics");
    APFloatStorage::setValue(C, Val);
  }

  /// Get a raw enumeration value representing the floating-point semantics of
  /// this literal (32-bit IEEE, x87, ...), suitable for serialisation.
  APFloatSemantics getRawSemantics() const {
    return static_cast<APFloatSemantics>(FloatingLiteralBits.Semantics);
  }

  /// Set the raw enumeration value representing the floating-point semantics of
  /// this literal (32-bit IEEE, x87, ...), suitable for serialisation.
  void setRawSemantics(APFloatSemantics Sem) {
    FloatingLiteralBits.Semantics = Sem;
  }

  /// Return the APFloat semantics this literal uses.
  const llvm::fltSemantics &getSemantics() const;

  /// Set the APFloat semantics this literal uses.
  void setSemantics(const llvm::fltSemantics &Sem);

  bool isExact() const { return FloatingLiteralBits.IsExact; }
  void setExact(bool E) { FloatingLiteralBits.IsExact = E; }

  /// getValueAsApproximateDouble - This returns the value as an inaccurate
  /// double.  Note that this may cause loss of precision, but is useful for
  /// debugging dumps, etc.
  double getValueAsApproximateDouble() const;

  SourceLocation getLocation() const { return Loc; }
  void setLocation(SourceLocation L) { Loc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return Loc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return Loc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == FloatingLiteralClass;
  }

  // Iterators
  child_range children() { return child_range(); }
};

/// ImaginaryLiteral - We support imaginary integer and floating point literals,
/// like "1.0i".  We represent these as a wrapper around FloatingLiteral and
/// IntegerLiteral classes.  Instances of this class always have a Complex type
/// whose element type matches the subexpression.
///
class ImaginaryLiteral : public Expr {
  Stmt *Val;
public:
  ImaginaryLiteral(Expr *val, QualType Ty)
    : Expr(ImaginaryLiteralClass, Ty, VK_RValue, OK_Ordinary, false, false,
           false, false),
      Val(val) {}

  /// \brief Build an empty imaginary literal.
  explicit ImaginaryLiteral(EmptyShell Empty)
    : Expr(ImaginaryLiteralClass, Empty) { }

  const Expr *getSubExpr() const { return cast<Expr>(Val); }
  Expr *getSubExpr() { return cast<Expr>(Val); }
  void setSubExpr(Expr *E) { Val = E; }

  SourceLocation getLocStart() const LLVM_READONLY { return Val->getLocStart(); }
  SourceLocation getLocEnd() const LLVM_READONLY { return Val->getLocEnd(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ImaginaryLiteralClass;
  }

  // Iterators
  child_range children() { return child_range(&Val, &Val+1); }
};

/// StringLiteral - This represents a string literal expression, e.g. "foo"
/// or L"bar" (wide strings).  The actual string is returned by getBytes()
/// is NOT null-terminated, and the length of the string is determined by
/// calling getByteLength().  The C type for a string is always a
/// ConstantArrayType.  In C++, the char type is const qualified, in C it is
/// not.
///
/// Note that strings in C can be formed by concatenation of multiple string
/// literal pptokens in translation phase #6.  This keeps track of the locations
/// of each of these pieces.
///
/// Strings in C can also be truncated and extended by assigning into arrays,
/// e.g. with constructs like:
///   char X[2] = "foobar";
/// In this case, getByteLength() will return 6, but the string literal will
/// have type "char[2]".
class StringLiteral : public Expr {
public:
  enum StringKind {
    Ascii,
    Wide,
    UTF8,
    UTF16,
    UTF32
  };

private:
  friend class ASTStmtReader;

  union {
    const char *asChar;
    const uint16_t *asUInt16;
    const uint32_t *asUInt32;
  } StrData;
  unsigned Length;
  unsigned CharByteWidth : 4;
  unsigned Kind : 3;
  unsigned IsPascal : 1;
  unsigned NumConcatenated;
  SourceLocation TokLocs[1];

  StringLiteral(QualType Ty) :
    Expr(StringLiteralClass, Ty, VK_LValue, OK_Ordinary, false, false, false,
         false) {}

  static int mapCharByteWidth(TargetInfo const &target,StringKind k);

public:
  /// This is the "fully general" constructor that allows representation of
  /// strings formed from multiple concatenated tokens.
  static StringLiteral *Create(const ASTContext &C, StringRef Str,
                               StringKind Kind, bool Pascal, QualType Ty,
                               const SourceLocation *Loc, unsigned NumStrs);

  /// Simple constructor for string literals made from one token.
  static StringLiteral *Create(const ASTContext &C, StringRef Str,
                               StringKind Kind, bool Pascal, QualType Ty,
                               SourceLocation Loc) {
    return Create(C, Str, Kind, Pascal, Ty, &Loc, 1);
  }

  /// \brief Construct an empty string literal.
  static StringLiteral *CreateEmpty(const ASTContext &C, unsigned NumStrs);

  StringRef getString() const {
    assert(CharByteWidth==1
           && "This function is used in places that assume strings use char");
    return StringRef(StrData.asChar, getByteLength());
  }

  /// Allow access to clients that need the byte representation, such as
  /// ASTWriterStmt::VisitStringLiteral().
  StringRef getBytes() const {
    // FIXME: StringRef may not be the right type to use as a result for this.
    if (CharByteWidth == 1)
      return StringRef(StrData.asChar, getByteLength());
    if (CharByteWidth == 4)
      return StringRef(reinterpret_cast<const char*>(StrData.asUInt32),
                       getByteLength());
    assert(CharByteWidth == 2 && "unsupported CharByteWidth");
    return StringRef(reinterpret_cast<const char*>(StrData.asUInt16),
                     getByteLength());
  }

  void outputString(raw_ostream &OS) const;

  uint32_t getCodeUnit(size_t i) const {
    assert(i < Length && "out of bounds access");
    if (CharByteWidth == 1)
      return static_cast<unsigned char>(StrData.asChar[i]);
    if (CharByteWidth == 4)
      return StrData.asUInt32[i];
    assert(CharByteWidth == 2 && "unsupported CharByteWidth");
    return StrData.asUInt16[i];
  }

  unsigned getByteLength() const { return CharByteWidth*Length; }
  unsigned getLength() const { return Length; }
  unsigned getCharByteWidth() const { return CharByteWidth; }

  /// \brief Sets the string data to the given string data.
  void setString(const ASTContext &C, StringRef Str,
                 StringKind Kind, bool IsPascal);

  StringKind getKind() const { return static_cast<StringKind>(Kind); }


  bool isAscii() const { return Kind == Ascii; }
  bool isWide() const { return Kind == Wide; }
  bool isUTF8() const { return Kind == UTF8; }
  bool isUTF16() const { return Kind == UTF16; }
  bool isUTF32() const { return Kind == UTF32; }
  bool isPascal() const { return IsPascal; }

  bool containsNonAsciiOrNull() const {
    StringRef Str = getString();
    for (unsigned i = 0, e = Str.size(); i != e; ++i)
      if (!isASCII(Str[i]) || !Str[i])
        return true;
    return false;
  }

  /// getNumConcatenated - Get the number of string literal tokens that were
  /// concatenated in translation phase #6 to form this string literal.
  unsigned getNumConcatenated() const { return NumConcatenated; }

  SourceLocation getStrTokenLoc(unsigned TokNum) const {
    assert(TokNum < NumConcatenated && "Invalid tok number");
    return TokLocs[TokNum];
  }
  void setStrTokenLoc(unsigned TokNum, SourceLocation L) {
    assert(TokNum < NumConcatenated && "Invalid tok number");
    TokLocs[TokNum] = L;
  }

  /// getLocationOfByte - Return a source location that points to the specified
  /// byte of this string literal.
  ///
  /// Strings are amazingly complex.  They can be formed from multiple tokens
  /// and can have escape sequences in them in addition to the usual trigraph
  /// and escaped newline business.  This routine handles this complexity.
  ///
  SourceLocation getLocationOfByte(unsigned ByteNo, const SourceManager &SM,
                                   const LangOptions &Features,
                                   const TargetInfo &Target) const;

  typedef const SourceLocation *tokloc_iterator;
  tokloc_iterator tokloc_begin() const { return TokLocs; }
  tokloc_iterator tokloc_end() const { return TokLocs+NumConcatenated; }

  SourceLocation getLocStart() const LLVM_READONLY { return TokLocs[0]; }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return TokLocs[NumConcatenated - 1];
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == StringLiteralClass;
  }

  // Iterators
  child_range children() { return child_range(); }
};

/// ParenExpr - This represents a parethesized expression, e.g. "(1)".  This
/// AST node is only formed if full location information is requested.
class ParenExpr : public Expr {
  SourceLocation L, R;
  Stmt *Val;
public:
  ParenExpr(SourceLocation l, SourceLocation r, Expr *val)
    : Expr(ParenExprClass, val->getType(),
           val->getValueKind(), val->getObjectKind(),
           val->isTypeDependent(), val->isValueDependent(),
           val->isInstantiationDependent(),
           val->containsUnexpandedParameterPack()),
      L(l), R(r), Val(val) {}

  /// \brief Construct an empty parenthesized expression.
  explicit ParenExpr(EmptyShell Empty)
    : Expr(ParenExprClass, Empty) { }

  const Expr *getSubExpr() const { return cast<Expr>(Val); }
  Expr *getSubExpr() { return cast<Expr>(Val); }
  void setSubExpr(Expr *E) { Val = E; }

  SourceLocation getLocStart() const LLVM_READONLY { return L; }
  SourceLocation getLocEnd() const LLVM_READONLY { return R; }

  /// \brief Get the location of the left parentheses '('.
  SourceLocation getLParen() const { return L; }
  void setLParen(SourceLocation Loc) { L = Loc; }

  /// \brief Get the location of the right parentheses ')'.
  SourceLocation getRParen() const { return R; }
  void setRParen(SourceLocation Loc) { R = Loc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ParenExprClass;
  }

  // Iterators
  child_range children() { return child_range(&Val, &Val+1); }
};


/// UnaryOperator - This represents the unary-expression's (except sizeof and
/// alignof), the postinc/postdec operators from postfix-expression, and various
/// extensions.
///
/// Notes on various nodes:
///
/// Real/Imag - These return the real/imag part of a complex operand.  If
///   applied to a non-complex value, the former returns its operand and the
///   later returns zero in the type of the operand.
///
class UnaryOperator : public Expr {
public:
  typedef UnaryOperatorKind Opcode;

private:
  unsigned Opc : 5;
  SourceLocation Loc;
  Stmt *Val;
public:

  UnaryOperator(Expr *input, Opcode opc, QualType type,
                ExprValueKind VK, ExprObjectKind OK, SourceLocation l)
    : Expr(UnaryOperatorClass, type, VK, OK,
           input->isTypeDependent() || type->isDependentType(),
           input->isValueDependent(),
           (input->isInstantiationDependent() ||
            type->isInstantiationDependentType()),
           input->containsUnexpandedParameterPack()),
      Opc(opc), Loc(l), Val(input) {}

  /// \brief Build an empty unary operator.
  explicit UnaryOperator(EmptyShell Empty)
    : Expr(UnaryOperatorClass, Empty), Opc(UO_AddrOf) { }

  Opcode getOpcode() const { return static_cast<Opcode>(Opc); }
  void setOpcode(Opcode O) { Opc = O; }

  Expr *getSubExpr() const { return cast<Expr>(Val); }
  void setSubExpr(Expr *E) { Val = E; }

  /// getOperatorLoc - Return the location of the operator.
  SourceLocation getOperatorLoc() const { return Loc; }
  void setOperatorLoc(SourceLocation L) { Loc = L; }

  /// isPostfix - Return true if this is a postfix operation, like x++.
  static bool isPostfix(Opcode Op) {
    return Op == UO_PostInc || Op == UO_PostDec;
  }

  /// isPrefix - Return true if this is a prefix operation, like --x.
  static bool isPrefix(Opcode Op) {
    return Op == UO_PreInc || Op == UO_PreDec;
  }

  bool isPrefix() const { return isPrefix(getOpcode()); }
  bool isPostfix() const { return isPostfix(getOpcode()); }

  static bool isIncrementOp(Opcode Op) {
    return Op == UO_PreInc || Op == UO_PostInc;
  }
  bool isIncrementOp() const {
    return isIncrementOp(getOpcode());
  }

  static bool isDecrementOp(Opcode Op) {
    return Op == UO_PreDec || Op == UO_PostDec;
  }
  bool isDecrementOp() const {
    return isDecrementOp(getOpcode());
  }

  static bool isIncrementDecrementOp(Opcode Op) { return Op <= UO_PreDec; }
  bool isIncrementDecrementOp() const {
    return isIncrementDecrementOp(getOpcode());
  }

  static bool isArithmeticOp(Opcode Op) {
    return Op >= UO_Plus && Op <= UO_LNot;
  }
  bool isArithmeticOp() const { return isArithmeticOp(getOpcode()); }

  /// getOpcodeStr - Turn an Opcode enum value into the punctuation char it
  /// corresponds to, e.g. "sizeof" or "[pre]++"
  static StringRef getOpcodeStr(Opcode Op);

  /// \brief Retrieve the unary opcode that corresponds to the given
  /// overloaded operator.
  static Opcode getOverloadedOpcode(OverloadedOperatorKind OO, bool Postfix);

  /// \brief Retrieve the overloaded operator kind that corresponds to
  /// the given unary opcode.
  static OverloadedOperatorKind getOverloadedOperator(Opcode Opc);

  SourceLocation getLocStart() const LLVM_READONLY {
    return isPostfix() ? Val->getLocStart() : Loc;
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return isPostfix() ? Loc : Val->getLocEnd();
  }
  SourceLocation getExprLoc() const LLVM_READONLY { return Loc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == UnaryOperatorClass;
  }

  // Iterators
  child_range children() { return child_range(&Val, &Val+1); }
};

/// OffsetOfExpr - [C99 7.17] - This represents an expression of the form
/// offsetof(record-type, member-designator). For example, given:
/// @code
/// struct S {
///   float f;
///   double d;
/// };
/// struct T {
///   int i;
///   struct S s[10];
/// };
/// @endcode
/// we can represent and evaluate the expression @c offsetof(struct T, s[2].d).

class OffsetOfExpr : public Expr {
public:
  // __builtin_offsetof(type, identifier(.identifier|[expr])*)
  class OffsetOfNode {
  public:
    /// \brief The kind of offsetof node we have.
    enum Kind {
      /// \brief An index into an array.
      Array = 0x00,
      /// \brief A field.
      Field = 0x01,
      /// \brief A field in a dependent type, known only by its name.
      Identifier = 0x02,
      /// \brief An implicit indirection through a C++ base class, when the
      /// field found is in a base class.
      Base = 0x03
    };

  private:
    enum { MaskBits = 2, Mask = 0x03 };

    /// \brief The source range that covers this part of the designator.
    SourceRange Range;

    /// \brief The data describing the designator, which comes in three
    /// different forms, depending on the lower two bits.
    ///   - An unsigned index into the array of Expr*'s stored after this node
    ///     in memory, for [constant-expression] designators.
    ///   - A FieldDecl*, for references to a known field.
    ///   - An IdentifierInfo*, for references to a field with a given name
    ///     when the class type is dependent.
    ///   - A CXXBaseSpecifier*, for references that look at a field in a
    ///     base class.
    uintptr_t Data;

  public:
    /// \brief Create an offsetof node that refers to an array element.
    OffsetOfNode(SourceLocation LBracketLoc, unsigned Index,
                 SourceLocation RBracketLoc)
      : Range(LBracketLoc, RBracketLoc), Data((Index << 2) | Array) { }

    /// \brief Create an offsetof node that refers to a field.
    OffsetOfNode(SourceLocation DotLoc, FieldDecl *Field,
                 SourceLocation NameLoc)
      : Range(DotLoc.isValid()? DotLoc : NameLoc, NameLoc),
        Data(reinterpret_cast<uintptr_t>(Field) | OffsetOfNode::Field) { }

    /// \brief Create an offsetof node that refers to an identifier.
    OffsetOfNode(SourceLocation DotLoc, IdentifierInfo *Name,
                 SourceLocation NameLoc)
      : Range(DotLoc.isValid()? DotLoc : NameLoc, NameLoc),
        Data(reinterpret_cast<uintptr_t>(Name) | Identifier) { }

    /// \brief Create an offsetof node that refers into a C++ base class.
    explicit OffsetOfNode(const CXXBaseSpecifier *Base)
      : Range(), Data(reinterpret_cast<uintptr_t>(Base) | OffsetOfNode::Base) {}

    /// \brief Determine what kind of offsetof node this is.
    Kind getKind() const {
      return static_cast<Kind>(Data & Mask);
    }

    /// \brief For an array element node, returns the index into the array
    /// of expressions.
    unsigned getArrayExprIndex() const {
      assert(getKind() == Array);
      return Data >> 2;
    }

    /// \brief For a field offsetof node, returns the field.
    FieldDecl *getField() const {
      assert(getKind() == Field);
      return reinterpret_cast<FieldDecl *>(Data & ~(uintptr_t)Mask);
    }

    /// \brief For a field or identifier offsetof node, returns the name of
    /// the field.
    IdentifierInfo *getFieldName() const;

    /// \brief For a base class node, returns the base specifier.
    CXXBaseSpecifier *getBase() const {
      assert(getKind() == Base);
      return reinterpret_cast<CXXBaseSpecifier *>(Data & ~(uintptr_t)Mask);
    }

    /// \brief Retrieve the source range that covers this offsetof node.
    ///
    /// For an array element node, the source range contains the locations of
    /// the square brackets. For a field or identifier node, the source range
    /// contains the location of the period (if there is one) and the
    /// identifier.
    SourceRange getSourceRange() const LLVM_READONLY { return Range; }
    SourceLocation getLocStart() const LLVM_READONLY { return Range.getBegin(); }
    SourceLocation getLocEnd() const LLVM_READONLY { return Range.getEnd(); }
  };

private:

  SourceLocation OperatorLoc, RParenLoc;
  // Base type;
  TypeSourceInfo *TSInfo;
  // Number of sub-components (i.e. instances of OffsetOfNode).
  unsigned NumComps;
  // Number of sub-expressions (i.e. array subscript expressions).
  unsigned NumExprs;

  OffsetOfExpr(const ASTContext &C, QualType type,
               SourceLocation OperatorLoc, TypeSourceInfo *tsi,
               ArrayRef<OffsetOfNode> comps, ArrayRef<Expr*> exprs,
               SourceLocation RParenLoc);

  explicit OffsetOfExpr(unsigned numComps, unsigned numExprs)
    : Expr(OffsetOfExprClass, EmptyShell()),
      TSInfo(nullptr), NumComps(numComps), NumExprs(numExprs) {}

public:

  static OffsetOfExpr *Create(const ASTContext &C, QualType type,
                              SourceLocation OperatorLoc, TypeSourceInfo *tsi,
                              ArrayRef<OffsetOfNode> comps,
                              ArrayRef<Expr*> exprs, SourceLocation RParenLoc);

  static OffsetOfExpr *CreateEmpty(const ASTContext &C,
                                   unsigned NumComps, unsigned NumExprs);

  /// getOperatorLoc - Return the location of the operator.
  SourceLocation getOperatorLoc() const { return OperatorLoc; }
  void setOperatorLoc(SourceLocation L) { OperatorLoc = L; }

  /// \brief Return the location of the right parentheses.
  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation R) { RParenLoc = R; }

  TypeSourceInfo *getTypeSourceInfo() const {
    return TSInfo;
  }
  void setTypeSourceInfo(TypeSourceInfo *tsi) {
    TSInfo = tsi;
  }

  const OffsetOfNode &getComponent(unsigned Idx) const {
    assert(Idx < NumComps && "Subscript out of range");
    return reinterpret_cast<const OffsetOfNode *> (this + 1)[Idx];
  }

  void setComponent(unsigned Idx, OffsetOfNode ON) {
    assert(Idx < NumComps && "Subscript out of range");
    reinterpret_cast<OffsetOfNode *> (this + 1)[Idx] = ON;
  }

  unsigned getNumComponents() const {
    return NumComps;
  }

  Expr* getIndexExpr(unsigned Idx) {
    assert(Idx < NumExprs && "Subscript out of range");
    return reinterpret_cast<Expr **>(
                    reinterpret_cast<OffsetOfNode *>(this+1) + NumComps)[Idx];
  }
  const Expr *getIndexExpr(unsigned Idx) const {
    return const_cast<OffsetOfExpr*>(this)->getIndexExpr(Idx);
  }

  void setIndexExpr(unsigned Idx, Expr* E) {
    assert(Idx < NumComps && "Subscript out of range");
    reinterpret_cast<Expr **>(
                reinterpret_cast<OffsetOfNode *>(this+1) + NumComps)[Idx] = E;
  }

  unsigned getNumExpressions() const {
    return NumExprs;
  }

  SourceLocation getLocStart() const LLVM_READONLY { return OperatorLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == OffsetOfExprClass;
  }

  // Iterators
  child_range children() {
    Stmt **begin =
      reinterpret_cast<Stmt**>(reinterpret_cast<OffsetOfNode*>(this + 1)
                               + NumComps);
    return child_range(begin, begin + NumExprs);
  }
};

/// UnaryExprOrTypeTraitExpr - expression with either a type or (unevaluated)
/// expression operand.  Used for sizeof/alignof (C99 6.5.3.4) and
/// vec_step (OpenCL 1.1 6.11.12).
class UnaryExprOrTypeTraitExpr : public Expr {
  union {
    TypeSourceInfo *Ty;
    Stmt *Ex;
  } Argument;
  SourceLocation OpLoc, RParenLoc;

public:
  UnaryExprOrTypeTraitExpr(UnaryExprOrTypeTrait ExprKind, TypeSourceInfo *TInfo,
                           QualType resultType, SourceLocation op,
                           SourceLocation rp) :
      Expr(UnaryExprOrTypeTraitExprClass, resultType, VK_RValue, OK_Ordinary,
           false, // Never type-dependent (C++ [temp.dep.expr]p3).
           // Value-dependent if the argument is type-dependent.
           TInfo->getType()->isDependentType(),
           TInfo->getType()->isInstantiationDependentType(),
           TInfo->getType()->containsUnexpandedParameterPack()),
      OpLoc(op), RParenLoc(rp) {
    UnaryExprOrTypeTraitExprBits.Kind = ExprKind;
    UnaryExprOrTypeTraitExprBits.IsType = true;
    Argument.Ty = TInfo;
  }

  UnaryExprOrTypeTraitExpr(UnaryExprOrTypeTrait ExprKind, Expr *E,
                           QualType resultType, SourceLocation op,
                           SourceLocation rp);

  /// \brief Construct an empty sizeof/alignof expression.
  explicit UnaryExprOrTypeTraitExpr(EmptyShell Empty)
    : Expr(UnaryExprOrTypeTraitExprClass, Empty) { }

  UnaryExprOrTypeTrait getKind() const {
    return static_cast<UnaryExprOrTypeTrait>(UnaryExprOrTypeTraitExprBits.Kind);
  }
  void setKind(UnaryExprOrTypeTrait K) { UnaryExprOrTypeTraitExprBits.Kind = K;}

  bool isArgumentType() const { return UnaryExprOrTypeTraitExprBits.IsType; }
  QualType getArgumentType() const {
    return getArgumentTypeInfo()->getType();
  }
  TypeSourceInfo *getArgumentTypeInfo() const {
    assert(isArgumentType() && "calling getArgumentType() when arg is expr");
    return Argument.Ty;
  }
  Expr *getArgumentExpr() {
    assert(!isArgumentType() && "calling getArgumentExpr() when arg is type");
    return static_cast<Expr*>(Argument.Ex);
  }
  const Expr *getArgumentExpr() const {
    return const_cast<UnaryExprOrTypeTraitExpr*>(this)->getArgumentExpr();
  }

  void setArgument(Expr *E) {
    Argument.Ex = E;
    UnaryExprOrTypeTraitExprBits.IsType = false;
  }
  void setArgument(TypeSourceInfo *TInfo) {
    Argument.Ty = TInfo;
    UnaryExprOrTypeTraitExprBits.IsType = true;
  }

  /// Gets the argument type, or the type of the argument expression, whichever
  /// is appropriate.
  QualType getTypeOfArgument() const {
    return isArgumentType() ? getArgumentType() : getArgumentExpr()->getType();
  }

  SourceLocation getOperatorLoc() const { return OpLoc; }
  void setOperatorLoc(SourceLocation L) { OpLoc = L; }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return OpLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == UnaryExprOrTypeTraitExprClass;
  }

  // Iterators
  child_range children();
};

//===----------------------------------------------------------------------===//
// Postfix Operators.
//===----------------------------------------------------------------------===//

/// ArraySubscriptExpr - [C99 6.5.2.1] Array Subscripting.
class ArraySubscriptExpr : public Expr {
  enum { LHS, RHS, END_EXPR=2 };
  Stmt* SubExprs[END_EXPR];
  SourceLocation RBracketLoc;
public:
  ArraySubscriptExpr(Expr *lhs, Expr *rhs, QualType t,
                     ExprValueKind VK, ExprObjectKind OK,
                     SourceLocation rbracketloc)
  : Expr(ArraySubscriptExprClass, t, VK, OK,
         lhs->isTypeDependent() || rhs->isTypeDependent(),
         lhs->isValueDependent() || rhs->isValueDependent(),
         (lhs->isInstantiationDependent() ||
          rhs->isInstantiationDependent()),
         (lhs->containsUnexpandedParameterPack() ||
          rhs->containsUnexpandedParameterPack())),
    RBracketLoc(rbracketloc) {
    SubExprs[LHS] = lhs;
    SubExprs[RHS] = rhs;
  }

  /// \brief Create an empty array subscript expression.
  explicit ArraySubscriptExpr(EmptyShell Shell)
    : Expr(ArraySubscriptExprClass, Shell) { }

  /// An array access can be written A[4] or 4[A] (both are equivalent).
  /// - getBase() and getIdx() always present the normalized view: A[4].
  ///    In this case getBase() returns "A" and getIdx() returns "4".
  /// - getLHS() and getRHS() present the syntactic view. e.g. for
  ///    4[A] getLHS() returns "4".
  /// Note: Because vector element access is also written A[4] we must
  /// predicate the format conversion in getBase and getIdx only on the
  /// the type of the RHS, as it is possible for the LHS to be a vector of
  /// integer type
  Expr *getLHS() { return cast<Expr>(SubExprs[LHS]); }
  const Expr *getLHS() const { return cast<Expr>(SubExprs[LHS]); }
  void setLHS(Expr *E) { SubExprs[LHS] = E; }

  Expr *getRHS() { return cast<Expr>(SubExprs[RHS]); }
  const Expr *getRHS() const { return cast<Expr>(SubExprs[RHS]); }
  void setRHS(Expr *E) { SubExprs[RHS] = E; }

  Expr *getBase() {
    return cast<Expr>(getRHS()->getType()->isIntegerType() ? getLHS():getRHS());
  }

  const Expr *getBase() const {
    return cast<Expr>(getRHS()->getType()->isIntegerType() ? getLHS():getRHS());
  }

  Expr *getIdx() {
    return cast<Expr>(getRHS()->getType()->isIntegerType() ? getRHS():getLHS());
  }

  const Expr *getIdx() const {
    return cast<Expr>(getRHS()->getType()->isIntegerType() ? getRHS():getLHS());
  }

  SourceLocation getLocStart() const LLVM_READONLY {
    return getLHS()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY { return RBracketLoc; }

  SourceLocation getRBracketLoc() const { return RBracketLoc; }
  void setRBracketLoc(SourceLocation L) { RBracketLoc = L; }

  SourceLocation getExprLoc() const LLVM_READONLY {
    return getBase()->getExprLoc();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ArraySubscriptExprClass;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[0]+END_EXPR);
  }
};


/// CallExpr - Represents a function call (C99 6.5.2.2, C++ [expr.call]).
/// CallExpr itself represents a normal function call, e.g., "f(x, 2)",
/// while its subclasses may represent alternative syntax that (semantically)
/// results in a function call. For example, CXXOperatorCallExpr is
/// a subclass for overloaded operator calls that use operator syntax, e.g.,
/// "str1 + str2" to resolve to a function call.
class CallExpr : public Expr {
  enum { FN=0, PREARGS_START=1 };
  Stmt **SubExprs;
  unsigned NumArgs;
  SourceLocation RParenLoc;

protected:
  // These versions of the constructor are for derived classes.
  CallExpr(const ASTContext& C, StmtClass SC, Expr *fn, unsigned NumPreArgs,
           ArrayRef<Expr*> args, QualType t, ExprValueKind VK,
           SourceLocation rparenloc);
  CallExpr(const ASTContext &C, StmtClass SC, unsigned NumPreArgs,
           EmptyShell Empty);

  Stmt *getPreArg(unsigned i) {
    assert(i < getNumPreArgs() && "Prearg access out of range!");
    return SubExprs[PREARGS_START+i];
  }
  const Stmt *getPreArg(unsigned i) const {
    assert(i < getNumPreArgs() && "Prearg access out of range!");
    return SubExprs[PREARGS_START+i];
  }
  void setPreArg(unsigned i, Stmt *PreArg) {
    assert(i < getNumPreArgs() && "Prearg access out of range!");
    SubExprs[PREARGS_START+i] = PreArg;
  }

  unsigned getNumPreArgs() const { return CallExprBits.NumPreArgs; }

public:
  CallExpr(const ASTContext& C, Expr *fn, ArrayRef<Expr*> args, QualType t,
           ExprValueKind VK, SourceLocation rparenloc);

  /// \brief Build an empty call expression.
  CallExpr(const ASTContext &C, StmtClass SC, EmptyShell Empty);

  const Expr *getCallee() const { return cast<Expr>(SubExprs[FN]); }
  Expr *getCallee() { return cast<Expr>(SubExprs[FN]); }
  void setCallee(Expr *F) { SubExprs[FN] = F; }

  Decl *getCalleeDecl();
  const Decl *getCalleeDecl() const {
    return const_cast<CallExpr*>(this)->getCalleeDecl();
  }

  /// \brief If the callee is a FunctionDecl, return it. Otherwise return 0.
  FunctionDecl *getDirectCallee();
  const FunctionDecl *getDirectCallee() const {
    return const_cast<CallExpr*>(this)->getDirectCallee();
  }

  /// getNumArgs - Return the number of actual arguments to this call.
  ///
  unsigned getNumArgs() const { return NumArgs; }

  /// \brief Retrieve the call arguments.
  Expr **getArgs() {
    return reinterpret_cast<Expr **>(SubExprs+getNumPreArgs()+PREARGS_START);
  }
  const Expr *const *getArgs() const {
    return const_cast<CallExpr*>(this)->getArgs();
  }

  /// getArg - Return the specified argument.
  Expr *getArg(unsigned Arg) {
    assert(Arg < NumArgs && "Arg access out of range!");
    return cast_or_null<Expr>(SubExprs[Arg + getNumPreArgs() + PREARGS_START]);
  }
  const Expr *getArg(unsigned Arg) const {
    assert(Arg < NumArgs && "Arg access out of range!");
    return cast_or_null<Expr>(SubExprs[Arg + getNumPreArgs() + PREARGS_START]);
  }

  /// setArg - Set the specified argument.
  void setArg(unsigned Arg, Expr *ArgExpr) {
    assert(Arg < NumArgs && "Arg access out of range!");
    SubExprs[Arg+getNumPreArgs()+PREARGS_START] = ArgExpr;
  }

  /// setNumArgs - This changes the number of arguments present in this call.
  /// Any orphaned expressions are deleted by this, and any new operands are set
  /// to null.
  void setNumArgs(const ASTContext& C, unsigned NumArgs);

  typedef ExprIterator arg_iterator;
  typedef ConstExprIterator const_arg_iterator;
  typedef llvm::iterator_range<arg_iterator> arg_range;
  typedef llvm::iterator_range<const_arg_iterator> arg_const_range;

  arg_range arguments() { return arg_range(arg_begin(), arg_end()); }
  arg_const_range arguments() const {
    return arg_const_range(arg_begin(), arg_end());
  }

  arg_iterator arg_begin() { return SubExprs+PREARGS_START+getNumPreArgs(); }
  arg_iterator arg_end() {
    return SubExprs+PREARGS_START+getNumPreArgs()+getNumArgs();
  }
  const_arg_iterator arg_begin() const {
    return SubExprs+PREARGS_START+getNumPreArgs();
  }
  const_arg_iterator arg_end() const {
    return SubExprs+PREARGS_START+getNumPreArgs()+getNumArgs();
  }

  /// This method provides fast access to all the subexpressions of
  /// a CallExpr without going through the slower virtual child_iterator
  /// interface.  This provides efficient reverse iteration of the
  /// subexpressions.  This is currently used for CFG construction.
  ArrayRef<Stmt*> getRawSubExprs() {
    return llvm::makeArrayRef(SubExprs,
                              getNumPreArgs() + PREARGS_START + getNumArgs());
  }

  /// getNumCommas - Return the number of commas that must have been present in
  /// this function call.
  unsigned getNumCommas() const { return NumArgs ? NumArgs - 1 : 0; }

  /// getBuiltinCallee - If this is a call to a builtin, return the builtin ID
  /// of the callee. If not, return 0.
  unsigned getBuiltinCallee() const;

  /// \brief Returns \c true if this is a call to a builtin which does not
  /// evaluate side-effects within its arguments.
  bool isUnevaluatedBuiltinCall(const ASTContext &Ctx) const;

  /// getCallReturnType - Get the return type of the call expr. This is not
  /// always the type of the expr itself, if the return type is a reference
  /// type.
  QualType getCallReturnType(const ASTContext &Ctx) const;

  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY;
  SourceLocation getLocEnd() const LLVM_READONLY;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() >= firstCallExprConstant &&
           T->getStmtClass() <= lastCallExprConstant;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0],
                       &SubExprs[0]+NumArgs+getNumPreArgs()+PREARGS_START);
  }
};

/// MemberExpr - [C99 6.5.2.3] Structure and Union Members.  X->F and X.F.
///
class MemberExpr : public Expr {
  /// Extra data stored in some member expressions.
  struct MemberNameQualifier {
    /// \brief The nested-name-specifier that qualifies the name, including
    /// source-location information.
    NestedNameSpecifierLoc QualifierLoc;

    /// \brief The DeclAccessPair through which the MemberDecl was found due to
    /// name qualifiers.
    DeclAccessPair FoundDecl;
  };

  /// Base - the expression for the base pointer or structure references.  In
  /// X.F, this is "X".
  Stmt *Base;

  /// MemberDecl - This is the decl being referenced by the field/member name.
  /// In X.F, this is the decl referenced by F.
  ValueDecl *MemberDecl;

  /// MemberDNLoc - Provides source/type location info for the
  /// declaration name embedded in MemberDecl.
  DeclarationNameLoc MemberDNLoc;

  /// MemberLoc - This is the location of the member name.
  SourceLocation MemberLoc;

  /// This is the location of the -> or . in the expression.
  SourceLocation OperatorLoc;

  /// IsArrow - True if this is "X->F", false if this is "X.F".
  bool IsArrow : 1;

  /// \brief True if this member expression used a nested-name-specifier to
  /// refer to the member, e.g., "x->Base::f", or found its member via a using
  /// declaration.  When true, a MemberNameQualifier
  /// structure is allocated immediately after the MemberExpr.
  bool HasQualifierOrFoundDecl : 1;

  /// \brief True if this member expression specified a template keyword
  /// and/or a template argument list explicitly, e.g., x->f<int>,
  /// x->template f, x->template f<int>.
  /// When true, an ASTTemplateKWAndArgsInfo structure and its
  /// TemplateArguments (if any) are allocated immediately after
  /// the MemberExpr or, if the member expression also has a qualifier,
  /// after the MemberNameQualifier structure.
  bool HasTemplateKWAndArgsInfo : 1;

  /// \brief True if this member expression refers to a method that
  /// was resolved from an overloaded set having size greater than 1.
  bool HadMultipleCandidates : 1;

  /// \brief Retrieve the qualifier that preceded the member name, if any.
  MemberNameQualifier *getMemberQualifier() {
    assert(HasQualifierOrFoundDecl);
    return reinterpret_cast<MemberNameQualifier *> (this + 1);
  }

  /// \brief Retrieve the qualifier that preceded the member name, if any.
  const MemberNameQualifier *getMemberQualifier() const {
    return const_cast<MemberExpr *>(this)->getMemberQualifier();
  }

public:
  MemberExpr(Expr *base, bool isarrow, SourceLocation operatorloc,
             ValueDecl *memberdecl, const DeclarationNameInfo &NameInfo,
             QualType ty, ExprValueKind VK, ExprObjectKind OK)
      : Expr(MemberExprClass, ty, VK, OK, base->isTypeDependent(),
             base->isValueDependent(), base->isInstantiationDependent(),
             base->containsUnexpandedParameterPack()),
        Base(base), MemberDecl(memberdecl), MemberDNLoc(NameInfo.getInfo()),
        MemberLoc(NameInfo.getLoc()), OperatorLoc(operatorloc),
        IsArrow(isarrow), HasQualifierOrFoundDecl(false),
        HasTemplateKWAndArgsInfo(false), HadMultipleCandidates(false) {
    assert(memberdecl->getDeclName() == NameInfo.getName());
  }

  // NOTE: this constructor should be used only when it is known that
  // the member name can not provide additional syntactic info
  // (i.e., source locations for C++ operator names or type source info
  // for constructors, destructors and conversion operators).
  MemberExpr(Expr *base, bool isarrow, SourceLocation operatorloc,
             ValueDecl *memberdecl, SourceLocation l, QualType ty,
             ExprValueKind VK, ExprObjectKind OK)
      : Expr(MemberExprClass, ty, VK, OK, base->isTypeDependent(),
             base->isValueDependent(), base->isInstantiationDependent(),
             base->containsUnexpandedParameterPack()),
        Base(base), MemberDecl(memberdecl), MemberDNLoc(), MemberLoc(l),
        OperatorLoc(operatorloc), IsArrow(isarrow),
        HasQualifierOrFoundDecl(false), HasTemplateKWAndArgsInfo(false),
        HadMultipleCandidates(false) {}

  static MemberExpr *Create(const ASTContext &C, Expr *base, bool isarrow,
                            SourceLocation OperatorLoc,
                            NestedNameSpecifierLoc QualifierLoc,
                            SourceLocation TemplateKWLoc, ValueDecl *memberdecl,
                            DeclAccessPair founddecl,
                            DeclarationNameInfo MemberNameInfo,
                            const TemplateArgumentListInfo *targs, QualType ty,
                            ExprValueKind VK, ExprObjectKind OK);

  void setBase(Expr *E) { Base = E; }
  Expr *getBase() const { return cast<Expr>(Base); }

  /// \brief Retrieve the member declaration to which this expression refers.
  ///
  /// The returned declaration will either be a FieldDecl or (in C++)
  /// a CXXMethodDecl.
  ValueDecl *getMemberDecl() const { return MemberDecl; }
  void setMemberDecl(ValueDecl *D) { MemberDecl = D; }

  /// \brief Retrieves the declaration found by lookup.
  DeclAccessPair getFoundDecl() const {
    if (!HasQualifierOrFoundDecl)
      return DeclAccessPair::make(getMemberDecl(),
                                  getMemberDecl()->getAccess());
    return getMemberQualifier()->FoundDecl;
  }

  /// \brief Determines whether this member expression actually had
  /// a C++ nested-name-specifier prior to the name of the member, e.g.,
  /// x->Base::foo.
  bool hasQualifier() const { return getQualifier() != nullptr; }

  /// \brief If the member name was qualified, retrieves the
  /// nested-name-specifier that precedes the member name. Otherwise, returns
  /// NULL.
  NestedNameSpecifier *getQualifier() const {
    if (!HasQualifierOrFoundDecl)
      return nullptr;

    return getMemberQualifier()->QualifierLoc.getNestedNameSpecifier();
  }

  /// \brief If the member name was qualified, retrieves the
  /// nested-name-specifier that precedes the member name, with source-location
  /// information.
  NestedNameSpecifierLoc getQualifierLoc() const {
    if (!hasQualifier())
      return NestedNameSpecifierLoc();

    return getMemberQualifier()->QualifierLoc;
  }

  /// \brief Return the optional template keyword and arguments info.
  ASTTemplateKWAndArgsInfo *getTemplateKWAndArgsInfo() {
    if (!HasTemplateKWAndArgsInfo)
      return nullptr;

    if (!HasQualifierOrFoundDecl)
      return reinterpret_cast<ASTTemplateKWAndArgsInfo *>(this + 1);

    return reinterpret_cast<ASTTemplateKWAndArgsInfo *>(
                                                      getMemberQualifier() + 1);
  }

  /// \brief Return the optional template keyword and arguments info.
  const ASTTemplateKWAndArgsInfo *getTemplateKWAndArgsInfo() const {
    return const_cast<MemberExpr*>(this)->getTemplateKWAndArgsInfo();
  }

  /// \brief Retrieve the location of the template keyword preceding
  /// the member name, if any.
  SourceLocation getTemplateKeywordLoc() const {
    if (!HasTemplateKWAndArgsInfo) return SourceLocation();
    return getTemplateKWAndArgsInfo()->getTemplateKeywordLoc();
  }

  /// \brief Retrieve the location of the left angle bracket starting the
  /// explicit template argument list following the member name, if any.
  SourceLocation getLAngleLoc() const {
    if (!HasTemplateKWAndArgsInfo) return SourceLocation();
    return getTemplateKWAndArgsInfo()->LAngleLoc;
  }

  /// \brief Retrieve the location of the right angle bracket ending the
  /// explicit template argument list following the member name, if any.
  SourceLocation getRAngleLoc() const {
    if (!HasTemplateKWAndArgsInfo) return SourceLocation();
    return getTemplateKWAndArgsInfo()->RAngleLoc;
  }

  /// Determines whether the member name was preceded by the template keyword.
  bool hasTemplateKeyword() const { return getTemplateKeywordLoc().isValid(); }

  /// \brief Determines whether the member name was followed by an
  /// explicit template argument list.
  bool hasExplicitTemplateArgs() const { return getLAngleLoc().isValid(); }

  /// \brief Copies the template arguments (if present) into the given
  /// structure.
  void copyTemplateArgumentsInto(TemplateArgumentListInfo &List) const {
    if (hasExplicitTemplateArgs())
      getExplicitTemplateArgs().copyInto(List);
  }

  /// \brief Retrieve the explicit template argument list that
  /// follow the member template name.  This must only be called on an
  /// expression with explicit template arguments.
  ASTTemplateArgumentListInfo &getExplicitTemplateArgs() {
    assert(hasExplicitTemplateArgs());
    return *getTemplateKWAndArgsInfo();
  }

  /// \brief Retrieve the explicit template argument list that
  /// followed the member template name.  This must only be called on
  /// an expression with explicit template arguments.
  const ASTTemplateArgumentListInfo &getExplicitTemplateArgs() const {
    return const_cast<MemberExpr *>(this)->getExplicitTemplateArgs();
  }

  /// \brief Retrieves the optional explicit template arguments.
  /// This points to the same data as getExplicitTemplateArgs(), but
  /// returns null if there are no explicit template arguments.
  const ASTTemplateArgumentListInfo *getOptionalExplicitTemplateArgs() const {
    if (!hasExplicitTemplateArgs()) return nullptr;
    return &getExplicitTemplateArgs();
  }

  /// \brief Retrieve the template arguments provided as part of this
  /// template-id.
  const TemplateArgumentLoc *getTemplateArgs() const {
    if (!hasExplicitTemplateArgs())
      return nullptr;

    return getExplicitTemplateArgs().getTemplateArgs();
  }

  /// \brief Retrieve the number of template arguments provided as part of this
  /// template-id.
  unsigned getNumTemplateArgs() const {
    if (!hasExplicitTemplateArgs())
      return 0;

    return getExplicitTemplateArgs().NumTemplateArgs;
  }

  /// \brief Retrieve the member declaration name info.
  DeclarationNameInfo getMemberNameInfo() const {
    return DeclarationNameInfo(MemberDecl->getDeclName(),
                               MemberLoc, MemberDNLoc);
  }

  SourceLocation getOperatorLoc() const LLVM_READONLY { return OperatorLoc; }

  bool isArrow() const { return IsArrow; }
  void setArrow(bool A) { IsArrow = A; }

  /// getMemberLoc - Return the location of the "member", in X->F, it is the
  /// location of 'F'.
  SourceLocation getMemberLoc() const { return MemberLoc; }
  void setMemberLoc(SourceLocation L) { MemberLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY;
  SourceLocation getLocEnd() const LLVM_READONLY;

  SourceLocation getExprLoc() const LLVM_READONLY { return MemberLoc; }

  /// \brief Determine whether the base of this explicit is implicit.
  bool isImplicitAccess() const {
    return getBase() && getBase()->isImplicitCXXThis();
  }

  /// \brief Returns true if this member expression refers to a method that
  /// was resolved from an overloaded set having size greater than 1.
  bool hadMultipleCandidates() const {
    return HadMultipleCandidates;
  }
  /// \brief Sets the flag telling whether this expression refers to
  /// a method that was resolved from an overloaded set having size
  /// greater than 1.
  void setHadMultipleCandidates(bool V = true) {
    HadMultipleCandidates = V;
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == MemberExprClass;
  }

  // Iterators
  child_range children() { return child_range(&Base, &Base+1); }

  friend class ASTReader;
  friend class ASTStmtWriter;
};

/// CompoundLiteralExpr - [C99 6.5.2.5]
///
class CompoundLiteralExpr : public Expr {
  /// LParenLoc - If non-null, this is the location of the left paren in a
  /// compound literal like "(int){4}".  This can be null if this is a
  /// synthesized compound expression.
  SourceLocation LParenLoc;

  /// The type as written.  This can be an incomplete array type, in
  /// which case the actual expression type will be different.
  /// The int part of the pair stores whether this expr is file scope.
  llvm::PointerIntPair<TypeSourceInfo *, 1, bool> TInfoAndScope;
  Stmt *Init;
public:
  CompoundLiteralExpr(SourceLocation lparenloc, TypeSourceInfo *tinfo,
                      QualType T, ExprValueKind VK, Expr *init, bool fileScope)
    : Expr(CompoundLiteralExprClass, T, VK, OK_Ordinary,
           tinfo->getType()->isDependentType(),
           init->isValueDependent(),
           (init->isInstantiationDependent() ||
            tinfo->getType()->isInstantiationDependentType()),
           init->containsUnexpandedParameterPack()),
      LParenLoc(lparenloc), TInfoAndScope(tinfo, fileScope), Init(init) {}

  /// \brief Construct an empty compound literal.
  explicit CompoundLiteralExpr(EmptyShell Empty)
    : Expr(CompoundLiteralExprClass, Empty) { }

  const Expr *getInitializer() const { return cast<Expr>(Init); }
  Expr *getInitializer() { return cast<Expr>(Init); }
  void setInitializer(Expr *E) { Init = E; }

  bool isFileScope() const { return TInfoAndScope.getInt(); }
  void setFileScope(bool FS) { TInfoAndScope.setInt(FS); }

  SourceLocation getLParenLoc() const { return LParenLoc; }
  void setLParenLoc(SourceLocation L) { LParenLoc = L; }

  TypeSourceInfo *getTypeSourceInfo() const {
    return TInfoAndScope.getPointer();
  }
  void setTypeSourceInfo(TypeSourceInfo *tinfo) {
    TInfoAndScope.setPointer(tinfo);
  }

  SourceLocation getLocStart() const LLVM_READONLY {
    // FIXME: Init should never be null.
    if (!Init)
      return SourceLocation();
    if (LParenLoc.isInvalid())
      return Init->getLocStart();
    return LParenLoc;
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    // FIXME: Init should never be null.
    if (!Init)
      return SourceLocation();
    return Init->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CompoundLiteralExprClass;
  }

  // Iterators
  child_range children() { return child_range(&Init, &Init+1); }
};

/// CastExpr - Base class for type casts, including both implicit
/// casts (ImplicitCastExpr) and explicit casts that have some
/// representation in the source code (ExplicitCastExpr's derived
/// classes).
class CastExpr : public Expr {
private:
  Stmt *Op;

  bool CastConsistency() const;

  const CXXBaseSpecifier * const *path_buffer() const {
    return const_cast<CastExpr*>(this)->path_buffer();
  }
  CXXBaseSpecifier **path_buffer();

  void setBasePathSize(unsigned basePathSize) {
    CastExprBits.BasePathSize = basePathSize;
    assert(CastExprBits.BasePathSize == basePathSize &&
           "basePathSize doesn't fit in bits of CastExprBits.BasePathSize!");
  }

protected:
  CastExpr(StmtClass SC, QualType ty, ExprValueKind VK, const CastKind kind,
           Expr *op, unsigned BasePathSize)
      : Expr(SC, ty, VK, OK_Ordinary,
             // Cast expressions are type-dependent if the type is
             // dependent (C++ [temp.dep.expr]p3).
             ty->isDependentType(),
             // Cast expressions are value-dependent if the type is
             // dependent or if the subexpression is value-dependent.
             ty->isDependentType() || (op && op->isValueDependent()),
             (ty->isInstantiationDependentType() ||
              (op && op->isInstantiationDependent())),
             // An implicit cast expression doesn't (lexically) contain an
             // unexpanded pack, even if its target type does.
             ((SC != ImplicitCastExprClass &&
               ty->containsUnexpandedParameterPack()) ||
              (op && op->containsUnexpandedParameterPack()))),
        Op(op) {
    assert(kind != CK_Invalid && "creating cast with invalid cast kind");
    CastExprBits.Kind = kind;
    setBasePathSize(BasePathSize);
    assert(CastConsistency());
  }

  /// \brief Construct an empty cast.
  CastExpr(StmtClass SC, EmptyShell Empty, unsigned BasePathSize)
    : Expr(SC, Empty) {
    setBasePathSize(BasePathSize);
  }

public:
  CastKind getCastKind() const { return (CastKind) CastExprBits.Kind; }
  void setCastKind(CastKind K) { CastExprBits.Kind = K; }
  const char *getCastKindName() const;

  Expr *getSubExpr() { return cast<Expr>(Op); }
  const Expr *getSubExpr() const { return cast<Expr>(Op); }
  void setSubExpr(Expr *E) { Op = E; }

  /// \brief Retrieve the cast subexpression as it was written in the source
  /// code, looking through any implicit casts or other intermediate nodes
  /// introduced by semantic analysis.
  Expr *getSubExprAsWritten();
  const Expr *getSubExprAsWritten() const {
    return const_cast<CastExpr *>(this)->getSubExprAsWritten();
  }

  typedef CXXBaseSpecifier **path_iterator;
  typedef const CXXBaseSpecifier * const *path_const_iterator;
  bool path_empty() const { return CastExprBits.BasePathSize == 0; }
  unsigned path_size() const { return CastExprBits.BasePathSize; }
  path_iterator path_begin() { return path_buffer(); }
  path_iterator path_end() { return path_buffer() + path_size(); }
  path_const_iterator path_begin() const { return path_buffer(); }
  path_const_iterator path_end() const { return path_buffer() + path_size(); }

  void setCastPath(const CXXCastPath &Path);

  static bool classof(const Stmt *T) {
    return T->getStmtClass() >= firstCastExprConstant &&
           T->getStmtClass() <= lastCastExprConstant;
  }

  // Iterators
  child_range children() { return child_range(&Op, &Op+1); }
};

/// ImplicitCastExpr - Allows us to explicitly represent implicit type
/// conversions, which have no direct representation in the original
/// source code. For example: converting T[]->T*, void f()->void
/// (*f)(), float->double, short->int, etc.
///
/// In C, implicit casts always produce rvalues. However, in C++, an
/// implicit cast whose result is being bound to a reference will be
/// an lvalue or xvalue. For example:
///
/// @code
/// class Base { };
/// class Derived : public Base { };
/// Derived &&ref();
/// void f(Derived d) {
///   Base& b = d; // initializer is an ImplicitCastExpr
///                // to an lvalue of type Base
///   Base&& r = ref(); // initializer is an ImplicitCastExpr
///                     // to an xvalue of type Base
/// }
/// @endcode
class ImplicitCastExpr : public CastExpr {
private:
  ImplicitCastExpr(QualType ty, CastKind kind, Expr *op,
                   unsigned BasePathLength, ExprValueKind VK)
    : CastExpr(ImplicitCastExprClass, ty, VK, kind, op, BasePathLength) {
  }

  /// \brief Construct an empty implicit cast.
  explicit ImplicitCastExpr(EmptyShell Shell, unsigned PathSize)
    : CastExpr(ImplicitCastExprClass, Shell, PathSize) { }

public:
  enum OnStack_t { OnStack };
  ImplicitCastExpr(OnStack_t _, QualType ty, CastKind kind, Expr *op,
                   ExprValueKind VK)
    : CastExpr(ImplicitCastExprClass, ty, VK, kind, op, 0) {
  }

  static ImplicitCastExpr *Create(const ASTContext &Context, QualType T,
                                  CastKind Kind, Expr *Operand,
                                  const CXXCastPath *BasePath,
                                  ExprValueKind Cat);

  static ImplicitCastExpr *CreateEmpty(const ASTContext &Context,
                                       unsigned PathSize);

  SourceLocation getLocStart() const LLVM_READONLY {
    return getSubExpr()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getSubExpr()->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ImplicitCastExprClass;
  }
};

inline Expr *Expr::IgnoreImpCasts() {
  Expr *e = this;
  while (ImplicitCastExpr *ice = dyn_cast<ImplicitCastExpr>(e))
    e = ice->getSubExpr();
  return e;
}

/// ExplicitCastExpr - An explicit cast written in the source
/// code.
///
/// This class is effectively an abstract class, because it provides
/// the basic representation of an explicitly-written cast without
/// specifying which kind of cast (C cast, functional cast, static
/// cast, etc.) was written; specific derived classes represent the
/// particular style of cast and its location information.
///
/// Unlike implicit casts, explicit cast nodes have two different
/// types: the type that was written into the source code, and the
/// actual type of the expression as determined by semantic
/// analysis. These types may differ slightly. For example, in C++ one
/// can cast to a reference type, which indicates that the resulting
/// expression will be an lvalue or xvalue. The reference type, however,
/// will not be used as the type of the expression.
class ExplicitCastExpr : public CastExpr {
  /// TInfo - Source type info for the (written) type
  /// this expression is casting to.
  TypeSourceInfo *TInfo;

protected:
  ExplicitCastExpr(StmtClass SC, QualType exprTy, ExprValueKind VK,
                   CastKind kind, Expr *op, unsigned PathSize,
                   TypeSourceInfo *writtenTy)
    : CastExpr(SC, exprTy, VK, kind, op, PathSize), TInfo(writtenTy) {}

  /// \brief Construct an empty explicit cast.
  ExplicitCastExpr(StmtClass SC, EmptyShell Shell, unsigned PathSize)
    : CastExpr(SC, Shell, PathSize) { }

public:
  /// getTypeInfoAsWritten - Returns the type source info for the type
  /// that this expression is casting to.
  TypeSourceInfo *getTypeInfoAsWritten() const { return TInfo; }
  void setTypeInfoAsWritten(TypeSourceInfo *writtenTy) { TInfo = writtenTy; }

  /// getTypeAsWritten - Returns the type that this expression is
  /// casting to, as written in the source code.
  QualType getTypeAsWritten() const { return TInfo->getType(); }

  static bool classof(const Stmt *T) {
     return T->getStmtClass() >= firstExplicitCastExprConstant &&
            T->getStmtClass() <= lastExplicitCastExprConstant;
  }
};

/// CStyleCastExpr - An explicit cast in C (C99 6.5.4) or a C-style
/// cast in C++ (C++ [expr.cast]), which uses the syntax
/// (Type)expr. For example: @c (int)f.
class CStyleCastExpr : public ExplicitCastExpr {
  SourceLocation LPLoc; // the location of the left paren
  SourceLocation RPLoc; // the location of the right paren

  CStyleCastExpr(QualType exprTy, ExprValueKind vk, CastKind kind, Expr *op,
                 unsigned PathSize, TypeSourceInfo *writtenTy,
                 SourceLocation l, SourceLocation r)
    : ExplicitCastExpr(CStyleCastExprClass, exprTy, vk, kind, op, PathSize,
                       writtenTy), LPLoc(l), RPLoc(r) {}

  /// \brief Construct an empty C-style explicit cast.
  explicit CStyleCastExpr(EmptyShell Shell, unsigned PathSize)
    : ExplicitCastExpr(CStyleCastExprClass, Shell, PathSize) { }

public:
  static CStyleCastExpr *Create(const ASTContext &Context, QualType T,
                                ExprValueKind VK, CastKind K,
                                Expr *Op, const CXXCastPath *BasePath,
                                TypeSourceInfo *WrittenTy, SourceLocation L,
                                SourceLocation R);

  static CStyleCastExpr *CreateEmpty(const ASTContext &Context,
                                     unsigned PathSize);

  SourceLocation getLParenLoc() const { return LPLoc; }
  void setLParenLoc(SourceLocation L) { LPLoc = L; }

  SourceLocation getRParenLoc() const { return RPLoc; }
  void setRParenLoc(SourceLocation L) { RPLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return LPLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getSubExpr()->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == CStyleCastExprClass;
  }
};

/// \brief A builtin binary operation expression such as "x + y" or "x <= y".
///
/// This expression node kind describes a builtin binary operation,
/// such as "x + y" for integer values "x" and "y". The operands will
/// already have been converted to appropriate types (e.g., by
/// performing promotions or conversions).
///
/// In C++, where operators may be overloaded, a different kind of
/// expression node (CXXOperatorCallExpr) is used to express the
/// invocation of an overloaded operator with operator syntax. Within
/// a C++ template, whether BinaryOperator or CXXOperatorCallExpr is
/// used to store an expression "x + y" depends on the subexpressions
/// for x and y. If neither x or y is type-dependent, and the "+"
/// operator resolves to a built-in operation, BinaryOperator will be
/// used to express the computation (x and y may still be
/// value-dependent). If either x or y is type-dependent, or if the
/// "+" resolves to an overloaded operator, CXXOperatorCallExpr will
/// be used to express the computation.
class BinaryOperator : public Expr {
public:
  typedef BinaryOperatorKind Opcode;

private:
  unsigned Opc : 6;

  // Records the FP_CONTRACT pragma status at the point that this binary
  // operator was parsed. This bit is only meaningful for operations on
  // floating point types. For all other types it should default to
  // false.
  unsigned FPContractable : 1;
  SourceLocation OpLoc;

  enum { LHS, RHS, END_EXPR };
  Stmt* SubExprs[END_EXPR];
public:

  BinaryOperator(Expr *lhs, Expr *rhs, Opcode opc, QualType ResTy,
                 ExprValueKind VK, ExprObjectKind OK,
                 SourceLocation opLoc, bool fpContractable)
    : Expr(BinaryOperatorClass, ResTy, VK, OK,
           lhs->isTypeDependent() || rhs->isTypeDependent(),
           lhs->isValueDependent() || rhs->isValueDependent(),
           (lhs->isInstantiationDependent() ||
            rhs->isInstantiationDependent()),
           (lhs->containsUnexpandedParameterPack() ||
            rhs->containsUnexpandedParameterPack())),
      Opc(opc), FPContractable(fpContractable), OpLoc(opLoc) {
    SubExprs[LHS] = lhs;
    SubExprs[RHS] = rhs;
    assert(!isCompoundAssignmentOp() &&
           "Use CompoundAssignOperator for compound assignments");
  }

  /// \brief Construct an empty binary operator.
  explicit BinaryOperator(EmptyShell Empty)
    : Expr(BinaryOperatorClass, Empty), Opc(BO_Comma) { }

  SourceLocation getExprLoc() const LLVM_READONLY { return OpLoc; }
  SourceLocation getOperatorLoc() const { return OpLoc; }
  void setOperatorLoc(SourceLocation L) { OpLoc = L; }

  Opcode getOpcode() const { return static_cast<Opcode>(Opc); }
  void setOpcode(Opcode O) { Opc = O; }

  Expr *getLHS() const { return cast<Expr>(SubExprs[LHS]); }
  void setLHS(Expr *E) { SubExprs[LHS] = E; }
  Expr *getRHS() const { return cast<Expr>(SubExprs[RHS]); }
  void setRHS(Expr *E) { SubExprs[RHS] = E; }

  SourceLocation getLocStart() const LLVM_READONLY {
    return getLHS()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getRHS()->getLocEnd();
  }

  /// getOpcodeStr - Turn an Opcode enum value into the punctuation char it
  /// corresponds to, e.g. "<<=".
  static StringRef getOpcodeStr(Opcode Op);

  StringRef getOpcodeStr() const { return getOpcodeStr(getOpcode()); }

  /// \brief Retrieve the binary opcode that corresponds to the given
  /// overloaded operator.
  static Opcode getOverloadedOpcode(OverloadedOperatorKind OO);

  /// \brief Retrieve the overloaded operator kind that corresponds to
  /// the given binary opcode.
  static OverloadedOperatorKind getOverloadedOperator(Opcode Opc);

  /// predicates to categorize the respective opcodes.
  bool isPtrMemOp() const { return Opc == BO_PtrMemD || Opc == BO_PtrMemI; }
  bool isMultiplicativeOp() const { return Opc >= BO_Mul && Opc <= BO_Rem; }
  static bool isAdditiveOp(Opcode Opc) { return Opc == BO_Add || Opc==BO_Sub; }
  bool isAdditiveOp() const { return isAdditiveOp(getOpcode()); }
  static bool isShiftOp(Opcode Opc) { return Opc == BO_Shl || Opc == BO_Shr; }
  bool isShiftOp() const { return isShiftOp(getOpcode()); }

  static bool isBitwiseOp(Opcode Opc) { return Opc >= BO_And && Opc <= BO_Or; }
  bool isBitwiseOp() const { return isBitwiseOp(getOpcode()); }

  static bool isRelationalOp(Opcode Opc) { return Opc >= BO_LT && Opc<=BO_GE; }
  bool isRelationalOp() const { return isRelationalOp(getOpcode()); }

  static bool isEqualityOp(Opcode Opc) { return Opc == BO_EQ || Opc == BO_NE; }
  bool isEqualityOp() const { return isEqualityOp(getOpcode()); }

  static bool isComparisonOp(Opcode Opc) { return Opc >= BO_LT && Opc<=BO_NE; }
  bool isComparisonOp() const { return isComparisonOp(getOpcode()); }

  static Opcode negateComparisonOp(Opcode Opc) {
    switch (Opc) {
    default:
      llvm_unreachable("Not a comparsion operator.");
    case BO_LT: return BO_GE;
    case BO_GT: return BO_LE;
    case BO_LE: return BO_GT;
    case BO_GE: return BO_LT;
    case BO_EQ: return BO_NE;
    case BO_NE: return BO_EQ;
    }
  }

  static Opcode reverseComparisonOp(Opcode Opc) {
    switch (Opc) {
    default:
      llvm_unreachable("Not a comparsion operator.");
    case BO_LT: return BO_GT;
    case BO_GT: return BO_LT;
    case BO_LE: return BO_GE;
    case BO_GE: return BO_LE;
    case BO_EQ:
    case BO_NE:
      return Opc;
    }
  }

  static bool isLogicalOp(Opcode Opc) { return Opc == BO_LAnd || Opc==BO_LOr; }
  bool isLogicalOp() const { return isLogicalOp(getOpcode()); }

  static bool isAssignmentOp(Opcode Opc) {
    return Opc >= BO_Assign && Opc <= BO_OrAssign;
  }
  bool isAssignmentOp() const { return isAssignmentOp(getOpcode()); }

  static bool isCompoundAssignmentOp(Opcode Opc) {
    return Opc > BO_Assign && Opc <= BO_OrAssign;
  }
  bool isCompoundAssignmentOp() const {
    return isCompoundAssignmentOp(getOpcode());
  }
  static Opcode getOpForCompoundAssignment(Opcode Opc) {
    assert(isCompoundAssignmentOp(Opc));
    if (Opc >= BO_AndAssign)
      return Opcode(unsigned(Opc) - BO_AndAssign + BO_And);
    else
      return Opcode(unsigned(Opc) - BO_MulAssign + BO_Mul);
  }

  static bool isShiftAssignOp(Opcode Opc) {
    return Opc == BO_ShlAssign || Opc == BO_ShrAssign;
  }
  bool isShiftAssignOp() const {
    return isShiftAssignOp(getOpcode());
  }

  static bool classof(const Stmt *S) {
    return S->getStmtClass() >= firstBinaryOperatorConstant &&
           S->getStmtClass() <= lastBinaryOperatorConstant;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[0]+END_EXPR);
  }

  // Set the FP contractability status of this operator. Only meaningful for
  // operations on floating point types.
  void setFPContractable(bool FPC) { FPContractable = FPC; }

  // Get the FP contractability status of this operator. Only meaningful for
  // operations on floating point types.
  bool isFPContractable() const { return FPContractable; }

protected:
  BinaryOperator(Expr *lhs, Expr *rhs, Opcode opc, QualType ResTy,
                 ExprValueKind VK, ExprObjectKind OK,
                 SourceLocation opLoc, bool fpContractable, bool dead2)
    : Expr(CompoundAssignOperatorClass, ResTy, VK, OK,
           lhs->isTypeDependent() || rhs->isTypeDependent(),
           lhs->isValueDependent() || rhs->isValueDependent(),
           (lhs->isInstantiationDependent() ||
            rhs->isInstantiationDependent()),
           (lhs->containsUnexpandedParameterPack() ||
            rhs->containsUnexpandedParameterPack())),
      Opc(opc), FPContractable(fpContractable), OpLoc(opLoc) {
    SubExprs[LHS] = lhs;
    SubExprs[RHS] = rhs;
  }

  BinaryOperator(StmtClass SC, EmptyShell Empty)
    : Expr(SC, Empty), Opc(BO_MulAssign) { }
};

/// CompoundAssignOperator - For compound assignments (e.g. +=), we keep
/// track of the type the operation is performed in.  Due to the semantics of
/// these operators, the operands are promoted, the arithmetic performed, an
/// implicit conversion back to the result type done, then the assignment takes
/// place.  This captures the intermediate type which the computation is done
/// in.
class CompoundAssignOperator : public BinaryOperator {
  QualType ComputationLHSType;
  QualType ComputationResultType;
public:
  CompoundAssignOperator(Expr *lhs, Expr *rhs, Opcode opc, QualType ResType,
                         ExprValueKind VK, ExprObjectKind OK,
                         QualType CompLHSType, QualType CompResultType,
                         SourceLocation OpLoc, bool fpContractable)
    : BinaryOperator(lhs, rhs, opc, ResType, VK, OK, OpLoc, fpContractable,
                     true),
      ComputationLHSType(CompLHSType),
      ComputationResultType(CompResultType) {
    assert(isCompoundAssignmentOp() &&
           "Only should be used for compound assignments");
  }

  /// \brief Build an empty compound assignment operator expression.
  explicit CompoundAssignOperator(EmptyShell Empty)
    : BinaryOperator(CompoundAssignOperatorClass, Empty) { }

  // The two computation types are the type the LHS is converted
  // to for the computation and the type of the result; the two are
  // distinct in a few cases (specifically, int+=ptr and ptr-=ptr).
  QualType getComputationLHSType() const { return ComputationLHSType; }
  void setComputationLHSType(QualType T) { ComputationLHSType = T; }

  QualType getComputationResultType() const { return ComputationResultType; }
  void setComputationResultType(QualType T) { ComputationResultType = T; }

  static bool classof(const Stmt *S) {
    return S->getStmtClass() == CompoundAssignOperatorClass;
  }
};

/// AbstractConditionalOperator - An abstract base class for
/// ConditionalOperator and BinaryConditionalOperator.
class AbstractConditionalOperator : public Expr {
  SourceLocation QuestionLoc, ColonLoc;
  friend class ASTStmtReader;

protected:
  AbstractConditionalOperator(StmtClass SC, QualType T,
                              ExprValueKind VK, ExprObjectKind OK,
                              bool TD, bool VD, bool ID,
                              bool ContainsUnexpandedParameterPack,
                              SourceLocation qloc,
                              SourceLocation cloc)
    : Expr(SC, T, VK, OK, TD, VD, ID, ContainsUnexpandedParameterPack),
      QuestionLoc(qloc), ColonLoc(cloc) {}

  AbstractConditionalOperator(StmtClass SC, EmptyShell Empty)
    : Expr(SC, Empty) { }

public:
  // getCond - Return the expression representing the condition for
  //   the ?: operator.
  Expr *getCond() const;

  // getTrueExpr - Return the subexpression representing the value of
  //   the expression if the condition evaluates to true.
  Expr *getTrueExpr() const;

  // getFalseExpr - Return the subexpression representing the value of
  //   the expression if the condition evaluates to false.  This is
  //   the same as getRHS.
  Expr *getFalseExpr() const;

  SourceLocation getQuestionLoc() const { return QuestionLoc; }
  SourceLocation getColonLoc() const { return ColonLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ConditionalOperatorClass ||
           T->getStmtClass() == BinaryConditionalOperatorClass;
  }
};

/// ConditionalOperator - The ?: ternary operator.  The GNU "missing
/// middle" extension is a BinaryConditionalOperator.
class ConditionalOperator : public AbstractConditionalOperator {
  enum { COND, LHS, RHS, END_EXPR };
  Stmt* SubExprs[END_EXPR]; // Left/Middle/Right hand sides.

  friend class ASTStmtReader;
public:
  ConditionalOperator(Expr *cond, SourceLocation QLoc, Expr *lhs,
                      SourceLocation CLoc, Expr *rhs,
                      QualType t, ExprValueKind VK, ExprObjectKind OK)
    : AbstractConditionalOperator(ConditionalOperatorClass, t, VK, OK,
           // FIXME: the type of the conditional operator doesn't
           // depend on the type of the conditional, but the standard
           // seems to imply that it could. File a bug!
           (lhs->isTypeDependent() || rhs->isTypeDependent()),
           (cond->isValueDependent() || lhs->isValueDependent() ||
            rhs->isValueDependent()),
           (cond->isInstantiationDependent() ||
            lhs->isInstantiationDependent() ||
            rhs->isInstantiationDependent()),
           (cond->containsUnexpandedParameterPack() ||
            lhs->containsUnexpandedParameterPack() ||
            rhs->containsUnexpandedParameterPack()),
                                  QLoc, CLoc) {
    SubExprs[COND] = cond;
    SubExprs[LHS] = lhs;
    SubExprs[RHS] = rhs;
  }

  /// \brief Build an empty conditional operator.
  explicit ConditionalOperator(EmptyShell Empty)
    : AbstractConditionalOperator(ConditionalOperatorClass, Empty) { }

  // getCond - Return the expression representing the condition for
  //   the ?: operator.
  Expr *getCond() const { return cast<Expr>(SubExprs[COND]); }

  // getTrueExpr - Return the subexpression representing the value of
  //   the expression if the condition evaluates to true.
  Expr *getTrueExpr() const { return cast<Expr>(SubExprs[LHS]); }

  // getFalseExpr - Return the subexpression representing the value of
  //   the expression if the condition evaluates to false.  This is
  //   the same as getRHS.
  Expr *getFalseExpr() const { return cast<Expr>(SubExprs[RHS]); }

  Expr *getLHS() const { return cast<Expr>(SubExprs[LHS]); }
  Expr *getRHS() const { return cast<Expr>(SubExprs[RHS]); }

  SourceLocation getLocStart() const LLVM_READONLY {
    return getCond()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getRHS()->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ConditionalOperatorClass;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[0]+END_EXPR);
  }
};

/// BinaryConditionalOperator - The GNU extension to the conditional
/// operator which allows the middle operand to be omitted.
///
/// This is a different expression kind on the assumption that almost
/// every client ends up needing to know that these are different.
class BinaryConditionalOperator : public AbstractConditionalOperator {
  enum { COMMON, COND, LHS, RHS, NUM_SUBEXPRS };

  /// - the common condition/left-hand-side expression, which will be
  ///   evaluated as the opaque value
  /// - the condition, expressed in terms of the opaque value
  /// - the left-hand-side, expressed in terms of the opaque value
  /// - the right-hand-side
  Stmt *SubExprs[NUM_SUBEXPRS];
  OpaqueValueExpr *OpaqueValue;

  friend class ASTStmtReader;
public:
  BinaryConditionalOperator(Expr *common, OpaqueValueExpr *opaqueValue,
                            Expr *cond, Expr *lhs, Expr *rhs,
                            SourceLocation qloc, SourceLocation cloc,
                            QualType t, ExprValueKind VK, ExprObjectKind OK)
    : AbstractConditionalOperator(BinaryConditionalOperatorClass, t, VK, OK,
           (common->isTypeDependent() || rhs->isTypeDependent()),
           (common->isValueDependent() || rhs->isValueDependent()),
           (common->isInstantiationDependent() ||
            rhs->isInstantiationDependent()),
           (common->containsUnexpandedParameterPack() ||
            rhs->containsUnexpandedParameterPack()),
                                  qloc, cloc),
      OpaqueValue(opaqueValue) {
    SubExprs[COMMON] = common;
    SubExprs[COND] = cond;
    SubExprs[LHS] = lhs;
    SubExprs[RHS] = rhs;
    assert(OpaqueValue->getSourceExpr() == common && "Wrong opaque value");
  }

  /// \brief Build an empty conditional operator.
  explicit BinaryConditionalOperator(EmptyShell Empty)
    : AbstractConditionalOperator(BinaryConditionalOperatorClass, Empty) { }

  /// \brief getCommon - Return the common expression, written to the
  ///   left of the condition.  The opaque value will be bound to the
  ///   result of this expression.
  Expr *getCommon() const { return cast<Expr>(SubExprs[COMMON]); }

  /// \brief getOpaqueValue - Return the opaque value placeholder.
  OpaqueValueExpr *getOpaqueValue() const { return OpaqueValue; }

  /// \brief getCond - Return the condition expression; this is defined
  ///   in terms of the opaque value.
  Expr *getCond() const { return cast<Expr>(SubExprs[COND]); }

  /// \brief getTrueExpr - Return the subexpression which will be
  ///   evaluated if the condition evaluates to true;  this is defined
  ///   in terms of the opaque value.
  Expr *getTrueExpr() const {
    return cast<Expr>(SubExprs[LHS]);
  }

  /// \brief getFalseExpr - Return the subexpression which will be
  ///   evaluated if the condnition evaluates to false; this is
  ///   defined in terms of the opaque value.
  Expr *getFalseExpr() const {
    return cast<Expr>(SubExprs[RHS]);
  }

  SourceLocation getLocStart() const LLVM_READONLY {
    return getCommon()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getFalseExpr()->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == BinaryConditionalOperatorClass;
  }

  // Iterators
  child_range children() {
    return child_range(SubExprs, SubExprs + NUM_SUBEXPRS);
  }
};

inline Expr *AbstractConditionalOperator::getCond() const {
  if (const ConditionalOperator *co = dyn_cast<ConditionalOperator>(this))
    return co->getCond();
  return cast<BinaryConditionalOperator>(this)->getCond();
}

inline Expr *AbstractConditionalOperator::getTrueExpr() const {
  if (const ConditionalOperator *co = dyn_cast<ConditionalOperator>(this))
    return co->getTrueExpr();
  return cast<BinaryConditionalOperator>(this)->getTrueExpr();
}

inline Expr *AbstractConditionalOperator::getFalseExpr() const {
  if (const ConditionalOperator *co = dyn_cast<ConditionalOperator>(this))
    return co->getFalseExpr();
  return cast<BinaryConditionalOperator>(this)->getFalseExpr();
}

/// AddrLabelExpr - The GNU address of label extension, representing &&label.
class AddrLabelExpr : public Expr {
  SourceLocation AmpAmpLoc, LabelLoc;
  LabelDecl *Label;
public:
  AddrLabelExpr(SourceLocation AALoc, SourceLocation LLoc, LabelDecl *L,
                QualType t)
    : Expr(AddrLabelExprClass, t, VK_RValue, OK_Ordinary, false, false, false,
           false),
      AmpAmpLoc(AALoc), LabelLoc(LLoc), Label(L) {}

  /// \brief Build an empty address of a label expression.
  explicit AddrLabelExpr(EmptyShell Empty)
    : Expr(AddrLabelExprClass, Empty) { }

  SourceLocation getAmpAmpLoc() const { return AmpAmpLoc; }
  void setAmpAmpLoc(SourceLocation L) { AmpAmpLoc = L; }
  SourceLocation getLabelLoc() const { return LabelLoc; }
  void setLabelLoc(SourceLocation L) { LabelLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return AmpAmpLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return LabelLoc; }

  LabelDecl *getLabel() const { return Label; }
  void setLabel(LabelDecl *L) { Label = L; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AddrLabelExprClass;
  }

  // Iterators
  child_range children() { return child_range(); }
};

/// StmtExpr - This is the GNU Statement Expression extension: ({int X=4; X;}).
/// The StmtExpr contains a single CompoundStmt node, which it evaluates and
/// takes the value of the last subexpression.
///
/// A StmtExpr is always an r-value; values "returned" out of a
/// StmtExpr will be copied.
class StmtExpr : public Expr {
  Stmt *SubStmt;
  SourceLocation LParenLoc, RParenLoc;
public:
  // FIXME: Does type-dependence need to be computed differently?
  // FIXME: Do we need to compute instantiation instantiation-dependence for
  // statements? (ugh!)
  StmtExpr(CompoundStmt *substmt, QualType T,
           SourceLocation lp, SourceLocation rp) :
    Expr(StmtExprClass, T, VK_RValue, OK_Ordinary,
         T->isDependentType(), false, false, false),
    SubStmt(substmt), LParenLoc(lp), RParenLoc(rp) { }

  /// \brief Build an empty statement expression.
  explicit StmtExpr(EmptyShell Empty) : Expr(StmtExprClass, Empty) { }

  CompoundStmt *getSubStmt() { return cast<CompoundStmt>(SubStmt); }
  const CompoundStmt *getSubStmt() const { return cast<CompoundStmt>(SubStmt); }
  void setSubStmt(CompoundStmt *S) { SubStmt = S; }

  SourceLocation getLocStart() const LLVM_READONLY { return LParenLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  SourceLocation getLParenLoc() const { return LParenLoc; }
  void setLParenLoc(SourceLocation L) { LParenLoc = L; }
  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == StmtExprClass;
  }

  // Iterators
  child_range children() { return child_range(&SubStmt, &SubStmt+1); }
};


/// ShuffleVectorExpr - clang-specific builtin-in function
/// __builtin_shufflevector.
/// This AST node represents a operator that does a constant
/// shuffle, similar to LLVM's shufflevector instruction. It takes
/// two vectors and a variable number of constant indices,
/// and returns the appropriately shuffled vector.
class ShuffleVectorExpr : public Expr {
  SourceLocation BuiltinLoc, RParenLoc;

  // SubExprs - the list of values passed to the __builtin_shufflevector
  // function. The first two are vectors, and the rest are constant
  // indices.  The number of values in this list is always
  // 2+the number of indices in the vector type.
  Stmt **SubExprs;
  unsigned NumExprs;

public:
  ShuffleVectorExpr(const ASTContext &C, ArrayRef<Expr*> args, QualType Type,
                    SourceLocation BLoc, SourceLocation RP);

  /// \brief Build an empty vector-shuffle expression.
  explicit ShuffleVectorExpr(EmptyShell Empty)
    : Expr(ShuffleVectorExprClass, Empty), SubExprs(nullptr) { }

  SourceLocation getBuiltinLoc() const { return BuiltinLoc; }
  void setBuiltinLoc(SourceLocation L) { BuiltinLoc = L; }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return BuiltinLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ShuffleVectorExprClass;
  }

  /// getNumSubExprs - Return the size of the SubExprs array.  This includes the
  /// constant expression, the actual arguments passed in, and the function
  /// pointers.
  unsigned getNumSubExprs() const { return NumExprs; }

  /// \brief Retrieve the array of expressions.
  Expr **getSubExprs() { return reinterpret_cast<Expr **>(SubExprs); }

  /// getExpr - Return the Expr at the specified index.
  Expr *getExpr(unsigned Index) {
    assert((Index < NumExprs) && "Arg access out of range!");
    return cast<Expr>(SubExprs[Index]);
  }
  const Expr *getExpr(unsigned Index) const {
    assert((Index < NumExprs) && "Arg access out of range!");
    return cast<Expr>(SubExprs[Index]);
  }

  void setExprs(const ASTContext &C, ArrayRef<Expr *> Exprs);

  llvm::APSInt getShuffleMaskIdx(const ASTContext &Ctx, unsigned N) const {
    assert((N < NumExprs - 2) && "Shuffle idx out of range!");
    return getExpr(N+2)->EvaluateKnownConstInt(Ctx);
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[0]+NumExprs);
  }
};

/// ConvertVectorExpr - Clang builtin function __builtin_convertvector
/// This AST node provides support for converting a vector type to another
/// vector type of the same arity.
class ConvertVectorExpr : public Expr {
private:
  Stmt *SrcExpr;
  TypeSourceInfo *TInfo;
  SourceLocation BuiltinLoc, RParenLoc;

  friend class ASTReader;
  friend class ASTStmtReader;
  explicit ConvertVectorExpr(EmptyShell Empty) : Expr(ConvertVectorExprClass, Empty) {}

public:
  ConvertVectorExpr(Expr* SrcExpr, TypeSourceInfo *TI, QualType DstType,
             ExprValueKind VK, ExprObjectKind OK,
             SourceLocation BuiltinLoc, SourceLocation RParenLoc)
    : Expr(ConvertVectorExprClass, DstType, VK, OK,
           DstType->isDependentType(),
           DstType->isDependentType() || SrcExpr->isValueDependent(),
           (DstType->isInstantiationDependentType() ||
            SrcExpr->isInstantiationDependent()),
           (DstType->containsUnexpandedParameterPack() ||
            SrcExpr->containsUnexpandedParameterPack())),
  SrcExpr(SrcExpr), TInfo(TI), BuiltinLoc(BuiltinLoc), RParenLoc(RParenLoc) {}

  /// getSrcExpr - Return the Expr to be converted.
  Expr *getSrcExpr() const { return cast<Expr>(SrcExpr); }

  /// getTypeSourceInfo - Return the destination type.
  TypeSourceInfo *getTypeSourceInfo() const {
    return TInfo;
  }
  void setTypeSourceInfo(TypeSourceInfo *ti) {
    TInfo = ti;
  }

  /// getBuiltinLoc - Return the location of the __builtin_convertvector token.
  SourceLocation getBuiltinLoc() const { return BuiltinLoc; }

  /// getRParenLoc - Return the location of final right parenthesis.
  SourceLocation getRParenLoc() const { return RParenLoc; }

  SourceLocation getLocStart() const LLVM_READONLY { return BuiltinLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ConvertVectorExprClass;
  }

  // Iterators
  child_range children() { return child_range(&SrcExpr, &SrcExpr+1); }
};

/// ChooseExpr - GNU builtin-in function __builtin_choose_expr.
/// This AST node is similar to the conditional operator (?:) in C, with
/// the following exceptions:
/// - the test expression must be a integer constant expression.
/// - the expression returned acts like the chosen subexpression in every
///   visible way: the type is the same as that of the chosen subexpression,
///   and all predicates (whether it's an l-value, whether it's an integer
///   constant expression, etc.) return the same result as for the chosen
///   sub-expression.
class ChooseExpr : public Expr {
  enum { COND, LHS, RHS, END_EXPR };
  Stmt* SubExprs[END_EXPR]; // Left/Middle/Right hand sides.
  SourceLocation BuiltinLoc, RParenLoc;
  bool CondIsTrue;
public:
  ChooseExpr(SourceLocation BLoc, Expr *cond, Expr *lhs, Expr *rhs,
             QualType t, ExprValueKind VK, ExprObjectKind OK,
             SourceLocation RP, bool condIsTrue,
             bool TypeDependent, bool ValueDependent)
    : Expr(ChooseExprClass, t, VK, OK, TypeDependent, ValueDependent,
           (cond->isInstantiationDependent() ||
            lhs->isInstantiationDependent() ||
            rhs->isInstantiationDependent()),
           (cond->containsUnexpandedParameterPack() ||
            lhs->containsUnexpandedParameterPack() ||
            rhs->containsUnexpandedParameterPack())),
      BuiltinLoc(BLoc), RParenLoc(RP), CondIsTrue(condIsTrue) {
      SubExprs[COND] = cond;
      SubExprs[LHS] = lhs;
      SubExprs[RHS] = rhs;
    }

  /// \brief Build an empty __builtin_choose_expr.
  explicit ChooseExpr(EmptyShell Empty) : Expr(ChooseExprClass, Empty) { }

  /// isConditionTrue - Return whether the condition is true (i.e. not
  /// equal to zero).
  bool isConditionTrue() const {
    assert(!isConditionDependent() &&
           "Dependent condition isn't true or false");
    return CondIsTrue;
  }
  void setIsConditionTrue(bool isTrue) { CondIsTrue = isTrue; }

  bool isConditionDependent() const {
    return getCond()->isTypeDependent() || getCond()->isValueDependent();
  }

  /// getChosenSubExpr - Return the subexpression chosen according to the
  /// condition.
  Expr *getChosenSubExpr() const {
    return isConditionTrue() ? getLHS() : getRHS();
  }

  Expr *getCond() const { return cast<Expr>(SubExprs[COND]); }
  void setCond(Expr *E) { SubExprs[COND] = E; }
  Expr *getLHS() const { return cast<Expr>(SubExprs[LHS]); }
  void setLHS(Expr *E) { SubExprs[LHS] = E; }
  Expr *getRHS() const { return cast<Expr>(SubExprs[RHS]); }
  void setRHS(Expr *E) { SubExprs[RHS] = E; }

  SourceLocation getBuiltinLoc() const { return BuiltinLoc; }
  void setBuiltinLoc(SourceLocation L) { BuiltinLoc = L; }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return BuiltinLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ChooseExprClass;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[0]+END_EXPR);
  }
};

/// GNUNullExpr - Implements the GNU __null extension, which is a name
/// for a null pointer constant that has integral type (e.g., int or
/// long) and is the same size and alignment as a pointer. The __null
/// extension is typically only used by system headers, which define
/// NULL as __null in C++ rather than using 0 (which is an integer
/// that may not match the size of a pointer).
class GNUNullExpr : public Expr {
  /// TokenLoc - The location of the __null keyword.
  SourceLocation TokenLoc;

public:
  GNUNullExpr(QualType Ty, SourceLocation Loc)
    : Expr(GNUNullExprClass, Ty, VK_RValue, OK_Ordinary, false, false, false,
           false),
      TokenLoc(Loc) { }

  /// \brief Build an empty GNU __null expression.
  explicit GNUNullExpr(EmptyShell Empty) : Expr(GNUNullExprClass, Empty) { }

  /// getTokenLocation - The location of the __null token.
  SourceLocation getTokenLocation() const { return TokenLoc; }
  void setTokenLocation(SourceLocation L) { TokenLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return TokenLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return TokenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == GNUNullExprClass;
  }

  // Iterators
  child_range children() { return child_range(); }
};

/// VAArgExpr, used for the builtin function __builtin_va_arg.
class VAArgExpr : public Expr {
  Stmt *Val;
  TypeSourceInfo *TInfo;
  SourceLocation BuiltinLoc, RParenLoc;
public:
  VAArgExpr(SourceLocation BLoc, Expr* e, TypeSourceInfo *TInfo,
            SourceLocation RPLoc, QualType t)
    : Expr(VAArgExprClass, t, VK_RValue, OK_Ordinary,
           t->isDependentType(), false,
           (TInfo->getType()->isInstantiationDependentType() ||
            e->isInstantiationDependent()),
           (TInfo->getType()->containsUnexpandedParameterPack() ||
            e->containsUnexpandedParameterPack())),
      Val(e), TInfo(TInfo),
      BuiltinLoc(BLoc),
      RParenLoc(RPLoc) { }

  /// \brief Create an empty __builtin_va_arg expression.
  explicit VAArgExpr(EmptyShell Empty) : Expr(VAArgExprClass, Empty) { }

  const Expr *getSubExpr() const { return cast<Expr>(Val); }
  Expr *getSubExpr() { return cast<Expr>(Val); }
  void setSubExpr(Expr *E) { Val = E; }

  TypeSourceInfo *getWrittenTypeInfo() const { return TInfo; }
  void setWrittenTypeInfo(TypeSourceInfo *TI) { TInfo = TI; }

  SourceLocation getBuiltinLoc() const { return BuiltinLoc; }
  void setBuiltinLoc(SourceLocation L) { BuiltinLoc = L; }

  SourceLocation getRParenLoc() const { return RParenLoc; }
  void setRParenLoc(SourceLocation L) { RParenLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return BuiltinLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == VAArgExprClass;
  }

  // Iterators
  child_range children() { return child_range(&Val, &Val+1); }
};

/// @brief Describes an C or C++ initializer list.
///
/// InitListExpr describes an initializer list, which can be used to
/// initialize objects of different types, including
/// struct/class/union types, arrays, and vectors. For example:
///
/// @code
/// struct foo x = { 1, { 2, 3 } };
/// @endcode
///
/// Prior to semantic analysis, an initializer list will represent the
/// initializer list as written by the user, but will have the
/// placeholder type "void". This initializer list is called the
/// syntactic form of the initializer, and may contain C99 designated
/// initializers (represented as DesignatedInitExprs), initializations
/// of subobject members without explicit braces, and so on. Clients
/// interested in the original syntax of the initializer list should
/// use the syntactic form of the initializer list.
///
/// After semantic analysis, the initializer list will represent the
/// semantic form of the initializer, where the initializations of all
/// subobjects are made explicit with nested InitListExpr nodes and
/// C99 designators have been eliminated by placing the designated
/// initializations into the subobject they initialize. Additionally,
/// any "holes" in the initialization, where no initializer has been
/// specified for a particular subobject, will be replaced with
/// implicitly-generated ImplicitValueInitExpr expressions that
/// value-initialize the subobjects. Note, however, that the
/// initializer lists may still have fewer initializers than there are
/// elements to initialize within the object.
///
/// After semantic analysis has completed, given an initializer list,
/// method isSemanticForm() returns true if and only if this is the
/// semantic form of the initializer list (note: the same AST node
/// may at the same time be the syntactic form).
/// Given the semantic form of the initializer list, one can retrieve
/// the syntactic form of that initializer list (when different)
/// using method getSyntacticForm(); the method returns null if applied
/// to a initializer list which is already in syntactic form.
/// Similarly, given the syntactic form (i.e., an initializer list such
/// that isSemanticForm() returns false), one can retrieve the semantic
/// form using method getSemanticForm().
/// Since many initializer lists have the same syntactic and semantic forms,
/// getSyntacticForm() may return NULL, indicating that the current
/// semantic initializer list also serves as its syntactic form.
class InitListExpr : public Expr {
  // FIXME: Eliminate this vector in favor of ASTContext allocation
  typedef ASTVector<Stmt *> InitExprsTy;
  InitExprsTy InitExprs;
  SourceLocation LBraceLoc, RBraceLoc;

  /// The alternative form of the initializer list (if it exists).
  /// The int part of the pair stores whether this initializer list is
  /// in semantic form. If not null, the pointer points to:
  ///   - the syntactic form, if this is in semantic form;
  ///   - the semantic form, if this is in syntactic form.
  llvm::PointerIntPair<InitListExpr *, 1, bool> AltForm;

  /// \brief Either:
  ///  If this initializer list initializes an array with more elements than
  ///  there are initializers in the list, specifies an expression to be used
  ///  for value initialization of the rest of the elements.
  /// Or
  ///  If this initializer list initializes a union, specifies which
  ///  field within the union will be initialized.
  llvm::PointerUnion<Expr *, FieldDecl *> ArrayFillerOrUnionFieldInit;

public:
  InitListExpr(const ASTContext &C, SourceLocation lbraceloc,
               ArrayRef<Expr*> initExprs, SourceLocation rbraceloc);

  /// \brief Build an empty initializer list.
  explicit InitListExpr(EmptyShell Empty)
    : Expr(InitListExprClass, Empty) { }

  unsigned getNumInits() const { return InitExprs.size(); }

  /// \brief Retrieve the set of initializers.
  Expr **getInits() { return reinterpret_cast<Expr **>(InitExprs.data()); }

  const Expr *getInit(unsigned Init) const {
    assert(Init < getNumInits() && "Initializer access out of range!");
    return cast_or_null<Expr>(InitExprs[Init]);
  }

  Expr *getInit(unsigned Init) {
    assert(Init < getNumInits() && "Initializer access out of range!");
    return cast_or_null<Expr>(InitExprs[Init]);
  }

  void setInit(unsigned Init, Expr *expr) {
    assert(Init < getNumInits() && "Initializer access out of range!");
    InitExprs[Init] = expr;

    if (expr) {
      ExprBits.TypeDependent |= expr->isTypeDependent();
      ExprBits.ValueDependent |= expr->isValueDependent();
      ExprBits.InstantiationDependent |= expr->isInstantiationDependent();
      ExprBits.ContainsUnexpandedParameterPack |=
          expr->containsUnexpandedParameterPack();
    }
  }

  /// \brief Reserve space for some number of initializers.
  void reserveInits(const ASTContext &C, unsigned NumInits);

  /// @brief Specify the number of initializers
  ///
  /// If there are more than @p NumInits initializers, the remaining
  /// initializers will be destroyed. If there are fewer than @p
  /// NumInits initializers, NULL expressions will be added for the
  /// unknown initializers.
  void resizeInits(const ASTContext &Context, unsigned NumInits);

  /// @brief Updates the initializer at index @p Init with the new
  /// expression @p expr, and returns the old expression at that
  /// location.
  ///
  /// When @p Init is out of range for this initializer list, the
  /// initializer list will be extended with NULL expressions to
  /// accommodate the new entry.
  Expr *updateInit(const ASTContext &C, unsigned Init, Expr *expr);

  /// \brief If this initializer list initializes an array with more elements
  /// than there are initializers in the list, specifies an expression to be
  /// used for value initialization of the rest of the elements.
  Expr *getArrayFiller() {
    return ArrayFillerOrUnionFieldInit.dyn_cast<Expr *>();
  }
  const Expr *getArrayFiller() const {
    return const_cast<InitListExpr *>(this)->getArrayFiller();
  }
  void setArrayFiller(Expr *filler);

  /// \brief Return true if this is an array initializer and its array "filler"
  /// has been set.
  bool hasArrayFiller() const { return getArrayFiller(); }

  /// \brief If this initializes a union, specifies which field in the
  /// union to initialize.
  ///
  /// Typically, this field is the first named field within the
  /// union. However, a designated initializer can specify the
  /// initialization of a different field within the union.
  FieldDecl *getInitializedFieldInUnion() {
    return ArrayFillerOrUnionFieldInit.dyn_cast<FieldDecl *>();
  }
  const FieldDecl *getInitializedFieldInUnion() const {
    return const_cast<InitListExpr *>(this)->getInitializedFieldInUnion();
  }
  void setInitializedFieldInUnion(FieldDecl *FD) {
    assert((FD == nullptr
            || getInitializedFieldInUnion() == nullptr
            || getInitializedFieldInUnion() == FD)
           && "Only one field of a union may be initialized at a time!");
    ArrayFillerOrUnionFieldInit = FD;
  }

  // Explicit InitListExpr's originate from source code (and have valid source
  // locations). Implicit InitListExpr's are created by the semantic analyzer.
  bool isExplicit() {
    return LBraceLoc.isValid() && RBraceLoc.isValid();
  }

  // Is this an initializer for an array of characters, initialized by a string
  // literal or an @encode?
  bool isStringLiteralInit() const;

  SourceLocation getLBraceLoc() const { return LBraceLoc; }
  void setLBraceLoc(SourceLocation Loc) { LBraceLoc = Loc; }
  SourceLocation getRBraceLoc() const { return RBraceLoc; }
  void setRBraceLoc(SourceLocation Loc) { RBraceLoc = Loc; }

  bool isSemanticForm() const { return AltForm.getInt(); }
  InitListExpr *getSemanticForm() const {
    return isSemanticForm() ? nullptr : AltForm.getPointer();
  }
  InitListExpr *getSyntacticForm() const {
    return isSemanticForm() ? AltForm.getPointer() : nullptr;
  }

  void setSyntacticForm(InitListExpr *Init) {
    AltForm.setPointer(Init);
    AltForm.setInt(true);
    Init->AltForm.setPointer(this);
    Init->AltForm.setInt(false);
  }

  bool hadArrayRangeDesignator() const {
    return InitListExprBits.HadArrayRangeDesignator != 0;
  }
  void sawArrayRangeDesignator(bool ARD = true) {
    InitListExprBits.HadArrayRangeDesignator = ARD;
  }

  bool isVectorInitWithCXXFunctionalCastExpr() const {
    return InitListExprBits.VectorInitWithCXXFunctionalCastExpr != 0;
  }
  void sawVectorInitWithCXXFunctionalCastExpr(bool InitWithCast) {
    InitListExprBits.VectorInitWithCXXFunctionalCastExpr = InitWithCast;
  }

  SourceLocation getLocStart() const LLVM_READONLY;
  SourceLocation getLocEnd() const LLVM_READONLY;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == InitListExprClass;
  }

  // Iterators
  child_range children() {
    // FIXME: This does not include the array filler expression.
    if (InitExprs.empty()) return child_range();
    return child_range(&InitExprs[0], &InitExprs[0] + InitExprs.size());
  }

  typedef InitExprsTy::iterator iterator;
  typedef InitExprsTy::const_iterator const_iterator;
  typedef InitExprsTy::reverse_iterator reverse_iterator;
  typedef InitExprsTy::const_reverse_iterator const_reverse_iterator;

  iterator begin() { return InitExprs.begin(); }
  const_iterator begin() const { return InitExprs.begin(); }
  iterator end() { return InitExprs.end(); }
  const_iterator end() const { return InitExprs.end(); }
  reverse_iterator rbegin() { return InitExprs.rbegin(); }
  const_reverse_iterator rbegin() const { return InitExprs.rbegin(); }
  reverse_iterator rend() { return InitExprs.rend(); }
  const_reverse_iterator rend() const { return InitExprs.rend(); }

  friend class ASTStmtReader;
  friend class ASTStmtWriter;
};

/// @brief Represents a C99 designated initializer expression.
///
/// A designated initializer expression (C99 6.7.8) contains one or
/// more designators (which can be field designators, array
/// designators, or GNU array-range designators) followed by an
/// expression that initializes the field or element(s) that the
/// designators refer to. For example, given:
///
/// @code
/// struct point {
///   double x;
///   double y;
/// };
/// struct point ptarray[10] = { [2].y = 1.0, [2].x = 2.0, [0].x = 1.0 };
/// @endcode
///
/// The InitListExpr contains three DesignatedInitExprs, the first of
/// which covers @c [2].y=1.0. This DesignatedInitExpr will have two
/// designators, one array designator for @c [2] followed by one field
/// designator for @c .y. The initialization expression will be 1.0.
class DesignatedInitExpr : public Expr {
public:
  /// \brief Forward declaration of the Designator class.
  class Designator;

private:
  /// The location of the '=' or ':' prior to the actual initializer
  /// expression.
  SourceLocation EqualOrColonLoc;

  /// Whether this designated initializer used the GNU deprecated
  /// syntax rather than the C99 '=' syntax.
  bool GNUSyntax : 1;

  /// The number of designators in this initializer expression.
  unsigned NumDesignators : 15;

  /// The number of subexpressions of this initializer expression,
  /// which contains both the initializer and any additional
  /// expressions used by array and array-range designators.
  unsigned NumSubExprs : 16;

  /// \brief The designators in this designated initialization
  /// expression.
  Designator *Designators;


  DesignatedInitExpr(const ASTContext &C, QualType Ty, unsigned NumDesignators,
                     const Designator *Designators,
                     SourceLocation EqualOrColonLoc, bool GNUSyntax,
                     ArrayRef<Expr*> IndexExprs, Expr *Init);

  explicit DesignatedInitExpr(unsigned NumSubExprs)
    : Expr(DesignatedInitExprClass, EmptyShell()),
      NumDesignators(0), NumSubExprs(NumSubExprs), Designators(nullptr) { }

public:
  /// A field designator, e.g., ".x".
  struct FieldDesignator {
    /// Refers to the field that is being initialized. The low bit
    /// of this field determines whether this is actually a pointer
    /// to an IdentifierInfo (if 1) or a FieldDecl (if 0). When
    /// initially constructed, a field designator will store an
    /// IdentifierInfo*. After semantic analysis has resolved that
    /// name, the field designator will instead store a FieldDecl*.
    uintptr_t NameOrField;

    /// The location of the '.' in the designated initializer.
    unsigned DotLoc;

    /// The location of the field name in the designated initializer.
    unsigned FieldLoc;
  };

  /// An array or GNU array-range designator, e.g., "[9]" or "[10..15]".
  struct ArrayOrRangeDesignator {
    /// Location of the first index expression within the designated
    /// initializer expression's list of subexpressions.
    unsigned Index;
    /// The location of the '[' starting the array range designator.
    unsigned LBracketLoc;
    /// The location of the ellipsis separating the start and end
    /// indices. Only valid for GNU array-range designators.
    unsigned EllipsisLoc;
    /// The location of the ']' terminating the array range designator.
    unsigned RBracketLoc;
  };

  /// @brief Represents a single C99 designator.
  ///
  /// @todo This class is infuriatingly similar to clang::Designator,
  /// but minor differences (storing indices vs. storing pointers)
  /// keep us from reusing it. Try harder, later, to rectify these
  /// differences.
  class Designator {
    /// @brief The kind of designator this describes.
    enum {
      FieldDesignator,
      ArrayDesignator,
      ArrayRangeDesignator
    } Kind;

    union {
      /// A field designator, e.g., ".x".
      struct FieldDesignator Field;
      /// An array or GNU array-range designator, e.g., "[9]" or "[10..15]".
      struct ArrayOrRangeDesignator ArrayOrRange;
    };
    friend class DesignatedInitExpr;

  public:
    Designator() {}

    /// @brief Initializes a field designator.
    Designator(const IdentifierInfo *FieldName, SourceLocation DotLoc,
               SourceLocation FieldLoc)
      : Kind(FieldDesignator) {
      Field.NameOrField = reinterpret_cast<uintptr_t>(FieldName) | 0x01;
      Field.DotLoc = DotLoc.getRawEncoding();
      Field.FieldLoc = FieldLoc.getRawEncoding();
    }

    /// @brief Initializes an array designator.
    Designator(unsigned Index, SourceLocation LBracketLoc,
               SourceLocation RBracketLoc)
      : Kind(ArrayDesignator) {
      ArrayOrRange.Index = Index;
      ArrayOrRange.LBracketLoc = LBracketLoc.getRawEncoding();
      ArrayOrRange.EllipsisLoc = SourceLocation().getRawEncoding();
      ArrayOrRange.RBracketLoc = RBracketLoc.getRawEncoding();
    }

    /// @brief Initializes a GNU array-range designator.
    Designator(unsigned Index, SourceLocation LBracketLoc,
               SourceLocation EllipsisLoc, SourceLocation RBracketLoc)
      : Kind(ArrayRangeDesignator) {
      ArrayOrRange.Index = Index;
      ArrayOrRange.LBracketLoc = LBracketLoc.getRawEncoding();
      ArrayOrRange.EllipsisLoc = EllipsisLoc.getRawEncoding();
      ArrayOrRange.RBracketLoc = RBracketLoc.getRawEncoding();
    }

    bool isFieldDesignator() const { return Kind == FieldDesignator; }
    bool isArrayDesignator() const { return Kind == ArrayDesignator; }
    bool isArrayRangeDesignator() const { return Kind == ArrayRangeDesignator; }

    IdentifierInfo *getFieldName() const;

    FieldDecl *getField() const {
      assert(Kind == FieldDesignator && "Only valid on a field designator");
      if (Field.NameOrField & 0x01)
        return nullptr;
      else
        return reinterpret_cast<FieldDecl *>(Field.NameOrField);
    }

    void setField(FieldDecl *FD) {
      assert(Kind == FieldDesignator && "Only valid on a field designator");
      Field.NameOrField = reinterpret_cast<uintptr_t>(FD);
    }

    SourceLocation getDotLoc() const {
      assert(Kind == FieldDesignator && "Only valid on a field designator");
      return SourceLocation::getFromRawEncoding(Field.DotLoc);
    }

    SourceLocation getFieldLoc() const {
      assert(Kind == FieldDesignator && "Only valid on a field designator");
      return SourceLocation::getFromRawEncoding(Field.FieldLoc);
    }

    SourceLocation getLBracketLoc() const {
      assert((Kind == ArrayDesignator || Kind == ArrayRangeDesignator) &&
             "Only valid on an array or array-range designator");
      return SourceLocation::getFromRawEncoding(ArrayOrRange.LBracketLoc);
    }

    SourceLocation getRBracketLoc() const {
      assert((Kind == ArrayDesignator || Kind == ArrayRangeDesignator) &&
             "Only valid on an array or array-range designator");
      return SourceLocation::getFromRawEncoding(ArrayOrRange.RBracketLoc);
    }

    SourceLocation getEllipsisLoc() const {
      assert(Kind == ArrayRangeDesignator &&
             "Only valid on an array-range designator");
      return SourceLocation::getFromRawEncoding(ArrayOrRange.EllipsisLoc);
    }

    unsigned getFirstExprIndex() const {
      assert((Kind == ArrayDesignator || Kind == ArrayRangeDesignator) &&
             "Only valid on an array or array-range designator");
      return ArrayOrRange.Index;
    }

    SourceLocation getLocStart() const LLVM_READONLY {
      if (Kind == FieldDesignator)
        return getDotLoc().isInvalid()? getFieldLoc() : getDotLoc();
      else
        return getLBracketLoc();
    }
    SourceLocation getLocEnd() const LLVM_READONLY {
      return Kind == FieldDesignator ? getFieldLoc() : getRBracketLoc();
    }
    SourceRange getSourceRange() const LLVM_READONLY {
      return SourceRange(getLocStart(), getLocEnd());
    }
  };

  static DesignatedInitExpr *Create(const ASTContext &C,
                                    Designator *Designators,
                                    unsigned NumDesignators,
                                    ArrayRef<Expr*> IndexExprs,
                                    SourceLocation EqualOrColonLoc,
                                    bool GNUSyntax, Expr *Init);

  static DesignatedInitExpr *CreateEmpty(const ASTContext &C,
                                         unsigned NumIndexExprs);

  /// @brief Returns the number of designators in this initializer.
  unsigned size() const { return NumDesignators; }

  // Iterator access to the designators.
  typedef Designator *designators_iterator;
  designators_iterator designators_begin() { return Designators; }
  designators_iterator designators_end() {
    return Designators + NumDesignators;
  }

  typedef const Designator *const_designators_iterator;
  const_designators_iterator designators_begin() const { return Designators; }
  const_designators_iterator designators_end() const {
    return Designators + NumDesignators;
  }

  typedef llvm::iterator_range<designators_iterator> designators_range;
  designators_range designators() {
    return designators_range(designators_begin(), designators_end());
  }

  typedef llvm::iterator_range<const_designators_iterator>
          designators_const_range;
  designators_const_range designators() const {
    return designators_const_range(designators_begin(), designators_end());
  }

  typedef std::reverse_iterator<designators_iterator>
          reverse_designators_iterator;
  reverse_designators_iterator designators_rbegin() {
    return reverse_designators_iterator(designators_end());
  }
  reverse_designators_iterator designators_rend() {
    return reverse_designators_iterator(designators_begin());
  }

  typedef std::reverse_iterator<const_designators_iterator>
          const_reverse_designators_iterator;
  const_reverse_designators_iterator designators_rbegin() const {
    return const_reverse_designators_iterator(designators_end());
  }
  const_reverse_designators_iterator designators_rend() const {
    return const_reverse_designators_iterator(designators_begin());
  }

  Designator *getDesignator(unsigned Idx) { return &designators_begin()[Idx]; }

  void setDesignators(const ASTContext &C, const Designator *Desigs,
                      unsigned NumDesigs);

  Expr *getArrayIndex(const Designator &D) const;
  Expr *getArrayRangeStart(const Designator &D) const;
  Expr *getArrayRangeEnd(const Designator &D) const;

  /// @brief Retrieve the location of the '=' that precedes the
  /// initializer value itself, if present.
  SourceLocation getEqualOrColonLoc() const { return EqualOrColonLoc; }
  void setEqualOrColonLoc(SourceLocation L) { EqualOrColonLoc = L; }

  /// @brief Determines whether this designated initializer used the
  /// deprecated GNU syntax for designated initializers.
  bool usesGNUSyntax() const { return GNUSyntax; }
  void setGNUSyntax(bool GNU) { GNUSyntax = GNU; }

  /// @brief Retrieve the initializer value.
  Expr *getInit() const {
    return cast<Expr>(*const_cast<DesignatedInitExpr*>(this)->child_begin());
  }

  void setInit(Expr *init) {
    *child_begin() = init;
  }

  /// \brief Retrieve the total number of subexpressions in this
  /// designated initializer expression, including the actual
  /// initialized value and any expressions that occur within array
  /// and array-range designators.
  unsigned getNumSubExprs() const { return NumSubExprs; }

  Expr *getSubExpr(unsigned Idx) const {
    assert(Idx < NumSubExprs && "Subscript out of range");
    return cast<Expr>(reinterpret_cast<Stmt *const *>(this + 1)[Idx]);
  }

  void setSubExpr(unsigned Idx, Expr *E) {
    assert(Idx < NumSubExprs && "Subscript out of range");
    reinterpret_cast<Stmt **>(this + 1)[Idx] = E;
  }

  /// \brief Replaces the designator at index @p Idx with the series
  /// of designators in [First, Last).
  void ExpandDesignator(const ASTContext &C, unsigned Idx,
                        const Designator *First, const Designator *Last);

  SourceRange getDesignatorsSourceRange() const;

  SourceLocation getLocStart() const LLVM_READONLY;
  SourceLocation getLocEnd() const LLVM_READONLY;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == DesignatedInitExprClass;
  }

  // Iterators
  child_range children() {
    Stmt **begin = reinterpret_cast<Stmt**>(this + 1);
    return child_range(begin, begin + NumSubExprs);
  }
};

/// \brief Represents a place-holder for an object not to be initialized by
/// anything.
///
/// This only makes sense when it appears as part of an updater of a
/// DesignatedInitUpdateExpr (see below). The base expression of a DIUE
/// initializes a big object, and the NoInitExpr's mark the spots within the
/// big object not to be overwritten by the updater.
///
/// \see DesignatedInitUpdateExpr
class NoInitExpr : public Expr {
public:
  explicit NoInitExpr(QualType ty)
    : Expr(NoInitExprClass, ty, VK_RValue, OK_Ordinary,
           false, false, ty->isInstantiationDependentType(), false) { }

  explicit NoInitExpr(EmptyShell Empty)
    : Expr(NoInitExprClass, Empty) { }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == NoInitExprClass;
  }

  SourceLocation getLocStart() const LLVM_READONLY { return SourceLocation(); }
  SourceLocation getLocEnd() const LLVM_READONLY { return SourceLocation(); }

  // Iterators
  child_range children() { return child_range(); }
};

// In cases like:
//   struct Q { int a, b, c; };
//   Q *getQ();
//   void foo() {
//     struct A { Q q; } a = { *getQ(), .q.b = 3 };
//   }
//
// We will have an InitListExpr for a, with type A, and then a
// DesignatedInitUpdateExpr for "a.q" with type Q. The "base" for this DIUE
// is the call expression *getQ(); the "updater" for the DIUE is ".q.b = 3"
//
class DesignatedInitUpdateExpr : public Expr {
  // BaseAndUpdaterExprs[0] is the base expression;
  // BaseAndUpdaterExprs[1] is an InitListExpr overwriting part of the base.
  Stmt *BaseAndUpdaterExprs[2];

public:
  DesignatedInitUpdateExpr(const ASTContext &C, SourceLocation lBraceLoc,
                           Expr *baseExprs, SourceLocation rBraceLoc);

  explicit DesignatedInitUpdateExpr(EmptyShell Empty)
    : Expr(DesignatedInitUpdateExprClass, Empty) { }

  SourceLocation getLocStart() const LLVM_READONLY;
  SourceLocation getLocEnd() const LLVM_READONLY;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == DesignatedInitUpdateExprClass;
  }

  Expr *getBase() const { return cast<Expr>(BaseAndUpdaterExprs[0]); }
  void setBase(Expr *Base) { BaseAndUpdaterExprs[0] = Base; }

  InitListExpr *getUpdater() const {
    return cast<InitListExpr>(BaseAndUpdaterExprs[1]);
  }
  void setUpdater(Expr *Updater) { BaseAndUpdaterExprs[1] = Updater; }

  // Iterators
  // children = the base and the updater
  child_range children() {
    return child_range(&BaseAndUpdaterExprs[0], &BaseAndUpdaterExprs[0] + 2);
  }
};

/// \brief Represents an implicitly-generated value initialization of
/// an object of a given type.
///
/// Implicit value initializations occur within semantic initializer
/// list expressions (InitListExpr) as placeholders for subobject
/// initializations not explicitly specified by the user.
///
/// \see InitListExpr
class ImplicitValueInitExpr : public Expr {
public:
  explicit ImplicitValueInitExpr(QualType ty)
    : Expr(ImplicitValueInitExprClass, ty, VK_RValue, OK_Ordinary,
           false, false, ty->isInstantiationDependentType(), false) { }

  /// \brief Construct an empty implicit value initialization.
  explicit ImplicitValueInitExpr(EmptyShell Empty)
    : Expr(ImplicitValueInitExprClass, Empty) { }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ImplicitValueInitExprClass;
  }

  SourceLocation getLocStart() const LLVM_READONLY { return SourceLocation(); }
  SourceLocation getLocEnd() const LLVM_READONLY { return SourceLocation(); }

  // Iterators
  child_range children() { return child_range(); }
};


class ParenListExpr : public Expr {
  Stmt **Exprs;
  unsigned NumExprs;
  SourceLocation LParenLoc, RParenLoc;

public:
  ParenListExpr(const ASTContext& C, SourceLocation lparenloc,
                ArrayRef<Expr*> exprs, SourceLocation rparenloc);

  /// \brief Build an empty paren list.
  explicit ParenListExpr(EmptyShell Empty) : Expr(ParenListExprClass, Empty) { }

  unsigned getNumExprs() const { return NumExprs; }

  const Expr* getExpr(unsigned Init) const {
    assert(Init < getNumExprs() && "Initializer access out of range!");
    return cast_or_null<Expr>(Exprs[Init]);
  }

  Expr* getExpr(unsigned Init) {
    assert(Init < getNumExprs() && "Initializer access out of range!");
    return cast_or_null<Expr>(Exprs[Init]);
  }

  Expr **getExprs() { return reinterpret_cast<Expr **>(Exprs); }

  SourceLocation getLParenLoc() const { return LParenLoc; }
  SourceLocation getRParenLoc() const { return RParenLoc; }

  SourceLocation getLocStart() const LLVM_READONLY { return LParenLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ParenListExprClass;
  }

  // Iterators
  child_range children() {
    return child_range(&Exprs[0], &Exprs[0]+NumExprs);
  }

  friend class ASTStmtReader;
  friend class ASTStmtWriter;
};


/// \brief Represents a C11 generic selection.
///
/// A generic selection (C11 6.5.1.1) contains an unevaluated controlling
/// expression, followed by one or more generic associations.  Each generic
/// association specifies a type name and an expression, or "default" and an
/// expression (in which case it is known as a default generic association).
/// The type and value of the generic selection are identical to those of its
/// result expression, which is defined as the expression in the generic
/// association with a type name that is compatible with the type of the
/// controlling expression, or the expression in the default generic association
/// if no types are compatible.  For example:
///
/// @code
/// _Generic(X, double: 1, float: 2, default: 3)
/// @endcode
///
/// The above expression evaluates to 1 if 1.0 is substituted for X, 2 if 1.0f
/// or 3 if "hello".
///
/// As an extension, generic selections are allowed in C++, where the following
/// additional semantics apply:
///
/// Any generic selection whose controlling expression is type-dependent or
/// which names a dependent type in its association list is result-dependent,
/// which means that the choice of result expression is dependent.
/// Result-dependent generic associations are both type- and value-dependent.
class GenericSelectionExpr : public Expr {
  enum { CONTROLLING, END_EXPR };
  TypeSourceInfo **AssocTypes;
  Stmt **SubExprs;
  unsigned NumAssocs, ResultIndex;
  SourceLocation GenericLoc, DefaultLoc, RParenLoc;

public:
  GenericSelectionExpr(const ASTContext &Context,
                       SourceLocation GenericLoc, Expr *ControllingExpr,
                       ArrayRef<TypeSourceInfo*> AssocTypes,
                       ArrayRef<Expr*> AssocExprs,
                       SourceLocation DefaultLoc, SourceLocation RParenLoc,
                       bool ContainsUnexpandedParameterPack,
                       unsigned ResultIndex);

  /// This constructor is used in the result-dependent case.
  GenericSelectionExpr(const ASTContext &Context,
                       SourceLocation GenericLoc, Expr *ControllingExpr,
                       ArrayRef<TypeSourceInfo*> AssocTypes,
                       ArrayRef<Expr*> AssocExprs,
                       SourceLocation DefaultLoc, SourceLocation RParenLoc,
                       bool ContainsUnexpandedParameterPack);

  explicit GenericSelectionExpr(EmptyShell Empty)
    : Expr(GenericSelectionExprClass, Empty) { }

  unsigned getNumAssocs() const { return NumAssocs; }

  SourceLocation getGenericLoc() const { return GenericLoc; }
  SourceLocation getDefaultLoc() const { return DefaultLoc; }
  SourceLocation getRParenLoc() const { return RParenLoc; }

  const Expr *getAssocExpr(unsigned i) const {
    return cast<Expr>(SubExprs[END_EXPR+i]);
  }
  Expr *getAssocExpr(unsigned i) { return cast<Expr>(SubExprs[END_EXPR+i]); }

  const TypeSourceInfo *getAssocTypeSourceInfo(unsigned i) const {
    return AssocTypes[i];
  }
  TypeSourceInfo *getAssocTypeSourceInfo(unsigned i) { return AssocTypes[i]; }

  QualType getAssocType(unsigned i) const {
    if (const TypeSourceInfo *TS = getAssocTypeSourceInfo(i))
      return TS->getType();
    else
      return QualType();
  }

  const Expr *getControllingExpr() const {
    return cast<Expr>(SubExprs[CONTROLLING]);
  }
  Expr *getControllingExpr() { return cast<Expr>(SubExprs[CONTROLLING]); }

  /// Whether this generic selection is result-dependent.
  bool isResultDependent() const { return ResultIndex == -1U; }

  /// The zero-based index of the result expression's generic association in
  /// the generic selection's association list.  Defined only if the
  /// generic selection is not result-dependent.
  unsigned getResultIndex() const {
    assert(!isResultDependent() && "Generic selection is result-dependent");
    return ResultIndex;
  }

  /// The generic selection's result expression.  Defined only if the
  /// generic selection is not result-dependent.
  const Expr *getResultExpr() const { return getAssocExpr(getResultIndex()); }
  Expr *getResultExpr() { return getAssocExpr(getResultIndex()); }

  SourceLocation getLocStart() const LLVM_READONLY { return GenericLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == GenericSelectionExprClass;
  }

  child_range children() {
    return child_range(SubExprs, SubExprs+END_EXPR+NumAssocs);
  }

  friend class ASTStmtReader;
};

//===----------------------------------------------------------------------===//
// Clang Extensions
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


/// ExtVectorElementExpr - This represents access to specific elements of a
/// vector, and may occur on the left hand side or right hand side.  For example
/// the following is legal:  "V.xy = V.zw" if V is a 4 element extended vector.
///
/// Note that the base may have either vector or pointer to vector type, just
/// like a struct field reference.
///
class ExtVectorElementExpr : public Expr {
  Stmt *Base;
  IdentifierInfo *Accessor;
  SourceLocation AccessorLoc;
public:
  ExtVectorElementExpr(QualType ty, ExprValueKind VK, Expr *base,
                       IdentifierInfo &accessor, SourceLocation loc)
    : Expr(ExtVectorElementExprClass, ty, VK,
           (VK == VK_RValue ? OK_Ordinary : OK_VectorComponent),
           base->isTypeDependent(), base->isValueDependent(),
           base->isInstantiationDependent(),
           base->containsUnexpandedParameterPack()),
      Base(base), Accessor(&accessor), AccessorLoc(loc) {}

  /// \brief Build an empty vector element expression.
  explicit ExtVectorElementExpr(EmptyShell Empty)
    : Expr(ExtVectorElementExprClass, Empty) { }

  const Expr *getBase() const { return cast<Expr>(Base); }
  Expr *getBase() { return cast<Expr>(Base); }
  void setBase(Expr *E) { Base = E; }

  IdentifierInfo &getAccessor() const { return *Accessor; }
  void setAccessor(IdentifierInfo *II) { Accessor = II; }

  SourceLocation getAccessorLoc() const { return AccessorLoc; }
  void setAccessorLoc(SourceLocation L) { AccessorLoc = L; }

  /// getNumElements - Get the number of components being selected.
  unsigned getNumElements() const;

  /// containsDuplicateElements - Return true if any element access is
  /// repeated.
  bool containsDuplicateElements() const;

  /// getEncodedElementAccess - Encode the elements accessed into an llvm
  /// aggregate Constant of ConstantInt(s).
  void getEncodedElementAccess(SmallVectorImpl<unsigned> &Elts) const;

  SourceLocation getLocStart() const LLVM_READONLY {
    return getBase()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY { return AccessorLoc; }

  /// isArrow - Return true if the base expression is a pointer to vector,
  /// return false if the base expression is a vector.
  bool isArrow() const;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ExtVectorElementExprClass;
  }

  // Iterators
  child_range children() { return child_range(&Base, &Base+1); }
};

// HLSL Change Starts
/// ExtMatrixElementExpr - This represents access to specific elements of a
/// matrix, and may occur on the left hand side or right hand side.
///
/// Note that the base may have either matrix or pointer to matrix type, just
/// like a struct field reference.
///
class ExtMatrixElementExpr : public Expr {
  Stmt *Base;
  IdentifierInfo *Accessor;
  SourceLocation AccessorLoc;
  hlsl::MatrixMemberAccessPositions Positions;
public:
  ExtMatrixElementExpr(QualType ty, ExprValueKind VK, Expr *base,
    IdentifierInfo &accessor, SourceLocation loc, hlsl::MatrixMemberAccessPositions positions)
    : Expr(ExtMatrixElementExprClass, ty, VK,
    (VK == VK_RValue ? OK_Ordinary : OK_VectorComponent),
    base->isTypeDependent(), base->isValueDependent(),
    base->isInstantiationDependent(),
    base->containsUnexpandedParameterPack()),
    Base(base), Accessor(&accessor), AccessorLoc(loc), Positions(positions) {}

  /// \brief Build an empty vector element expression.
  explicit ExtMatrixElementExpr(EmptyShell Empty)
    : Expr(ExtMatrixElementExprClass, Empty) { }

  const Expr *getBase() const { return cast<Expr>(Base); }
  Expr *getBase() { return cast<Expr>(Base); }
  void setBase(Expr *E) { Base = E; }

  IdentifierInfo &getAccessor() const { return *Accessor; }
  void setAccessor(IdentifierInfo *II) { Accessor = II; }

  SourceLocation getAccessorLoc() const { return AccessorLoc; }
  void setAccessorLoc(SourceLocation L) { AccessorLoc = L; }

  /// getNumElements - Get the number of components being selected.
  unsigned getNumElements() const { return Positions.Count; }

  /// containsDuplicateElements - Return true if any element access is
  /// repeated.
  bool containsDuplicateElements() const { return Positions.ContainsDuplicateElements(); }

  /// getEncodedElementAccess - Encode the elements accessed
  hlsl::MatrixMemberAccessPositions getEncodedElementAccess() const { return Positions; }
  /// getEncodedElementAccess - Encode the elements accessed
  void getEncodedElementAccess(SmallVectorImpl<unsigned> &Elts) const {
    for (uint32_t i = 0; i < Positions.Count; i++) {
      uint32_t row, col;
      // Save both row and col.
      Positions.GetPosition(i, &row, &col);
      Elts.push_back(row);
      Elts.push_back(col);
    }
  }
  SourceLocation getLocStart() const LLVM_READONLY{
    return getBase()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY{ return AccessorLoc; }

  /// isArrow - Return true if the base expression is a pointer to vector,
  /// return false if the base expression is a vector.
  bool isArrow() const { return getBase()->getType()->isPointerType(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == ExtMatrixElementExprClass;
  }

  // Iterators
  child_range children() { return child_range(&Base, &Base + 1); }
};
/// HLSLVectorElementExpr - This represents access to specific elements of a
/// vector, and may occur on the left hand side or right hand side.
///
/// Note that the base may have either vector or pointer to vector type, just
/// like a struct field reference.
///
class HLSLVectorElementExpr : public Expr {
  Stmt *Base;
  IdentifierInfo *Accessor;
  SourceLocation AccessorLoc;
  hlsl::VectorMemberAccessPositions Positions;
public:
  HLSLVectorElementExpr(QualType ty, ExprValueKind VK, Expr *base,
    IdentifierInfo &accessor, SourceLocation loc, hlsl::VectorMemberAccessPositions positions)
    : Expr(HLSLVectorElementExprClass, ty, VK,
    (VK == VK_RValue ? OK_Ordinary : OK_VectorComponent),
    base->isTypeDependent(), base->isValueDependent(),
    base->isInstantiationDependent(),
    base->containsUnexpandedParameterPack()),
    Base(base), Accessor(&accessor), AccessorLoc(loc), Positions(positions) {}

  /// \brief Build an empty vector element expression.
  explicit HLSLVectorElementExpr(EmptyShell Empty)
    : Expr(HLSLVectorElementExprClass, Empty) { }

  const Expr *getBase() const { return cast<Expr>(Base); }
  Expr *getBase() { return cast<Expr>(Base); }
  void setBase(Expr *E) { Base = E; }

  IdentifierInfo &getAccessor() const { return *Accessor; }
  void setAccessor(IdentifierInfo *II) { Accessor = II; }

  SourceLocation getAccessorLoc() const { return AccessorLoc; }
  void setAccessorLoc(SourceLocation L) { AccessorLoc = L; }

  /// getNumElements - Get the number of components being selected.
  unsigned getNumElements() const { return Positions.Count; }

  /// containsDuplicateElements - Return true if any element access is
  /// repeated.
  bool containsDuplicateElements() const { return Positions.ContainsDuplicateElements(); }

  /// getEncodedElementAccess - Encode the elements accessed
  hlsl::VectorMemberAccessPositions getEncodedElementAccess() const { return Positions; }

  /// getEncodedElementAccess - Encode the elements accessed
  void getEncodedElementAccess(SmallVectorImpl<unsigned> &Elts) const {
    StringRef Comp = Accessor->getName();
    if (Comp[0] == 's' || Comp[0] == 'S')
      Comp = Comp.substr(1);

    for (unsigned i = 0, e = getNumElements(); i != e; ++i) {
      uint64_t Index = ExtVectorType::getPointAccessorIdx(Comp[i]);
      Elts.push_back(Index);
    }
  }

  SourceLocation getLocStart() const LLVM_READONLY{
    return getBase()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY{ return AccessorLoc; }

  /// isArrow - Return true if the base expression is a pointer to vector,
  /// return false if the base expression is a vector.
  bool isArrow() const { return getBase()->getType()->isPointerType(); }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == HLSLVectorElementExprClass;
  }

  // Iterators
  child_range children() { return child_range(&Base, &Base + 1); }
};
// HLSL Change Ends

/// BlockExpr - Adaptor class for mixing a BlockDecl with expressions.
/// ^{ statement-body }   or   ^(int arg1, float arg2){ statement-body }
class BlockExpr : public Expr {
protected:
  BlockDecl *TheBlock;
public:
  BlockExpr(BlockDecl *BD, QualType ty)
    : Expr(BlockExprClass, ty, VK_RValue, OK_Ordinary,
           ty->isDependentType(), ty->isDependentType(),
           ty->isInstantiationDependentType() || BD->isDependentContext(),
           false),
      TheBlock(BD) {}

  /// \brief Build an empty block expression.
  explicit BlockExpr(EmptyShell Empty) : Expr(BlockExprClass, Empty) { }

  const BlockDecl *getBlockDecl() const { return TheBlock; }
  BlockDecl *getBlockDecl() { return TheBlock; }
  void setBlockDecl(BlockDecl *BD) { TheBlock = BD; }

  // Convenience functions for probing the underlying BlockDecl.
  SourceLocation getCaretLocation() const;
  const Stmt *getBody() const;
  Stmt *getBody();

  SourceLocation getLocStart() const LLVM_READONLY { return getCaretLocation(); }
  SourceLocation getLocEnd() const LLVM_READONLY { return getBody()->getLocEnd(); }

  /// getFunctionType - Return the underlying function type for this block.
  const FunctionProtoType *getFunctionType() const;

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == BlockExprClass;
  }

  // Iterators
  child_range children() { return child_range(); }
};

/// AsTypeExpr - Clang builtin function __builtin_astype [OpenCL 6.2.4.2]
/// This AST node provides support for reinterpreting a type to another
/// type of the same size.
class AsTypeExpr : public Expr {
private:
  Stmt *SrcExpr;
  SourceLocation BuiltinLoc, RParenLoc;

  friend class ASTReader;
  friend class ASTStmtReader;
  explicit AsTypeExpr(EmptyShell Empty) : Expr(AsTypeExprClass, Empty) {}

public:
  AsTypeExpr(Expr* SrcExpr, QualType DstType,
             ExprValueKind VK, ExprObjectKind OK,
             SourceLocation BuiltinLoc, SourceLocation RParenLoc)
    : Expr(AsTypeExprClass, DstType, VK, OK,
           DstType->isDependentType(),
           DstType->isDependentType() || SrcExpr->isValueDependent(),
           (DstType->isInstantiationDependentType() ||
            SrcExpr->isInstantiationDependent()),
           (DstType->containsUnexpandedParameterPack() ||
            SrcExpr->containsUnexpandedParameterPack())),
  SrcExpr(SrcExpr), BuiltinLoc(BuiltinLoc), RParenLoc(RParenLoc) {}

  /// getSrcExpr - Return the Expr to be converted.
  Expr *getSrcExpr() const { return cast<Expr>(SrcExpr); }

  /// getBuiltinLoc - Return the location of the __builtin_astype token.
  SourceLocation getBuiltinLoc() const { return BuiltinLoc; }

  /// getRParenLoc - Return the location of final right parenthesis.
  SourceLocation getRParenLoc() const { return RParenLoc; }

  SourceLocation getLocStart() const LLVM_READONLY { return BuiltinLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AsTypeExprClass;
  }

  // Iterators
  child_range children() { return child_range(&SrcExpr, &SrcExpr+1); }
};

/// PseudoObjectExpr - An expression which accesses a pseudo-object
/// l-value.  A pseudo-object is an abstract object, accesses to which
/// are translated to calls.  The pseudo-object expression has a
/// syntactic form, which shows how the expression was actually
/// written in the source code, and a semantic form, which is a series
/// of expressions to be executed in order which detail how the
/// operation is actually evaluated.  Optionally, one of the semantic
/// forms may also provide a result value for the expression.
///
/// If any of the semantic-form expressions is an OpaqueValueExpr,
/// that OVE is required to have a source expression, and it is bound
/// to the result of that source expression.  Such OVEs may appear
/// only in subsequent semantic-form expressions and as
/// sub-expressions of the syntactic form.
///
/// PseudoObjectExpr should be used only when an operation can be
/// usefully described in terms of fairly simple rewrite rules on
/// objects and functions that are meant to be used by end-developers.
/// For example, under the Itanium ABI, dynamic casts are implemented
/// as a call to a runtime function called __dynamic_cast; using this
/// class to describe that would be inappropriate because that call is
/// not really part of the user-visible semantics, and instead the
/// cast is properly reflected in the AST and IR-generation has been
/// taught to generate the call as necessary.  In contrast, an
/// Objective-C property access is semantically defined to be
/// equivalent to a particular message send, and this is very much
/// part of the user model.  The name of this class encourages this
/// modelling design.
class PseudoObjectExpr : public Expr {
  // PseudoObjectExprBits.NumSubExprs - The number of sub-expressions.
  // Always at least two, because the first sub-expression is the
  // syntactic form.

  // PseudoObjectExprBits.ResultIndex - The index of the
  // sub-expression holding the result.  0 means the result is void,
  // which is unambiguous because it's the index of the syntactic
  // form.  Note that this is therefore 1 higher than the value passed
  // in to Create, which is an index within the semantic forms.
  // Note also that ASTStmtWriter assumes this encoding.

  Expr **getSubExprsBuffer() { return reinterpret_cast<Expr**>(this + 1); }
  const Expr * const *getSubExprsBuffer() const {
    return reinterpret_cast<const Expr * const *>(this + 1);
  }

  friend class ASTStmtReader;

  PseudoObjectExpr(QualType type, ExprValueKind VK,
                   Expr *syntactic, ArrayRef<Expr*> semantic,
                   unsigned resultIndex);

  PseudoObjectExpr(EmptyShell shell, unsigned numSemanticExprs);

  unsigned getNumSubExprs() const {
    return PseudoObjectExprBits.NumSubExprs;
  }

public:
  /// NoResult - A value for the result index indicating that there is
  /// no semantic result.
  enum : unsigned { NoResult = ~0U };

  static PseudoObjectExpr *Create(const ASTContext &Context, Expr *syntactic,
                                  ArrayRef<Expr*> semantic,
                                  unsigned resultIndex);

  static PseudoObjectExpr *Create(const ASTContext &Context, EmptyShell shell,
                                  unsigned numSemanticExprs);

  /// Return the syntactic form of this expression, i.e. the
  /// expression it actually looks like.  Likely to be expressed in
  /// terms of OpaqueValueExprs bound in the semantic form.
  Expr *getSyntacticForm() { return getSubExprsBuffer()[0]; }
  const Expr *getSyntacticForm() const { return getSubExprsBuffer()[0]; }

  /// Return the index of the result-bearing expression into the semantics
  /// expressions, or PseudoObjectExpr::NoResult if there is none.
  unsigned getResultExprIndex() const {
    if (PseudoObjectExprBits.ResultIndex == 0) return NoResult;
    return PseudoObjectExprBits.ResultIndex - 1;
  }

  /// Return the result-bearing expression, or null if there is none.
  Expr *getResultExpr() {
    if (PseudoObjectExprBits.ResultIndex == 0)
      return nullptr;
    return getSubExprsBuffer()[PseudoObjectExprBits.ResultIndex];
  }
  const Expr *getResultExpr() const {
    return const_cast<PseudoObjectExpr*>(this)->getResultExpr();
  }

  unsigned getNumSemanticExprs() const { return getNumSubExprs() - 1; }

  typedef Expr * const *semantics_iterator;
  typedef const Expr * const *const_semantics_iterator;
  semantics_iterator semantics_begin() {
    return getSubExprsBuffer() + 1;
  }
  const_semantics_iterator semantics_begin() const {
    return getSubExprsBuffer() + 1;
  }
  semantics_iterator semantics_end() {
    return getSubExprsBuffer() + getNumSubExprs();
  }
  const_semantics_iterator semantics_end() const {
    return getSubExprsBuffer() + getNumSubExprs();
  }
  Expr *getSemanticExpr(unsigned index) {
    assert(index + 1 < getNumSubExprs());
    return getSubExprsBuffer()[index + 1];
  }
  const Expr *getSemanticExpr(unsigned index) const {
    return const_cast<PseudoObjectExpr*>(this)->getSemanticExpr(index);
  }

  SourceLocation getExprLoc() const LLVM_READONLY {
    return getSyntacticForm()->getExprLoc();
  }

  SourceLocation getLocStart() const LLVM_READONLY {
    return getSyntacticForm()->getLocStart();
  }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getSyntacticForm()->getLocEnd();
  }

  child_range children() {
    Stmt **cs = reinterpret_cast<Stmt**>(getSubExprsBuffer());
    return child_range(cs, cs + getNumSubExprs());
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == PseudoObjectExprClass;
  }
};

/// AtomicExpr - Variadic atomic builtins: __atomic_exchange, __atomic_fetch_*,
/// __atomic_load, __atomic_store, and __atomic_compare_exchange_*, for the
/// similarly-named C++11 instructions, and __c11 variants for <stdatomic.h>.
/// All of these instructions take one primary pointer and at least one memory
/// order.
class AtomicExpr : public Expr {
public:
  enum AtomicOp {
#define BUILTIN(ID, TYPE, ATTRS)
#define ATOMIC_BUILTIN(ID, TYPE, ATTRS) AO ## ID,
#include "clang/Basic/Builtins.def"
    // Avoid trailing comma
    BI_First = 0
  };

  // The ABI values for various atomic memory orderings.
  enum AtomicOrderingKind {
    AO_ABI_memory_order_relaxed = 0,
    AO_ABI_memory_order_consume = 1,
    AO_ABI_memory_order_acquire = 2,
    AO_ABI_memory_order_release = 3,
    AO_ABI_memory_order_acq_rel = 4,
    AO_ABI_memory_order_seq_cst = 5
  };

private:
  enum { PTR, ORDER, VAL1, ORDER_FAIL, VAL2, WEAK, END_EXPR };
  Stmt* SubExprs[END_EXPR];
  unsigned NumSubExprs;
  SourceLocation BuiltinLoc, RParenLoc;
  AtomicOp Op;

  friend class ASTStmtReader;

public:
  AtomicExpr(SourceLocation BLoc, ArrayRef<Expr*> args, QualType t,
             AtomicOp op, SourceLocation RP);

  /// \brief Determine the number of arguments the specified atomic builtin
  /// should have.
  static unsigned getNumSubExprs(AtomicOp Op);

  /// \brief Build an empty AtomicExpr.
  explicit AtomicExpr(EmptyShell Empty) : Expr(AtomicExprClass, Empty) { }

  Expr *getPtr() const {
    return cast<Expr>(SubExprs[PTR]);
  }
  Expr *getOrder() const {
    return cast<Expr>(SubExprs[ORDER]);
  }
  Expr *getVal1() const {
    if (Op == AO__c11_atomic_init)
      return cast<Expr>(SubExprs[ORDER]);
    assert(NumSubExprs > VAL1);
    return cast<Expr>(SubExprs[VAL1]);
  }
  Expr *getOrderFail() const {
    assert(NumSubExprs > ORDER_FAIL);
    return cast<Expr>(SubExprs[ORDER_FAIL]);
  }
  Expr *getVal2() const {
    if (Op == AO__atomic_exchange)
      return cast<Expr>(SubExprs[ORDER_FAIL]);
    assert(NumSubExprs > VAL2);
    return cast<Expr>(SubExprs[VAL2]);
  }
  Expr *getWeak() const {
    assert(NumSubExprs > WEAK);
    return cast<Expr>(SubExprs[WEAK]);
  }

  AtomicOp getOp() const { return Op; }
  unsigned getNumSubExprs() { return NumSubExprs; }

  Expr **getSubExprs() { return reinterpret_cast<Expr **>(SubExprs); }

  bool isVolatile() const {
    return getPtr()->getType()->getPointeeType().isVolatileQualified();
  }

  bool isCmpXChg() const {
    return getOp() == AO__c11_atomic_compare_exchange_strong ||
           getOp() == AO__c11_atomic_compare_exchange_weak ||
           getOp() == AO__atomic_compare_exchange ||
           getOp() == AO__atomic_compare_exchange_n;
  }

  SourceLocation getBuiltinLoc() const { return BuiltinLoc; }
  SourceLocation getRParenLoc() const { return RParenLoc; }

  SourceLocation getLocStart() const LLVM_READONLY { return BuiltinLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY { return RParenLoc; }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == AtomicExprClass;
  }

  // Iterators
  child_range children() {
    return child_range(SubExprs, SubExprs+NumSubExprs);
  }
};

/// TypoExpr - Internal placeholder for expressions where typo correction
/// still needs to be performed and/or an error diagnostic emitted.
class TypoExpr : public Expr {
public:
  TypoExpr(QualType T)
      : Expr(TypoExprClass, T, VK_LValue, OK_Ordinary,
             /*isTypeDependent*/ true,
             /*isValueDependent*/ true,
             /*isInstantiationDependent*/ true,
             /*containsUnexpandedParameterPack*/ false) {
    assert(T->isDependentType() && "TypoExpr given a non-dependent type");
  }

  child_range children() { return child_range(); }
  SourceLocation getLocStart() const LLVM_READONLY { return SourceLocation(); }
  SourceLocation getLocEnd() const LLVM_READONLY { return SourceLocation(); }
};
}  // end namespace clang

#endif
