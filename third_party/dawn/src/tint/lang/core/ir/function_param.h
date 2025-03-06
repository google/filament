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

#ifndef SRC_TINT_LANG_CORE_IR_FUNCTION_PARAM_H_
#define SRC_TINT_LANG_CORE_IR_FUNCTION_PARAM_H_

#include <utility>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/io_attributes.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class Function;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A function parameter in the IR.
class FunctionParam : public Castable<FunctionParam, Value> {
  public:
    /// Constructor
    /// @param type the type of the var
    explicit FunctionParam(const core::type::Type* type);
    ~FunctionParam() override;

    /// Sets the function that this parameter belongs to.
    /// @param func the function
    void SetFunction(ir::Function* func) { func_ = func; }

    /// @returns the function that this parameter belongs to, or nullptr
    ir::Function* Function() { return func_; }

    /// @returns the function that this parameter belongs to, or nullptr
    const ir::Function* Function() const { return func_; }

    /// Sets the index of the parameter in the function's parameter list
    /// @param index the index
    void SetIndex(uint32_t index) { index_ = index; }

    /// @returns the index of the parameter in the function's parameter list
    uint32_t Index() const { return index_; }

    /// @returns the type of the var
    const core::type::Type* Type() const override { return type_; }

    /// Sets the type of the parameter to @p type
    /// @param type the new type of the parameter
    void SetType(const core::type::Type* type) { type_ = type; }

    /// @copydoc Value::Clone()
    FunctionParam* Clone(CloneContext& ctx) override;

    /// Sets the IO attributes.
    /// @param attrs the attributes
    void SetAttributes(const IOAttributes& attrs) { attributes_ = attrs; }
    /// @returns the IO attributes
    const IOAttributes& Attributes() const { return attributes_; }

    /// Sets the builtin information. Note, it is currently an error if the builtin is already set.
    /// @param val the builtin to set
    void SetBuiltin(core::BuiltinValue val) {
        TINT_ASSERT(!attributes_.builtin.has_value());
        attributes_.builtin = val;
    }
    /// @returns the builtin set for the parameter
    std::optional<core::BuiltinValue> Builtin() const { return attributes_.builtin; }

    /// Sets the parameter as invariant
    /// @param val the value to set for invariant
    void SetInvariant(bool val) { attributes_.invariant = val; }

    /// @returns true if parameter is invariant
    bool Invariant() const { return attributes_.invariant; }

    /// Sets the location.
    /// @param loc the optional location value
    void SetLocation(std::optional<uint32_t> loc) { attributes_.location = loc; }

    /// @returns the optional location attribute value
    std::optional<uint32_t> Location() const { return attributes_.location; }

    /// Sets the color.
    /// @param col the optional color value
    void SetColor(std::optional<uint32_t> col) { attributes_.color = col; }

    /// @returns the optional color attribute value
    std::optional<uint32_t> Color() const { return attributes_.color; }

    /// Sets the interpolation.
    /// @param interpolation the optional location interpolation settings
    void SetInterpolation(std::optional<core::Interpolation> interpolation) {
        attributes_.interpolation = interpolation;
    }

    /// @returns the optional interpolation attribute value
    std::optional<core::Interpolation> Interpolation() const { return attributes_.interpolation; }

    /// Sets the binding point
    /// @param group the group
    /// @param binding the binding
    void SetBindingPoint(uint32_t group, uint32_t binding) { binding_point_ = {group, binding}; }

    /// Sets the binding point
    /// @param binding_point the binding point
    void SetBindingPoint(std::optional<struct BindingPoint> binding_point) {
        binding_point_ = binding_point;
    }

    /// @returns the binding points if `Attributes` contains `kBindingPoint`
    std::optional<struct BindingPoint> BindingPoint() const { return binding_point_; }

  private:
    ir::Function* func_ = nullptr;
    uint32_t index_ = 0xffffffff;
    const core::type::Type* type_ = nullptr;
    std::optional<struct BindingPoint> binding_point_;
    IOAttributes attributes_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_FUNCTION_PARAM_H_
