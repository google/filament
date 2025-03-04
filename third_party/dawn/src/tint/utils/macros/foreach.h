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

#ifndef SRC_TINT_UTILS_MACROS_FOREACH_H_
#define SRC_TINT_UTILS_MACROS_FOREACH_H_

// Macro magic to perform macro variadic dispatch.
// See:
// https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/preprocessor/macros/__VA_ARGS__/count-arguments
// Note, this doesn't attempt to use the ##__VA_ARGS__ trick to handle 0

// Helper macro to force expanding __VA_ARGS__ to satisfy MSVC compiler.
#define TINT_MSVC_EXPAND_BUG(X) X

/// TINT_COUNT_ARGUMENTS_NTH_ARG is used by TINT_COUNT_ARGUMENTS to get the number of arguments in a
/// variadic macro call.
#define TINT_COUNT_ARGUMENTS_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, \
                                     _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,  \
                                     _27, _28, _29, _30, _31, _32, N, ...)                        \
    N

/// TINT_COUNT_ARGUMENTS evaluates to the number of arguments passed to the macro
#define TINT_COUNT_ARGUMENTS(...)                                                                \
    TINT_MSVC_EXPAND_BUG(TINT_COUNT_ARGUMENTS_NTH_ARG(                                           \
        __VA_ARGS__, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, \
        13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

// Correctness checks.
static_assert(1 == TINT_COUNT_ARGUMENTS(a), "TINT_COUNT_ARGUMENTS broken");
static_assert(2 == TINT_COUNT_ARGUMENTS(a, b), "TINT_COUNT_ARGUMENTS broken");
static_assert(3 == TINT_COUNT_ARGUMENTS(a, b, c), "TINT_COUNT_ARGUMENTS broken");

/// TINT_FOREACH calls CB with each of the variadic arguments.
#define TINT_FOREACH(CB, ...) \
    TINT_MSVC_EXPAND_BUG(     \
        TINT_CONCAT(TINT_FOREACH_, TINT_COUNT_ARGUMENTS(__VA_ARGS__))(CB, __VA_ARGS__))

#define TINT_FOREACH_1(CB, _1) CB(_1)
#define TINT_FOREACH_2(CB, _1, _2) \
    TINT_FOREACH_1(CB, _1)         \
    CB(_2)
#define TINT_FOREACH_3(CB, _1, _2, _3) \
    TINT_FOREACH_2(CB, _1, _2)         \
    CB(_3)
#define TINT_FOREACH_4(CB, _1, _2, _3, _4) \
    TINT_FOREACH_3(CB, _1, _2, _3)         \
    CB(_4)
#define TINT_FOREACH_5(CB, _1, _2, _3, _4, _5) \
    TINT_FOREACH_4(CB, _1, _2, _3, _4)         \
    CB(_5)
#define TINT_FOREACH_6(CB, _1, _2, _3, _4, _5, _6) \
    TINT_FOREACH_5(CB, _1, _2, _3, _4, _5)         \
    CB(_6)
#define TINT_FOREACH_7(CB, _1, _2, _3, _4, _5, _6, _7) \
    TINT_FOREACH_6(CB, _1, _2, _3, _4, _5, _6)         \
    CB(_7)
#define TINT_FOREACH_8(CB, _1, _2, _3, _4, _5, _6, _7, _8) \
    TINT_FOREACH_7(CB, _1, _2, _3, _4, _5, _6, _7)         \
    CB(_8)
#define TINT_FOREACH_9(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    TINT_FOREACH_8(CB, _1, _2, _3, _4, _5, _6, _7, _8)         \
    CB(_9)
#define TINT_FOREACH_10(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    TINT_FOREACH_9(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9)           \
    CB(_10)
#define TINT_FOREACH_11(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11) \
    TINT_FOREACH_10(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10)          \
    CB(_11)
#define TINT_FOREACH_12(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12) \
    TINT_FOREACH_11(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)          \
    CB(_12)
#define TINT_FOREACH_13(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13) \
    TINT_FOREACH_12(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)          \
    CB(_13)
#define TINT_FOREACH_14(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14) \
    TINT_FOREACH_13(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)          \
    CB(_14)
#define TINT_FOREACH_15(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15) \
    TINT_FOREACH_14(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)          \
    CB(_15)
#define TINT_FOREACH_16(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
    TINT_FOREACH_15(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)          \
    CB(_16)
#define TINT_FOREACH_17(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17)                                                                       \
    TINT_FOREACH_16(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)     \
    CB(_17)
#define TINT_FOREACH_18(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18)                                                                  \
    TINT_FOREACH_17(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17)                                                                           \
    CB(_18)
#define TINT_FOREACH_19(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19)                                                             \
    TINT_FOREACH_18(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18)                                                                      \
    CB(_19)
#define TINT_FOREACH_20(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20)                                                        \
    TINT_FOREACH_19(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19)                                                                 \
    CB(_20)
#define TINT_FOREACH_21(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21)                                                   \
    TINT_FOREACH_20(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20)                                                            \
    CB(_21)
#define TINT_FOREACH_22(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22)                                              \
    TINT_FOREACH_21(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21)                                                       \
    CB(_22)
#define TINT_FOREACH_23(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23)                                         \
    TINT_FOREACH_22(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22)                                                  \
    CB(_23)
#define TINT_FOREACH_24(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24)                                    \
    TINT_FOREACH_23(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23)                                             \
    CB(_24)
#define TINT_FOREACH_25(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25)                               \
    TINT_FOREACH_24(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24)                                        \
    CB(_25)
#define TINT_FOREACH_26(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26)                          \
    TINT_FOREACH_25(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24, _25)                                   \
    CB(_26)
#define TINT_FOREACH_27(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27)                     \
    TINT_FOREACH_26(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26)                              \
    CB(_27)
#define TINT_FOREACH_28(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28)                \
    TINT_FOREACH_27(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27)                         \
    CB(_28)
#define TINT_FOREACH_29(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29)           \
    TINT_FOREACH_28(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28)                    \
    CB(_29)
#define TINT_FOREACH_30(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)      \
    TINT_FOREACH_29(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29)               \
    CB(_30)
#define TINT_FOREACH_31(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31) \
    TINT_FOREACH_30(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)          \
    CB(_31)
#define TINT_FOREACH_32(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, \
                        _32)                                                                       \
    TINT_FOREACH_31(CB, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16,     \
                    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)     \
    CB(_32)

#endif  // SRC_TINT_UTILS_MACROS_FOREACH_H_
