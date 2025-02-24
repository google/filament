// Copyright 2020 The Marl Authors.
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

#define MARL_REG_a0 0x00
#define MARL_REG_a1 0x08
#define MARL_REG_s0 0x10
#define MARL_REG_s1 0x18
#define MARL_REG_s2 0x20
#define MARL_REG_s3 0x28
#define MARL_REG_s4 0x30
#define MARL_REG_s5 0x38
#define MARL_REG_s6 0x40
#define MARL_REG_s7 0x48
#define MARL_REG_f24 0x50
#define MARL_REG_f25 0x58
#define MARL_REG_f26 0x60
#define MARL_REG_f27 0x68
#define MARL_REG_f28 0x70
#define MARL_REG_f29 0x78
#define MARL_REG_f30 0x80
#define MARL_REG_f31 0x88
#define MARL_REG_gp 0x90
#define MARL_REG_sp 0x98
#define MARL_REG_fp 0xa0
#define MARL_REG_ra 0xa8

#if defined(__APPLE__)
#define MARL_ASM_SYMBOL(x) _##x
#else
#define MARL_ASM_SYMBOL(x) x
#endif

#ifndef MARL_BUILD_ASM

#include <stdint.h>

struct marl_fiber_context {
  // parameter registers (First two)
  uintptr_t a0;
  uintptr_t a1;

  // callee-saved registers
  uintptr_t s0;
  uintptr_t s1;
  uintptr_t s2;
  uintptr_t s3;
  uintptr_t s4;
  uintptr_t s5;
  uintptr_t s6;
  uintptr_t s7;

  uintptr_t f24;
  uintptr_t f25;
  uintptr_t f26;
  uintptr_t f27;
  uintptr_t f28;
  uintptr_t f29;
  uintptr_t f30;
  uintptr_t f31;

  uintptr_t gp;
  uintptr_t sp;
  uintptr_t fp;
  uintptr_t ra;
};

#ifdef __cplusplus
#include <cstddef>
static_assert(offsetof(marl_fiber_context, a0) == MARL_REG_a0,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, a1) == MARL_REG_a1,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s0) == MARL_REG_s0,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s1) == MARL_REG_s1,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s2) == MARL_REG_s2,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s3) == MARL_REG_s3,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s4) == MARL_REG_s4,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s5) == MARL_REG_s5,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s6) == MARL_REG_s6,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, s7) == MARL_REG_s7,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f24) == MARL_REG_f24,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f25) == MARL_REG_f25,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f26) == MARL_REG_f26,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f27) == MARL_REG_f27,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f28) == MARL_REG_f28,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f29) == MARL_REG_f29,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f30) == MARL_REG_f30,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, f31) == MARL_REG_f31,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, gp) == MARL_REG_gp,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, sp) == MARL_REG_sp,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fp) == MARL_REG_fp,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, ra) == MARL_REG_ra,
              "Bad register offset");
#endif  // __cplusplus

#endif  // MARL_BUILD_ASM
