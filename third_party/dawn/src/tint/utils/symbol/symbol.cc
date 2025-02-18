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

#include "src/tint/utils/symbol/symbol.h"

#include <utility>

namespace tint {

Symbol::Symbol() = default;

Symbol::Symbol(uint32_t val, tint::GenerationID pid, std::string_view name)
    : val_(val), generation_id_(pid), name_(name) {}

Symbol::Symbol(const Symbol& o) = default;

Symbol::Symbol(Symbol&& o) = default;

Symbol::~Symbol() = default;

Symbol& Symbol::operator=(const Symbol& o) = default;

Symbol& Symbol::operator=(Symbol&& o) = default;

bool Symbol::operator==(const Symbol& other) const {
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(generation_id_, other.generation_id_);
    return val_ == other.val_;
}

bool Symbol::operator!=(const Symbol& other) const {
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(generation_id_, other.generation_id_);
    return val_ != other.val_;
}

bool Symbol::operator<(const Symbol& other) const {
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(generation_id_, other.generation_id_);
    return val_ < other.val_;
}

std::string Symbol::to_str() const {
    return "$" + std::to_string(val_);
}

std::string_view Symbol::NameView() const {
    return name_;
}

std::string Symbol::Name() const {
    return std::string(name_);
}

}  // namespace tint
