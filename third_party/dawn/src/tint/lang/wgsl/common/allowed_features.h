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

#ifndef SRC_TINT_LANG_WGSL_COMMON_ALLOWED_FEATURES_H_
#define SRC_TINT_LANG_WGSL_COMMON_ALLOWED_FEATURES_H_

#include <unordered_set>

#include "src/tint/lang/wgsl/extension.h"
#include "src/tint/lang/wgsl/features/language_feature.h"
#include "src/tint/utils/reflection.h"

namespace tint::wgsl {

/// AllowedFeatures describes the set of extensions and language features that are allowed by the
/// current environment.
struct AllowedFeatures {
    /// The extensions that are allowed.
    std::unordered_set<wgsl::Extension> extensions;
    /// The language features that are allowed.
    std::unordered_set<wgsl::LanguageFeature> features;

    /// Helper to produce an AllowedFeatures object that allows all extensions and features.
    /// @returns the AllowedFeatures object
    static AllowedFeatures Everything() {
        AllowedFeatures allowed_features;

        // Allow all extensions.
        for (auto extension : wgsl::kAllExtensions) {
            allowed_features.extensions.insert(extension);
        }

        // Allow all language features.
        for (auto feature : wgsl::kAllLanguageFeatures) {
            allowed_features.features.insert(feature);
        }

        return allowed_features;
    }

    /// Reflect the fields of this class so that it can be used by tint::ForeachField().
    TINT_REFLECT(AllowedFeatures, extensions, features);
};

}  // namespace tint::wgsl

#endif  // SRC_TINT_LANG_WGSL_COMMON_ALLOWED_FEATURES_H_
