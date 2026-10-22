// Copyright 2026 The Dawn & Tint Authors
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

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <string_view>

#include "src/dawn/common/DRMUtils.h"

namespace dawn {
namespace {

struct RenderNodeCandidate {
    std::string_view name;
    bool isCharacterDevice;
    uint64_t major;
    uint64_t minor;
};

TEST(DRMUtilsTests, SelectsRenderNodeMatchingVulkanDevice) {
    constexpr uint64_t kDRMMajor = 226;
    constexpr uint64_t kNvidiaRenderMinor = 128;
    constexpr uint64_t kIntelRenderMinor = 129;
    constexpr std::array<RenderNodeCandidate, 7> kCandidates = {{
        {"card2", true, kDRMMajor, kIntelRenderMinor},
        {"renderD", true, kDRMMajor, kIntelRenderMinor},
        {"renderD129x", true, kDRMMajor, kIntelRenderMinor},
        {"renderD129", false, kDRMMajor, kIntelRenderMinor},
        {"renderD129", true, 1, kIntelRenderMinor},
        {"renderD128", true, kDRMMajor, kNvidiaRenderMinor},
        {"renderD129", true, kDRMMajor, kIntelRenderMinor},
    }};

    auto matchesIntel = [](const RenderNodeCandidate& candidate) {
        return IsMatchingDRMRenderNode(candidate.name, candidate.isCharacterDevice, candidate.major,
                                       candidate.minor, kDRMMajor, kIntelRenderMinor);
    };

    auto matchingNode = std::ranges::find_if(kCandidates, matchesIntel);
    ASSERT_NE(matchingNode, kCandidates.end());
    EXPECT_EQ(matchingNode->name, "renderD129");
    EXPECT_EQ(std::ranges::count_if(kCandidates, matchesIntel), 1);
}

}  // anonymous namespace
}  // namespace dawn
