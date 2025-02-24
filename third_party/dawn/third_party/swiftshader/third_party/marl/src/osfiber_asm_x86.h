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

#define MARL_REG_EBX 0x00
#define MARL_REG_EBP 0x04
#define MARL_REG_ESI 0x08
#define MARL_REG_EDI 0x0c
#define MARL_REG_ESP 0x10
#define MARL_REG_EIP 0x14

#ifndef MARL_BUILD_ASM
#include <stdint.h>

// Assumes cdecl calling convention.
// Registers EAX, ECX, and EDX are caller-saved, and the rest are callee-saved.
struct marl_fiber_context {
  // callee-saved registers
  uintptr_t EBX;
  uintptr_t EBP;
  uintptr_t ESI;
  uintptr_t EDI;

  // stack and instruction registers
  uintptr_t ESP;
  uintptr_t EIP;
};

#ifdef __cplusplus
#include <cstddef>
static_assert(offsetof(marl_fiber_context, EBX) == MARL_REG_EBX,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, EBP) == MARL_REG_EBP,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, ESI) == MARL_REG_ESI,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, EDI) == MARL_REG_EDI,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, ESP) == MARL_REG_ESP,
              "Bad register offset");
static_assert(offsetof(marl_fiber_context, EIP) == MARL_REG_EIP,
              "Bad register offset");
#endif  // __cplusplus

#endif  // MARL_BUILD_ASM
