// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/buffer.h"

#include "src/tint/lang/core/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Buffer);

namespace tint::core::type {

const core::type::Flags sizedFlags{Flag::kHostShareable, Flag::kFixedFootprint};
const core::type::Flags unsizedFlags{Flag::kHostShareable};

Buffer::Buffer(const ArrayCount* size)
    : Base(Hash(tint::TypeCode::Of<Buffer>().bits, size),
           (size->Is<RuntimeArrayCount>() ? unsizedFlags : sizedFlags)),
      count_(size) {}

Buffer::~Buffer() = default;

bool Buffer::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<Buffer>()) {
        return count_ == v->count_;
    }
    return false;
}

uint32_t Buffer::Size() const {
    if (auto count = ConstantCount()) {
        return count.value();
    }
    return 0;
}

std::string Buffer::FriendlyName() const {
    StringStream out;
    out << "buffer";
    auto count_str = count_->FriendlyName();
    if (!count_str.empty()) {
        out << "<" << count_str << ">";
    }
    return out.str();
}

Buffer* Buffer::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<Buffer>(count_->Clone(ctx));
}

}  // namespace tint::core::type
