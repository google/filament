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

#include "dawn/wire/client/LimitsAndFeatures.h"

#include "dawn/common/Assert.h"
#include "dawn/wire/SupportedFeatures.h"

namespace dawn::wire::client {

LimitsAndFeatures::LimitsAndFeatures() = default;

LimitsAndFeatures::~LimitsAndFeatures() = default;

WGPUStatus LimitsAndFeatures::GetLimits(WGPUSupportedLimits* limits) const {
    DAWN_ASSERT(limits != nullptr);
    auto* originalNextInChain = limits->nextInChain;
    *limits = mLimits;
    limits->nextInChain = originalNextInChain;
    // Handle other requiring limits that chained after WGPUSupportedLimits
    for (auto* chain = limits->nextInChain; chain; chain = chain->next) {
        // Store the WGPUChainedStruct to restore the chain after assignment.
        WGPUChainedStruct originalChainedStruct = *chain;
        switch (chain->sType) {
            case (WGPUSType_DawnExperimentalSubgroupLimits): {
                auto* experimentalSubgroupLimits =
                    reinterpret_cast<WGPUDawnExperimentalSubgroupLimits*>(chain);
                // This assignment break the next field of WGPUChainedStruct head.
                *experimentalSubgroupLimits = mExperimentalSubgroupLimits;
                break;
            }
            case (WGPUSType_DawnExperimentalImmediateDataLimits): {
                auto* experimentalImmediateDataLimits =
                    reinterpret_cast<WGPUDawnExperimentalImmediateDataLimits*>(chain);
                // This assignment break the next field of WGPUChainedStruct head.
                *experimentalImmediateDataLimits = mExperimentalImmediateDataLimits;
                break;
            }
            case (WGPUSType_DawnTexelCopyBufferRowAlignmentLimits): {
                auto* texelCopyBufferRowAlignmentLimits =
                    reinterpret_cast<WGPUDawnTexelCopyBufferRowAlignmentLimits*>(chain);
                // This assignment break the next field of WGPUChainedStruct head.
                *texelCopyBufferRowAlignmentLimits = mTexelCopyBufferRowAlignmentLimits;
                break;
            }
            default:
                // Fail if unknown sType found.
                return WGPUStatus_Error;
        }
        // Restore the chain.
        *chain = originalChainedStruct;
    }
    return WGPUStatus_Success;
}

bool LimitsAndFeatures::HasFeature(WGPUFeatureName feature) const {
    return mFeatures.contains(feature);
}

void LimitsAndFeatures::ToSupportedFeatures(WGPUSupportedFeatures* supportedFeatures) const {
    if (!supportedFeatures) {
        return;
    }

    const size_t count = mFeatures.size();
    supportedFeatures->featureCount = count;
    supportedFeatures->features = nullptr;

    if (count == 0) {
        return;
    }

    // This will be freed by wgpuSupportedFeaturesFreeMembers.
    WGPUFeatureName* features = new WGPUFeatureName[count];
    uint32_t index = 0;
    for (WGPUFeatureName f : mFeatures) {
        features[index++] = f;
    }
    DAWN_ASSERT(index == count);
    supportedFeatures->features = features;
}

void LimitsAndFeatures::SetLimits(const WGPUSupportedLimits* limits) {
    DAWN_ASSERT(limits != nullptr);
    mLimits = *limits;
    mLimits.nextInChain = nullptr;
    // Handle other limits that chained after WGPUSupportedLimits
    for (auto* chain = limits->nextInChain; chain; chain = chain->next) {
        switch (chain->sType) {
            case (WGPUSType_DawnExperimentalSubgroupLimits): {
                // TODO(crbug.com/354751907) Remove this, as it is now in AdapterInfo.
                auto* experimentalSubgroupLimits =
                    reinterpret_cast<WGPUDawnExperimentalSubgroupLimits*>(chain);
                mExperimentalSubgroupLimits = *experimentalSubgroupLimits;
                mExperimentalSubgroupLimits.chain.next = nullptr;
                break;
            }
            case (WGPUSType_DawnExperimentalImmediateDataLimits): {
                auto* experimentalImmediateDataLimits =
                    reinterpret_cast<WGPUDawnExperimentalImmediateDataLimits*>(chain);
                mExperimentalImmediateDataLimits = *experimentalImmediateDataLimits;
                mExperimentalImmediateDataLimits.chain.next = nullptr;
                break;
            }
            case (WGPUSType_DawnTexelCopyBufferRowAlignmentLimits): {
                auto* texelCopyBufferRowAlignmentLimits =
                    reinterpret_cast<WGPUDawnTexelCopyBufferRowAlignmentLimits*>(chain);
                mTexelCopyBufferRowAlignmentLimits = *texelCopyBufferRowAlignmentLimits;
                mTexelCopyBufferRowAlignmentLimits.chain.next = nullptr;
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }
}

void LimitsAndFeatures::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    DAWN_ASSERT(features != nullptr || featuresCount == 0);
    for (uint32_t i = 0; i < featuresCount; ++i) {
        // Filter out features that the server supports, but the client does not.
        // (Could be different versions)
        if (!IsFeatureSupported(features[i])) {
            continue;
        }
        mFeatures.insert(features[i]);
    }
}

}  // namespace dawn::wire::client
