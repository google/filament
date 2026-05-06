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

#include "src/tint/lang/msl/writer/writer.h"

#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/texel_buffer.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/msl/writer/common/option_helpers.h"
#include "src/tint/lang/msl/writer/printer/printer.h"
#include "src/tint/lang/msl/writer/raise/raise.h"

namespace tint::msl::writer {

namespace {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // Check for unsupported types.
    for (auto* ty : ir.Types()) {
        if (auto* m = ty->As<core::type::SubgroupMatrix>()) {
            if (!m->Type()->IsAnyOf<core::type::F16, core::type::F32>()) {
                return Failure("non-float subgroup matrices are not supported by the MSL backend");
            }
            if (m->Columns() != 8 || m->Rows() != 8) {
                return Failure("the MSL backend only supports 8x8 subgroup matrices");
            }
        } else if (ty->Is<core::type::TexelBuffer>()) {
            // TODO(crbug/382544164): Prototype texel buffer feature
            return Failure("texel buffers are not supported by the MSL backend");
        }
        if (ty->Is<core::type::InputAttachment>()) {
            return Failure("input_attachment not supported by the MSL backend");
        }
        if (ty->Is<core::type::Buffer>()) {
            return Failure("buffers are not supported by the MSL backend");
        }
        if (ty->Is<core::type::U16>()) {
            return Failure("16-bit unsigned integers are not supported by the MSL backend");
        }
    }

    for (auto* i : ir.Instructions()) {
        auto* call = i->As<core::ir::CoreBuiltinCall>();
        if (!call) {
            continue;
        }

        if (call->Func() == core::BuiltinFn::kGetResource ||
            call->Func() == core::BuiltinFn::kHasResource) {
            return Failure("resource tables not supported by the MSL backend");
        }
    }

    // Check for unsupported shader IO builtins.
    auto check_io_attributes = [&](const core::IOAttributes& attributes) -> Result<SuccessType> {
        if (attributes.builtin == core::BuiltinValue::kCullDistance) {
            return Failure("cull_distance is not supported by the MSL backend");
        }
        return Success;
    };

    core::ir::Function* ep_func = nullptr;
    for (auto* f : ir.functions) {
        if (!f->IsEntryPoint()) {
            continue;
        }

        // Check `@subgroup_size` attribute.
        if (f->SubgroupSize().has_value()) {
            return Failure("subgroup_size attribute is not supported by the MSL backend");
        }

        // Check input attributes.
        for (auto* param : f->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    TINT_CHECK_RESULT(check_io_attributes(member->Attributes()));
                }
            } else {
                TINT_CHECK_RESULT(check_io_attributes(param->Attributes()));
            }
        }

        // Check output attributes.
        if (auto* str = f->ReturnType()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                TINT_CHECK_RESULT(check_io_attributes(member->Attributes()));
            }
        } else {
            TINT_CHECK_RESULT(check_io_attributes(f->ReturnAttributes()));
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
            return Failure("pixel_local address space is not supported by the MSL backend");
        }
    }

    // Check the vertex pulling config, if provided.
    if (options.vertex_pulling_config) {
        if (!ep_func->IsVertex()) {
            return Failure("vertex pulling config provided without a vertex shader");
        }

        // Gather all of the vertex attribute locations in the config.
        Hashset<uint32_t, 4> locations;
        for (auto& buffer : options.vertex_pulling_config->vertex_state) {
            if (buffer.array_stride & 3) {
                return Failure(
                    "vertex pulling config contains array stride that is not a multiple of 4");
            }
            for (auto& attr : buffer.attributes) {
                if (!locations.Add(attr.shader_location)) {
                    return Failure("vertex pulling config contains duplicate shader locations");
                }
            }
        }

        // Check the parameters to make sure all vertex attributes are present in the config.
        for (auto* param : ep_func->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    if (auto loc = member->Attributes().location) {
                        if (!locations.Contains(*loc)) {
                            return Failure("shader location " + std::to_string(*loc) +
                                           " missing from vertex pulling map");
                        }
                    }
                }
            } else if (auto loc = param->Location()) {
                if (!locations.Contains(*loc)) {
                    return Failure("shader location " + std::to_string(*loc) +
                                   " missing from vertex pulling map");
                }
            }
        }
    }

    TINT_CHECK_RESULT(ValidateBindingOptions(options));

    return Success;
}

}  // namespace

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    TINT_CHECK_RESULT(CanGenerate(ir, options));

    // Raise from core-dialect to MSL-dialect.
    TINT_CHECK_RESULT_UNWRAP(raise_result, Raise(ir, options));
    TINT_CHECK_RESULT_UNWRAP(result, Print(ir, options));

    result.needs_storage_buffer_sizes = raise_result.needs_storage_buffer_sizes;
    return result;
}

}  // namespace tint::msl::writer
