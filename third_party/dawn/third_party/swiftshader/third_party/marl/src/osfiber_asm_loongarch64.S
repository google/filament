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

#if defined(__loongarch_lp64)

#define MARL_BUILD_ASM 1
#include "osfiber_asm_loongarch64.h"

// void marl_fiber_swap(marl_fiber_context* from, const marl_fiber_context* to)
// a0: from
// a1: to
.text
.global marl_fiber_swap
.align 4
marl_fiber_swap:

    // Save context 'from'

    // Store callee-preserved registers
    st.d $s0, $a0, MARL_REG_s0
    st.d $s1, $a0, MARL_REG_s1
    st.d $s2, $a0, MARL_REG_s2
    st.d $s3, $a0, MARL_REG_s3
    st.d $s4, $a0, MARL_REG_s4
    st.d $s5, $a0, MARL_REG_s5
    st.d $s6, $a0, MARL_REG_s6
    st.d $s7, $a0, MARL_REG_s7
    st.d $s8, $a0, MARL_REG_s8

    fst.d $fs0, $a0, MARL_REG_fs0
    fst.d $fs1, $a0, MARL_REG_fs1
    fst.d $fs2, $a0, MARL_REG_fs2
    fst.d $fs3, $a0, MARL_REG_fs3
    fst.d $fs4, $a0, MARL_REG_fs4
    fst.d $fs5, $a0, MARL_REG_fs5
    fst.d $fs6, $a0, MARL_REG_fs6
    fst.d $fs7, $a0, MARL_REG_fs7

    st.d $ra, $a0, MARL_REG_ra
    st.d $sp, $a0, MARL_REG_sp
    st.d $fp, $a0, MARL_REG_fp

    // Recover callee-preserved registers
    ld.d $s0, $a1, MARL_REG_s0
    ld.d $s1, $a1, MARL_REG_s1
    ld.d $s2, $a1, MARL_REG_s2
    ld.d $s3, $a1, MARL_REG_s3
    ld.d $s4, $a1, MARL_REG_s4
    ld.d $s5, $a1, MARL_REG_s5
    ld.d $s6, $a1, MARL_REG_s6
    ld.d $s7, $a1, MARL_REG_s7
    ld.d $s8, $a1, MARL_REG_s8

    fld.d $fs0, $a1, MARL_REG_fs0
    fld.d $fs1, $a1, MARL_REG_fs1
    fld.d $fs2, $a1, MARL_REG_fs2
    fld.d $fs3, $a1, MARL_REG_fs3
    fld.d $fs4, $a1, MARL_REG_fs4
    fld.d $fs5, $a1, MARL_REG_fs5
    fld.d $fs6, $a1, MARL_REG_fs6
    fld.d $fs7, $a1, MARL_REG_fs7

    ld.d $ra, $a1, MARL_REG_ra
    ld.d $sp, $a1, MARL_REG_sp
    ld.d $fp, $a1, MARL_REG_fp

    // Recover arguments
    ld.d $a0, $a1, MARL_REG_a0
    ld.d $a1, $a1, MARL_REG_a1

    jr $ra // Jump to the trampoline

#endif // defined(__loongarch_lp64)
