// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_SEM_FUNCTION_H_
#define SRC_TINT_LANG_WGSL_SEM_FUNCTION_H_

#include <array>
#include <optional>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/ast/variable.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations
namespace tint::ast {
class BuiltinAttribute;
class Function;
class LocationAttribute;
class ReturnStatement;
}  // namespace tint::ast
namespace tint::sem {
class BuiltinFn;
class Variable;
}  // namespace tint::sem

namespace tint::sem {

/// WorkgroupSize is a three-dimensional array of WorkgroupDimensions.
/// Each dimension is a std::optional as a workgroup size can be a const-expression or
/// override-expression. Override expressions are not known at compilation time, so these will be
/// std::nullopt.
using WorkgroupSize = std::array<std::optional<uint32_t>, 3>;

/// Function holds the semantic information for function nodes.
class Function final : public Castable<Function, CallTarget> {
  public:
    /// A vector of [Variable*, BindingPoint] pairs
    using VariableBindings = std::vector<std::pair<const Variable*, BindingPoint>>;

    /// Constructor
    /// @param declaration the ast::Function
    explicit Function(const ast::Function* declaration);

    /// Destructor
    ~Function() override;

    /// Sets the function's return location
    /// @param return_location the location value
    void SetReturnLocation(uint32_t return_location) { return_location_ = return_location; }

    // Sets the function's return index
    /// @param return_index the index value
    void SetReturnIndex(uint32_t return_index) { return_index_ = return_index; }

    /// @returns the ast::Function declaration
    const ast::Function* Declaration() const { return declaration_; }

    /// @returns the workgroup size {x, y, z} for the function.
    const sem::WorkgroupSize& WorkgroupSize() const { return workgroup_size_; }

    /// Sets the workgroup size {x, y, z} for the function.
    /// @param workgroup_size the new workgroup size of the function
    void SetWorkgroupSize(sem::WorkgroupSize workgroup_size) {
        workgroup_size_ = std::move(workgroup_size);
    }

    /// @returns all directly referenced global variables
    const UniqueVector<const GlobalVariable*, 4>& DirectlyReferencedGlobals() const {
        return directly_referenced_globals_;
    }

    /// Records that this function directly references the given global variable.
    /// Note: Implicitly adds this global to the transitively-called globals.
    /// @param global the module-scope variable
    void AddDirectlyReferencedGlobal(const sem::GlobalVariable* global) {
        directly_referenced_globals_.Add(global);
        AddTransitivelyReferencedGlobal(global);
    }

    /// @returns all transitively referenced global variables
    const UniqueVector<const GlobalVariable*, 8>& TransitivelyReferencedGlobals() const {
        return transitively_referenced_globals_;
    }

    /// Records that this function transitively references the given global
    /// variable.
    /// @param global the module-scoped variable
    void AddTransitivelyReferencedGlobal(const sem::GlobalVariable* global);

    /// @returns the list of functions that this function transitively calls.
    const UniqueVector<const Function*, 8>& TransitivelyCalledFunctions() const {
        return transitively_called_functions_;
    }

    /// Records that this function transitively calls `function`.
    /// @param function the function this function transitively calls
    void AddTransitivelyCalledFunction(const Function* function) {
        transitively_called_functions_.Add(function);
    }

    /// @returns the list of builtins that this function directly calls.
    const UniqueVector<const BuiltinFn*, 4>& DirectlyCalledBuiltins() const {
        return directly_called_builtins_;
    }

    /// Records that this function transitively calls `builtin`.
    /// @param builtin the builtin this function directly calls
    void AddDirectlyCalledBuiltin(const BuiltinFn* builtin) {
        directly_called_builtins_.Add(builtin);
    }

    /// Adds the given texture/sampler pair to the list of unique pairs
    /// that this function uses (directly or indirectly). These can only
    /// be parameters to this function or global variables. Uniqueness is
    /// ensured by texture_sampler_pairs_ being a UniqueVector.
    /// @param texture the texture (must be non-null)
    /// @param sampler the sampler (null indicates a texture-only reference)
    void AddTextureSamplerPair(const sem::Variable* texture, const sem::Variable* sampler) {
        TINT_ASSERT(texture != nullptr);
        texture_sampler_pairs_.Add(VariablePair(texture, sampler));
    }

    /// @returns the list of texture/sampler pairs that this function uses
    /// (directly or indirectly).
    VectorRef<VariablePair> TextureSamplerPairs() const { return texture_sampler_pairs_; }

    /// @returns the list of direct calls to functions / builtins made by this
    /// function
    const Vector<const Call*, 1>& DirectCalls() const { return direct_calls_; }

    /// Adds a record of the direct function / builtin calls made by this
    /// function
    /// @param call the call
    void AddDirectCall(const Call* call) { direct_calls_.Push(call); }

    /// @param target the target of a call
    /// @returns the Call to the given CallTarget, or nullptr the target was not
    /// called by this function.
    const Call* FindDirectCallTo(const CallTarget* target) const {
        for (auto* call : direct_calls_) {
            if (call->Target() == target) {
                return call;
            }
        }
        return nullptr;
    }

    /// @returns the list of callsites to this function
    const Vector<const Call*, 1>& CallSites() const { return callsites_; }

    /// Adds a record of a callsite to this function
    /// @param call the callsite
    void AddCallSite(const Call* call) { callsites_.Push(call); }

    /// @returns the ancestor entry points
    const Vector<const Function*, 1>& AncestorEntryPoints() const { return ancestor_entry_points_; }

    /// Adds a record that the given entry point transitively calls this function
    /// @param entry_point the entry point that transtively calls this function
    void AddAncestorEntryPoint(const sem::Function* entry_point) {
        ancestor_entry_points_.Push(entry_point);
    }

    /// Retrieves any referenced location variables
    /// @returns the <variable, attribute> pair.
    std::vector<std::pair<const Variable*, const ast::LocationAttribute*>>
    TransitivelyReferencedLocationVariables() const;

    /// Retrieves any referenced builtin variables
    /// @returns the <variable, attribute> pair.
    std::vector<std::pair<const Variable*, const ast::BuiltinAttribute*>>
    TransitivelyReferencedBuiltinVariables() const;

    /// Retrieves any referenced uniform variables. Note, the variables must be
    /// decorated with both binding and group attributes.
    /// @returns the referenced uniforms
    VariableBindings TransitivelyReferencedUniformVariables() const;

    /// Retrieves any referenced storagebuffer variables. Note, the variables
    /// must be decorated with both binding and group attributes.
    /// @returns the referenced storagebuffers
    VariableBindings TransitivelyReferencedStorageBufferVariables() const;

    /// Retrieves any referenced regular Sampler variables. Note, the
    /// variables must be decorated with both binding and group attributes.
    /// @returns the referenced storagebuffers
    VariableBindings TransitivelyReferencedSamplerVariables() const;

    /// Retrieves any referenced comparison Sampler variables. Note, the
    /// variables must be decorated with both binding and group attributes.
    /// @returns the referenced storagebuffers
    VariableBindings TransitivelyReferencedComparisonSamplerVariables() const;

    /// Retrieves any referenced sampled textures variables. Note, the
    /// variables must be decorated with both binding and group attributes.
    /// @returns the referenced sampled textures
    VariableBindings TransitivelyReferencedSampledTextureVariables() const;

    /// Retrieves any referenced multisampled textures variables. Note, the
    /// variables must be decorated with both binding and group attributes.
    /// @returns the referenced sampled textures
    VariableBindings TransitivelyReferencedMultisampledTextureVariables() const;

    /// Retrieves any referenced variables of the given type. Note, the variables
    /// must be decorated with both binding and group attributes.
    /// @param type the type of the variables to find
    /// @returns the referenced variables
    VariableBindings TransitivelyReferencedVariablesOfType(const tint::TypeInfo* type) const;

    /// Retrieves any referenced variables of the given type. Note, the variables
    /// must be decorated with both binding and group attributes.
    /// @returns the referenced variables
    template <typename T>
    VariableBindings TransitivelyReferencedVariablesOfType() const {
        return TransitivelyReferencedVariablesOfType(&tint::TypeInfo::Of<T>());
    }

    /// Checks if the given entry point is an ancestor
    /// @param sym the entry point symbol
    /// @returns true if `sym` is an ancestor entry point of this function
    bool HasAncestorEntryPoint(Symbol sym) const;

    /// Records the first discard statement in the function
    /// @param stmt the `discard` statement.
    void SetDiscardStatement(const Statement* stmt) {
        if (!discard_stmt_) {
            discard_stmt_ = stmt;
        }
    }

    /// @returns the first discard statement for the function, or nullptr if the function does not
    /// use `discard`.
    const Statement* DiscardStatement() const { return discard_stmt_; }

    /// @return the behaviors of this function
    const sem::Behaviors& Behaviors() const { return behaviors_; }

    /// @return the behaviors of this function
    sem::Behaviors& Behaviors() { return behaviors_; }

    /// @return the location for the return, if provided
    std::optional<uint32_t> ReturnLocation() const { return return_location_; }

    /// @return the index for the return, if provided
    std::optional<uint32_t> ReturnIndex() const { return return_index_; }

    /// Modifies the severity of a specific diagnostic rule for this function.
    /// @param rule the diagnostic rule
    /// @param severity the new diagnostic severity
    void SetDiagnosticSeverity(wgsl::DiagnosticRule rule, wgsl::DiagnosticSeverity severity);

    /// @returns the diagnostic severity modifications applied to this function
    const wgsl::DiagnosticRuleSeverities& DiagnosticSeverities() const {
        return diagnostic_severities_;
    }

  private:
    Function(const Function&) = delete;
    Function(Function&&) = delete;

    VariableBindings TransitivelyReferencedSamplerVariablesImpl(core::type::SamplerKind kind) const;
    VariableBindings TransitivelyReferencedSampledTextureVariablesImpl(bool multisampled) const;

    const ast::Function* const declaration_;

    sem::WorkgroupSize workgroup_size_;
    UniqueVector<const GlobalVariable*, 4> directly_referenced_globals_;
    UniqueVector<const GlobalVariable*, 8> transitively_referenced_globals_;
    UniqueVector<const Function*, 8> transitively_called_functions_;
    UniqueVector<const BuiltinFn*, 4> directly_called_builtins_;
    UniqueVector<VariablePair, 8> texture_sampler_pairs_;
    Vector<const Call*, 1> direct_calls_;
    Vector<const Call*, 1> callsites_;
    Vector<const Function*, 1> ancestor_entry_points_;
    const Statement* discard_stmt_ = nullptr;
    sem::Behaviors behaviors_{sem::Behavior::kNext};
    wgsl::DiagnosticRuleSeverities diagnostic_severities_;

    std::optional<uint32_t> return_location_;
    std::optional<uint32_t> return_index_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_FUNCTION_H_
