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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_SHADER_IO_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_SHADER_IO_H_

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/type/manager.h"

namespace tint::core::ir::transform {

/// Abstract base class for the state needed to handle IO for a particular backend target.
struct ShaderIOBackendState {
    /// Constructor
    /// @param mod the IR module
    /// @param f the entry point function
    ShaderIOBackendState(Module& mod, Function* f) : ir(mod), func(f) {}

    /// Destructor
    virtual ~ShaderIOBackendState();

    /// Add an input.
    /// @param name the name of the input
    /// @param type the type of the input
    /// @param attributes the IO attributes
    /// @returns the index of the input
    uint32_t AddInput(Symbol name, const core::type::Type* type, core::IOAttributes attributes) {
        inputs.Push({name, type, std::move(attributes)});
        return uint32_t(inputs.Length() - 1);
    }

    /// Add an output.
    /// @param name the name of the output
    /// @param type the type of the output
    /// @param attributes the IO attributes
    /// @returns the index of the output
    uint32_t AddOutput(Symbol name, const core::type::Type* type, core::IOAttributes attributes) {
        outputs.Push({name, type, std::move(attributes)});
        return uint32_t(outputs.Length() - 1);
    }

    /// @returns true if inputs were added
    bool HasInputs() const { return !inputs.IsEmpty(); }

    // @returns true if outputs were added
    bool HasOutputs() const { return !outputs.IsEmpty(); }

    /// Finalize the shader inputs and create any state needed for the new entry point function.
    /// @returns the list of function parameters for the new entry point
    virtual Vector<FunctionParam*, 4> FinalizeInputs() = 0;

    /// Finalize the shader outputs and create state needed for the new entry point function.
    /// @returns the return type for the new entry point
    virtual const type::Type* FinalizeOutputs() = 0;

    /// Get the value of the input at index @p idx
    /// @param builder the IR builder for new instructions
    /// @param idx the index of the input
    /// @returns the value of the input
    virtual Value* GetInput(Builder& builder, uint32_t idx) = 0;

    /// Set the value of the output at index @p idx
    /// @param builder the IR builder for new instructions
    /// @param idx the index of the output
    /// @param value the value to set
    virtual void SetOutput(Builder& builder, uint32_t idx, Value* value) = 0;

    /// Create the return value for the entry point, based on the output values that have been set.
    /// @param builder the IR builder for new instructions
    /// @returns the return value for the new entry point
    virtual Value* MakeReturnValue([[maybe_unused]] Builder& builder) { return nullptr; }

    /// @returns true if a vertex point size builtin should be added
    virtual bool NeedsVertexPointSize() const { return false; }

  protected:
    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The original entry point function.
    Function* func = nullptr;

    /// The list of shader inputs.
    Vector<core::type::Manager::StructMemberDesc, 4> inputs;

    /// The list of shader outputs.
    Vector<core::type::Manager::StructMemberDesc, 4> outputs;
};

/// The signature for a function that creates a backend state object.
using MakeBackendStateFunc = std::unique_ptr<ShaderIOBackendState>(Module&, Function*);

/// @param module the module to transform
/// @param make_backend_state a function that creates a backend state object
void RunShaderIOBase(Module& module, std::function<MakeBackendStateFunc> make_backend_state);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_SHADER_IO_H_
