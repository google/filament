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

#if defined(__arm__)

#define MARL_BUILD_ASM 1
#include "osfiber_asm_arm.h"

// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// x0: from
// x1: to
.text
.global marl_fiber_swap
.align 4
.type marl_fiber_swap, %function
marl_fiber_swap:

    // Save context 'from'
    // TODO: multiple registers can be stored in a single instruction with: stm rA, {rB-rC}

    // Store special purpose registers
    str r12, [r0, #MARL_REG_r12]

    // Store callee-preserved registers
    str r4, [r0, #MARL_REG_r4]
    str r5, [r0, #MARL_REG_r5]
    str r6, [r0, #MARL_REG_r6]
    str r7, [r0, #MARL_REG_r7]
    str r8, [r0, #MARL_REG_r8]
    str r9, [r0, #MARL_REG_r9]
    str r10, [r0, #MARL_REG_r10]
    str r11, [r0, #MARL_REG_r11]

    // Store sp, lr and pc
    str sp, [r0, #MARL_REG_SP]
    str lr, [r0, #MARL_REG_LR]

    // Load context 'to'
    // TODO: multiple registers can be loaded in a single instruction with: ldm rA, {rB-rC}
    mov r3, r1

    // Load special purpose registers
    ldr r12, [r3, #MARL_REG_r12]

    // Load callee-preserved registers
    ldr r4, [r3, #MARL_REG_r4]
    ldr r5, [r3, #MARL_REG_r5]
    ldr r6, [r3, #MARL_REG_r6]
    ldr r7, [r3, #MARL_REG_r7]
    ldr r8, [r3, #MARL_REG_r8]
    ldr r9, [r3, #MARL_REG_r9]
    ldr r10, [r3, #MARL_REG_r10]
    ldr r11, [r3, #MARL_REG_r11]

    // Load parameter registers
    ldr r0, [r3, #MARL_REG_r0]
    ldr r1, [r3, #MARL_REG_r1]

    // Load sp, lr and pc
    ldr sp, [r3, #MARL_REG_SP]
    ldr lr, [r3, #MARL_REG_LR]
    mov pc, lr

#endif // defined(__arm__)
