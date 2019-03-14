/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#if defined (WIN32)

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifdef far
#undef far
#endif

#ifdef near
#undef near
#endif

#endif

// compatibility with non-clang compilers...
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_expect)
#   ifdef __cplusplus
#      define MATH_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#      define MATH_UNLIKELY( exp )  (__builtin_expect( !!(exp), false ))
#   else
#      define MATH_LIKELY( exp )    (__builtin_expect( !!(exp), 1 ))
#      define MATH_UNLIKELY( exp )  (__builtin_expect( !!(exp), 0 ))
#   endif
#else
#   define MATH_LIKELY( exp )    (exp)
#   define MATH_UNLIKELY( exp )  (exp)
#endif

#if __has_attribute(pure)
#   define MATH_PURE __attribute__((pure))
#else
#   define MATH_PURE
#endif

#ifdef _MSC_VER
#   define MATH_EMPTY_BASES __declspec(empty_bases)
// MSVC does not support loop unrolling hints
#   define MATH_NOUNROLL

// Sadly, MSVC does not support __builtin_constant_p
#   ifndef MAKE_CONSTEXPR
#       define MAKE_CONSTEXPR(e) (e)
#   endif

#else // _MSC_VER

#   define MATH_EMPTY_BASES
// C++11 allows pragmas to be specified as part of defines using the _Pragma syntax.
#   define MATH_NOUNROLL _Pragma("nounroll")

#   ifndef MAKE_CONSTEXPR
#       define MAKE_CONSTEXPR(e) __builtin_constant_p(e) ? (e) : (e)
#   endif

#endif // _MSC_VER
