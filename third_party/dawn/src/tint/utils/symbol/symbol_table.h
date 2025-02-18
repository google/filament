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

#ifndef SRC_TINT_UTILS_SYMBOL_SYMBOL_TABLE_H_
#define SRC_TINT_UTILS_SYMBOL_SYMBOL_TABLE_H_

#include <string>

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/memory/bump_allocator.h"
#include "src/tint/utils/symbol/symbol.h"

namespace tint {

/// Holds mappings from symbols to their associated string names
class SymbolTable {
  public:
    /// Constructor
    /// @param generation_id the identifier of the program that owns this symbol
    /// table
    explicit SymbolTable(tint::GenerationID generation_id);
    /// Move Constructor
    SymbolTable(SymbolTable&&);
    /// Destructor
    ~SymbolTable();

    /// Move assignment
    /// @param other the symbol table to move
    /// @returns the symbol table
    SymbolTable& operator=(SymbolTable&& other);

    /// @returns a symbol table to hold symbols which point to the allocated names in @p o.
    /// The symbol table after Wrap is intended to temporarily extend the objects of an existing
    /// immutable SymbolTable.
    /// @warning As the copied objects are owned by @p o, @p o must not be destructed or assigned
    /// while using this symbol table.
    /// @param o the immutable SymbolTable to extend
    static SymbolTable Wrap(const SymbolTable& o) {
        SymbolTable out(o.generation_id_);
        out.next_symbol_ = o.next_symbol_;
        out.name_to_symbol_ = o.name_to_symbol_;
        out.last_prefix_to_index_ = o.last_prefix_to_index_;
        out.generation_id_ = o.generation_id_;
        return out;
    }

    /// Registers a name into the symbol table, returning the Symbol.
    /// @param name the name to register
    /// @returns the symbol representing the given name
    Symbol Register(std::string_view name);

    /// Returns the symbol for the given `name`
    /// @param name the name to lookup
    /// @returns the symbol for the name or Symbol() if not found.
    Symbol Get(std::string_view name) const;

    /// Returns a new unique symbol with the given name, possibly suffixed with a
    /// unique number.
    /// @param name the symbol name
    /// @returns a new, unnamed symbol with the given name. If the name is already
    /// taken then this will be suffixed with an underscore and a unique numerical
    /// value
    Symbol New(std::string_view name = "");

    /// Foreach calls the callback function `F` for each symbol in the table.
    /// @param callback must be a function or function-like object with the
    /// signature: `void(Symbol)`
    template <typename F>
    void Foreach(F&& callback) const {
        for (auto& it : name_to_symbol_) {
            callback(Symbol{it.value, generation_id_, it.key});
        }
    }

    /// @returns the identifier of the Program that owns this symbol table.
    tint::GenerationID GenerationID() const { return generation_id_; }

  private:
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable& other) = delete;

    std::string_view Allocate(std::string_view name);

    // The value to be associated to the next registered symbol table entry.
    uint32_t next_symbol_ = 1;

    Hashmap<std::string_view, uint32_t, 0> name_to_symbol_;
    Hashmap<std::string, size_t, 0> last_prefix_to_index_;
    tint::GenerationID generation_id_;

    tint::BumpAllocator name_allocator_;
};

/// @param symbol_table the SymbolTable
/// @returns the GenerationID that owns the given SymbolTable
inline GenerationID GenerationIDOf(const SymbolTable& symbol_table) {
    return symbol_table.GenerationID();
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_SYMBOL_SYMBOL_TABLE_H_
