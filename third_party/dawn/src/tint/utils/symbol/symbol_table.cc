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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include "src/tint/utils/symbol/symbol_table.h"

#include "src/tint/utils/ice/ice.h"

namespace tint {

SymbolTable::SymbolTable() : generation_id_(GenerationID::New()) {}

SymbolTable::SymbolTable(GenerationID gen_id) : generation_id_(gen_id) {}

SymbolTable::SymbolTable(SymbolTable&&) = default;

SymbolTable::~SymbolTable() = default;

SymbolTable& SymbolTable::operator=(SymbolTable&&) = default;

Symbol SymbolTable::Register(std::string_view name) {
    TINT_ASSERT(!name.empty());

    auto& it = name_to_symbol_.GetOrAddZeroEntry(name);
    if (it.value) {
        return Symbol{it.value, generation_id_, it.key};
    }

    auto view = Allocate(name);
    it.key = view;
    it.value = next_symbol_++;
    return Symbol{it.value, generation_id_, view};
}

Symbol SymbolTable::Get(std::string_view name) const {
    if (auto* entry = name_to_symbol_.GetEntry(name)) {
        return Symbol{entry->value, generation_id_, entry->key};
    }
    return Symbol{};
}

Symbol SymbolTable::New(std::string_view prefix_view /* = "" */) {
    std::string prefix;
    if (prefix_view.empty()) {
        prefix = "tint_symbol";
    } else {
        prefix = std::string(prefix_view);
    }

    auto& it = name_to_symbol_.GetOrAddZeroEntry(prefix);
    if (it.value == 0) {
        // prefix is a unique name
        auto view = Allocate(prefix);
        it.key = view;
        it.value = next_symbol_++;
        return Symbol{it.value, generation_id_, view};
    }

    size_t& i = last_prefix_to_index_.GetOrAddZero(prefix);
    std::string name;
    do {
        ++i;
        name = prefix + "_" + std::to_string(i);
    } while (name_to_symbol_.Contains(name));

    auto view = Allocate(name);
    auto id = name_to_symbol_.Add(view, next_symbol_++);
    return Symbol{id.value, generation_id_, view};
}

std::string_view SymbolTable::Allocate(std::string_view name) {
    static_assert(sizeof(char) == 1);
    char* name_mem = Bitcast<char*>(name_allocator_.Allocate(name.length() + 1));
    TINT_ASSERT(name_mem != nullptr) << "failed to allocate memory for symbol's string";

    memcpy(name_mem, name.data(), name.length() + 1);
    return {name_mem, name.length()};
}

}  // namespace tint
