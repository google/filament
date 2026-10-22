// Copyright 2026 The Dawn & Tint Authors
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

#ifndef UTILS_FORCE_CRASH_H_
#define UTILS_FORCE_CRASH_H_

#include "src/utils/compiler.h"
#include "src/utils/platform.h"

// DAWN_FORCE_CRASH()
//
// Immediately causes the program to terminate as fast as possible, at an address that's inside the
// function that used the macro, and with a signal that's different from a segfault.
#if DAWN_COMPILER_IS(CLANG) || DAWN_COMPILER_IS(GCC)
#if DAWN_PLATFORM_IS(X86)
#define DAWN_DETAIL_TRAP() asm volatile("int $3\n\t");
#elif DAWN_PLATFORM_IS(ARM32)
#define DAWN_DETAIL_TRAP() asm volatile("bkpt 0");
#elif DAWN_PLATFORM_IS(ARM64)
#define DAWN_DETAIL_TRAP() asm volatile("brk 0xf000");
#elif DAWN_PLATFORM_IS(LOONGARCH)
#define DAWN_DETAIL_TRAP() asm volatile("break 0");
#elif DAWN_PLATFORM_IS(RISCV)
#define DAWN_DETAIL_TRAP() asm volatile("ebreak");
#elif DAWN_PLATFORM_IS(MIPS)
#define DAWN_DETAIL_TRAP() asm volatile("break");
#elif DAWN_PLATFORM_IS(S390) || DAWN_PLATFORM_IS(S390X)
#define DAWN_DETAIL_TRAP() asm volatile(".word 0x0001");
#elif DAWN_PLATFORM_IS(PPC) || DAWN_PLATFORM_IS(PPC64)
#define DAWN_DETAIL_TRAP() asm volatile("twge 2,2");
#elif DAWN_PLATFORM_IS(WASM32) || DAWN_PLATFORM_IS(WASM64)
#define DAWN_DETAIL_TRAP() EM_ASM(debugger;);
#else
#error "Unsupported platform"
#endif

#define DAWN_FORCE_CRASH()       \
    do {                         \
        DAWN_DETAIL_TRAP();      \
        __builtin_unreachable(); \
    } while (false)

#elif DAWN_COMPILER_IS(MSVC)
#define DAWN_FORCE_CRASH() __debugbreak()

#else
#error "Unsupported compiler"
#endif

#endif  // UTILS_FORCE_CRASH_H_
