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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_UNIFORMITY_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_UNIFORMITY_H_

// Forward declarations.
namespace tint::resolver {
struct DependencyGraph;
}  // namespace tint::resolver
namespace tint {
class ProgramBuilder;
}  // namespace tint

namespace tint::resolver {

/// If true, uniformity analysis failures will be treated as an error, else as a warning.
constexpr bool kUniformityFailuresAsError = true;

/// Analyze the uniformity of a program.
/// @param builder the program to analyze
/// @param dependency_graph the dependency-ordered module-scope declarations
/// @param subgroup_uniformity Whether subgroup_uniformity feature is supported
/// @returns true if there are no uniformity issues, false otherwise
bool AnalyzeUniformity(ProgramBuilder& builder,
                       const resolver::DependencyGraph& dependency_graph,
                       bool subgroup_uniformity = false);

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_UNIFORMITY_H_
