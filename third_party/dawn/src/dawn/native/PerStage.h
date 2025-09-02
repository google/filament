// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_PERSTAGE_H_
#define SRC_DAWN_NATIVE_PERSTAGE_H_

#include <array>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Error.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

enum class SingleShaderStage { Vertex, Fragment, Compute };

static_assert(static_cast<uint32_t>(SingleShaderStage::Vertex) < kNumStages);
static_assert(static_cast<uint32_t>(SingleShaderStage::Fragment) < kNumStages);
static_assert(static_cast<uint32_t>(SingleShaderStage::Compute) < kNumStages);

static_assert(static_cast<uint32_t>(wgpu::ShaderStage::Vertex) ==
              (1 << static_cast<uint32_t>(SingleShaderStage::Vertex)));
static_assert(static_cast<uint32_t>(wgpu::ShaderStage::Fragment) ==
              (1 << static_cast<uint32_t>(SingleShaderStage::Fragment)));
static_assert(static_cast<uint32_t>(wgpu::ShaderStage::Compute) ==
              (1 << static_cast<uint32_t>(SingleShaderStage::Compute)));

ityp::bitset<SingleShaderStage, kNumStages> IterateStages(wgpu::ShaderStage stages);
wgpu::ShaderStage StageBit(SingleShaderStage stage);

static constexpr wgpu::ShaderStage kAllStages =
    static_cast<wgpu::ShaderStage>((1 << kNumStages) - 1);

template <typename T>
class PerStage {
  public:
    PerStage() = default;
    explicit PerStage(const T& initialValue) { mData.fill(initialValue); }

    T& operator[](SingleShaderStage stage) {
        DAWN_ASSERT(static_cast<uint32_t>(stage) < kNumStages);
        return mData[static_cast<uint32_t>(stage)];
    }
    const T& operator[](SingleShaderStage stage) const {
        DAWN_ASSERT(static_cast<uint32_t>(stage) < kNumStages);
        return mData[static_cast<uint32_t>(stage)];
    }

    T& operator[](wgpu::ShaderStage stageBit) {
        uint32_t bit = static_cast<uint32_t>(stageBit);
        DAWN_ASSERT(bit != 0 && IsPowerOfTwo(bit) && bit <= (1 << kNumStages));
        return mData[Log2(bit)];
    }
    const T& operator[](wgpu::ShaderStage stageBit) const {
        uint32_t bit = static_cast<uint32_t>(stageBit);
        DAWN_ASSERT(bit != 0 && IsPowerOfTwo(bit) && bit <= (1 << kNumStages));
        return mData[Log2(bit)];
    }

  private:
    std::array<T, kNumStages> mData;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PERSTAGE_H_
