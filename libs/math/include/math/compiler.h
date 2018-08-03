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
#else
#   define MATH_EMPTY_BASES
#endif
