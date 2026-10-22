// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/analysis/subgroup_matrix.h"

#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::analysis {
namespace {

struct State {
    /// The IR module.
    core::ir::Module& ir;

    SubgroupMatrixInfo info{};

    /// Process the module.
    SubgroupMatrixInfo Process() {
        for (const auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                GatherCall(call);
            }

            for (auto* res : inst->Results()) {
                GatherType(res->Type()->UnwrapPtr());
            }
        }

        return std::move(info);
    }

    SubgroupMatrixType TypeToSMType(const core::type::Type* ty) {
        return tint::Switch(
            ty,  //
            [&](const core::type::F16*) { return SubgroupMatrixType::kF16; },
            [&](const core::type::F32*) { return SubgroupMatrixType::kF32; },
            [&](const core::type::U8*) { return SubgroupMatrixType::kU8; },
            [&](const core::type::I8*) { return SubgroupMatrixType::kI8; },
            [&](const core::type::U32*) { return SubgroupMatrixType::kU32; },
            [&](const core::type::I32*) { return SubgroupMatrixType::kI32; },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void GatherCall(const core::ir::CoreBuiltinCall* call) {
        if (call->Func() != BuiltinFn::kSubgroupMatrixMultiply &&
            call->Func() != BuiltinFn::kSubgroupMatrixMultiplyAccumulate) {
            return;
        }

        auto* result_ty = call->Result()->Type()->As<core::type::SubgroupMatrix>();
        TINT_ASSERT(result_ty);

        auto* left_ty = call->Args()[0]->Type()->As<core::type::SubgroupMatrix>();
        TINT_ASSERT(left_ty);

        auto* right_ty = call->Args()[1]->Type()->As<core::type::SubgroupMatrix>();
        TINT_ASSERT(right_ty);

        SubgroupMatrixMultiply cfg{
            .M = left_ty->Rows(),
            .N = right_ty->Columns(),
            .K = right_ty->Rows(),
            .input_type = TypeToSMType(left_ty->Type()),
            .output_type = TypeToSMType(result_ty->Type()),
        };

        info.multiplies.insert(cfg);
    }

    void GatherType(const core::type::Type* ty) {
        if (auto* str = ty->As<core::type::Struct>()) {
            for (auto* mem : str->Members()) {
                GatherType(mem->Type());
            }
            return;
        }
        if (auto* arr = ty->As<core::type::Array>()) {
            GatherType(arr->ElemType());
            return;
        }

        auto* sm = ty->As<core::type::SubgroupMatrix>();
        if (!sm) {
            return;
        }

        SubgroupMatrixDirection dir = SubgroupMatrixDirection::kResult;

        uint32_t M = 0;
        uint32_t N = 0;
        uint32_t K = 0;

        switch (sm->Kind()) {
            case SubgroupMatrixKind::kResult:
                dir = SubgroupMatrixDirection::kResult;
                M = sm->Rows();
                N = sm->Columns();
                break;
            case SubgroupMatrixKind::kLeft:
                dir = SubgroupMatrixDirection::kLeft;
                M = sm->Rows();
                K = sm->Columns();
                break;
            case SubgroupMatrixKind::kRight:
                dir = SubgroupMatrixDirection::kRight;
                K = sm->Rows();
                N = sm->Columns();
                break;
            case SubgroupMatrixKind::kUndefined:
                TINT_UNREACHABLE();
        }

        SubgroupMatrixConfig cfg{
            .M = M,
            .N = N,
            .K = K,
            .type = TypeToSMType(sm->Type()),
            .direction = dir,
        };

        info.configs.insert(cfg);
    }
};

}  // namespace

SubgroupMatrixInfo GatherSubgroupMatrixInfo(core::ir::Module& ir) {
    return State{ir}.Process();
}

}  // namespace tint::core::ir::analysis
