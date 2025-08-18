// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_TYPE_SUBGROUP_MATRIX_H_
#define SRC_TINT_LANG_CORE_TYPE_SUBGROUP_MATRIX_H_

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/clone_context.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::core::type {

/// A subgroup_matrix type
class SubgroupMatrix : public Castable<SubgroupMatrix, Type> {
  public:
    /// Constructor
    /// @param kind the kind of the matrix
    /// @param subtype the inner type of the matrix
    /// @param columns the number of columns in the matrix
    /// @param rows the number of rows in the matrix
    SubgroupMatrix(SubgroupMatrixKind kind, const Type* subtype, uint32_t columns, uint32_t rows);

    /// Destructor
    ~SubgroupMatrix() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the kind of the matrix
    SubgroupMatrixKind Kind() const { return kind_; }
    /// @returns the type of the matrix
    const type::Type* Type() const { return subtype_; }
    /// @returns the number of columns in the matrix
    uint32_t Columns() const { return columns_; }
    /// @returns the number of rows in the matrix
    uint32_t Rows() const { return rows_; }

    /// @returns the alignment in bytes of the type. This may include tail
    /// padding.
    uint32_t Align() const override;

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    SubgroupMatrix* Clone(CloneContext& ctx) const override;

  private:
    const SubgroupMatrixKind kind_;
    const type::Type* const subtype_;
    const uint32_t columns_;
    const uint32_t rows_;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_SUBGROUP_MATRIX_H_
