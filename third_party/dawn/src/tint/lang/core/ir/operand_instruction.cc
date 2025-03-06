// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/operand_instruction.h"

using Op10 = tint::core::ir::OperandInstruction<1, 0>;
using Op11 = tint::core::ir::OperandInstruction<1, 1>;
using Op20 = tint::core::ir::OperandInstruction<2, 0>;
using Op30 = tint::core::ir::OperandInstruction<3, 0>;
using Op21 = tint::core::ir::OperandInstruction<2, 1>;
using Op31 = tint::core::ir::OperandInstruction<3, 1>;
using Op41 = tint::core::ir::OperandInstruction<4, 1>;

TINT_INSTANTIATE_TYPEINFO(Op10);
TINT_INSTANTIATE_TYPEINFO(Op11);
TINT_INSTANTIATE_TYPEINFO(Op20);
TINT_INSTANTIATE_TYPEINFO(Op30);
TINT_INSTANTIATE_TYPEINFO(Op21);
TINT_INSTANTIATE_TYPEINFO(Op31);
TINT_INSTANTIATE_TYPEINFO(Op41);

namespace tint::core::ir {}  // namespace tint::core::ir
