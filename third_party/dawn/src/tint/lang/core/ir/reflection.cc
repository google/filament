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

#include "src/tint/lang/core/ir/reflection.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/pointer.h"

namespace tint::core::ir {

Result<WorkgroupInfo> GetWorkgroupInfo(core::ir::Module& ir) {
    std::optional<std::array<uint32_t, 3>> const_wg_size;
    for (auto func : ir.functions) {
        if (!func->IsEntryPoint()) {
            continue;
        }
        const_wg_size = func->WorkgroupSizeAsConst();
    }

    if (!const_wg_size) {
        return Failure{"IR GetWorkgroupInfo: Could not find workgroup size"};
    }

    size_t wg_storage_size = 0u;
    for (auto* inst : *ir.root_block) {
        if (auto* as_var = inst->As<core::ir::Var>()) {
            auto* ptr = as_var->Result()->Type()->As<core::type::Pointer>();
            if (ptr->AddressSpace() != core::AddressSpace::kWorkgroup) {
                continue;
            }
            auto* ty = ptr->StoreType();
            uint32_t align = ty->Align();
            uint32_t size = ty->Size();

            // This essentially matches std430 layout rules from GLSL, which are in
            // turn specified as an upper bound for Vulkan layout sizing.
            wg_storage_size += tint::RoundUp(16u, tint::RoundUp(align, size));
        }
    }
    return WorkgroupInfo{(*const_wg_size)[0], (*const_wg_size)[1], (*const_wg_size)[2],
                         wg_storage_size};
}

}  // namespace tint::core::ir
