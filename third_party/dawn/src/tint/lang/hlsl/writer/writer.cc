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

#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/texel_buffer.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/hlsl/writer/common/option_helpers.h"
#include "src/tint/lang/hlsl/writer/printer/printer.h"
#include "src/tint/lang/hlsl/writer/raise/raise.h"

namespace tint::hlsl::writer {

namespace {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // Check for unsupported types.
    for (auto* ty : ir.Types()) {
        if (ty->Is<core::type::SubgroupMatrix>()) {
            return Failure("subgroup matrices are not supported by the HLSL backend");
        }
        if (ty->Is<core::type::TexelBuffer>()) {
            // TODO(crbug/382544164): Prototype texel buffer feature
            return Failure("texel buffers are not supported by the HLSL backend");
        }
        if (options.compiler == Options::Compiler::kFXC) {
            if (auto* ba = ty->As<core::type::BindingArray>()) {
                if (ba->Count()->Is<core::type::RuntimeArrayCount>()) {
                    return Failure("runtime binding array not supported by the HLSL FXC backend");
                }
            }
            if (ty->Is<core::type::U16>()) {
                return Failure("16-bit integers are not supported by the HLSL FXC backend");
            }
        }
        if (ty->Is<core::type::Buffer>()) {
            return Failure("buffers are not supported by the HLSL backend");
        }
    }

    for (auto* i : ir.Instructions()) {
        auto* call = i->As<core::ir::CoreBuiltinCall>();
        if (!call) {
            continue;
        }

        if ((call->Func() == core::BuiltinFn::kGetResource ||
             call->Func() == core::BuiltinFn::kHasResource) &&
            options.compiler == Options::Compiler::kFXC) {
            return Failure(
                "resource tables not supported by the HLSL backend for compiling with FXC");
        }
        if (call->Func() == core::BuiltinFn::kPrint) {
            return Failure("print is not supported by the HLSL backend");
        }
        if (call->Func() == core::BuiltinFn::kAtomicStoreMax ||
            call->Func() == core::BuiltinFn::kAtomicStoreMin) {
            return Failure(
                "64-bit (vec2u) atomic operations are not yet supported by the HLSL backend");
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

    core::ir::ReferencedModuleVars<const core::ir::Module> referenced_module_vars{ir};
    auto& refs = referenced_module_vars.TransitiveReferences(ep_func);

    // Check for unsupported module-scope variable address spaces and types.
    for (auto* var : refs) {
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            // Check the pixel_local variables have corresponding entries in the PLS attachment
            // map.
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

    // Check for unsupported shader IO builtins.
    auto check_io_attributes = [&](const core::IOAttributes& attributes) -> Result<SuccessType> {
        if (attributes.color.has_value()) {
            return Failure("@color attribute is not supported by the HLSL backend");
        }
        if (attributes.builtin == core::BuiltinValue::kSubgroupId ||
            attributes.builtin == core::BuiltinValue::kSubgroupInvocationId ||
            attributes.builtin == core::BuiltinValue::kSubgroupSize ||
            attributes.builtin == core::BuiltinValue::kNumSubgroups) {
            if (options.compiler == Options::Compiler::kFXC) {
                return Failure("subgroups are not supported by FXC");
            }
        }
        if (attributes.builtin == core::BuiltinValue::kBarycentricCoord &&
            options.compiler == Options::Compiler::kFXC) {
            return Failure("barycentric_coord is not supported by the FXC HLSL backend");
        }
        if (attributes.builtin == core::BuiltinValue::kCullDistance) {
            return Failure("cull_distance is not supported by the HLSL backend");
        }
        if (options.truncate_interstage_variables) {
            if (attributes.location >= 30u) {
                return Failure("too many locations for interstage variable truncation");
            }
        }
        return Success;
    };
    // Check input attributes.
    for (auto* param : ep_func->Params()) {
        if (auto* str = param->Type()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                TINT_CHECK_RESULT(check_io_attributes(member->Attributes()));
            }
        } else {
            TINT_CHECK_RESULT(check_io_attributes(param->Attributes()));
        }
    }
    // Check output attributes.
    if (auto* str = ep_func->ReturnType()->As<core::type::Struct>()) {
        for (auto* member : str->Members()) {
            TINT_CHECK_RESULT(check_io_attributes(member->Attributes()));
        }
    } else {
        TINT_CHECK_RESULT(check_io_attributes(ep_func->ReturnAttributes()));
    }

    TINT_CHECK_RESULT(ValidateBindingOptions(ir, options));

    return Success;
}

}  // namespace

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    TINT_CHECK_RESULT(CanGenerate(ir, options));

    // Raise the core-dialect to HLSL-dialect
    TINT_CHECK_RESULT(Raise(ir, options));

    return Print(ir, options);
}

}  // namespace tint::hlsl::writer
