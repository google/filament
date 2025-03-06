//===- OperationKinds.h - Operation enums -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file enumerates the different kinds of operations that can be
// performed by various expressions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_OPERATIONKINDS_H
#define LLVM_CLANG_AST_OPERATIONKINDS_H

#include <limits> // HLSL Change

namespace clang {
  
/// CastKind - The kind of operation required for a conversion.
enum CastKind {
  /// CK_Dependent - A conversion which cannot yet be analyzed because
  /// either the expression or target type is dependent.  These are
  /// created only for explicit casts; dependent ASTs aren't required
  /// to even approximately type-check.
  ///   (T*) malloc(sizeof(T))
  ///   reinterpret_cast<intptr_t>(A<T>::alloc());
  CK_Dependent,

  /// CK_BitCast - A conversion which causes a bit pattern of one type
  /// to be reinterpreted as a bit pattern of another type.  Generally
  /// the operands must have equivalent size and unrelated types.
  ///
  /// The pointer conversion char* -> int* is a bitcast.  A conversion
  /// from any pointer type to a C pointer type is a bitcast unless
  /// it's actually BaseToDerived or DerivedToBase.  A conversion to a
  /// block pointer or ObjC pointer type is a bitcast only if the
  /// operand has the same type kind; otherwise, it's one of the
  /// specialized casts below.
  ///
  /// Vector coercions are bitcasts.
  CK_BitCast,

  /// CK_LValueBitCast - A conversion which reinterprets the address of
  /// an l-value as an l-value of a different kind.  Used for
  /// reinterpret_casts of l-value expressions to reference types.
  ///    bool b; reinterpret_cast<char&>(b) = 'a';
  CK_LValueBitCast,

  /// CK_LValueToRValue - A conversion which causes the extraction of
  /// an r-value from the operand gl-value.  The result of an r-value
  /// conversion is always unqualified.
  CK_LValueToRValue,

  /// CK_NoOp - A conversion which does not affect the type other than
  /// (possibly) adding qualifiers.
  ///   int    -> int
  ///   char** -> const char * const *
  CK_NoOp,

  /// CK_BaseToDerived - A conversion from a C++ class pointer/reference
  /// to a derived class pointer/reference.
  ///   B *b = static_cast<B*>(a);
  CK_BaseToDerived,

  /// CK_DerivedToBase - A conversion from a C++ class pointer
  /// to a base class pointer.
  ///   A *a = new B();
  CK_DerivedToBase,

  /// CK_UncheckedDerivedToBase - A conversion from a C++ class
  /// pointer/reference to a base class that can assume that the
  /// derived pointer is not null.
  ///   const A &a = B();
  ///   b->method_from_a();
  CK_UncheckedDerivedToBase,

  /// CK_Dynamic - A C++ dynamic_cast.
  CK_Dynamic,

  /// CK_ToUnion - The GCC cast-to-union extension.
  ///   int   -> union { int x; float y; }
  ///   float -> union { int x; float y; }
  CK_ToUnion,

  /// CK_ArrayToPointerDecay - Array to pointer decay.
  ///   int[10] -> int*
  ///   char[5][6] -> char(*)[6]
  CK_ArrayToPointerDecay,

  /// CK_FunctionToPointerDecay - Function to pointer decay.
  ///   void(int) -> void(*)(int)
  CK_FunctionToPointerDecay,

  /// CK_NullToPointer - Null pointer constant to pointer, ObjC
  /// pointer, or block pointer.
  ///   (void*) 0
  ///   void (^block)() = 0;
  CK_NullToPointer,

  /// CK_NullToMemberPointer - Null pointer constant to member pointer.
  ///   int A::*mptr = 0;
  ///   int (A::*fptr)(int) = nullptr;
  CK_NullToMemberPointer,

  /// CK_BaseToDerivedMemberPointer - Member pointer in base class to
  /// member pointer in derived class.
  ///   int B::*mptr = &A::member;
  CK_BaseToDerivedMemberPointer,

  /// CK_DerivedToBaseMemberPointer - Member pointer in derived class to
  /// member pointer in base class.
  ///   int A::*mptr = static_cast<int A::*>(&B::member);
  CK_DerivedToBaseMemberPointer,

  /// CK_MemberPointerToBoolean - Member pointer to boolean.  A check
  /// against the null member pointer.
  CK_MemberPointerToBoolean,

  /// CK_ReinterpretMemberPointer - Reinterpret a member pointer as a
  /// different kind of member pointer.  C++ forbids this from
  /// crossing between function and object types, but otherwise does
  /// not restrict it.  However, the only operation that is permitted
  /// on a "punned" member pointer is casting it back to the original
  /// type, which is required to be a lossless operation (although
  /// many ABIs do not guarantee this on all possible intermediate types).
  CK_ReinterpretMemberPointer,

  /// CK_UserDefinedConversion - Conversion using a user defined type
  /// conversion function.
  ///    struct A { operator int(); }; int i = int(A());
  CK_UserDefinedConversion,

  /// CK_ConstructorConversion - Conversion by constructor.
  ///    struct A { A(int); }; A a = A(10);
  CK_ConstructorConversion,

  /// CK_IntegralToPointer - Integral to pointer.  A special kind of
  /// reinterpreting conversion.  Applies to normal, ObjC, and block
  /// pointers.
  ///    (char*) 0x1001aab0
  ///    reinterpret_cast<int*>(0)
  CK_IntegralToPointer,

  /// CK_PointerToIntegral - Pointer to integral.  A special kind of
  /// reinterpreting conversion.  Applies to normal, ObjC, and block
  /// pointers.
  ///    (intptr_t) "help!"
  CK_PointerToIntegral,

  /// CK_PointerToBoolean - Pointer to boolean conversion.  A check
  /// against null.  Applies to normal, ObjC, and block pointers.
  CK_PointerToBoolean,

  /// CK_ToVoid - Cast to void, discarding the computed value.
  ///    (void) malloc(2048)
  CK_ToVoid,

  /// CK_VectorSplat - A conversion from an arithmetic type to a
  /// vector of that element type.  Fills all elements ("splats") with
  /// the source value.
  ///    __attribute__((ext_vector_type(4))) int v = 5;
  CK_VectorSplat,

  /// CK_IntegralCast - A cast between integral types (other than to
  /// boolean).  Variously a bitcast, a truncation, a sign-extension,
  /// or a zero-extension.
  ///    long l = 5;
  ///    (unsigned) i
  CK_IntegralCast,

  /// CK_IntegralToBoolean - Integral to boolean.  A check against zero.
  ///    (bool) i
  CK_IntegralToBoolean,

  /// CK_IntegralToFloating - Integral to floating point.
  ///    float f = i;
  CK_IntegralToFloating,

  /// CK_FloatingToIntegral - Floating point to integral.  Rounds
  /// towards zero, discarding any fractional component.
  ///    (int) f
  CK_FloatingToIntegral,

  /// CK_FloatingToBoolean - Floating point to boolean.
  ///    (bool) f
  CK_FloatingToBoolean,

  /// CK_FloatingCast - Casting between floating types of different size.
  ///    (double) f
  ///    (float) ld
  CK_FloatingCast,

  /// CK_CPointerToObjCPointerCast - Casting a C pointer kind to an
  /// Objective-C pointer.
  CK_CPointerToObjCPointerCast,

  /// CK_BlockPointerToObjCPointerCast - Casting a block pointer to an
  /// ObjC pointer.
  CK_BlockPointerToObjCPointerCast,

  /// CK_AnyPointerToBlockPointerCast - Casting any non-block pointer
  /// to a block pointer.  Block-to-block casts are bitcasts.
  CK_AnyPointerToBlockPointerCast,

  /// \brief Converting between two Objective-C object types, which
  /// can occur when performing reference binding to an Objective-C
  /// object.
  CK_ObjCObjectLValueCast,

  /// \brief A conversion of a floating point real to a floating point
  /// complex of the original type.  Injects the value as the real
  /// component with a zero imaginary component.
  ///   float -> _Complex float
  CK_FloatingRealToComplex,

  /// \brief Converts a floating point complex to floating point real
  /// of the source's element type.  Just discards the imaginary
  /// component.
  ///   _Complex long double -> long double
  CK_FloatingComplexToReal,

  /// \brief Converts a floating point complex to bool by comparing
  /// against 0+0i.
  CK_FloatingComplexToBoolean,

  /// \brief Converts between different floating point complex types.
  ///   _Complex float -> _Complex double
  CK_FloatingComplexCast,

  /// \brief Converts from a floating complex to an integral complex.
  ///   _Complex float -> _Complex int
  CK_FloatingComplexToIntegralComplex,

  /// \brief Converts from an integral real to an integral complex
  /// whose element type matches the source.  Injects the value as
  /// the real component with a zero imaginary component.
  ///   long -> _Complex long
  CK_IntegralRealToComplex,

  /// \brief Converts an integral complex to an integral real of the
  /// source's element type by discarding the imaginary component.
  ///   _Complex short -> short
  CK_IntegralComplexToReal,

  /// \brief Converts an integral complex to bool by comparing against
  /// 0+0i.
  CK_IntegralComplexToBoolean,

  /// \brief Converts between different integral complex types.
  ///   _Complex char -> _Complex long long
  ///   _Complex unsigned int -> _Complex signed int
  CK_IntegralComplexCast,

  /// \brief Converts from an integral complex to a floating complex.
  ///   _Complex unsigned -> _Complex float
  CK_IntegralComplexToFloatingComplex,

  /// \brief [ARC] Produces a retainable object pointer so that it may
  /// be consumed, e.g. by being passed to a consuming parameter.
  /// Calls objc_retain.
  CK_ARCProduceObject,

  /// \brief [ARC] Consumes a retainable object pointer that has just
  /// been produced, e.g. as the return value of a retaining call.
  /// Enters a cleanup to call objc_release at some indefinite time.
  CK_ARCConsumeObject,

  /// \brief [ARC] Reclaim a retainable object pointer object that may
  /// have been produced and autoreleased as part of a function return
  /// sequence.
  CK_ARCReclaimReturnedObject,

  /// \brief [ARC] Causes a value of block type to be copied to the
  /// heap, if it is not already there.  A number of other operations
  /// in ARC cause blocks to be copied; this is for cases where that
  /// would not otherwise be guaranteed, such as when casting to a
  /// non-block pointer type.
  CK_ARCExtendBlockObject,

  /// \brief Converts from _Atomic(T) to T.
  CK_AtomicToNonAtomic,
  /// \brief Converts from T to _Atomic(T).
  CK_NonAtomicToAtomic,

  /// \brief Causes a block literal to by copied to the heap and then
  /// autoreleased.
  ///
  /// This particular cast kind is used for the conversion from a C++11
  /// lambda expression to a block pointer.
  CK_CopyAndAutoreleaseBlockObject,

  // Convert a builtin function to a function pointer; only allowed in the
  // callee of a call expression.
  CK_BuiltinFnToFnPtr,

  // Convert a zero value for OpenCL event_t initialization.
  CK_ZeroToOCLEvent,

  // Convert a pointer to a different address space.
  CK_AddressSpaceConversion

  // HLSL Change Starts
  ,
  CK_FlatConversion,
  CK_HLSLVectorSplat,
  CK_HLSLMatrixSplat,
  CK_HLSLVectorToScalarCast,
  CK_HLSLMatrixToScalarCast,
  CK_HLSLVectorTruncationCast,
  CK_HLSLMatrixTruncationCast,
  CK_HLSLVectorToMatrixCast,
  CK_HLSLMatrixToVectorCast,
  CK_HLSLDerivedToBase,
  // HLSL ComponentConversion (HLSLCC) Casts:
  CK_HLSLCC_IntegralCast,
  CK_HLSLCC_IntegralToBoolean,
  CK_HLSLCC_IntegralToFloating,
  CK_HLSLCC_FloatingToIntegral,
  CK_HLSLCC_FloatingToBoolean,
  CK_HLSLCC_FloatingCast,

  // HLSL Change - Made CK_Invalid an enum case because otherwise it is UB to
  // assign it to a value of CastKind.
  CK_Invalid = std::numeric_limits<unsigned int>::max()
};

static_assert(
    sizeof(CastKind) == sizeof(unsigned int),
    "Cast Kind larger than expected. Must increase value of CK_Invalid.");
// HLSL Change Ends

enum BinaryOperatorKind {
  // Operators listed in order of precedence.
  // Note that additions to this should also update the StmtVisitor class.
  BO_PtrMemD, BO_PtrMemI,       // [C++ 5.5] Pointer-to-member operators.
  BO_Mul, BO_Div, BO_Rem,       // [C99 6.5.5] Multiplicative operators.
  BO_Add, BO_Sub,               // [C99 6.5.6] Additive operators.
  BO_Shl, BO_Shr,               // [C99 6.5.7] Bitwise shift operators.
  BO_LT, BO_GT, BO_LE, BO_GE,   // [C99 6.5.8] Relational operators.
  BO_EQ, BO_NE,                 // [C99 6.5.9] Equality operators.
  BO_And,                       // [C99 6.5.10] Bitwise AND operator.
  BO_Xor,                       // [C99 6.5.11] Bitwise XOR operator.
  BO_Or,                        // [C99 6.5.12] Bitwise OR operator.
  BO_LAnd,                      // [C99 6.5.13] Logical AND operator.
  BO_LOr,                       // [C99 6.5.14] Logical OR operator.
  BO_Assign, BO_MulAssign,      // [C99 6.5.16] Assignment operators.
  BO_DivAssign, BO_RemAssign,
  BO_AddAssign, BO_SubAssign,
  BO_ShlAssign, BO_ShrAssign,
  BO_AndAssign, BO_XorAssign,
  BO_OrAssign,
  BO_Comma                      // [C99 6.5.17] Comma operator.
};

enum UnaryOperatorKind {
  // Note that additions to this should also update the StmtVisitor class.
  UO_PostInc, UO_PostDec, // [C99 6.5.2.4] Postfix increment and decrement
  UO_PreInc, UO_PreDec,   // [C99 6.5.3.1] Prefix increment and decrement
  UO_AddrOf, UO_Deref,    // [C99 6.5.3.2] Address and indirection
  UO_Plus, UO_Minus,      // [C99 6.5.3.3] Unary arithmetic
  UO_Not, UO_LNot,        // [C99 6.5.3.3] Unary arithmetic
  UO_Real, UO_Imag,       // "__real expr"/"__imag expr" Extension.
  UO_Extension            // __extension__ marker.
};

/// \brief The kind of bridging performed by the Objective-C bridge cast.
enum ObjCBridgeCastKind {
  /// \brief Bridging via __bridge, which does nothing but reinterpret
  /// the bits.
  OBC_Bridge,
  /// \brief Bridging via __bridge_transfer, which transfers ownership of an
  /// Objective-C pointer into ARC.
  OBC_BridgeTransfer,
  /// \brief Bridging via __bridge_retain, which makes an ARC object available
  /// as a +1 C pointer.
  OBC_BridgeRetained
};

}

#endif
