// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_TYPE_BUILTIN_STRUCTS_H_
#define SRC_TINT_LANG_CORE_TYPE_BUILTIN_STRUCTS_H_

// Forward declarations
namespace tint {
class SymbolTable;
}  // namespace tint
namespace tint::core::type {
class Manager;
class Struct;
class Type;
}  // namespace tint::core::type

namespace tint::core::type {

/// @param types the type manager
/// @param symbols the symbol table
/// @param ty the type of the `fract` and `whole` struct members.
/// @returns the builtin struct type for a modf() builtin call.
Struct* CreateModfResult(Manager& types, SymbolTable& symbols, const Type* ty);

/// @param types the type manager
/// @param symbols the symbol table
/// @param fract the type of the `fract` struct member.
/// @returns the builtin struct type for a frexp() builtin call.
Struct* CreateFrexpResult(Manager& types, SymbolTable& symbols, const Type* fract);

/// @param types the type manager
/// @param symbols the symbol table
/// @param ty the type of the `old_value` struct member.
/// @returns the builtin struct type for a atomic_compare_exchange() builtin call.
Struct* CreateAtomicCompareExchangeResult(Manager& types, SymbolTable& symbols, const Type* ty);

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_BUILTIN_STRUCTS_H_
