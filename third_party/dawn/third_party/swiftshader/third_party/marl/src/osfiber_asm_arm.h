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
#define MARL_REG_r1 0x04
#define MARL_REG_r12 0x08
#define MARL_REG_r4 0x0c
#define MARL_REG_r5 0x10
#define MARL_REG_r6 0x14
#define MARL_REG_r7 0x18
#define MARL_REG_r8 0x1c
#define MARL_REG_r9 0x20
#define MARL_REG_r10 0x24
#define MARL_REG_r11 0x28
#define MARL_REG_v8 0x2c
#define MARL_REG_v9 0x30
#define MARL_REG_v10 0x34
#define MARL_REG_v11 0x38
#define MARL_REG_v12 0x3c
#define MARL_REG_v13 0x40
#define MARL_REG_v14 0x44
#define MARL_REG_v15 0x48
#define MARL_REG_SP 0x4c
#define MARL_REG_LR 0x50

#ifndef MARL_BUILD_ASM
#include <stdint.h>

// Procedure Call Standard for the ARM 64-bit Architecture
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
struct marl_fiber_context {
  // parameter registers
  uintptr_t r0;
  uintptr_t r1;

  // special purpose registers
  uintptr_t r12;  // Intra-Procedure-call

  // callee-saved registers
  uintptr_t r4;
  uintptr_t r5;
  uintptr_t r6;
  uintptr_t r7;
  uintptr_t r8;
  uintptr_t r9;
  uintptr_t r10;
  uintptr_t r11;

  uintptr_t v8;
  uintptr_t v9;
  uintptr_t v10;
  uintptr_t v11;
  uintptr_t v12;
  uintptr_t v13;
  uintptr_t v14;
  uintptr_t v15;

  uintptr_t SP;  // stack pointer (r13)
  uintptr_t LR;  // link register (r14)
};

#ifdef __cplusplus
#include <cstddef>
static_assert(offsetof(marl_fiber_context, r0) == MARL_REG_r0,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r1) == MARL_REG_r1,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r12) == MARL_REG_r12,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r4) == MARL_REG_r4,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r5) == MARL_REG_r5,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r6) == MARL_REG_r6,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r7) == MARL_REG_r7,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r8) == MARL_REG_r8,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r9) == MARL_REG_r9,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r10) == MARL_REG_r10,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, r11) == MARL_REG_r11,
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
