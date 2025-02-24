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

#include "dawn/common/Assert.h"

#include <cstdlib>

#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"

#if DAWN_COMPILER_IS(MSVC)
extern void __cdecl __debugbreak(void);
#endif

namespace dawn {

#if DAWN_COMPILER_IS(CLANG) || DAWN_COMPILER_IS(GCC)
void BreakPoint() {
#if DAWN_PLATFORM_IS(X86)
    __asm__ __volatile__("int $3\n\t");
#elif DAWN_PLATFORM_IS(ARM32)
    __asm__ __volatile__("bkpt 0");
#elif DAWN_PLATFORM_IS(ARM64)
    __asm__ __volatile__("brk 0xf000");
#elif DAWN_PLATFORM_IS(LOONGARCH)
    __asm__ __volatile__("break 0");
#elif DAWN_PLATFORM_IS(RISCV)
    __asm__ __volatile__("ebreak");
#elif DAWN_PLATFORM_IS(MIPS)
    __asm__ __volatile__("break");
#elif DAWN_PLATFORM_IS(S390) || DAWN_PLATFORM_IS(S390X)
    __asm__ __volatile__(".word 0x0001");
#elif DAWN_PLATFORM_IS(PPC) || DAWN_PLATFORM_IS(PPC64)
    __asm__ __volatile__("twge 2,2");
#elif DAWN_PLATFORM_IS(WASM32) || DAWN_PLATFORM_IS(WASM64)
    EM_ASM(debugger;);
#else
#error "Unsupported platform"
#endif
}

#elif DAWN_COMPILER_IS(MSVC)
void BreakPoint() {
    __debugbreak();
}

#else
#error "Unsupported compiler"
#endif

void HandleAssertionFailure(const char* file,
                            const char* function,
                            int line,
                            const char* condition) {
    dawn::ErrorLog() << "Assertion failure at " << file << ":" << line << " (" << function
                     << "): " << condition;
#if defined(DAWN_ABORT_ON_ASSERT)
    abort();
#else
    BreakPoint();
#endif
}

}  // namespace dawn
