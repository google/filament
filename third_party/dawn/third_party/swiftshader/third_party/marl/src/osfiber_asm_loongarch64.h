// Copyright 2022 The Marl Authors.
//
// Licensed under the Apache License. Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
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
#define MARL_REG_s8 0x50
#define MARL_REG_fs0 0x58
#define MARL_REG_fs1 0x60
#define MARL_REG_fs2 0x68
#define MARL_REG_fs3 0x70
#define MARL_REG_fs4 0x78
#define MARL_REG_fs5 0x80
#define MARL_REG_fs6 0x88
#define MARL_REG_fs7 0x90
#define MARL_REG_ra 0x98
#define MARL_REG_sp 0xa0
#define MARL_REG_fp 0xa8

#ifndef MARL_BUILD_ASM

#include <stdint.h>

// Procedure Call Standard for the LoongArch 64-bit Architecture
// https://loongson.github.io/LoongArch-Documentation/LoongArch-ELF-ABI-EN.html
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
  uintptr_t s8;

  uintptr_t fs0;
  uintptr_t fs1;
  uintptr_t fs2;
  uintptr_t fs3;
  uintptr_t fs4;
  uintptr_t fs5;
  uintptr_t fs6;
  uintptr_t fs7;

  uintptr_t ra;
  uintptr_t sp;
  uintptr_t fp;
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
static_assert(offsetof(marl_fiber_context, s8) == MARL_REG_s8,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs0) == MARL_REG_fs0,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs1) == MARL_REG_fs1,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs2) == MARL_REG_fs2,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs3) == MARL_REG_fs3,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs4) == MARL_REG_fs4,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs5) == MARL_REG_fs5,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs6) == MARL_REG_fs6,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fs7) == MARL_REG_fs7,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, ra) == MARL_REG_ra,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, sp) == MARL_REG_sp,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, fp) == MARL_REG_fp,
              "Bad register offset");
#endif // __cplusplus

#endif // MARL_BUILD_ASM
