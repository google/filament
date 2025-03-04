//===--- Specifiers.h - Declaration and Type Specifiers ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines various enumerations that describe declaration and
/// type specifiers.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_SPECIFIERS_H
#define LLVM_CLANG_BASIC_SPECIFIERS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/ADT/ArrayRef.h" // HLSL Change

namespace clang {
  /// \brief Specifies the width of a type, e.g., short, long, or long long.
  enum TypeSpecifierWidth {
    TSW_unspecified,
    TSW_short,
    TSW_long,
    TSW_longlong
  };
  
  /// \brief Specifies the signedness of a type, e.g., signed or unsigned.
  enum TypeSpecifierSign {
    TSS_unspecified,
    TSS_signed,
    TSS_unsigned
  };
  
  /// \brief Specifies the kind of type.
  enum TypeSpecifierType {
    TST_unspecified,
    TST_void,
    TST_char,
    TST_wchar,        // C++ wchar_t
    TST_char16,       // C++11 char16_t
    TST_char32,       // C++11 char32_t
    TST_int,
    TST_int128,
    // HLSL Changes begin
    TST_min16float,
    TST_min16int,
    TST_min16uint,
    TST_min10float,
    TST_min12int,
    TST_int8_4packed,
    TST_uint8_4packed,
    // HLSL Changes end
    TST_half,         // OpenCL half, ARM NEON __fp16
    TST_halffloat, // HLSL Change
    TST_float,
    TST_double,
    TST_bool,         // _Bool
    TST_decimal32,    // _Decimal32
    TST_decimal64,    // _Decimal64
    TST_decimal128,   // _Decimal128
    TST_enum,
    TST_union,
    TST_struct,
    TST_class,        // C++ class type
    TST_interface,    // C++ (Microsoft-specific) __interface type
    TST_typename,     // Typedef, C++ class-name or enum name, etc.
    TST_typeofType,
    TST_typeofExpr,
    TST_decltype,         // C++11 decltype
    TST_underlyingType,   // __underlying_type for C++11
    TST_auto,             // C++11 auto
    TST_decltype_auto,    // C++1y decltype(auto)
    TST_unknown_anytype,  // __unknown_anytype extension
    TST_atomic,           // C11 _Atomic
    TST_error         // erroneous type
  };
  
  /// \brief Structure that packs information about the type specifiers that
  /// were written in a particular type specifier sequence.
  struct WrittenBuiltinSpecs {
    /*DeclSpec::TST*/ unsigned Type  : 5;
    /*DeclSpec::TSS*/ unsigned Sign  : 2;
    /*DeclSpec::TSW*/ unsigned Width : 2;
    bool ModeAttr : 1;
  };  

  /// \brief A C++ access specifier (public, private, protected), plus the
  /// special value "none" which means different things in different contexts.
  enum AccessSpecifier {
    AS_public,
    AS_protected,
    AS_private,
    AS_none
  };

  /// \brief The categorization of expression values, currently following the
  /// C++11 scheme.
  enum ExprValueKind {
    /// \brief An r-value expression (a pr-value in the C++11 taxonomy)
    /// produces a temporary value.
    VK_RValue,

    /// \brief An l-value expression is a reference to an object with
    /// independent storage.
    VK_LValue,

    /// \brief An x-value expression is a reference to an object with
    /// independent storage but which can be "moved", i.e.
    /// efficiently cannibalized for its resources.
    VK_XValue
  };

  /// \brief A further classification of the kind of object referenced by an
  /// l-value or x-value.
  enum ExprObjectKind {
    /// An ordinary object is located at an address in memory.
    OK_Ordinary,

    /// A bitfield object is a bitfield on a C or C++ record.
    OK_BitField,

    /// A vector component is an element or range of elements on a vector.
    OK_VectorComponent,

    /// An Objective-C property is a logical field of an Objective-C
    /// object which is read and written via Objective-C method calls.
    OK_ObjCProperty,
    
    /// An Objective-C array/dictionary subscripting which reads an
    /// object or writes at the subscripted array/dictionary element via
    /// Objective-C method calls.
    OK_ObjCSubscript
  };

  /// \brief Describes the kind of template specialization that a
  /// particular template specialization declaration represents.
  enum TemplateSpecializationKind {
    /// This template specialization was formed from a template-id but
    /// has not yet been declared, defined, or instantiated.
    TSK_Undeclared = 0,
    /// This template specialization was implicitly instantiated from a
    /// template. (C++ [temp.inst]).
    TSK_ImplicitInstantiation,
    /// This template specialization was declared or defined by an
    /// explicit specialization (C++ [temp.expl.spec]) or partial
    /// specialization (C++ [temp.class.spec]).
    TSK_ExplicitSpecialization,
    /// This template specialization was instantiated from a template
    /// due to an explicit instantiation declaration request
    /// (C++11 [temp.explicit]).
    TSK_ExplicitInstantiationDeclaration,
    /// This template specialization was instantiated from a template
    /// due to an explicit instantiation definition request
    /// (C++ [temp.explicit]).
    TSK_ExplicitInstantiationDefinition
  };

  /// \brief Determine whether this template specialization kind refers
  /// to an instantiation of an entity (as opposed to a non-template or
  /// an explicit specialization).
  inline bool isTemplateInstantiation(TemplateSpecializationKind Kind) {
    return Kind != TSK_Undeclared && Kind != TSK_ExplicitSpecialization;
  }

  /// \brief Thread storage-class-specifier.
  enum ThreadStorageClassSpecifier {
    TSCS_unspecified,
    /// GNU __thread.
    TSCS___thread,
    /// C++11 thread_local. Implies 'static' at block scope, but not at
    /// class scope.
    TSCS_thread_local,
    /// C11 _Thread_local. Must be combined with either 'static' or 'extern'
    /// if used at block scope.
    TSCS__Thread_local
  };

  /// \brief Storage classes.
  enum StorageClass {
    // These are legal on both functions and variables.
    SC_None,
    SC_Extern,
    SC_Static,
    SC_PrivateExtern,

    // These are only legal on variables.
    SC_OpenCLWorkGroupLocal,
    SC_Auto,
    SC_Register
  };

  /// \brief Checks whether the given storage class is legal for functions.
  inline bool isLegalForFunction(StorageClass SC) {
    return SC <= SC_PrivateExtern;
  }

  /// \brief Checks whether the given storage class is legal for variables.
  inline bool isLegalForVariable(StorageClass SC) {
    return true;
  }

  /// \brief In-class initialization styles for non-static data members.
  enum InClassInitStyle {
    ICIS_NoInit,   ///< No in-class initializer.
    ICIS_CopyInit, ///< Copy initialization.
    ICIS_ListInit  ///< Direct list-initialization.
  };

  /// \brief CallingConv - Specifies the calling convention that a function uses.
  enum CallingConv {
    CC_C,           // __attribute__((cdecl))
    CC_X86StdCall,  // __attribute__((stdcall))
    CC_X86FastCall, // __attribute__((fastcall))
    CC_X86ThisCall, // __attribute__((thiscall))
    CC_X86VectorCall, // __attribute__((vectorcall))
    CC_X86Pascal,   // __attribute__((pascal))
    CC_X86_64Win64, // __attribute__((ms_abi))
    CC_X86_64SysV,  // __attribute__((sysv_abi))
    CC_AAPCS,       // __attribute__((pcs("aapcs")))
    CC_AAPCS_VFP,   // __attribute__((pcs("aapcs-vfp")))
    CC_IntelOclBicc, // __attribute__((intel_ocl_bicc))
    CC_SpirFunction, // default for OpenCL functions on SPIR target
    CC_SpirKernel    // inferred for OpenCL kernels on SPIR target
  };

  /// \brief Checks whether the given calling convention supports variadic
  /// calls. Unprototyped calls also use the variadic call rules.
  inline bool supportsVariadicCall(CallingConv CC) {
    switch (CC) {
    case CC_X86StdCall:
    case CC_X86FastCall:
    case CC_X86ThisCall:
    case CC_X86Pascal:
    case CC_X86VectorCall:
    case CC_SpirFunction:
    case CC_SpirKernel:
      return false;
    default:
      return true;
    }
  }

  /// \brief The storage duration for an object (per C++ [basic.stc]).
  enum StorageDuration {
    SD_FullExpression, ///< Full-expression storage duration (for temporaries).
    SD_Automatic,      ///< Automatic storage duration (most local variables).
    SD_Thread,         ///< Thread storage duration.
    SD_Static,         ///< Static storage duration.
    SD_Dynamic         ///< Dynamic storage duration.
  };

  /// Describes the nullability of a particular type.
  enum class NullabilityKind : uint8_t {
    /// Values of this type can never be null.
    NonNull = 0,
    /// Values of this type can be null.
    Nullable,
    /// Whether values of this type can be null is (explicitly)
    /// unspecified. This captures a (fairly rare) case where we
    /// can't conclude anything about the nullability of the type even
    /// though it has been considered.
    Unspecified
  };

  /// Retrieve the spelling of the given nullability kind.
  llvm::StringRef getNullabilitySpelling(NullabilityKind kind,
                                         bool isContextSensitive = false);
} // end namespace clang

// HLSL Change Starts
namespace hlsl {
  /// In/inout/out/uniform parameter modifier.
  class ParameterModifier {
  public:
    enum Kind { In, InOut, Out, Uniform, Ref, Invalid };

    ParameterModifier() : m_Kind(Kind::In) { }
    ParameterModifier(enum Kind Kind) : m_Kind(Kind) { }

    static ParameterModifier FromInOut(bool isIn, bool isOut) {
      if (isIn && !isOut)
        return hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::In);
      if (isIn && isOut)
        return hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::InOut);
      assert(!isIn && isOut && "else args are invalid");
      return hlsl::ParameterModifier(hlsl::ParameterModifier::Kind::Out);
    }

    Kind GetKind() const { return m_Kind; }

    unsigned getAsUnsigned() const { return (unsigned)m_Kind; }

    bool isIn() const { return m_Kind == Kind::In; }
    bool isOut() const { return m_Kind == Kind::Out; }
    bool isInOut() const { return m_Kind == Kind::InOut; }
    bool isAnyIn() const { return isIn() || isInOut(); }
    bool isAnyOut() const { return isOut() || isInOut(); }
    bool operator==(const ParameterModifier& other) const { return m_Kind == other.m_Kind; }
    bool operator!=(const ParameterModifier& other) const { return m_Kind != other.m_Kind; }

  private:
    enum Kind m_Kind;
  };
}

// HLSL Change Ends

#endif // LLVM_CLANG_BASIC_SPECIFIERS_H
