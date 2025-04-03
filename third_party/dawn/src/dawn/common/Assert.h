// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_ASSERT_H_
#define SRC_DAWN_COMMON_ASSERT_H_

#include <cstdlib>

#include "dawn/common/Compiler.h"

// Dawn asserts to be used instead of the regular C stdlib assert function (if you don't use assert
// yet, you should start now!). In debug DAWN_ASSERT(condition) will trigger an error, otherwise in
// release it does nothing at runtime.
//
// In case of name clashes (with for example a testing library), you can define the
// DAWN_SKIP_ASSERT_SHORTHANDS to only define the DAWN_ prefixed macros.
//
// These asserts feature:
//     - Logging of the error with file, line and function information.
//     - Breaking in the debugger when an assert is triggered and a debugger is attached.
//     - Use the assert information to help the compiler optimizer in release builds.

// MSVC triggers a warning in /W4 for do {} while(0). SDL worked around this by using (0,0) and
// points out that it looks like an owl face.
#if DAWN_COMPILER_IS(MSVC)
#define DAWN_ASSERT_LOOP_CONDITION (0, 0)
#else
#define DAWN_ASSERT_LOOP_CONDITION (0)
#endif

// DAWN_ASSERT_CALLSITE_HELPER generates the actual assert code. In Debug it does what you would
// expect of an assert and in release it tries to give hints to make the compiler generate better
// code.
#if defined(DAWN_ENABLE_ASSERTS)

#define DAWN_CHECK_CALLSITE_HELPER(file, func, line, condition)           \
    do {                                                                  \
        if (DAWN_UNLIKELY(!(condition))) {                                \
            ::dawn::HandleAssertionFailure(file, func, line, #condition); \
            abort();                                                      \
        }                                                                 \
    } while (DAWN_ASSERT_LOOP_CONDITION)

#define DAWN_ASSERT_CALLSITE_HELPER(file, func, line, condition)          \
    do {                                                                  \
        if (DAWN_UNLIKELY(!(condition))) {                                \
            ::dawn::HandleAssertionFailure(file, func, line, #condition); \
        }                                                                 \
    } while (DAWN_ASSERT_LOOP_CONDITION)

#else  // defined(DAWN_ENABLE_ASSERTS)

#define DAWN_CHECK_CALLSITE_HELPER(file, func, line, condition) \
    do {                                                        \
        if (DAWN_UNLIKELY(!(condition))) {                      \
            abort();                                            \
        }                                                       \
    } while (DAWN_ASSERT_LOOP_CONDITION)

#if DAWN_COMPILER_IS(MSVC)
#define DAWN_ASSERT_CALLSITE_HELPER(file, func, line, condition) __assume(condition)
#elif DAWN_COMPILER_IS(CLANG) && __has_builtin(__builtin_assume)
#define DAWN_ASSERT_CALLSITE_HELPER(file, func, line, condition) __builtin_assume(!!(condition))
#else  // DAWN_COMPILER_IS(*)
#define DAWN_ASSERT_CALLSITE_HELPER(file, func, line, condition) \
    do {                                                         \
        [[maybe_unused]] auto unused = sizeof(condition);        \
    } while (DAWN_ASSERT_LOOP_CONDITION)
#endif  // DAWN_COMPILER_IS(*)

#endif  // defined(DAWN_ENABLE_ASSERTS)

// Debug-only assert (similar to Chromium DCHECK).
// In release, this provides optimization hints to the compiler.
#define DAWN_ASSERT(condition) DAWN_ASSERT_CALLSITE_HELPER(__FILE__, __func__, __LINE__, condition)
// Debug-only assert-false (similar to Chromium NOTREACHED).
// In release, this provides optimization hints to the compiler.
#define DAWN_UNREACHABLE()                                                 \
    do {                                                                   \
        DAWN_ASSERT(DAWN_ASSERT_LOOP_CONDITION && "Unreachable code hit"); \
        DAWN_BUILTIN_UNREACHABLE();                                        \
    } while (DAWN_ASSERT_LOOP_CONDITION)
// Release-mode assert (similar to Chromium DAWN_CHECK).
// First does a debug-mode assert for better a better debugging experience, then hard-aborts.
#define DAWN_CHECK(condition) DAWN_CHECK_CALLSITE_HELPER(__FILE__, __func__, __LINE__, condition)

namespace dawn {

void BreakPoint();
void HandleAssertionFailure(const char* file,
                            const char* function,
                            int line,
                            const char* condition);

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_ASSERT_H_
