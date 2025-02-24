// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#if defined(__x86_64__)

#define MARL_BUILD_ASM 1
#include "osfiber_asm_x64.h"

// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// rdi: from
// rsi: to
.text
.global MARL_ASM_SYMBOL(marl_fiber_swap)
.align 4
MARL_ASM_SYMBOL(marl_fiber_swap):

    // Save context 'from'

    // Store callee-preserved registers
    movq        %rbx, MARL_REG_RBX(%rdi)
    movq        %rbp, MARL_REG_RBP(%rdi)
    movq        %r12, MARL_REG_R12(%rdi)
    movq        %r13, MARL_REG_R13(%rdi)
    movq        %r14, MARL_REG_R14(%rdi)
    movq        %r15, MARL_REG_R15(%rdi)

    movq        (%rsp), %rcx             /* call stores the return address on the stack before jumping */
    movq        %rcx, MARL_REG_RIP(%rdi)
    leaq        8(%rsp), %rcx            /* skip the pushed return address */
    movq        %rcx, MARL_REG_RSP(%rdi)

    // Load context 'to'
    movq        %rsi, %r8

    // Load callee-preserved registers
    movq        MARL_REG_RBX(%r8), %rbx
    movq        MARL_REG_RBP(%r8), %rbp
    movq        MARL_REG_R12(%r8), %r12
    movq        MARL_REG_R13(%r8), %r13
    movq        MARL_REG_R14(%r8), %r14
    movq        MARL_REG_R15(%r8), %r15

    // Load first two call parameters
    movq        MARL_REG_RDI(%r8), %rdi
    movq        MARL_REG_RSI(%r8), %rsi

    // Load stack pointer
    movq        MARL_REG_RSP(%r8), %rsp

    // Load instruction pointer, and jump
    movq        MARL_REG_RIP(%r8), %rcx
    jmp         *%rcx

#endif // defined(__x86_64__)
