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

#include "src/tint/lang/hlsl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/hlsl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/hlsl/writer/printer/printer.h"
#include "src/tint/lang/hlsl/writer/raise/raise.h"
#include "src/tint/lang/wgsl/ast/pipeline_stage.h"

namespace tint::hlsl::writer {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // Check for unsupported types.
    for (auto* ty : ir.Types()) {
        if (ty->Is<core::type::SubgroupMatrix>()) {
            return Failure("subgroup matrices are not supported by the HLSL backend");
        }
    }

    // Check for unsupported module-scope variable address spaces and types.
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<core::ir::Var>();
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPushConstant) {
            return Failure("push constants are not supported by the HLSL backend");
        }
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            // Check the pixel_local variables have corresponding entries in the PLS attachment map.
            auto* str = ptr->StoreType()->As<core::type::Struct>();
            for (uint32_t i = 0; i < str->Members().Length(); i++) {
                if (options.pixel_local.attachments.count(i) == 0) {
                    return Failure("missing pixel local attachment for member index " +
                                   std::to_string(i));
                }
            }
        }
        if (ptr->StoreType()->Is<core::type::InputAttachment>()) {
            return Failure("input attachments are not supported by the HLSL backend");
        }
    }
    return Success;
}

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    // Raise the core-dialect to HLSL-dialect
    auto res = Raise(ir, options);
    if (res != Success) {
        return res.Failure();
    }

    return Print(ir, options);
}

Result<Output> Generate(const Program& program, const Options& options) {
    if (!program.IsValid()) {
        return Failure{program.Diagnostics().Str()};
    }

    // Sanitize the program.
    auto sanitized_result = Sanitize(program, options);
    if (!sanitized_result.program.IsValid()) {
        return Failure{sanitized_result.program.Diagnostics().Str()};
    }

    // Generate the HLSL code.
    auto impl = std::make_unique<ASTPrinter>(sanitized_result.program);
    if (!impl->Generate()) {
        return Failure{impl->Diagnostics().Str()};
    }

    Output output;
    output.hlsl = impl->Result();

    // Collect the list of entry points in the sanitized program.
    for (auto* func : sanitized_result.program.AST().Functions()) {
        if (func->IsEntryPoint()) {
            auto name = func->name->symbol.Name();
            output.entry_points.push_back({name, func->PipelineStage()});
        }
    }

    output.used_array_length_from_uniform_indices =
        std::move(sanitized_result.used_array_length_from_uniform_indices);

    return output;
}

}  // namespace tint::hlsl::writer
