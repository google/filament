// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/macros/concat.h"
#include "src/utils/compiler.h"

#ifndef SRC_TINT_UTILS_MACROS_COMPILER_H_
#define SRC_TINT_UTILS_MACROS_COMPILER_H_

#define TINT_REQUIRE_SEMICOLON static_assert(true)

#if defined(_MSC_VER) && !defined(__clang__)
////////////////////////////////////////////////////////////////////////////////
// MSVC
////////////////////////////////////////////////////////////////////////////////
#define TINT_BUILD_IS_MSVC 1
#define TINT_DISABLE_WARNING_CONSTANT_OVERFLOW __pragma(warning(disable : 4756))
#define TINT_DISABLE_WARNING_DEPRECATED __pragma(warning(disable : 4996))
#define TINT_DISABLE_WARNING_DESTRUCTOR_NEVER_RETURNS __pragma(warning(disable : 4722))
#define TINT_DISABLE_WARNING_EXTRA_SEMICOLON /* currently no-op */
#define TINT_DISABLE_WARNING_FLOAT_EQUAL     /* currently no-op */
#define TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED __pragma(warning(disable : 4701))
#define TINT_DISABLE_WARNING_MISSING_DESTRUCTOR_OVERRIDE /* currently no-op */
#define TINT_DISABLE_WARNING_NEWLINE_EOF                 /* currently no-op */
#define TINT_DISABLE_WARNING_OLD_STYLE_CAST              /* currently no-op */
#define TINT_DISABLE_WARNING_PEDANTIC                    /* currently no-op */
#define TINT_DISABLE_WARNING_REDUNDANT_PARENS            /* currently no-op */
#define TINT_DISABLE_WARNING_RESERVED_IDENTIFIER         /* currently no-op */
#define TINT_DISABLE_WARNING_RESERVED_MACRO_IDENTIFIER   /* currently no-op */
#define TINT_DISABLE_WARNING_SHADOW_FIELD_IN_CONSTRUCTOR /* currently no-op */
#define TINT_DISABLE_WARNING_SIGN_CONVERSION             /* currently no-op */
#define TINT_DISABLE_WARNING_UNDEFINED_REINTERPRET_CAST  /* currently no-op */
#define TINT_DISABLE_WARNING_UNREACHABLE_CODE __pragma(warning(disable : 4702))
#define TINT_DISABLE_WARNING_UNSAFE_BUFFER_USAGE /* currently no-op */
#define TINT_DISABLE_WARNING_UNUSED_PARAMETER __pragma(warning(disable : 4100))
#define TINT_DISABLE_WARNING_UNSUED_VARIABLE __pragma(warning(disable : 4189))
#define TINT_DISABLE_WARNING_UNUSED_VALUE    /* currently no-op */
#define TINT_DISABLE_WARNING_WEAK_VTABLES    /* currently no-op */
#define TINT_DISABLE_WARNING_ZERO_AS_NULLPTR /* currently no-op */

#define TINT_BEGIN_DISABLE_OPTIMIZATIONS() __pragma(optimize("", off)) TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_OPTIMIZATIONS() __pragma(optimize("", on)) TINT_REQUIRE_SEMICOLON

#define TINT_BEGIN_DISABLE_ALL_WARNINGS() __pragma(warning(push, 0)) TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_ALL_WARNINGS() __pragma(warning(pop)) TINT_REQUIRE_SEMICOLON

// clang-format off
#define TINT_BEGIN_DISABLE_WARNING(name)     \
    __pragma(warning(push))                  \
    TINT_CONCAT(TINT_DISABLE_WARNING_, name) \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name)       \
    __pragma(warning(pop))                   \
    TINT_REQUIRE_SEMICOLON

#define TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS()      \
    __pragma(warning(push))                         \
    TINT_DISABLE_WARNING_UNUSED_PARAMETER           \
    TINT_DISABLE_WARNING_UNSUED_VARIABLE            \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_PROTOBUF_WARNINGS() \
    __pragma(warning(pop))                   \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#elif defined(__clang__)
////////////////////////////////////////////////////////////////////////////////
// Clang
////////////////////////////////////////////////////////////////////////////////
#define TINT_BUILD_IS_CLANG 1
#define TINT_DISABLE_WARNING_CONSTANT_OVERFLOW        /* currently no-op */
#define TINT_DISABLE_WARNING_DEPRECATED               /* currently no-op */
#define TINT_DISABLE_WARNING_DESTRUCTOR_NEVER_RETURNS /* currently no-op */
#define TINT_DISABLE_WARNING_COVERED_SWITCH_DEFAULT \
    _Pragma("clang diagnostic ignored \"-Wcovered-switch-default\"")
#define TINT_DISABLE_WARNING_DEPRECATED_REDUNDANT_CONSTEXPR_STATIC_DEF \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-redundant-constexpr-static-def\"")
#define TINT_DISABLE_WARNING_DOUBLE_PROMOTION \
    _Pragma("clang diagnostic ignored \"-Wdouble-promotion\"")
#define TINT_DISABLE_WARNING_EXTRA_SEMICOLON \
    _Pragma("clang diagnostic ignored \"-Wextra-semi-stmt\"")
#define TINT_DISABLE_WARNING_FLOAT_EQUAL _Pragma("clang diagnostic ignored \"-Wfloat-equal\"")
#define TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED \
    _Pragma("clang diagnostic ignored \"-Wconditional-uninitialized\"")
#define TINT_DISABLE_WARNING_MISSING_DESTRUCTOR_OVERRIDE                  \
    _Pragma("clang diagnostic ignored \"-Wsuggest-destructor-override\"") \
        _Pragma("clang diagnostic ignored \"-Winconsistent-missing-destructor-override\"")
#define TINT_DISABLE_WARNING_NEWLINE_EOF _Pragma("clang diagnostic ignored \"-Wnewline-eof\"")
#define TINT_DISABLE_WARNING_OLD_STYLE_CAST _Pragma("clang diagnostic ignored \"-Wold-style-cast\"")
#define TINT_DISABLE_WARNING_PEDANTIC /* currently no-op */
#define TINT_DISABLE_WARNING_REDUNDANT_PARENS \
    _Pragma("clang diagnostic ignored \"-Wredundant-parens\"")
#define TINT_DISABLE_WARNING_RESERVED_IDENTIFIER \
    _Pragma("clang diagnostic ignored \"-Wreserved-identifier\"")
#define TINT_DISABLE_WARNING_RESERVED_MACRO_IDENTIFIER                  \
    _Pragma("clang diagnostic ignored \"-Wreserved-macro-identifier\"") \
        _Pragma("clang diagnostic ignored \"-Wreserved-id-macro\"")
#define TINT_DISABLE_WARNING_SHADOW_FIELD_IN_CONSTRUCTOR \
    _Pragma("clang diagnostic ignored \"-Wshadow-field-in-constructor\"")
#define TINT_DISABLE_WARNING_SIGN_CONVERSION \
    _Pragma("clang diagnostic ignored \"-Wsign-conversion\"")
#define TINT_DISABLE_WARNING_THREAD_SAFETY_NEGATIVE \
    _Pragma("clang diagnostic ignored \"-Wthread-safety-negative\"")
#define TINT_DISABLE_WARNING_UNDEFINED_REINTERPRET_CAST \
    _Pragma("clang diagnostic ignored \"-Wundefined-reinterpret-cast\"")
#define TINT_DISABLE_WARNING_UNREACHABLE_CODE /* currently no-op */
#define TINT_DISABLE_WARNING_UNSAFE_BUFFER_USAGE \
    _Pragma("clang diagnostic ignored \"-Wunsafe-buffer-usage\"")
#define TINT_DISABLE_WARNING_UNUSED_PARAMETER \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"")
#define TINT_DISABLE_WARNING_UNUSED_VALUE _Pragma("clang diagnostic ignored \"-Wunused-value\"")
#define TINT_DISABLE_WARNING_WEAK_VTABLES _Pragma("clang diagnostic ignored \"-Wweak-vtables\"")
#define TINT_DISABLE_WARNING_ZERO_AS_NULLPTR \
    _Pragma("clang diagnostic ignored \"-Wzero-as-null-pointer-constant\"")

// clang-format off
#define TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS()                                 \
    _Pragma("clang diagnostic push")                                           \
    TINT_DISABLE_WARNING_COVERED_SWITCH_DEFAULT                                \
    TINT_DISABLE_WARNING_DEPRECATED_REDUNDANT_CONSTEXPR_STATIC_DEF             \
    TINT_DISABLE_WARNING_DOUBLE_PROMOTION                                      \
    TINT_DISABLE_WARNING_EXTRA_SEMICOLON                                       \
    TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED                                   \
    TINT_DISABLE_WARNING_MISSING_DESTRUCTOR_OVERRIDE                           \
    TINT_DISABLE_WARNING_PEDANTIC                                              \
    TINT_DISABLE_WARNING_REDUNDANT_PARENS                                      \
    TINT_DISABLE_WARNING_RESERVED_IDENTIFIER                                   \
    TINT_DISABLE_WARNING_RESERVED_MACRO_IDENTIFIER                             \
    TINT_DISABLE_WARNING_SHADOW_FIELD_IN_CONSTRUCTOR                           \
    TINT_DISABLE_WARNING_SIGN_CONVERSION                                       \
    TINT_DISABLE_WARNING_THREAD_SAFETY_NEGATIVE                                \
    TINT_DISABLE_WARNING_UNDEFINED_REINTERPRET_CAST                            \
    TINT_DISABLE_WARNING_UNUSED_PARAMETER                                      \
    TINT_DISABLE_WARNING_UNSAFE_BUFFER_USAGE                                   \
    TINT_DISABLE_WARNING_WEAK_VTABLES                                          \
    TINT_DISABLE_WARNING_ZERO_AS_NULLPTR                                       \
    TINT_REQUIRE_SEMICOLON

#define TINT_END_DISABLE_PROTOBUF_WARNINGS() \
    _Pragma("clang diagnostic pop")          \
    TINT_REQUIRE_SEMICOLON

#define TINT_BEGIN_DISABLE_OPTIMIZATIONS() /* currently no-op */ TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_OPTIMIZATIONS() /* currently no-op */ TINT_REQUIRE_SEMICOLON

#define TINT_BEGIN_DISABLE_ALL_WARNINGS() \
    _Pragma("clang diagnostic push")      \
    _Pragma("clang diagnostic ignored \"-Weverything\"")       \
    TINT_DISABLE_WARNING_UNSAFE_BUFFER_USAGE \
    TINT_REQUIRE_SEMICOLON

#define TINT_END_DISABLE_ALL_WARNINGS() \
    _Pragma("clang diagnostic pop")     \
    TINT_REQUIRE_SEMICOLON

#define TINT_BEGIN_DISABLE_WARNING(name)     \
    _Pragma("clang diagnostic push")         \
    TINT_CONCAT(TINT_DISABLE_WARNING_, name) \
    TINT_REQUIRE_SEMICOLON

#define TINT_END_DISABLE_WARNING(name)       \
    _Pragma("clang diagnostic pop")          \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#elif defined(__GNUC__)
////////////////////////////////////////////////////////////////////////////////
// GCC
////////////////////////////////////////////////////////////////////////////////
#define TINT_BUILD_IS_GCC 1
#define TINT_DISABLE_WARNING_CONSTANT_OVERFLOW        /* currently no-op */
#define TINT_DISABLE_WARNING_DEPRECATED               /* currently no-op */
#define TINT_DISABLE_WARNING_DESTRUCTOR_NEVER_RETURNS /* currently no-op */
#define TINT_DISABLE_WARNING_EXTRA_SEMICOLON          /* currently no-op */
#define TINT_DISABLE_WARNING_FLOAT_EQUAL              /* currently no-op */
#define TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED \
    _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#define TINT_DISABLE_WARNING_MISSING_DESTRUCTOR_OVERRIDE /* currently no-op */
#define TINT_DISABLE_WARNING_NEWLINE_EOF                 /* currently no-op */
#define TINT_DISABLE_WARNING_OLD_STYLE_CAST              /* currently no-op */
#define TINT_DISABLE_WARNING_PEDANTIC _Pragma("GCC diagnostic ignored \"-Wpedantic\"")
#define TINT_DISABLE_WARNING_REDUNDANT_PARENS            /* currently no-op */
#define TINT_DISABLE_WARNING_RESERVED_IDENTIFIER         /* currently no-op */
#define TINT_DISABLE_WARNING_RESERVED_MACRO_IDENTIFIER   /* currently no-op */
#define TINT_DISABLE_WARNING_SHADOW_FIELD_IN_CONSTRUCTOR /* currently no-op */
#define TINT_DISABLE_WARNING_SIGN_CONVERSION             /* currently no-op */
#define TINT_DISABLE_WARNING_UNDEFINED_REINTERPRET_CAST  /* currently no-op */
#define TINT_DISABLE_WARNING_UNREACHABLE_CODE            /* currently no-op */
#define TINT_DISABLE_WARNING_UNSAFE_BUFFER_USAGE         /* currently no-op */
#define TINT_DISABLE_WARNING_UNUSED_PARAMETER \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")
#define TINT_DISABLE_WARNING_UNUSED_VALUE _Pragma("GCC diagnostic ignored \"-Wunused-value\"")
#define TINT_DISABLE_WARNING_WEAK_VTABLES    /* currently no-op */
#define TINT_DISABLE_WARNING_ZERO_AS_NULLPTR /* currently no-op */

// clang-format off
#define TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS()  \
    _Pragma("GCC diagnostic push")              \
        TINT_DISABLE_WARNING_UNUSED_PARAMETER   \
        TINT_DISABLE_WARNING_PEDANTIC           \
        TINT_REQUIRE_SEMICOLON

#define TINT_END_DISABLE_PROTOBUF_WARNINGS()    \
    _Pragma("GCC diagnostic pop")               \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#define TINT_BEGIN_DISABLE_OPTIMIZATIONS() /* currently no-op */ TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_OPTIMIZATIONS() /* currently no-op */ TINT_REQUIRE_SEMICOLON

// clang-format off
#define TINT_BEGIN_DISABLE_ALL_WARNINGS()             \
    _Pragma("GCC diagnostic push")                    \
    TINT_DISABLE_WARNING_CONSTANT_OVERFLOW            \
    TINT_DISABLE_WARNING_MAYBE_UNINITIALIZED          \
    TINT_DISABLE_WARNING_NEWLINE_EOF                  \
    TINT_DISABLE_WARNING_OLD_STYLE_CAST               \
    TINT_DISABLE_WARNING_PEDANTIC                     \
    TINT_DISABLE_WARNING_SIGN_CONVERSION              \
    TINT_DISABLE_WARNING_UNREACHABLE_CODE             \
    TINT_DISABLE_WARNING_WEAK_VTABLES                 \
    TINT_DISABLE_WARNING_FLOAT_EQUAL                  \
    TINT_DISABLE_WARNING_DEPRECATED                   \
    TINT_DISABLE_WARNING_REDUNDANT_PARENS             \
    TINT_DISABLE_WARNING_RESERVED_IDENTIFIER          \
    TINT_DISABLE_WARNING_RESERVED_MACRO_IDENTIFIER    \
    TINT_DISABLE_WARNING_UNUSED_VALUE                 \
    TINT_DISABLE_WARNING_UNUSED_PARAMETER             \
    TINT_DISABLE_WARNING_SHADOW_FIELD_IN_CONSTRUCTOR  \
    TINT_DISABLE_WARNING_EXTRA_SEMICOLON              \
    TINT_DISABLE_WARNING_ZERO_AS_NULLPTR              \
    TINT_DISABLE_WARNING_MISSING_DESTRUCTOR_OVERRIDE  \
    TINT_DISABLE_WARNING_UNSAFE_BUFFER_USAGE          \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#define TINT_END_DISABLE_ALL_WARNINGS() _Pragma("GCC diagnostic pop") TINT_REQUIRE_SEMICOLON

// clang-format off
#define TINT_BEGIN_DISABLE_WARNING(name)     \
    _Pragma("GCC diagnostic push")           \
    TINT_CONCAT(TINT_DISABLE_WARNING_, name) \
    TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name)       \
    _Pragma("GCC diagnostic pop")            \
    TINT_REQUIRE_SEMICOLON
// clang-format on

#else
////////////////////////////////////////////////////////////////////////////////
// Other
////////////////////////////////////////////////////////////////////////////////
#define TINT_BEGIN_DISABLE_OPTIMIZATIONS() TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_OPTIMIZATIONS() TINT_REQUIRE_SEMICOLON
#define TINT_BEGIN_DISABLE_ALL_WARNINGS() TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_ALL_WARNINGS TINT_REQUIRE_SEMICOLON
#define TINT_BEGIN_DISABLE_WARNING(name) TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_WARNING(name) TINT_REQUIRE_SEMICOLON
#define TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS() TINT_REQUIRE_SEMICOLON
#define TINT_END_DISABLE_PROTOBUF_WARNINGS() TINT_REQUIRE_SEMICOLON

#endif

#ifndef TINT_BUILD_IS_MSVC
#define TINT_BUILD_IS_MSVC 0
#endif

#ifndef TINT_BUILD_IS_CLANG
#define TINT_BUILD_IS_CLANG 0
#endif

#ifndef TINT_BUILD_IS_GCC
#define TINT_BUILD_IS_GCC 0
#endif

#if TINT_BUILD_IS_MSVC
#define TINT_MSVC_ONLY(x) x
#else
#define TINT_MSVC_ONLY(x)
#endif

#if TINT_BUILD_IS_CLANG
#define TINT_CLANG_ONLY(x) x
#else
#define TINT_CLANG_ONLY(x)
#endif

#if TINT_BUILD_IS_GCC
#define TINT_GCC_ONLY(x) x
#else
#define TINT_GCC_ONLY(x)
#endif

#endif  // SRC_TINT_UTILS_MACROS_COMPILER_H_
