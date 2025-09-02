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

#ifndef SRC_TINT_LANG_WGSL_FEATURE_STATUS_H_
#define SRC_TINT_LANG_WGSL_FEATURE_STATUS_H_

#include <cstdint>

namespace tint::wgsl {

enum class LanguageFeature : uint8_t;

/// The status of the implementation of a WGSL language feature so that other components (like Dawn)
/// can query it. The enum values are in the order of least implemented to most implemented.
enum class FeatureStatus : uint8_t {
    // The feature is not known.
    kUnknown,
    // The feature is known in wgsl.def but not implemented at all.
    kUnimplemented,
    // The feature is at least partially implemented but might contain big security of correctness
    // issues.
    kUnsafeExperimental,
    // The feature is implemented and should be safe from a security standpoint, but shouldn't be
    // exposed by default.
    kExperimental,
    // The feature is implemented and can be exposed by default, but is only turned on if the
    // feature is explicitly enabled in the wgsl reader options.
    kShippedWithKillswitch,
    // The feature is exposed by default and cannot be turned off.
    kShipped,
};

/// @param f the feature to get the status of.
/// @returns the status, or kUnknown if the feature is not known.
FeatureStatus GetLanguageFeatureStatus(LanguageFeature f);

}  // namespace tint::wgsl

#endif  // SRC_TINT_LANG_WGSL_FEATURE_STATUS_H_
