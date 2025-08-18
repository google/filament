// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/shader_io.h"

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/containers/vector.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The function that creates a backend state object.
    std::function<MakeBackendStateFunc> make_backend_state;

    /// The IR module.
    Module& ir;
    /// The IR builder.
    Builder b{ir};
    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The entry point currently being processed.
    Function* ep = nullptr;

    /// The backend state object for the current entry point.
    std::unique_ptr<ShaderIOBackendState> backend{};

    /// Process the module.
    void Process() {
        // Collect all structures before the transform has run, so that we can strip their shader IO
        // attributes later.
        Vector<const type::Struct*, 16> structures_to_strip;
        for (auto* type : ir.Types()) {
            if (auto* str = type->As<type::Struct>()) {
                structures_to_strip.Push(str);
            }
        }

        // Process the entry points.
        // Take a copy of the function list since the transform adds new functions to the module.
        auto functions = ir.functions;
        for (auto& func : functions) {
            // Only process entry points.
            if (!func->IsEntryPoint()) {
                continue;
            }

            ProcessEntryPoint(func, make_backend_state(ir, func));
        }

        // Remove IO attributes from all structure members that had them prior to this transform.
        for (auto* str : structures_to_strip) {
            for (auto* member : str->Members()) {
                // TODO(crbug.com/tint/745): Remove the const_cast.
                const_cast<core::type::StructMember*>(member)->ResetAttributes();
            }
        }
    }

    /// Process an entry point.
    /// @param f the original entry point function
    /// @param bs the backend state object
    void ProcessEntryPoint(Function* f, std::unique_ptr<ShaderIOBackendState> bs) {
        TINT_SCOPED_ASSIGNMENT(ep, f);
        backend = std::move(bs);
        TINT_DEFER(backend = nullptr);

        // Process the parameters and return value to prepare for building a wrapper function.
        GatherInputs();  // Calls backend->AddInput() for each input
        GatherOutput();  // Calls backend->AddOutput() for each output

        // Add an output for the vertex point size if needed.
        std::optional<uint32_t> vertex_point_size_index;
        if (ep->IsVertex() && backend->NeedsVertexPointSize()) {
            vertex_point_size_index =
                backend->AddOutput(ir.symbols.New("vertex_point_size"), ty.f32(),
                                   core::IOAttributes{
                                       .builtin = core::BuiltinValue::kPointSize,
                                   });
        }

        auto new_params = backend->FinalizeInputs();
        auto* new_ret_ty = backend->FinalizeOutputs();

        // Skip entry points with no new inputs or outputs.
        if (!backend->HasInputs() && !backend->HasOutputs()) {
            return;
        }

        // Rename the old function and remove its pipeline stage and workgroup size, as we will be
        // wrapping it with a new entry point.
        auto name = ir.NameOf(ep).Name();
        auto stage = ep->Stage();
        auto wgsize = ep->WorkgroupSize();
        ir.SetName(ep, name + "_inner");
        ep->SetStage(Function::PipelineStage::kUndefined);
        ep->ClearWorkgroupSize();

        // Create the entry point wrapper function.
        auto* wrapper_ep = b.Function(name, new_ret_ty);
        wrapper_ep->SetParams(std::move(new_params));
        wrapper_ep->SetStage(stage);
        if (wgsize) {
            wrapper_ep->SetWorkgroupSize((*wgsize)[0], (*wgsize)[1], (*wgsize)[2]);
        }
        auto wrapper = b.Append(wrapper_ep->Block());

        // Call the original function, passing it the inputs and capturing its return value.
        auto inner_call_args = BuildInnerCallArgs(wrapper);
        auto* inner_result = wrapper.Call(ep->ReturnType(), ep, std::move(inner_call_args));
        SetOutputs(wrapper, inner_result->Result());
        if (vertex_point_size_index) {
            backend->SetOutput(wrapper, vertex_point_size_index.value(), b.Constant(1_f));
        }

        // Return the new result.
        wrapper.Return(wrapper_ep, backend->MakeReturnValue(wrapper));
    }

    /// Gather the shader inputs.
    void GatherInputs() {
        for (auto* param : ep->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    auto name = str->Name().Name() + "_" + member->Name().Name();
                    auto attributes = member->Attributes();
                    if (attributes.interpolation && !ep->IsFragment()) {
                        // Strip interpolation on non-fragment inputs
                        attributes.interpolation = {};
                    }
                    backend->AddInput(ir.symbols.Register(name), member->Type(),
                                      std::move(attributes));
                }
            } else {
                // Pull out the IO attributes and remove them from the parameter.
                auto attributes = param->Attributes();
                if (attributes.interpolation && !ep->IsFragment()) {
                    // Strip interpolation on non-fragment inputs
                    attributes.interpolation = {};
                }
                param->ResetAttributes();

                auto name = ir.NameOf(param);
                backend->AddInput(name, param->Type(), std::move(attributes));
            }
        }
    }

    /// Gather the shader outputs.
    void GatherOutput() {
        if (ep->ReturnType()->Is<core::type::Void>()) {
            return;
        }

        if (auto* str = ep->ReturnType()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                auto name = str->Name().Name() + "_" + member->Name().Name();
                auto attributes = member->Attributes();
                if (attributes.interpolation && !ep->IsVertex()) {
                    // Strip interpolation on non-vertex outputs
                    attributes.interpolation = {};
                }
                backend->AddOutput(ir.symbols.Register(name), member->Type(),
                                   std::move(attributes));
            }
        } else {
            // Pull out the IO attributes and remove them from the original function.
            auto attributes = ep->ReturnAttributes();
            if (attributes.interpolation && !ep->IsVertex()) {
                // Strip interpolation on non-vertex outputs
                attributes.interpolation = {};
            }
            ep->SetReturnAttributes({});

            backend->AddOutput(ir.symbols.New(), ep->ReturnType(), std::move(attributes));
        }
    }

    /// Build the argument list to call the original entry point function.
    /// @param builder the IR builder for new instructions
    /// @returns the argument list
    Vector<Value*, 4> BuildInnerCallArgs(Builder& builder) {
        uint32_t input_idx = 0;
        Vector<Value*, 4> args;
        for (auto* param : ep->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                Vector<Value*, 4> construct_args;
                for (uint32_t i = 0; i < str->Members().Length(); i++) {
                    construct_args.Push(backend->GetInput(builder, input_idx++));
                }
                args.Push(builder.Construct(param->Type(), construct_args)->Result());
            } else {
                args.Push(backend->GetInput(builder, input_idx++));
            }
        }

        return args;
    }

    /// Propagate outputs from the inner function call to their final destination.
    /// @param builder the IR builder for new instructions
    /// @param inner_result the return value from calling the original entry point function
    void SetOutputs(Builder& builder, Value* inner_result) {
        if (auto* str = inner_result->Type()->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                Value* from =
                    builder.Access(member->Type(), inner_result, u32(member->Index()))->Result();
                backend->SetOutput(builder, member->Index(), from);
            }
        } else if (!inner_result->Type()->Is<core::type::Void>()) {
            backend->SetOutput(builder, 0u, inner_result);
        }
    }
};

}  // namespace

void RunShaderIOBase(Module& module, std::function<MakeBackendStateFunc> make_backend_state) {
    State{make_backend_state, module}.Process();
}

ShaderIOBackendState::~ShaderIOBackendState() = default;

}  // namespace tint::core::ir::transform
