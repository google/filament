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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_ARRAY_LENGTH_FROM_UNIFORM_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_ARRAY_LENGTH_FROM_UNIFORM_H_

#include <unordered_map>
#include <unordered_set>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection.h"

namespace tint::ast::transform {

/// ArrayLengthFromUniform is a transform that implements calls to arrayLength()
/// by calculating the length from the total size of the storage buffer, which
/// is received via a uniform buffer.
///
/// The generated uniform buffer will have the form:
/// ```
/// struct buffer_size_struct {
///  buffer_size : array<u32, 8>;
/// };
///
/// @group(0) @binding(30)
/// var<uniform> buffer_size_ubo : buffer_size_struct;
/// ```
/// The binding group and number used for this uniform buffer is provided via
/// the `Config` transform input. The `Config` struct also defines the mapping
/// from a storage buffer's `BindingPoint` to the array index that will be used
/// to get the size of that buffer.
///
/// This transform assumes that the `SimplifyPointers`
/// transforms have been run before it so that arguments to the arrayLength
/// builtin always have the form `&resource.array`.
///
/// @note Depends on the following transforms to have been run first:
/// * SimplifyPointers
class ArrayLengthFromUniform final : public Castable<ArrayLengthFromUniform, Transform> {
  public:
    /// Constructor
    ArrayLengthFromUniform();
    /// Destructor
    ~ArrayLengthFromUniform() override;

    /// Configuration options for the ArrayLengthFromUniform transform.
    struct Config final : public Castable<Config, Data> {
        /// Constructor
        Config();

        /// Constructor
        /// @param ubo_bp the binding point to use for the generated uniform buffer.
        explicit Config(BindingPoint ubo_bp);

        /// Copy constructor
        Config(const Config&);

        /// Copy assignment
        /// @return this Config
        Config& operator=(const Config&);

        /// Destructor
        ~Config() override;

        /// The binding point to use for the generated uniform buffer.
        BindingPoint ubo_binding;

        /// The mapping from binding point to the index for the buffer size lookup.
        std::unordered_map<BindingPoint, uint32_t> bindpoint_to_size_index;

        /// Reflection for this class
        TINT_REFLECT(Config, ubo_binding, bindpoint_to_size_index);
    };

    /// Information produced about what the transform did.
    /// If there were no calls to the arrayLength() builtin, then no Result will
    /// be emitted.
    struct Result final : public Castable<Result, Data> {
        /// Constructor
        /// @param used_size_indices Indices into the UBO that are statically used.
        explicit Result(std::unordered_set<uint32_t> used_size_indices);

        /// Copy constructor
        Result(const Result&);

        /// Destructor
        ~Result() override;

        /// Indices into the UBO that are statically used.
        std::unordered_set<uint32_t> used_size_indices;
    };

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_ARRAY_LENGTH_FROM_UNIFORM_H_
