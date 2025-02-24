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

#if defined(__aarch64__)

#define MARL_BUILD_ASM 1
#include "osfiber_asm_aarch64.h"


#if defined(__ARM_FEATURE_PAC_DEFAULT) && __ARM_FEATURE_PAC_DEFAULT
// ENABLE_PAUTH must be defined to 1 since this value will be used in
// bitwise-shift later!
#define ENABLE_PAUTH 1

#if ((__ARM_FEATURE_PAC_DEFAULT & ((1 << 0) | (1 << 1))) == 0)
#error Pointer authentication defines no valid key!
#endif
#else
#define ENABLE_PAUTH 0
#endif

#if defined(__ARM_FEATURE_BTI_DEFAULT) && (__ARM_FEATURE_BTI_DEFAULT == 1)
// ENABLE_BTI must be defined to 1 since this value will be used in
// bitwise-shift later!
#define ENABLE_BTI 1
#else
#define ENABLE_BTI 0
#endif

// Although Pointer Authentication and Branch Target Instructions are
// technically seperate features they work together, i.e. the paciasp and
// pacibsp instructions serve as BTI landing pads. Therefore PA-instructions are
// enabled when PA _or_ BTI is enabled!
#if ENABLE_PAUTH || ENABLE_BTI
// See section "Pointer Authentication" of
// https://developer.arm.com/documentation/101028/0012/5--Feature-test-macros
// for details how to interpret __ARM_FEATURE_PAC_DEFAULT
#if (__ARM_FEATURE_PAC_DEFAULT & (1 << 0))
#define PAUTH_SIGN_SP paciasp
#define PAUTH_AUTH_SP autiasp
#else
#define PAUTH_SIGN_SP pacibsp
#define PAUTH_AUTH_SP autibsp
#endif
#else
#define PAUTH_SIGN_SP
#define PAUTH_AUTH_SP
#endif


// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// x0: from
// x1: to
.text
.global MARL_ASM_SYMBOL(marl_fiber_swap)
.align 4
MARL_ASM_SYMBOL(marl_fiber_swap):

    // Save context 'from'
    // TODO: pairs of str can be combined with stp.

    PAUTH_SIGN_SP

    // Store special purpose registers
    str x16, [x0, #MARL_REG_r16]
    str x17, [x0, #MARL_REG_r17]
    str x18, [x0, #MARL_REG_r18]

    // Store callee-preserved registers
    str x19, [x0, #MARL_REG_r19]
    str x20, [x0, #MARL_REG_r20]
    str x21, [x0, #MARL_REG_r21]
    str x22, [x0, #MARL_REG_r22]
    str x23, [x0, #MARL_REG_r23]
    str x24, [x0, #MARL_REG_r24]
    str x25, [x0, #MARL_REG_r25]
    str x26, [x0, #MARL_REG_r26]
    str x27, [x0, #MARL_REG_r27]
    str x28, [x0, #MARL_REG_r28]
    str x29, [x0, #MARL_REG_r29]

    str d8,  [x0, #MARL_REG_v8]
    str d9,  [x0, #MARL_REG_v9]
    str d10, [x0, #MARL_REG_v10]
    str d11, [x0, #MARL_REG_v11]
    str d12, [x0, #MARL_REG_v12]
    str d13, [x0, #MARL_REG_v13]
    str d14, [x0, #MARL_REG_v14]
    str d15, [x0, #MARL_REG_v15]

    // Store sp and lr
    mov x2, sp
    str x2,  [x0, #MARL_REG_SP]
    str x30, [x0, #MARL_REG_LR]

    // Load context 'to'
    mov x7, x1

    // Load special purpose registers
    ldr x16, [x7, #MARL_REG_r16]
    ldr x17, [x7, #MARL_REG_r17]
    ldr x18, [x7, #MARL_REG_r18]

    // Load callee-preserved registers
    ldr x19, [x7, #MARL_REG_r19]
    ldr x20, [x7, #MARL_REG_r20]
    ldr x21, [x7, #MARL_REG_r21]
    ldr x22, [x7, #MARL_REG_r22]
    ldr x23, [x7, #MARL_REG_r23]
    ldr x24, [x7, #MARL_REG_r24]
    ldr x25, [x7, #MARL_REG_r25]
    ldr x26, [x7, #MARL_REG_r26]
    ldr x27, [x7, #MARL_REG_r27]
    ldr x28, [x7, #MARL_REG_r28]
    ldr x29, [x7, #MARL_REG_r29]

    ldr d8,  [x7, #MARL_REG_v8]
    ldr d9,  [x7, #MARL_REG_v9]
    ldr d10, [x7, #MARL_REG_v10]
    ldr d11, [x7, #MARL_REG_v11]
    ldr d12, [x7, #MARL_REG_v12]
    ldr d13, [x7, #MARL_REG_v13]
    ldr d14, [x7, #MARL_REG_v14]
    ldr d15, [x7, #MARL_REG_v15]

    // Load parameter registers
    ldr x0, [x7, #MARL_REG_r0]
    ldr x1, [x7, #MARL_REG_r1]

    // Load sp and lr
    ldr x30, [x7, #MARL_REG_LR]
    ldr x2,  [x7, #MARL_REG_SP]
    mov sp, x2

    PAUTH_AUTH_SP

    ret

#if ENABLE_PAUTH || ENABLE_BTI
// see
// https://github.com/ARM-software/abi-aa/blob/main/aaelf64/aaelf64.rst#program-property
.pushsection .note.gnu.property, "a";
    .balign 8
    .long 4
    .long 0x10
    .long 0x5
    .asciz "GNU"
    .long 0xc0000000 /* GNU_PROPERTY_AARCH64_FEATURE_1_AND */
    .long 4
    .long ((ENABLE_PAUTH)<<1) | ((ENABLE_BTI)<<0) /* PAuth and BTI */
    .long 0
.popsection
#endif

#endif // defined(__aarch64__)
