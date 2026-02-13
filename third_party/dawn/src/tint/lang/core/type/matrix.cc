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

#include "src/tint/lang/core/type/matrix.h"

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Matrix);

namespace tint::core::type {

Matrix::Matrix(const Vector* column_type, uint32_t columns)
    : Base(Hash(tint::TypeCode::Of<Vector>().bits, columns, column_type),
           core::type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
               Flag::kHostShareable,
           }),
      subtype_(column_type->Type()),
      column_type_(column_type),
      rows_(column_type->Width()),
      columns_(columns) {
    TINT_ASSERT(rows_ > 1);
    TINT_ASSERT(rows_ < 5);
    TINT_ASSERT(columns_ > 1);
    TINT_ASSERT(columns_ < 5);
}

Matrix::~Matrix() = default;

bool Matrix::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<Matrix>()) {
        return v->rows_ == rows_ && v->columns_ == columns_ && v->column_type_ == column_type_;
    }
    return false;
}

std::string Matrix::FriendlyName() const {
    StringStream out;
    out << "mat" << columns_ << "x" << rows_ << "<" << subtype_->FriendlyName() << ">";
    return out.str();
}

uint32_t Matrix::Size() const {
    return column_type_->Align() * Columns();
}

uint32_t Matrix::Align() const {
    return column_type_->Align();
}

uint32_t Matrix::ColumnStride() const {
    return column_type_->Align();
}

TypeAndCount Matrix::Elements(const type::Type* /* type_if_invalid = nullptr */,
                              uint32_t /* count_if_invalid = 0 */) const {
    return {column_type_, columns_};
}

const Vector* Matrix::Element(uint32_t index) const {
    return index < columns_ ? column_type_ : nullptr;
}

Matrix* Matrix::Clone(CloneContext& ctx) const {
    auto* col_ty = column_type_->Clone(ctx);
    return ctx.dst.mgr->Get<Matrix>(col_ty, columns_);
}

}  // namespace tint::core::type
