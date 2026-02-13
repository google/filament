// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/common/function.h"

#include <unordered_map>
#include <unordered_set>

#include "src/tint/utils/ice/ice.h"

namespace tint::spirv::writer {
namespace {

bool IsFunctionTerminator(spv::Op op) {
    return op == spv::Op::OpReturn || op == spv::Op::OpReturnValue || op == spv::Op::OpKill ||
           op == spv::Op::OpUnreachable || op == spv::Op::OpTerminateInvocation;
}

bool IsBranchTerminator(spv::Op op) {
    return op == spv::Op::OpBranch || op == spv::Op::OpBranchConditional || op == spv::Op::OpSwitch;
}

}  // namespace
Function::Function() : declaration_(Instruction{spv::Op::OpNop, {}}), label_op_(Operand(0u)) {}

Function::Function(const Instruction& declaration,
                   const Operand& label_op,
                   const InstructionList& params)
    : declaration_(declaration), label_op_(label_op), params_(params) {}

Function::Function(const Function& other) = default;

Function& Function::operator=(const Function& other) = default;

Function::~Function() = default;

void Function::Iterate(std::function<void(const Instruction&)> cb) const {
    cb(declaration_);

    for (const auto& param : params_) {
        cb(param);
    }

    cb(Instruction{spv::Op::OpLabel, {label_op_}});

    for (const auto& var : vars_) {
        cb(var);
    }

    std::vector<uint32_t> block_order;
    block_order.reserve(blocks_.size());

    std::unordered_set<uint32_t> seen_blocks;

    std::vector<uint32_t> block_idx_stack;
    block_idx_stack.push_back(0);
    seen_blocks.insert(0);

    auto idx_for_id = [&](uint32_t id) -> uint32_t {
        auto iter = block_id_to_block_.find(id);
        TINT_ASSERT(iter != block_id_to_block_.end());
        return iter->second;
    };

    auto push_id = [&](const Instruction& inst, size_t idx) {
        auto id = idx_for_id(std::get<uint32_t>(inst.Operands()[idx]));

        if (seen_blocks.find(id) != seen_blocks.end()) {
            return;
        }
        seen_blocks.insert(id);

        block_idx_stack.push_back(id);
    };

    while (!block_idx_stack.empty()) {
        auto idx = block_idx_stack.back();
        block_idx_stack.pop_back();

        block_order.push_back(idx);

        auto& blk = blocks_[idx];
        auto& term = blk.back();
        if (IsFunctionTerminator(term.Opcode())) {
            continue;
        }

        TINT_ASSERT(IsBranchTerminator(term.Opcode()));

        // The initial block doesn't have a label, so can end up with 1
        // instruction.
        if (blk.size() >= 2) {
            auto& pre_term = blk[blk.size() - 2];

            // Push the merges first so the emit after the branch conditional.
            switch (pre_term.Opcode()) {
                case spv::Op::OpSelectionMerge: {
                    push_id(pre_term, 0);
                    break;
                }
                case spv::Op::OpLoopMerge: {
                    // Push merge first, then continuing
                    push_id(pre_term, 0);
                    push_id(pre_term, 1);
                    break;
                }
                default:
                    break;
            }
        }

        switch (term.Opcode()) {
            case spv::Op::OpBranch: {
                push_id(term, 0);
                break;
            }
            case spv::Op::OpBranchConditional: {
                // Push false then true as we'll emit in reversed order
                push_id(term, 2);
                push_id(term, 1);

                break;
            }
            case spv::Op::OpSwitch: {
                auto& ops = term.Operands();
                for (size_t k = ops.size() - 1; k > 2; k -= 2) {
                    push_id(term, k);
                }
                push_id(term, 1);
                break;
            }
            default:
                TINT_UNREACHABLE();
        }
    }

    TINT_ASSERT(seen_blocks.size() == blocks_.size());

    // Emit the blocks in block order
    for (const auto& idx : block_order) {
        auto& blk = blocks_[idx];
        for (const auto& inst : blk) {
            cb(inst);
        }
    }

    cb(Instruction{spv::Op::OpFunctionEnd, {}});
}

}  // namespace tint::spirv::writer
