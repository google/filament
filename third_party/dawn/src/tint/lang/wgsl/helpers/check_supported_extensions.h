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

#ifndef SRC_TINT_LANG_WGSL_HELPERS_CHECK_SUPPORTED_EXTENSIONS_H_
#define SRC_TINT_LANG_WGSL_HELPERS_CHECK_SUPPORTED_EXTENSIONS_H_

#include "src/tint/lang/wgsl/extension.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::ast {
class Module;
}  // namespace tint::ast
namespace tint::diag {
class List;
}  // namespace tint::diag

namespace tint::wgsl {

/// Checks that all the extensions enabled in @p module are found in @p supported, raising an error
/// diagnostic if an enabled extension is not supported.
/// @param writer_name the name of the writer making this call
/// @param module the AST module
/// @param diags the diagnostics to append an error to, if needed.
/// @returns true if all extensions in use are supported, otherwise returns false.
bool CheckSupportedExtensions(std::string_view writer_name,
                              const ast::Module& module,
                              diag::List& diags,
                              VectorRef<wgsl::Extension> supported);

}  // namespace tint::wgsl

#endif  // SRC_TINT_LANG_WGSL_HELPERS_CHECK_SUPPORTED_EXTENSIONS_H_
