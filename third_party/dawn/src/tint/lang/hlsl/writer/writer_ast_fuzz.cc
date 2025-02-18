// Copyright 2024 The Dawn & Tint Authors
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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader)

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/hlsl/validate/validate.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/transform/renamer.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/text/string.h"

namespace tint::hlsl::writer {
namespace {

bool CanRun(const Program& program) {
    if (program.AST().HasOverrides()) {
        return false;
    }

    // The PixelLocal transform assumes only a single entry point, so check for multiple entry
    // points if chromium_experimental_pixel_local is enabled.
    uint32_t num_entry_points = 0;
    for (auto* fn : program.AST().Functions()) {
        if (fn->PipelineStage() != ast::PipelineStage::kNone) {
            num_entry_points++;
        }
    }
    for (auto* enable : program.AST().Enables()) {
        if (enable->HasExtension(tint::wgsl::Extension::kChromiumExperimentalPixelLocal)) {
            if (num_entry_points > 1) {
                return false;
            }
            break;
        }
    }

    return true;
}

void ASTFuzzer(const tint::Program& program,
               const fuzz::wgsl::Context& context,
               const Options& options) {
    if (!CanRun(program)) {
        return;
    }

    // Currently disabled, as DXC can error on HLSL emitted by Tint. For example: post optimization
    // infinite loops will fail to compile, but these are beyond Tint's analysis capabilities.
    constexpr bool must_validate = false;

    const char* dxc_path = validate::kDxcDLLName;
    if (!context.options.dxc.empty()) {
        dxc_path = context.options.dxc.c_str();
    }
    auto dxc = tint::Command::LookPath(dxc_path);

    Result<tint::hlsl::writer::Output> res;
    if (dxc.Found()) {
        // If validating with DXC, run renamer transform first to avoid DXC validation failures.
        ast::transform::DataMap inputs, outputs;
        inputs.Add<ast::transform::Renamer::Config>(ast::transform::Renamer::Target::kHlslKeywords);
        if (auto renamer_res = tint::ast::transform::Renamer{}.Apply(program, inputs, outputs)) {
            if (!renamer_res->IsValid()) {
                TINT_ICE() << renamer_res->Diagnostics();
            }
            res = tint::hlsl::writer::Generate(*renamer_res, options);
        }
    } else {
        res = tint::hlsl::writer::Generate(program, options);
    }

    if (res != Success) {
        return;
    }

    if (context.options.dump) {
        std::cout << "Dumping generated HLSL:\n" << res->hlsl << "\n";
    }

    if (dxc.Found()) {
        uint32_t hlsl_shader_model = 60;
        bool require_16bit_types = false;
        auto enable_list = program.AST().Enables();
        for (auto* enable : enable_list) {
            if (enable->HasExtension(tint::wgsl::Extension::kF16)) {
                hlsl_shader_model = 62;
                require_16bit_types = true;
                break;
            }
        }

        auto validate_res = validate::ValidateUsingDXC(dxc.Path(), res->hlsl, res->entry_points,
                                                       require_16bit_types, hlsl_shader_model);

        if (must_validate && validate_res.failed) {
            size_t line_num = 1;
            std::stringstream err;
            err << "DXC was expected to succeed, but failed:\n\n";
            for (auto line : Split(res->hlsl, "\n")) {
                err << line_num++ << ": " << line << "\n";
            }
            err << "\n\n" << validate_res.output;
            TINT_ICE() << err.str();
        }

    } else if (must_validate) {
        TINT_ICE() << "cannot validate with DXC as it was not found at: " << dxc_path;
    }
}

}  // namespace
}  // namespace tint::hlsl::writer

TINT_WGSL_PROGRAM_FUZZER(tint::hlsl::writer::ASTFuzzer);
