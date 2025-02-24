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

#include "src/tint/lang/spirv/writer/writer.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/spirv/writer/common/option_helpers.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"
#include "src/tint/lang/spirv/writer/raise/raise.h"

// Included by 'ast_printer.h', included again here for './tools/run gen' track the dependency.
#include "spirv/unified1/spirv.h"

namespace tint::spirv::writer {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // If a remapped entry point name is provided, it must not be empty, and must not contain
    // embedded null characters.
    if (!options.remapped_entry_point_name.empty()) {
        if (options.remapped_entry_point_name.find('\0') != std::string::npos) {
            return Failure("remapped entry point name contains null character");
        }

        // Check for multiple entry points.
        // TODO(375388101): Remove this check when SingleEntryPoint is part of the backend.
        bool has_entry_point = false;
        for (auto& func : ir.functions) {
            if (func->IsEntryPoint()) {
                if (has_entry_point) {
                    return Failure("module must only contain a single entry point");
                }
                has_entry_point = true;
            }
        }
    }

    // Check for unsupported module-scope variable address spaces and types.
    // Also make sure there is at most one user-declared push_constant, and make a note of its size.
    uint32_t user_push_constant_size = 0;
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<core::ir::Var>();
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return Failure("pixel_local address space is not supported by the SPIR-V backend");
        }

        if (ptr->AddressSpace() == core::AddressSpace::kPushConstant) {
            if (user_push_constant_size > 0) {
                // We've already seen a user-declared push constant.
                return Failure("module contains multiple user-declared push constants");
            }
            user_push_constant_size = tint::RoundUp(4u, ptr->StoreType()->Size());
        }
    }

    static constexpr uint32_t kMaxOffset = 0x1000;
    Hashset<uint32_t, 4> push_constant_word_offsets;
    auto check_push_constant_offset = [&](uint32_t offset) {
        // Excessive values can cause OOM / timeouts when padding structures in the printer.
        if (offset > kMaxOffset) {
            return false;
        }
        // Offset must be 4-byte aligned.
        if (offset & 0x3) {
            return false;
        }
        // Offset must not have already been used.
        if (!push_constant_word_offsets.Add(offset >> 2)) {
            return false;
        }
        // Offset must be after the user-defined push constants.
        if (offset < user_push_constant_size) {
            return false;
        }
        return true;
    };

    if (options.depth_range_offsets) {
        if (!check_push_constant_offset(options.depth_range_offsets->max) ||
            !check_push_constant_offset(options.depth_range_offsets->min)) {
            return Failure("invalid offsets for depth range push constants");
        }
    }

    return Success;
}

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    {
        auto res = ValidateBindingOptions(options);
        if (res != Success) {
            return res.Failure();
        }
    }

    // Raise from core-dialect to SPIR-V-dialect.
    if (auto res = Raise(ir, options); res != Success) {
        return std::move(res.Failure());
    }

    return Print(ir, options);
}

}  // namespace tint::spirv::writer
