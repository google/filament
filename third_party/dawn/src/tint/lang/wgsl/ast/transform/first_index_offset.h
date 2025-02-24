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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_FIRST_INDEX_OFFSET_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_FIRST_INDEX_OFFSET_H_

#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/utils/reflection.h"

namespace tint::ast::transform {

/// Adds firstVertex/Instance (injected via root constants) to
/// vertex/instance index builtins.
///
/// This transform assumes that Name transform has been run before.
///
/// Unlike other APIs, D3D always starts vertex and instance numbering at 0,
/// regardless of the firstVertex/Instance value specified. This transformer
/// adds the value of firstVertex/Instance to each builtin. This action is
/// performed by adding a new constant equal to original builtin +
/// firstVertex/Instance to each function that references one of these builtins.
///
/// Note that D3D does not have any semantics for firstVertex/Instance.
/// Therefore, these values must by passed to the shader.
///
/// Before:
/// ```
///   @builtin(vertex_index) var<in> vert_idx : u32;
///   fn func() -> u32 {
///     return vert_idx;
///   }
/// ```
///
/// After:
/// ```
///   struct TintFirstIndexOffsetData {
///     tint_first_vertex_index : u32;
///     tint_first_instance_index : u32;
///   };
///   @builtin(vertex_index) var<in> tint_first_index_offset_vert_idx : u32;
///   @binding(N) @group(M) var<uniform> tint_first_index_data :
///                                                    TintFirstIndexOffsetData;
///   fn func() -> u32 {
///     const vert_idx = (tint_first_index_offset_vert_idx +
///                       tint_first_index_data.tint_first_vertex_index);
///     return vert_idx;
///   }
/// ```
///
class FirstIndexOffset final : public Castable<FirstIndexOffset, Transform> {
  public:
    /// BindingPoint is consumed by the FirstIndexOffset transform.
    /// BindingPoint specifies the binding point of the first index uniform
    /// buffer.
    struct BindingPoint final : public Castable<BindingPoint, Data> {
        /// Constructor
        BindingPoint();

        /// Constructor
        /// @param b the binding index
        /// @param g the binding group
        BindingPoint(uint32_t b, uint32_t g);

        /// Destructor
        ~BindingPoint() override;

        /// `@binding()` for the first vertex / first instance uniform buffer
        uint32_t binding = 0;
        /// `@group()` for the first vertex / first instance uniform buffer
        uint32_t group = 0;

        /// Reflection for this struct
        TINT_REFLECT(BindingPoint, binding, group);
    };

    /// Data is outputted by the FirstIndexOffset transform.
    /// Data holds information about shader usage and constant buffer offsets.
    struct Data final : public Castable<Data, transform::Data> {
        /// Constructor
        Data();

        /// Constructor
        /// @param has_vtx_index True if the shader uses vertex_index
        /// @param has_inst_index True if the shader uses instance_index
        Data(bool has_vtx_index, bool has_inst_index);

        /// Copy constructor
        Data(const Data&);

        /// Destructor
        ~Data() override;

        /// True if the shader uses vertex_index
        bool has_vertex_index = false;
        /// True if the shader uses instance_index
        bool has_instance_index = false;

        /// Reflection for this struct
        TINT_REFLECT(Data, has_vertex_index, has_instance_index);
    };

    /// Constructor
    FirstIndexOffset();
    /// Destructor
    ~FirstIndexOffset() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program& program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    uint32_t binding_ = 0;
    uint32_t group_ = 0;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_FIRST_INDEX_OFFSET_H_
