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

#ifndef SRC_TINT_LANG_WGSL_AST_DIAGNOSTIC_CONTROL_H_
#define SRC_TINT_LANG_WGSL_AST_DIAGNOSTIC_CONTROL_H_

#include <string>
#include <unordered_map>

#include "src/tint/lang/wgsl/diagnostic_severity.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

// Forward declarations
namespace tint::ast {
class DiagnosticRuleName;
}  // namespace tint::ast

namespace tint::ast {

/// A diagnostic control used for diagnostic directives and attributes.
struct DiagnosticControl {
  public:
    /// Default constructor.
    DiagnosticControl();

    /// Constructor
    /// @param sev the diagnostic severity
    /// @param rule the diagnostic rule name
    DiagnosticControl(wgsl::DiagnosticSeverity sev, const DiagnosticRuleName* rule);

    /// Move constructor
    DiagnosticControl(DiagnosticControl&&);

    /// The diagnostic severity control.
    wgsl::DiagnosticSeverity severity = wgsl::DiagnosticSeverity::kUndefined;

    /// The diagnostic rule name.
    const DiagnosticRuleName* rule_name = nullptr;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_DIAGNOSTIC_CONTROL_H_
