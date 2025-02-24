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

#include "src/tint/lang/glsl/writer/raise/shader_io.h"

#include <memory>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/transform/shader_io.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/containers/vector.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {

namespace {

/// PIMPL state for the parts of the shader IO transform specific to GLSL.
/// For GLSL, we declare a global variable for each input and output. The wrapper entry point then
/// loads from and stores to these variables.
struct StateImpl : core::ir::transform::ShaderIOBackendState {
    /// The input variables.
    Vector<core::ir::Var*, 4> input_vars;
    /// The output variables.
    Vector<core::ir::Var*, 4> output_vars;

    /// The original type of vertex input variables that we are bgra-swizzling. Keyed by location.
    Hashmap<uint32_t, const core::type::Type*, 1> bgra_swizzle_original_types;

    /// The configuration options.
    const ShaderIOConfig& config;

    /// Constructor
    StateImpl(core::ir::Module& mod, core::ir::Function* f, const ShaderIOConfig& cfg)
        : ShaderIOBackendState(mod, f), config(cfg) {}

    /// Destructor
    ~StateImpl() override {}

    /// Declare a global variable for each IO entry listed in @p entries.
    /// @param vars the list of variables
    /// @param entries the entries to emit
    /// @param addrspace the address to use for the global variables
    /// @param access the access mode to use for the global variables
    /// @param name_suffix the suffix to add to struct and variable names
    void MakeVars(Vector<core::ir::Var*, 4>& vars,
                  Vector<core::type::Manager::StructMemberDesc, 4>& entries,
                  core::AddressSpace addrspace,
                  core::Access access,
                  const char* name_suffix) {
        for (auto io : entries) {
            StringStream name;
            name << ir.NameOf(func).Name();

            const core::type::MemoryView* ptr = nullptr;
            if (io.attributes.builtin) {
                switch (io.attributes.builtin.value()) {
                    case core::BuiltinValue::kSampleMask:
                        ptr = ty.ptr(addrspace, ty.array(ty.i32(), 1), access);
                        break;
                    case core::BuiltinValue::kVertexIndex:
                    case core::BuiltinValue::kInstanceIndex:
                    case core::BuiltinValue::kSampleIndex:
                        ptr = ty.ptr(addrspace, ty.i32(), access);
                        break;
                    default:
                        ptr = ty.ptr(addrspace, io.type, access);
                        break;
                }
                name << "_" << *io.attributes.builtin;
            } else {
                auto* type = io.type;

                if (io.attributes.location) {
                    name << "_loc" << io.attributes.location.value();
                    if (io.attributes.blend_src.has_value()) {
                        name << "_idx" << io.attributes.blend_src.value();
                    }

                    // When doing the BGRA swizzle, always emit vec4f. The component type must be
                    // f32 for unorm8x4-bgra, but the number of components can be anything. Even if
                    // the input variable is an f32 we need to get the full vec4f when swizzling as
                    // the components are reordered.
                    if (config.bgra_swizzle_locations.count(*io.attributes.location) != 0) {
                        bgra_swizzle_original_types.Add(*io.attributes.location, io.type);
                        type = ty.vec4<f32>();
                    }
                }
                name << name_suffix;
                ptr = ty.ptr(addrspace, type, access);
            }

            // Create an IO variable and add it to the root block.
            auto* var = b.Var(name.str(), ptr);
            var->SetAttributes(io.attributes);
            ir.root_block->Append(var);
            vars.Push(var);
        }
    }

    /// @copydoc ShaderIO::BackendState::FinalizeInputs
    Vector<core::ir::FunctionParam*, 4> FinalizeInputs() override {
        MakeVars(input_vars, inputs, core::AddressSpace::kIn, core::Access::kRead, "_Input");
        return tint::Empty;
    }

    /// @copydoc ShaderIO::BackendState::FinalizeOutputs
    const core::type::Type* FinalizeOutputs() override {
        MakeVars(output_vars, outputs, core::AddressSpace::kOut, core::Access::kWrite, "_Output");
        return ty.void_();
    }

    /// @copydoc ShaderIO::BackendState::GetInput
    core::ir::Value* GetInput(core::ir::Builder& builder, uint32_t idx) override {
        auto* from = input_vars[idx]->Result(0);
        auto* value = builder.Load(from)->Result(0);

        auto& builtin = inputs[idx].attributes.builtin;
        if (builtin.has_value()) {
            switch (builtin.value()) {
                case core::BuiltinValue::kVertexIndex:
                case core::BuiltinValue::kInstanceIndex:
                case core::BuiltinValue::kSampleIndex: {
                    // GLSL uses i32 for these, so convert to u32.
                    value = builder.Convert(ty.u32(), value)->Result(0);
                    break;
                }
                case core::BuiltinValue::kSampleMask: {
                    // gl_SampleMaskIn is an array of i32. Retrieve the first element and
                    // convert it to u32.
                    auto* elem = builder.Access(ty.i32(), value, 0_u);
                    value = builder.Convert(ty.u32(), elem)->Result(0);
                    break;
                }
                default:
                    break;
            }
        }

        // Replace the value with the BGRA-swizzled version of it when required.
        if (inputs[idx].attributes.location &&
            config.bgra_swizzle_locations.count(*inputs[idx].attributes.location) != 0) {
            auto* original_type =
                *bgra_swizzle_original_types.Get(*inputs[idx].attributes.location);

            Vector<uint32_t, 4> swizzles = {2, 1, 0, 3};
            swizzles.Resize(original_type->Elements(nullptr, 1).count);

            value = builder.Swizzle(original_type, value, swizzles)->Result(0);
        }

        return value;
    }

    /// @copydoc ShaderIO::BackendState::SetOutput
    void SetOutput(core::ir::Builder& builder, uint32_t idx, core::ir::Value* value) override {
        // Clamp frag_depth values if necessary.
        if (outputs[idx].attributes.builtin == core::BuiltinValue::kFragDepth) {
            value = ClampFragDepth(builder, value);
        }

        // Store the output to the global variable declared earlier.
        auto* to = output_vars[idx]->Result(0);

        if (outputs[idx].attributes.builtin == core::BuiltinValue::kSampleMask) {
            auto* ptr = ty.ptr(core::AddressSpace::kOut, ty.i32(), core::Access::kWrite);
            to = builder.Access(ptr, to, 0_u)->Result(0);
            value = builder.Convert(ty.i32(), value)->Result(0);
        } else if (outputs[idx].attributes.builtin == core::BuiltinValue::kPosition) {
            auto* x = builder.Swizzle(ty.f32(), value, {0});

            // Negate the gl_Position.y value
            auto* y = builder.Swizzle(ty.f32(), value, {1});
            auto* new_y = builder.Negation(ty.f32(), y);

            // Recalculate gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
            auto* z = builder.Swizzle(ty.f32(), value, {2});
            auto* w = builder.Swizzle(ty.f32(), value, {3});
            auto* mul = builder.Multiply(ty.f32(), 2_f, z);
            auto* new_z = builder.Subtract(ty.f32(), mul, w);
            value = builder.Construct(ty.vec4<f32>(), x, new_y, new_z, w)->Result(0);
        }

        builder.Store(to, value);
    }

    /// Clamp a frag_depth builtin value if necessary.
    /// @param builder the builder to use for new instructions
    /// @param frag_depth the incoming frag_depth value
    /// @returns the clamped value
    core::ir::Value* ClampFragDepth([[maybe_unused]] core::ir::Builder& builder,
                                    core::ir::Value* frag_depth) {
        if (!config.depth_range_offsets) {
            return frag_depth;
        }

        auto* push_constants = config.push_constant_layout.var;
        auto min_idx = u32(config.push_constant_layout.IndexOf(config.depth_range_offsets->min));
        auto max_idx = u32(config.push_constant_layout.IndexOf(config.depth_range_offsets->max));
        auto* min = builder.Load(builder.Access<ptr<push_constant, f32>>(push_constants, min_idx));
        auto* max = builder.Load(builder.Access<ptr<push_constant, f32>>(push_constants, max_idx));
        return builder.Call<f32>(core::BuiltinFn::kClamp, frag_depth, min, max)->Result(0);
    }

    /// @copydoc ShaderIO::BackendState::NeedsVertexPointSize
    bool NeedsVertexPointSize() const override { return true; }
};

}  // namespace

Result<SuccessType> ShaderIO(core::ir::Module& ir, const ShaderIOConfig& config) {
    auto result = ValidateAndDumpIfNeeded(
        ir, "glsl.ShaderIO",
        core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings});
    if (result != Success) {
        return result;
    }

    core::ir::transform::RunShaderIOBase(ir, [&](core::ir::Module& mod, core::ir::Function* func) {
        return std::make_unique<StateImpl>(mod, func, config);
    });

    return Success;
}

}  // namespace tint::glsl::writer::raise
