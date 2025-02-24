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

#if defined(__mips__) && _MIPS_SIM == _ABI64

#define MARL_BUILD_ASM 1
#include "osfiber_asm_mips64.h"

// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// a0: from
// v0: to
.text
.global MARL_ASM_SYMBOL(marl_fiber_swap)
.align 4
MARL_ASM_SYMBOL(marl_fiber_swap):

    // Save context 'from'

    // Store callee-preserved registers
    sd  $s0, MARL_REG_s0($a0)
    sd  $s1, MARL_REG_s1($a0)
    sd  $s2, MARL_REG_s2($a0)
    sd  $s3, MARL_REG_s3($a0)
    sd  $s4, MARL_REG_s4($a0)
    sd  $s5, MARL_REG_s5($a0)
    sd  $s6, MARL_REG_s6($a0)
    sd  $s7, MARL_REG_s7($a0)

    s.d  $f24, MARL_REG_f24($a0)
    s.d  $f25, MARL_REG_f25($a0)
    s.d  $f26, MARL_REG_f26($a0)
    s.d  $f27, MARL_REG_f27($a0)
    s.d  $f28, MARL_REG_f28($a0)
    s.d  $f29, MARL_REG_f29($a0)
    s.d  $f31, MARL_REG_f30($a0)
    s.d  $f31, MARL_REG_f31($a0)

    sd  $gp, MARL_REG_gp($a0)
    sd  $sp, MARL_REG_sp($a0)
    sd  $fp, MARL_REG_fp($a0)
    sd  $ra, MARL_REG_ra($a0)

    move  $v0, $a1 // Function have no return, so safe to touch v0

    // Recover callee-preserved registers
    ld  $s0, MARL_REG_s0($v0)
    ld  $s1, MARL_REG_s1($v0)
    ld  $s2, MARL_REG_s2($v0)
    ld  $s3, MARL_REG_s3($v0)
    ld  $s4, MARL_REG_s4($v0)
    ld  $s5, MARL_REG_s5($v0)
    ld  $s6, MARL_REG_s6($v0)
    ld  $s7, MARL_REG_s7($v0)

    l.d  $f24, MARL_REG_f24($v0)
    l.d  $f25, MARL_REG_f25($v0)
    l.d  $f26, MARL_REG_f26($v0)
    l.d  $f27, MARL_REG_f27($v0)
    l.d  $f28, MARL_REG_f28($v0)
    l.d  $f29, MARL_REG_f29($v0)
    l.d  $f31, MARL_REG_f30($v0)
    l.d  $f31, MARL_REG_f31($v0)

    ld  $gp, MARL_REG_gp($v0)
    ld  $sp, MARL_REG_sp($v0)
    ld  $fp, MARL_REG_fp($v0)
    ld  $ra, MARL_REG_ra($v0)

    // Recover arguments
    ld  $a0, MARL_REG_a0($v0)
    ld  $a1, MARL_REG_a1($v0)

    jr	$ra

#endif // defined(__mips__) && _MIPS_SIM == _ABI64
