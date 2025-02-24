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

#define MARL_REG_r0 0x00
#define MARL_REG_r1 0x08
#define MARL_REG_r16 0x10
#define MARL_REG_r17 0x18
#define MARL_REG_r18 0x20
#define MARL_REG_r19 0x28
#define MARL_REG_r20 0x30
#define MARL_REG_r21 0x38
#define MARL_REG_r22 0x40
#define MARL_REG_r23 0x48
#define MARL_REG_r24 0x50
#define MARL_REG_r25 0x58
#define MARL_REG_r26 0x60
#define MARL_REG_r27 0x68
#define MARL_REG_r28 0x70
#define MARL_REG_r29 0x78
#define MARL_REG_v8 0x80
#define MARL_REG_v9 0x88
#define MARL_REG_v10 0x90
#define MARL_REG_v11 0x98
#define MARL_REG_v12 0xa0
#define MARL_REG_v13 0xa8
#define MARL_REG_v14 0xb0
#define MARL_REG_v15 0xb8
#define MARL_REG_SP 0xc0
#define MARL_REG_LR 0xc8

#if defined(__APPLE__)
#define MARL_ASM_SYMBOL(x) _##x
#else
#define MARL_ASM_SYMBOL(x) x
#endif

#ifndef MARL_BUILD_ASM

#include <stdint.h>

// Procedure Call Standard for the ARM 64-bit Architecture
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
struct marl_fiber_context {
  // parameter registers
  uintptr_t r0;
  uintptr_t r1;

  // special purpose registers
  uintptr_t r16;
  uintptr_t r17;
  uintptr_t r18;  // platform specific (maybe inter-procedural state)

  // callee-saved registers
  uintptr_t r19;
  uintptr_t r20;
  uintptr_t r21;
  uintptr_t r22;
  uintptr_t r23;
  uintptr_t r24;
  uintptr_t r25;
  uintptr_t r26;
  uintptr_t r27;
  uintptr_t r28;
  uintptr_t r29;

  uintptr_t v8;
  uintptr_t v9;
  uintptr_t v10;
  uintptr_t v11;
  uintptr_t v12;
  uintptr_t v13;
  uintptr_t v14;
  uintptr_t v15;

  uintptr_t SP;  // stack pointer
  uintptr_t LR;  // link register (R30)
};

#ifdef __cplusplus
#include <cstddef>
static_assert(offsetof(marl_fiber_context, r0) == MARL_REG_r0,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r1) == MARL_REG_r1,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r16) == MARL_REG_r16,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r17) == MARL_REG_r17,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r18) == MARL_REG_r18,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r19) == MARL_REG_r19,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r20) == MARL_REG_r20,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r21) == MARL_REG_r21,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r22) == MARL_REG_r22,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r23) == MARL_REG_r23,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r24) == MARL_REG_r24,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r25) == MARL_REG_r25,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r26) == MARL_REG_r26,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r27) == MARL_REG_r27,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r28) == MARL_REG_r28,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r29) == MARL_REG_r29,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v8) == MARL_REG_v8,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v9) == MARL_REG_v9,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v10) == MARL_REG_v10,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v11) == MARL_REG_v11,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v12) == MARL_REG_v12,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v13) == MARL_REG_v13,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v14) == MARL_REG_v14,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, v15) == MARL_REG_v15,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, SP) == MARL_REG_SP,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, LR) == MARL_REG_LR,
              "Bad register offset");
#endif  // __cplusplus

#endif  // MARL_BUILD_ASM
