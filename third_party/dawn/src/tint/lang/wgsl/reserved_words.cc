// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/reserved_words.h"

namespace tint::wgsl {

bool IsReserved(std::string_view s) {
    return s == "NULL" || s == "Self" || s == "abstract" || s == "active" || s == "alignas" ||
           s == "alignof" || s == "as" || s == "asm" || s == "asm_fragment" || s == "async" ||
           s == "attribute" || s == "auto" || s == "await" || s == "become" || s == "cast" ||
           s == "catch" || s == "class" || s == "co_await" || s == "co_return" || s == "co_yield" ||
           s == "coherent" || s == "column_major" || s == "common" || s == "compile" ||
           s == "compile_fragment" || s == "concept" || s == "const_cast" || s == "consteval" ||
           s == "constexpr" || s == "constinit" || s == "crate" || s == "debugger" ||
           s == "decltype" || s == "delete" || s == "demote" || s == "demote_to_helper" ||
           s == "do" || s == "dynamic_cast" || s == "enum" || s == "explicit" || s == "export" ||
           s == "extends" || s == "extern" || s == "external" || s == "fallthrough" ||
           s == "filter" || s == "final" || s == "finally" || s == "friend" || s == "from" ||
           s == "fxgroup" || s == "get" || s == "goto" || s == "groupshared" || s == "highp" ||
           s == "impl" || s == "implements" || s == "import" || s == "inline" ||
           s == "instanceof" || s == "interface" || s == "layout" || s == "lowp" || s == "macro" ||
           s == "macro_rules" || s == "match" || s == "mediump" || s == "meta" || s == "mod" ||
           s == "module" || s == "move" || s == "mut" || s == "mutable" || s == "namespace" ||
           s == "new" || s == "nil" || s == "noexcept" || s == "noinline" ||
           s == "nointerpolation" || s == "non_coherent" || s == "noncoherent" ||
           s == "noperspective" || s == "null" || s == "nullptr" || s == "of" || s == "operator" ||
           s == "package" || s == "packoffset" || s == "partition" || s == "pass" || s == "patch" ||
           s == "pixelfragment" || s == "precise" || s == "precision" || s == "premerge" ||
           s == "priv" || s == "protected" || s == "pub" || s == "public" || s == "readonly" ||
           s == "ref" || s == "regardless" || s == "register" || s == "reinterpret_cast" ||
           s == "require" || s == "resource" || s == "restrict" || s == "self" || s == "set" ||
           s == "shared" || s == "sizeof" || s == "smooth" || s == "snorm" || s == "static" ||
           s == "static_assert" || s == "static_cast" || s == "std" || s == "subroutine" ||
           s == "super" || s == "target" || s == "template" || s == "this" || s == "thread_local" ||
           s == "throw" || s == "trait" || s == "try" || s == "type" || s == "typedef" ||
           s == "typeid" || s == "typename" || s == "typeof" || s == "union" || s == "unless" ||
           s == "unorm" || s == "unsafe" || s == "unsized" || s == "use" || s == "using" ||
           s == "varying" || s == "virtual" || s == "volatile" || s == "wgsl" || s == "where" ||
           s == "with" || s == "writeonly" || s == "yield";
}

}  // namespace tint::wgsl
