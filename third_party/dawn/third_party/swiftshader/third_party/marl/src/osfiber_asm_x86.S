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

#if defined(__i386__)

#define MARL_BUILD_ASM 1
#include "osfiber_asm_x86.h"

// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// esp+4: from
// esp+8: to
.text
.global marl_fiber_swap
.align 4
marl_fiber_swap:
    // Save context 'from'
    movl        4(%esp), %eax

    // Store callee-preserved registers
    movl        %ebx, MARL_REG_EBX(%eax)
    movl        %ebp, MARL_REG_EBP(%eax)
    movl        %esi, MARL_REG_ESI(%eax)
    movl        %edi, MARL_REG_EDI(%eax)

    movl        (%esp), %ecx             /* call stores the return address on the stack before jumping */
    movl        %ecx, MARL_REG_EIP(%eax)
    lea         4(%esp), %ecx            /* skip the pushed return address */
    movl        %ecx, MARL_REG_ESP(%eax)

    // Load context 'to'
    movl        8(%esp), %ecx

    // Load callee-preserved registers
    movl        MARL_REG_EBX(%ecx), %ebx
    movl        MARL_REG_EBP(%ecx), %ebp
    movl        MARL_REG_ESI(%ecx), %esi
    movl        MARL_REG_EDI(%ecx), %edi

    // Load stack pointer
    movl        MARL_REG_ESP(%ecx), %esp

    // Load instruction pointer, and jump
    movl        MARL_REG_EIP(%ecx), %ecx
    jmp         *%ecx

#endif // defined(__i386__)
