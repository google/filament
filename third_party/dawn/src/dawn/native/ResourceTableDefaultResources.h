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

#ifndef SRC_DAWN_NATIVE_RESOURCETABLEDEFAULTRESOURCES_H_
#define SRC_DAWN_NATIVE_RESOURCETABLEDEFAULTRESOURCES_H_

#include <memory>
#include <variant>

#include "src/dawn/common/Ref.h"
#include "src/dawn/common/ityp_vector.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/Sampler.h"
#include "src/utils/non_movable.h"
#include "src/utils/span.h"

namespace tint {
enum class ResourceType : uint32_t;
}  // namespace tint

namespace dawn::native {

// Used to cache the default resources on the device so they can be reused between resource tables.
class ResourceTableDefaultResources : public NonMovable {
  public:
    static ResultOrError<std::unique_ptr<ResourceTableDefaultResources>> Create(DeviceBase* device);

    using Resource = std::variant<Ref<TextureViewBase>, Ref<SamplerBase>>;

    // Returns the order in which we will put the default bindings at the end of the resource table.
    static ityp::span<ResourceTableSlot, const tint::ResourceType> GetOrder();

    // Returns the total number of default bindings
    static ResourceTableSlot GetCount();

    // Returns the total number of sampler default bindings
    static ResourceTableSlot GetSamplerCount();

    // Returns the total number of non-sampler default bindings
    static ResourceTableSlot GetNonSamplerCount();

    // Returns the list of all default resources, in order.
    ityp::span<ResourceTableSlot, const Resource> GetResources() const;

    // Returns any sampler / sampleable texture when needed as placeholder.
    TextureViewBase* GetPlaceholderSampleableTexture() const;
    SamplerBase* GetPlaceholderSampler() const;

  private:
    MaybeError Initialize(DeviceBase* device);

    ityp::vector<ResourceTableSlot, Resource> mDefaultResources;
    Ref<TextureViewBase> mPlaceholderSampleableTexture;
    Ref<SamplerBase> mPlaceholderSampler;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RESOURCETABLEDEFAULTRESOURCES_H_
