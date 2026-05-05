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

#include "src/tint/lang/core/ir/analysis/subgroup_matrix.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/spirv/writer/common/option_helpers.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"
#include "src/tint/lang/spirv/writer/raise/raise.h"

// Included by 'ast_printer.h', included again here for './tools/run gen' track the dependency.
#include "spirv/unified1/spirv.h"  // IWYU pragma: export

namespace tint::spirv::writer {

namespace {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // The enum is accessible in the API so ensure we have a valid value.
    switch (options.spirv_version) {
        case SpvVersion::kSpv13:
        case SpvVersion::kSpv14:
        case SpvVersion::kSpv15:
            break;
        default:
            return Failure("unsupported SPIR-V version");
    }

    // Check optionally supported types against their required options.
    for (auto* ty : ir.Types()) {
        if (ty->Is<core::type::SubgroupMatrix>()) {
            if (!options.extensions.use_vulkan_memory_model) {
                return Failure("using subgroup matrices requires the Vulkan Memory Model");
            }
        }
        if (ty->Is<core::type::Buffer>()) {
            return Failure("buffers are not supported by the SPIR-V backend");
        }
        if (ty->Is<core::type::U16>()) {
            return Failure("16-bit unsigned integers are not supported by the SPIR-V backend");
        }
    }

    // If a remapped entry point name is provided, it must not be empty, and must not contain
    // embedded null characters.
    if (!options.remapped_entry_point_name.empty()) {
        if (options.remapped_entry_point_name.find('\0') != std::string::npos) {
            return Failure("remapped entry point name contains null character");
        }
    }

    core::ir::Function* ep_func = nullptr;
    for (auto* f : ir.functions) {
        if (!f->IsEntryPoint()) {
            continue;
        }
        if (ir.NameOf(f).NameView() == options.entry_point_name) {
            ep_func = f;
            break;
        }
    }

    // No entrypoint, so no bindings needed
    if (!ep_func) {
        return Failure("entry point not found");
    }

    // Check for unsupported shader IO attributes.
    auto check_input_attributes = [&](const core::type::Type* ty,
                                      const core::IOAttributes& attributes) -> Result<SuccessType> {
        if (attributes.color.has_value() && ty->DeepestElement()->Is<core::type::F16>()) {
            return Failure(
                "@color attribute on f16 type is not supported by the Vulkan SPIR-V backend");
        }
        return Success;
    };

    // Check input attributes.
    for (auto* param : ep_func->Params()) {
        if (auto* str = param->Type()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                TINT_CHECK_RESULT(check_input_attributes(member->Type(), member->Attributes()));
            }
        } else {
            TINT_CHECK_RESULT(check_input_attributes(param->Type(), param->Attributes()));
        }
    }

    core::ir::ReferencedModuleVars<const core::ir::Module> referenced_module_vars{ir};
    auto& refs = referenced_module_vars.TransitiveReferences(ep_func);

    // Check for unsupported module-scope variable address spaces.
    for (auto* var : refs) {
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return Failure("pixel_local address space is not supported by the SPIR-V backend");
        }
    }

    // Check for calls to unsupported builtin functions.
    for (auto* inst : ir.Instructions()) {
        auto* call = inst->As<core::ir::CoreBuiltinCall>();
        if (!call) {
            continue;
        }

        if (call->Func() == core::BuiltinFn::kPrint) {
            return Failure("print is not supported by the SPIR-V backend");
        }
    }

    TINT_CHECK_RESULT(ValidateBindingOptions(options));

    return Success;
}

}  // namespace

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    TINT_CHECK_RESULT(CanGenerate(ir, options));

    // There are currently no plans on supporting override-expressions, so we can pull this
    // information out before the raise. If we want to support overrides then this either needs to
    // happen in raise, before the builtins are polyfilled, or the analysis needs to also look for
    // `*` operations with subgroup matrices.
    auto sm_info = core::ir::analysis::GatherSubgroupMatrixInfo(ir);

    // Raise from core-dialect to SPIR-V-dialect.
    TINT_CHECK_RESULT(Raise(ir, options));

    TINT_CHECK_RESULT_UNWRAP(res, Print(ir, options));
    res.subgroup_matrix_info = sm_info;

    return res;
}

}  // namespace tint::spirv::writer
