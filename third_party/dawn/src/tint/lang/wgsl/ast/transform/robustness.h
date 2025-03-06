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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_ROBUSTNESS_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_ROBUSTNESS_H_

#include <unordered_set>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::ast::transform {

/// This transform is responsible for ensuring that all out of bounds accesses are prevented,
/// either by conditioning the access (predication) or through clamping of the index to keep the
/// access in bounds.
/// @note Robustness must come after:
///       * PromoteSideEffectsToDecl as Robustness requires side-effecting expressions to be hoisted
///         to their own statements.
///       Robustness must come before:
///       * BuiltinPolyfill as 'clamp' and binary operators may need to be polyfilled.
///       * CanonicalizeEntryPointIO as the transform does not support the 'in' and 'out' address
///         spaces.
class Robustness final : public Castable<Robustness, Transform> {
  public:
    /// Robustness action for out-of-bounds indexing.
    enum class Action {
        /// Do nothing to prevent the out-of-bounds action.
        kIgnore,
        /// Clamp the index to be within bounds.
        kClamp,
        /// Do not execute the read or write if the index is out-of-bounds.
        kPredicate,

        /// The default action
        kDefault = kClamp,
    };

    /// Configuration options for the transform
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        Config();

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Assignment operator
        /// @returns this Config
        Config& operator=(const Config&);

        /// Robustness action for values
        Action value_action = Action::kDefault;

        /// Robustness action for non-sampling texture operations
        Action texture_action = Action::kDefault;

        /// Robustness action for variables in the 'function' address space
        Action function_action = Action::kDefault;
        /// Robustness action for variables in the 'private' address space
        Action private_action = Action::kDefault;
        /// Robustness action for variables in the 'push_constant' address space
        Action push_constant_action = Action::kDefault;
        /// Robustness action for variables in the 'storage' address space
        Action storage_action = Action::kDefault;
        /// Robustness action for variables in the 'uniform' address space
        Action uniform_action = Action::kDefault;
        /// Robustness action for variables in the 'workgroup' address space
        Action workgroup_action = Action::kDefault;

        /// Bindings that should always be applied Actions::kIgnore on
        std::unordered_set<tint::BindingPoint> bindings_ignored;

        /// If we should disable index clamping on runtime-sized arrays in robustness transform
        bool disable_runtime_sized_array_index_clamping = false;
    };

    /// Constructor
    Robustness();
    /// Destructor
    ~Robustness() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_ROBUSTNESS_H_
