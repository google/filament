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

#include "src/tint/lang/core/type/subgroup_matrix.h"

#include "src/tint/lang/core/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::SubgroupMatrix);

namespace tint::core::type {

SubgroupMatrix::SubgroupMatrix(SubgroupMatrixKind kind,
                               const type::Type* subtype,
                               uint32_t columns,
                               uint32_t rows)
    : Base(Hash(tint::TypeCode::Of<SubgroupMatrix>().bits, kind, columns, rows, subtype),
           core::type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }),
      kind_(kind),
      subtype_(subtype),
      columns_(columns),
      rows_(rows) {}

SubgroupMatrix::~SubgroupMatrix() = default;

bool SubgroupMatrix::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<SubgroupMatrix>()) {
        return v->kind_ == kind_ && v->rows_ == rows_ && v->columns_ == columns_ &&
               v->subtype_ == subtype_;
    }
    return false;
}

uint32_t SubgroupMatrix::Align() const {
    return subtype_->Align();
}

std::string SubgroupMatrix::FriendlyName() const {
    StringStream out;
    out << "subgroup_matrix_";
    switch (kind_) {
        case SubgroupMatrixKind::kLeft:
            out << "left";
            break;
        case SubgroupMatrixKind::kRight:
            out << "right";
            break;
        case SubgroupMatrixKind::kResult:
            out << "result";
            break;
        case SubgroupMatrixKind::kUndefined:
            TINT_UNREACHABLE();
    }
    out << "<" << subtype_->FriendlyName() << ", " << columns_ << ", " << rows_ << ">";
    return out.str();
}

SubgroupMatrix* SubgroupMatrix::Clone(CloneContext& ctx) const {
    auto* ty = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<SubgroupMatrix>(kind_, ty, columns_, rows_);
}

}  // namespace tint::core::type
