// Copyright 2021 The Dawn & Tint Authors
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

#include "src/dawn/wire/client/LimitsAndFeatures.h"

#include "dawn/wire/client/dawn_platform.h"
#include "src/dawn/wire/SupportedFeatures.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/heap_array.h"

namespace dawn::wire::client {

LimitsAndFeatures::LimitsAndFeatures() = default;

LimitsAndFeatures::~LimitsAndFeatures() = default;

wgpu::Status LimitsAndFeatures::GetLimits(Limits* limits) const {
    DAWN_ASSERT(limits != nullptr);
    auto* originalNextInChain = limits->nextInChain;
    *limits = mLimits;
    limits->nextInChain = originalNextInChain;
    // Handle other requiring limits that chained after Limits.
    for (auto* chain = limits->nextInChain; chain; chain = chain->nextInChain) {
        // Store the ChainedStruct to restore the chain after assignment.
        auto originalChainedStruct = *chain;
        switch (chain->sType) {
            case wgpu::SType::CompatibilityModeLimits: {
                *reinterpret_cast<CompatibilityModeLimits*>(chain) = mCompatLimits;
                break;
            }
            case wgpu::SType::DawnTexelCopyBufferRowAlignmentLimits: {
                *reinterpret_cast<DawnTexelCopyBufferRowAlignmentLimits*>(chain) =
                    mTexelCopyBufferRowAlignmentLimits;
                break;
            }
            default:
                // Fail if unknown sType found.
                return wgpu::Status::Error;
        }
        // Restore the chain (sType and next).
        *chain = originalChainedStruct;
    }
    return wgpu::Status::Success;
}

bool LimitsAndFeatures::HasFeature(wgpu::FeatureName feature) const {
    return mFeatures.contains(feature);
}

void LimitsAndFeatures::ToSupportedFeatures(SupportedFeatures* supportedFeatures) const {
    if (!supportedFeatures) {
        return;
    }

    // This will be freed by wgpuSupportedFeaturesFreeMembers.
    supportedFeatures->features = HeapArrayFrom(mFeatures).MoveToSpan();
}

void LimitsAndFeatures::SetLimits(const Limits* limits) {
    DAWN_ASSERT(limits != nullptr);
    mLimits = *limits;
    mLimits.nextInChain = nullptr;
    // Handle other limits that chained after WGPUSupportedLimits
    for (auto* chain = limits->nextInChain; chain; chain = chain->nextInChain) {
        switch (chain->sType) {
            case wgpu::SType::CompatibilityModeLimits: {
                mCompatLimits = *reinterpret_cast<CompatibilityModeLimits*>(chain);
                DAWN_ASSERT(mCompatLimits.sType == wgpu::SType::CompatibilityModeLimits);
                mCompatLimits.nextInChain = nullptr;
                break;
            }
            case wgpu::SType::DawnTexelCopyBufferRowAlignmentLimits: {
                mTexelCopyBufferRowAlignmentLimits =
                    *reinterpret_cast<DawnTexelCopyBufferRowAlignmentLimits*>(chain);
                DAWN_ASSERT(mTexelCopyBufferRowAlignmentLimits.sType ==
                            wgpu::SType::DawnTexelCopyBufferRowAlignmentLimits);
                mTexelCopyBufferRowAlignmentLimits.nextInChain = nullptr;
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }
}

void LimitsAndFeatures::SetFeatures(Span<const wgpu::FeatureName> features) {
    for (wgpu::FeatureName feature : features) {
        // Filter out features that the server supports, but the client does not.
        // (Could be different versions)
        if (!IsFeatureSupported(ToAPI(feature))) {
            continue;
        }
        mFeatures.insert(feature);
    }
}

}  // namespace dawn::wire::client
