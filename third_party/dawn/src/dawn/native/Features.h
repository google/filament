// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_FEATURES_H_
#define SRC_DAWN_NATIVE_FEATURES_H_

#include <webgpu/webgpu_cpp.h>

#include <bitset>

#include "dawn/common/ityp_bitset.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/Features_autogen.h"

namespace dawn::native {

extern const ityp::array<Feature, FeatureInfo, kEnumCount<Feature>> kFeatureNameAndInfoList;

wgpu::FeatureName ToAPI(Feature feature);
Feature FromAPI(wgpu::FeatureName feature);

// A wrapper of the bitset to store if an feature is enabled or not. This wrapper provides the
// convenience to convert the enums of enum class Feature to the indices of a bitset.
struct FeaturesSet {
    ityp::bitset<Feature, kEnumCount<Feature>> featuresBitSet;

    void EnableFeature(Feature feature);
    void EnableFeature(wgpu::FeatureName feature);
    bool IsEnabled(Feature feature) const;
    bool IsEnabled(wgpu::FeatureName feature) const;
    void ToSupportedFeatures(SupportedFeatures* supportedFeatures) const;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_FEATURES_H_
