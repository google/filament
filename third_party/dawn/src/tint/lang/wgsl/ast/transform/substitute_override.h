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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_SUBSTITUTE_OVERRIDE_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_SUBSTITUTE_OVERRIDE_H_

#include <string>
#include <unordered_map>

#include "src/tint/api/common/override_id.h"

#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection.h"

namespace tint::ast::transform {

/// A transform that replaces overrides with the constant values provided.
///
/// # Example
/// ```
/// override width: f32;
/// @id(1) override height: i32 = 4;
/// override depth = 1i;
/// ```
///
/// When transformed with `width` -> 1, `1` -> 22, `depth` -> 42
///
/// ```
/// const width: f32 = 1f;
/// const height: i32 = 22i;
/// const depth = 42i;
/// ```
///
/// @see crbug.com/tint/1582
class SubstituteOverride final : public Castable<SubstituteOverride, Transform> {
  public:
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

        /// The map of override identifier to the override value.
        /// The value is always a double coming into the transform and will be
        /// converted to the correct type through and initializer.
        std::unordered_map<OverrideId, double> map;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(Config, map);
    };

    /// Constructor
    SubstituteOverride();

    /// Destructor
    ~SubstituteOverride() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_SUBSTITUTE_OVERRIDE_H_
