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

#if defined(__powerpc64__)

#define MARL_BUILD_ASM 1
#include "osfiber_asm_ppc64.h"

// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// r3: from
// r4: to
.text
.global marl_fiber_swap
.align 4
#if !defined(_CALL_ELF) || (_CALL_ELF != 2)
.global .marl_fiber_swap
.pushsection ".opd","aw"
marl_fiber_swap:
.quad .marl_fiber_swap
.quad .TOC.@tocbase
.quad 0
.popsection
.type .marl_fiber_swap,@function
.marl_fiber_swap:
#else
.type marl_fiber_swap @function
marl_fiber_swap:
#endif

    // Store non-volatile registers
    std 1, MARL_REG_R1(3)
    std 2, MARL_REG_R2(3)
    std 13, MARL_REG_R13(3)
    std 14, MARL_REG_R14(3)
    std 15, MARL_REG_R15(3)
    std 16, MARL_REG_R16(3)
    std 17, MARL_REG_R17(3)
    std 18, MARL_REG_R18(3)
    std 19, MARL_REG_R19(3)
    std 20, MARL_REG_R20(3)
    std 21, MARL_REG_R21(3)
    std 22, MARL_REG_R22(3)
    std 23, MARL_REG_R23(3)
    std 24, MARL_REG_R24(3)
    std 25, MARL_REG_R25(3)
    std 26, MARL_REG_R26(3)
    std 27, MARL_REG_R27(3)
    std 28, MARL_REG_R28(3)
    std 29, MARL_REG_R29(3)
    std 30, MARL_REG_R30(3)
    std 31, MARL_REG_R31(3)

    // Store special registers
    mflr 5
    std 5, MARL_REG_LR(3)
    mfcr 5
    std 5, MARL_REG_CCR(3)

    // Store non-volatile floating point registers
    stfd 14, MARL_REG_FPR14(3)
    stfd 15, MARL_REG_FPR15(3)
    stfd 16, MARL_REG_FPR16(3)
    stfd 17, MARL_REG_FPR17(3)
    stfd 18, MARL_REG_FPR18(3)
    stfd 19, MARL_REG_FPR19(3)
    stfd 20, MARL_REG_FPR20(3)
    stfd 21, MARL_REG_FPR21(3)
    stfd 22, MARL_REG_FPR22(3)
    stfd 23, MARL_REG_FPR23(3)
    stfd 24, MARL_REG_FPR24(3)
    stfd 25, MARL_REG_FPR25(3)
    stfd 26, MARL_REG_FPR26(3)
    stfd 27, MARL_REG_FPR27(3)
    stfd 28, MARL_REG_FPR28(3)
    stfd 29, MARL_REG_FPR29(3)
    stfd 30, MARL_REG_FPR30(3)
    stfd 31, MARL_REG_FPR31(3)

    // Store non-volatile altivec registers
#ifdef __ALTIVEC__
    li 5, MARL_REG_VMX
    stvxl 20, 3, 5
    addi 5, 5, 16
    stvxl 21, 3, 5
    addi 5, 5, 16
    stvxl 22, 3, 5
    addi 5, 5, 16
    stvxl 23, 3, 5
    addi 5, 5, 16
    stvxl 24, 3, 5
    addi 5, 5, 16
    stvxl 25, 3, 5
    addi 5, 5, 16
    stvxl 26, 3, 5
    addi 5, 5, 16
    stvxl 27, 3, 5
    addi 5, 5, 16
    stvxl 28, 3, 5
    addi 5, 5, 16
    stvxl 29, 3, 5
    addi 5, 5, 16
    stvxl 30, 3, 5
    addi 5, 5, 16
    stvxl 31, 3, 5

    mfvrsave 5
    stw 5, MARL_REG_VRSAVE(3)
#endif // __ALTIVEC__

    // Load non-volatile registers
    ld 1, MARL_REG_R1(4)
    ld 2, MARL_REG_R2(4)
    ld 13, MARL_REG_R13(4)
    ld 14, MARL_REG_R14(4)
    ld 15, MARL_REG_R15(4)
    ld 16, MARL_REG_R16(4)
    ld 17, MARL_REG_R17(4)
    ld 18, MARL_REG_R18(4)
    ld 19, MARL_REG_R19(4)
    ld 20, MARL_REG_R20(4)
    ld 21, MARL_REG_R21(4)
    ld 22, MARL_REG_R22(4)
    ld 23, MARL_REG_R23(4)
    ld 24, MARL_REG_R24(4)
    ld 25, MARL_REG_R25(4)
    ld 26, MARL_REG_R26(4)
    ld 27, MARL_REG_R27(4)
    ld 28, MARL_REG_R28(4)
    ld 29, MARL_REG_R29(4)
    ld 30, MARL_REG_R30(4)
    ld 31, MARL_REG_R31(4)

    // Load non-volatile floating point registers
    lfd 14, MARL_REG_FPR14(4)
    lfd 15, MARL_REG_FPR15(4)
    lfd 16, MARL_REG_FPR16(4)
    lfd 17, MARL_REG_FPR17(4)
    lfd 18, MARL_REG_FPR18(4)
    lfd 19, MARL_REG_FPR19(4)
    lfd 20, MARL_REG_FPR20(4)
    lfd 21, MARL_REG_FPR21(4)
    lfd 22, MARL_REG_FPR22(4)
    lfd 23, MARL_REG_FPR23(4)
    lfd 24, MARL_REG_FPR24(4)
    lfd 25, MARL_REG_FPR25(4)
    lfd 26, MARL_REG_FPR26(4)
    lfd 27, MARL_REG_FPR27(4)
    lfd 28, MARL_REG_FPR28(4)
    lfd 29, MARL_REG_FPR29(4)
    lfd 30, MARL_REG_FPR30(4)
    lfd 31, MARL_REG_FPR31(4)

    // Load non-volatile altivec registers
#ifdef __ALTIVEC__
    li 5, MARL_REG_VMX
    lvxl 20, 4, 5
    addi 5, 5, 16
    lvxl 21, 4, 5
    addi 5, 5, 16
    lvxl 22, 4, 5
    addi 5, 5, 16
    lvxl 23, 4, 5
    addi 5, 5, 16
    lvxl 24, 4, 5
    addi 5, 5, 16
    lvxl 25, 4, 5
    addi 5, 5, 16
    lvxl 26, 4, 5
    addi 5, 5, 16
    lvxl 27, 4, 5
    addi 5, 5, 16
    lvxl 28, 4, 5
    addi 5, 5, 16
    lvxl 29, 4, 5
    addi 5, 5, 16
    lvxl 30, 4, 5
    addi 5, 5, 16
    lvxl 31, 4, 5

    lwz 5, MARL_REG_VRSAVE(4)
    mtvrsave 5
#endif // __ALTIVEC__

    // Load parameters and entrypoint
    ld 12, MARL_REG_LR(4)
    ld 3, MARL_REG_R3(4)
    ld 4, MARL_REG_R4(4)
    mtlr 12

    // Branch to entrypoint
    blr

#endif // defined(__powerpc64__)
