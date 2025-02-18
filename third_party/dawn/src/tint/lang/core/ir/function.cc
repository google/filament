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

#include "src/tint/lang/core/ir/function.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Function);

namespace tint::core::ir {

Function::Function() = default;

Function::Function(const core::type::Type* type,
                   const core::type::Type* rt,
                   PipelineStage stage,
                   std::optional<std::array<Value*, 3>> wg_size)
    : pipeline_stage_(stage), workgroup_size_(wg_size), type_(type) {
    TINT_ASSERT(rt != nullptr);

    return_.type = rt;
}

Function::~Function() = default;

Function* Function::Clone(CloneContext& ctx) {
    auto* new_func =
        ctx.ir.CreateValue<Function>(type_, return_.type, pipeline_stage_, workgroup_size_);
    new_func->block_ = ctx.ir.blocks.Create<ir::Block>();
    new_func->SetParams(ctx.Clone<1>(params_.Slice()));
    new_func->return_.attributes = return_.attributes;

    ctx.Replace(this, new_func);
    block_->CloneInto(ctx, new_func->block_);

    ctx.ir.SetName(new_func, ctx.ir.NameOf(this).Name());
    return new_func;
}

void Function::SetParams(VectorRef<FunctionParam*> params) {
    for (auto* param : params_) {
        param->SetFunction(nullptr);
    }
    params_ = std::move(params);
    TINT_ASSERT(!params_.Any(IsNull));
    uint32_t index = 0;
    for (auto* param : params_) {
        param->SetFunction(this);
        param->SetIndex(index++);
    }
}

void Function::SetParams(std::initializer_list<FunctionParam*> params) {
    for (auto* param : params_) {
        param->SetFunction(nullptr);
    }
    params_ = params;
    TINT_ASSERT(!params_.Any(IsNull));
    uint32_t index = 0;
    for (auto* param : params_) {
        param->SetFunction(this);
        param->SetIndex(index++);
    }
}

void Function::AppendParam(FunctionParam* param) {
    params_.Push(param);
    param->SetFunction(this);
    param->SetIndex(static_cast<uint32_t>(params_.Length() - 1u));
}

void Function::Destroy() {
    Base::Destroy();
    block_->Destroy();
}

std::string_view ToString(Function::PipelineStage value) {
    switch (value) {
        case Function::PipelineStage::kVertex:
            return "vertex";
        case Function::PipelineStage::kFragment:
            return "fragment";
        case Function::PipelineStage::kCompute:
            return "compute";
        default:
            break;
    }
    return "<unknown>";
}

}  // namespace tint::core::ir
