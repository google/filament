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

#ifndef SRC_TINT_LANG_CORE_IR_TRAVERSE_H_
#define SRC_TINT_LANG_CORE_IR_TRAVERSE_H_

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/traits.h"

namespace tint::core::ir {

/// Traverse calls @p callback with each instruction in @p block and all child blocks of @p block
/// that matches the callback parameter type.
/// @param block the block to traverse
/// @param callback a function with the signature `void(T*)`
template <typename CALLBACK>
void Traverse(Block* block, CALLBACK&& callback) {
    using T = std::remove_pointer_t<traits::ParameterType<CALLBACK, 0>>;

    Vector<ir::Instruction*, 8> queue;
    if (!block->IsEmpty()) {
        queue.Push(block->Front());
    }
    while (!queue.IsEmpty()) {
        for (auto* inst = queue.Pop(); inst != nullptr; inst = inst->next) {
            if (auto* as_t = inst->As<T>()) {
                callback(as_t);
            }
            if (auto* ctrl = inst->As<ControlInstruction>()) {
                if (Instruction* next = inst->next) {
                    queue.Push(next);  // Resume iteration of this block
                }

                Vector<ir::Instruction*, 8> children;
                ctrl->ForeachBlock([&](ir::Block* b) {
                    if (!b->IsEmpty()) {
                        children.Push(b->Front());
                    }
                });
                for (auto* child : Reverse(children)) {
                    queue.Push(child);
                }
                break;
            }
        }
    }
}

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_TRAVERSE_H_
