// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_PREPROCESSOR_H_
#define SRC_DAWN_COMMON_PREPROCESSOR_H_

// DAWN_PP_GET_HEAD: get the first element of a __VA_ARGS__ without triggering empty
// __VA_ARGS__ warnings.
#define DAWN_INTERNAL_PP_GET_HEAD(firstParam, ...) firstParam
#define DAWN_PP_GET_HEAD(...) DAWN_INTERNAL_PP_GET_HEAD(__VA_ARGS__, placeholderArg)

// DAWN_PP_CONCATENATE: Concatenate tokens, first expanding the arguments passed in.
#define DAWN_PP_CONCATENATE(arg1, arg2) DAWN_PP_CONCATENATE_1(arg1, arg2)
#define DAWN_PP_CONCATENATE_1(arg1, arg2) DAWN_PP_CONCATENATE_2(arg1, arg2)
#define DAWN_PP_CONCATENATE_2(arg1, arg2) arg1##arg2

// DAWN_PP_EXPAND: Needed to help expand __VA_ARGS__ out on MSVC
#define DAWN_PP_EXPAND(...) __VA_ARGS__

// Implementation of DAWN_PP_FOR_EACH, called by concatenating DAWN_PP_FOR_EACH_ with a number.
#define DAWN_PP_FOR_EACH_1(func, x) func(x)
#define DAWN_PP_FOR_EACH_2(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_1)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_3(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_2)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_4(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_3)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_5(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_4)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_6(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_5)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_7(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_6)(func, __VA_ARGS__))
#define DAWN_PP_FOR_EACH_8(func, x, ...) \
    func(x) DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_7)(func, __VA_ARGS__))

// Implementation for DAWN_PP_FOR_EACH. Get the number of args in __VA_ARGS__ so we can concat
// DAWN_PP_FOR_EACH_ and N.
// ex.) DAWN_PP_FOR_EACH_NARG(a, b, c) ->
//      DAWN_PP_FOR_EACH_NARG(a, b, c, DAWN_PP_FOR_EACH_RSEQ()) ->
//      DAWN_PP_FOR_EACH_NARG_(a, b, c, 8, 7, 6, 5, 4, 3, 2, 1, 0) ->
//      DAWN_PP_FOR_EACH_ARG_N(a, b, c, 8, 7, 6, 5, 4, 3, 2, 1, 0) ->
//      DAWN_PP_FOR_EACH_ARG_N( ,  ,  ,  ,  ,  ,  , ,  N) ->
//      3
#define DAWN_PP_FOR_EACH_NARG(...) DAWN_PP_FOR_EACH_NARG_(__VA_ARGS__, DAWN_PP_FOR_EACH_RSEQ())
#define DAWN_PP_FOR_EACH_NARG_(...) \
    DAWN_PP_EXPAND(DAWN_PP_EXPAND(DAWN_PP_FOR_EACH_ARG_N)(__VA_ARGS__))
#define DAWN_PP_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define DAWN_PP_FOR_EACH_RSEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

// Implementation for DAWN_PP_FOR_EACH.
// Creates a call to DAWN_PP_FOR_EACH_X where X is 1, 2, ..., etc.
#define DAWN_PP_FOR_EACH_(N, func, ...) DAWN_PP_CONCATENATE(DAWN_PP_FOR_EACH_, N)(func, __VA_ARGS__)

// DAWN_PP_FOR_EACH: Apply |func| to each argument in |x| and __VA_ARGS__
#define DAWN_PP_FOR_EACH(func, ...) \
    DAWN_PP_FOR_EACH_(DAWN_PP_FOR_EACH_NARG(__VA_ARGS__), func, __VA_ARGS__)

#endif  // SRC_DAWN_COMMON_PREPROCESSOR_H_
