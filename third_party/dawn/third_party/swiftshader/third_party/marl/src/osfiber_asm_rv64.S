// Copyright 2021 The Marl Authors.
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

#if defined(__riscv) && __riscv_xlen == 64

#define MARL_BUILD_ASM 1
#include "osfiber_asm_rv64.h"

// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// a0: from
// a1: to
.text
.global marl_fiber_swap
.align 4
marl_fiber_swap:

    // Save context 'from'

    // Store callee-preserved registers
    sd  s0, MARL_REG_s0(a0)
    sd  s1, MARL_REG_s1(a0)
    sd  s2, MARL_REG_s2(a0)
    sd  s3, MARL_REG_s3(a0)
    sd  s4, MARL_REG_s4(a0)
    sd  s5, MARL_REG_s5(a0)
    sd  s6, MARL_REG_s6(a0)
    sd  s7, MARL_REG_s7(a0)
    sd  s8, MARL_REG_s8(a0)
    sd  s9, MARL_REG_s9(a0)
    sd  s10, MARL_REG_s10(a0)
    sd  s11, MARL_REG_s11(a0)

    fsd  fs0, MARL_REG_fs0(a0)
    fsd  fs1, MARL_REG_fs1(a0)
    fsd  fs2, MARL_REG_fs2(a0)
    fsd  fs3, MARL_REG_fs3(a0)
    fsd  fs4, MARL_REG_fs4(a0)
    fsd  fs5, MARL_REG_fs5(a0)
    fsd  fs6, MARL_REG_fs6(a0)
    fsd  fs7, MARL_REG_fs7(a0)
    fsd  fs8, MARL_REG_fs8(a0)
    fsd  fs9, MARL_REG_fs9(a0)
    fsd  fs10, MARL_REG_fs10(a0)
    fsd  fs11, MARL_REG_fs11(a0)

    sd  sp, MARL_REG_sp(a0)
    // On RISC-V ra is caller-saved
    // but we need ra to jump to the trampoline
    sd  ra, MARL_REG_ra(a0)

    move  t0, a1 // Store a1 in temporary register

    // Recover callee-preserved registers
    ld  s0, MARL_REG_s0(t0)
    ld  s1, MARL_REG_s1(t0)
    ld  s2, MARL_REG_s2(t0)
    ld  s3, MARL_REG_s3(t0)
    ld  s4, MARL_REG_s4(t0)
    ld  s5, MARL_REG_s5(t0)
    ld  s6, MARL_REG_s6(t0)
    ld  s7, MARL_REG_s7(t0)
    ld  s8, MARL_REG_s8(t0)
    ld  s9, MARL_REG_s9(t0)
    ld  s10, MARL_REG_s10(t0)
    ld  s11, MARL_REG_s11(t0)

    fld  fs0, MARL_REG_fs0(t0)
    fld  fs1, MARL_REG_fs1(t0)
    fld  fs2, MARL_REG_fs2(t0)
    fld  fs3, MARL_REG_fs3(t0)
    fld  fs4, MARL_REG_fs4(t0)
    fld  fs5, MARL_REG_fs5(t0)
    fld  fs6, MARL_REG_fs6(t0)
    fld  fs7, MARL_REG_fs7(t0)
    fld  fs8, MARL_REG_fs8(t0)
    fld  fs9, MARL_REG_fs9(t0)
    fld  fs10, MARL_REG_fs10(t0)
    fld  fs11, MARL_REG_fs11(t0)

    ld  sp, MARL_REG_sp(t0)
    ld  ra, MARL_REG_ra(t0)

    // Recover arguments
    ld  a0, MARL_REG_a0(t0)
    ld  a1, MARL_REG_a1(t0)

    jr	ra // Jump to the trampoline

#endif // defined(__riscv) && __riscv_xlen == 64
