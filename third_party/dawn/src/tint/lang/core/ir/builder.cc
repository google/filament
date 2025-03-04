// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/builder.h"

#include <utility>

#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/type/function.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::core::ir {

Builder::Builder(Module& mod) : ir(mod) {}

Builder::Builder(Module& mod, ir::Block* block)
    : insertion_point_(InsertionPoints::AppendToBlock{block}), ir(mod) {}

Builder::~Builder() = default;

Block* Builder::Block() {
    return ir.blocks.Create<ir::Block>();
}

MultiInBlock* Builder::MultiInBlock() {
    return ir.blocks.Create<ir::MultiInBlock>();
}

Function* Builder::Function(const core::type::Type* return_type, Function::PipelineStage stage) {
    auto* ir_func = ir.CreateValue<ir::Function>(ir.Types().function(), return_type, stage);
    ir_func->SetBlock(Block());
    ir.functions.Push(ir_func);
    return ir_func;
}

Function* Builder::Function(std::string_view name,
                            const core::type::Type* return_type,
                            Function::PipelineStage stage) {
    auto* ir_func = Function(return_type, stage);
    ir.SetName(ir_func, name);
    return ir_func;
}

ir::Loop* Builder::Loop() {
    return Append(ir.CreateInstruction<ir::Loop>(Block(), MultiInBlock(), MultiInBlock()));
}

Block* Builder::Case(ir::Switch* s, VectorRef<ir::Constant*> values) {
    auto* block = Block();

    Switch::Case c;
    c.block = block;
    for (auto* value : values) {
        c.selectors.Push(Switch::CaseSelector{value});
    }
    s->Cases().Push(std::move(c));
    block->SetParent(s);
    return block;
}

Block* Builder::DefaultCase(ir::Switch* s) {
    return Case(s, Vector<ir::Constant*, 1>{nullptr});
}

Block* Builder::Case(ir::Switch* s, std::initializer_list<ir::Constant*> selectors) {
    return Case(s, Vector<ir::Constant*, 4>(selectors));
}

ir::Discard* Builder::Discard() {
    return Append(ir.CreateInstruction<ir::Discard>());
}

ir::Var* Builder::Var(const core::type::MemoryView* type) {
    return Append(ir.CreateInstruction<ir::Var>(InstructionResult(type)));
}

ir::Var* Builder::Var(std::string_view name, const core::type::MemoryView* type) {
    auto* var = Var(type);
    ir.SetName(var, name);
    return var;
}

ir::BlockParam* Builder::BlockParam(const core::type::Type* type) {
    return ir.CreateValue<ir::BlockParam>(type);
}

ir::BlockParam* Builder::BlockParam(std::string_view name, const core::type::Type* type) {
    auto* param = ir.CreateValue<ir::BlockParam>(type);
    ir.SetName(param, name);
    return param;
}

ir::FunctionParam* Builder::FunctionParam(const core::type::Type* type) {
    return ir.CreateValue<ir::FunctionParam>(type);
}

ir::FunctionParam* Builder::FunctionParam(std::string_view name, const core::type::Type* type) {
    auto* param = ir.CreateValue<ir::FunctionParam>(type);
    ir.SetName(param, name);
    return param;
}

ir::TerminateInvocation* Builder::TerminateInvocation() {
    return Append(ir.CreateInstruction<ir::TerminateInvocation>());
}

ir::Unreachable* Builder::Unreachable() {
    return Append(ir.CreateInstruction<ir::Unreachable>());
}

ir::Unused* Builder::Unused() {
    return ir.CreateValue<ir::Unused>();
}

const core::type::Type* Builder::VectorPtrElementType(const core::type::Type* type) {
    auto* vec_ptr_ty = type->As<core::type::Pointer>();
    TINT_ASSERT(vec_ptr_ty);
    if (DAWN_LIKELY(vec_ptr_ty)) {
        auto* vec_ty = vec_ptr_ty->StoreType()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);
        if (DAWN_LIKELY(vec_ty)) {
            return vec_ty->Type();
        }
    }
    return ir.Types().i32();
}

}  // namespace tint::core::ir
