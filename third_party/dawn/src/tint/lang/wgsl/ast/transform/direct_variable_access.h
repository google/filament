// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection.h"

namespace tint::ast::transform {

/// DirectVariableAccess is a transform that allows usage of pointer parameters in the 'storage',
/// 'uniform' and 'workgroup' address space, and passing of pointers to sub-objects. These pointers
/// are allowed with the `unrestricted_pointer_parameters` WGSL feature.
///
/// DirectVariableAccess works by creating specializations of functions that have pointer
/// parameters, one specialization for each pointer argument's unique access chain 'shape' from a
/// unique variable. Calls to specialized functions are transformed so that the pointer arguments
/// are replaced with an array of access-chain indicies, and if the pointer is in the 'function' or
/// 'private' address space, also with a pointer to the root object. For more information, see the
/// comments in src/tint/lang/wgsl/ast/transform/direct_variable_access.cc.
///
/// @note DirectVariableAccess requires the transform::Unshadow transform to have been run first.
class DirectVariableAccess final : public Castable<DirectVariableAccess, Transform> {
  public:
    /// Constructor
    DirectVariableAccess();
    /// Destructor
    ~DirectVariableAccess() override;

    /// Options adjusts the behaviour of the transform.
    struct Options {
        /// If true, then 'private' sub-object pointer arguments will be transformed.
        bool transform_private = false;
        /// If true, then 'function' sub-object pointer arguments will be transformed.
        bool transform_function = false;

        /// Reflection for this struct
        TINT_REFLECT(Options, transform_private, transform_function);
    };

    /// Config is consumed by the DirectVariableAccess transform.
    /// Config specifies the behavior of the transform.
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        Config();

        /// Constructor
        /// @param options behavior of the transform
        explicit Config(const Options& options);

        /// Destructor
        ~Config() override;

        /// The transform behavior options
        Options options;

        /// Reflection for this struct
        TINT_REFLECT(Config, options);
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_DIRECT_VARIABLE_ACCESS_H_
