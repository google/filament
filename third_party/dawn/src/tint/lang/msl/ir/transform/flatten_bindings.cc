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

#include "src/tint/lang/msl/ir/transform/flatten_bindings.h"

#include <unordered_map>
#include <utility>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::msl::ir::transform {

Result<SuccessType> FlattenBindings(core::ir::Module& ir) {
    uint32_t next_buffer_idx = 0;
    uint32_t next_sampler_idx = 0;
    uint32_t next_texture_idx = 0;

    std::unordered_map<BindingPoint, BindingPoint> binding_points;
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<core::ir::Var>();
        if (!var) {
            continue;
        }

        auto bp = var->BindingPoint();
        if (!bp.has_value()) {
            continue;
        }

        BindingPoint src = bp.value();
        if (binding_points.count(src) > 0) {
            continue;
        }

        auto* ty = var->Result()->Type()->As<core::type::Pointer>();
        TINT_ASSERT(ty);

        tint::Switch(
            ty->StoreType(),                   //
            [&](const core::type::Texture*) {  //
                binding_points.emplace(src, BindingPoint{0, next_texture_idx++});
            },                                 //
            [&](const core::type::Sampler*) {  //
                binding_points.emplace(src, BindingPoint{0, next_sampler_idx++});
            },                                         //
            [&](const core::type::InputAttachment*) {  //
                // flattening is not supported for input attachments.
                TINT_UNREACHABLE();
            },  //
            [&](Default) {
                switch (ty->AddressSpace()) {
                    case core::AddressSpace::kStorage:
                    case core::AddressSpace::kUniform: {
                        binding_points.emplace(src, BindingPoint{0, next_buffer_idx++});
                        break;
                    }
                    default: {
                        break;
                    }
                }
            });
    }

    // Run the binding remapper transform.
    return core::ir::transform::BindingRemapper(ir, binding_points);
}

}  // namespace tint::msl::ir::transform
