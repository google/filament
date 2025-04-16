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

#ifndef SRC_TINT_LANG_CORE_IR_VAR_H_
#define SRC_TINT_LANG_CORE_IR_VAR_H_

#include <string>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/interpolation.h"
#include "src/tint/lang/core/io_attributes.h"
#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A var instruction in the IR.
class Var : public Castable<Var, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the initializer
    static constexpr size_t kInitializerOperandOffset = 0;

    /// The fixed number of results returned by this instruction
    static constexpr size_t kNumResults = 1;

    /// The fixed number of operands expected by this instruction
    static constexpr size_t kNumOperands = 1;

    /// Constructor (no results, no operands)
    /// @param id the instruction id
    explicit Var(Id id);

    /// Constructor
    /// @param id the instruction id
    /// @param result the result value
    explicit Var(Id id, InstructionResult* result);

    ~Var() override;

    /// @copydoc Instruction::Clone()
    Var* Clone(CloneContext& ctx) override;

    /// Sets the var initializer
    /// @param initializer the initializer
    void SetInitializer(Value* initializer);
    /// @returns the initializer
    Value* Initializer() { return Operand(kInitializerOperandOffset); }
    /// @returns the initializer
    const Value* Initializer() const { return Operand(kInitializerOperandOffset); }

    /// Sets the binding point
    /// @param group the group
    /// @param binding the binding
    void SetBindingPoint(uint32_t group, uint32_t binding) { binding_point_ = {group, binding}; }
    /// @returns the binding points if `Attributes` contains `kBindingPoint`
    std::optional<struct BindingPoint> BindingPoint() const { return binding_point_; }

    /// Sets the input attachment index
    /// @param index the index
    void SetInputAttachmentIndex(uint32_t index) { input_attachment_index_ = index; }
    /// @returns the input attachment index if any
    std::optional<uint32_t> InputAttachmentIndex() const { return input_attachment_index_; }

    /// Sets the IO attributes
    /// @param attrs the attributes
    void SetAttributes(const IOAttributes& attrs) { attributes_ = attrs; }
    /// @returns the IO attributes
    const IOAttributes& Attributes() const { return attributes_; }

    /// Destroys this instruction along with any assignment instructions, if the var is never read.
    void DestroyIfOnlyAssigned();

    /// @returns the friendly name for the instruction
    std::string FriendlyName() const override { return "var"; }

  private:
    std::optional<struct BindingPoint> binding_point_;
    std::optional<uint32_t> input_attachment_index_;
    IOAttributes attributes_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_VAR_H_
