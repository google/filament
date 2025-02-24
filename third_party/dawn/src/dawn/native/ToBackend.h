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

#ifndef SRC_DAWN_NATIVE_TOBACKEND_H_
#define SRC_DAWN_NATIVE_TOBACKEND_H_

#include "dawn/native/Forward.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// ToBackendTraits implements the mapping from base type to member type of BackendTraits
template <typename T, typename BackendTraits>
struct ToBackendTraits;

template <typename BackendTraits>
struct ToBackendTraits<PhysicalDeviceBase, BackendTraits> {
    using BackendType = typename BackendTraits::PhysicalDeviceType;
};

template <typename BackendTraits>
struct ToBackendTraits<BindGroupBase, BackendTraits> {
    using BackendType = typename BackendTraits::BindGroupType;
};

template <typename BackendTraits>
struct ToBackendTraits<BindGroupLayoutInternalBase, BackendTraits> {
    using BackendType = typename BackendTraits::BindGroupLayoutType;
};

template <typename BackendTraits>
struct ToBackendTraits<BufferBase, BackendTraits> {
    using BackendType = typename BackendTraits::BufferType;
};

template <typename BackendTraits>
struct ToBackendTraits<CommandBufferBase, BackendTraits> {
    using BackendType = typename BackendTraits::CommandBufferType;
};

template <typename BackendTraits>
struct ToBackendTraits<ComputePipelineBase, BackendTraits> {
    using BackendType = typename BackendTraits::ComputePipelineType;
};

template <typename BackendTraits>
struct ToBackendTraits<DeviceBase, BackendTraits> {
    using BackendType = typename BackendTraits::DeviceType;
};

template <typename BackendTraits>
struct ToBackendTraits<PipelineCacheBase, BackendTraits> {
    using BackendType = typename BackendTraits::PipelineCacheType;
};

template <typename BackendTraits>
struct ToBackendTraits<PipelineLayoutBase, BackendTraits> {
    using BackendType = typename BackendTraits::PipelineLayoutType;
};

template <typename BackendTraits>
struct ToBackendTraits<QuerySetBase, BackendTraits> {
    using BackendType = typename BackendTraits::QuerySetType;
};

template <typename BackendTraits>
struct ToBackendTraits<QueueBase, BackendTraits> {
    using BackendType = typename BackendTraits::QueueType;
};

template <typename BackendTraits>
struct ToBackendTraits<RenderPipelineBase, BackendTraits> {
    using BackendType = typename BackendTraits::RenderPipelineType;
};

template <typename BackendTraits>
struct ToBackendTraits<ResourceHeapBase, BackendTraits> {
    using BackendType = typename BackendTraits::ResourceHeapType;
};

template <typename BackendTraits>
struct ToBackendTraits<SamplerBase, BackendTraits> {
    using BackendType = typename BackendTraits::SamplerType;
};

template <typename BackendTraits>
struct ToBackendTraits<ShaderModuleBase, BackendTraits> {
    using BackendType = typename BackendTraits::ShaderModuleType;
};

template <typename BackendTraits>
struct ToBackendTraits<SharedFenceBase, BackendTraits> {
    using BackendType = typename BackendTraits::SharedFenceType;
};

template <typename BackendTraits>
struct ToBackendTraits<SharedTextureMemoryBase, BackendTraits> {
    using BackendType = typename BackendTraits::SharedTextureMemoryType;
};

template <typename BackendTraits>
struct ToBackendTraits<TextureBase, BackendTraits> {
    using BackendType = typename BackendTraits::TextureType;
};

template <typename BackendTraits>
struct ToBackendTraits<SwapChainBase, BackendTraits> {
    using BackendType = typename BackendTraits::SwapChainType;
};

template <typename BackendTraits>
struct ToBackendTraits<TextureViewBase, BackendTraits> {
    using BackendType = typename BackendTraits::TextureViewType;
};

// ToBackendBase implements conversion to the given BackendTraits
// To use it in a backend, use the following:
//   template<typename T>
//   auto ToBackend(T&& common) -> decltype(ToBackendBase<MyBackendTraits>(common)) {
//       return ToBackendBase<MyBackendTraits>(common);
//   }

template <typename BackendTraits, typename T>
Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>& ToBackendBase(Ref<T>& common) {
    return reinterpret_cast<Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>&>(common);
}

template <typename BackendTraits, typename T>
Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>&& ToBackendBase(Ref<T>&& common) {
    return reinterpret_cast<Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>&&>(common);
}

template <typename BackendTraits, typename T>
const Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>& ToBackendBase(
    const Ref<T>& common) {
    return reinterpret_cast<const Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>&>(
        common);
}

template <typename BackendTraits, typename T>
typename ToBackendTraits<T, BackendTraits>::BackendType* ToBackendBase(T* common) {
    return reinterpret_cast<typename ToBackendTraits<T, BackendTraits>::BackendType*>(common);
}

template <typename BackendTraits, typename T>
typename ToBackendTraits<T, BackendTraits>::BackendType* ToBackendBase(raw_ptr<T> common) {
    return reinterpret_cast<typename ToBackendTraits<T, BackendTraits>::BackendType*>(common.get());
}

template <typename BackendTraits, typename T>
const typename ToBackendTraits<T, BackendTraits>::BackendType* ToBackendBase(const T* common) {
    return reinterpret_cast<const typename ToBackendTraits<T, BackendTraits>::BackendType*>(common);
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TOBACKEND_H_
