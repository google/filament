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

#ifndef SRC_DAWN_NATIVE_CHAINUTILSIMPL_INL_
#define SRC_DAWN_NATIVE_CHAINUTILSIMPL_INL_

namespace dawn::native {

struct DawnInstanceDescriptor;

namespace d3d {
struct RequestAdapterOptionsLUID;
}

namespace d3d11 {
struct RequestAdapterOptionsD3D11Device;
struct SharedTextureMemoryD3D11Texture2DDescriptor;
}

namespace d3d12 {
struct SharedBufferMemoryD3D12ResourceDescriptor;
}

namespace opengl {
struct RequestAdapterOptionsGetGLProc;
}

namespace vulkan {
struct YCbCrVulkanDescriptor;
}

namespace detail {

template <>
constexpr inline wgpu::SType STypeForImpl<DawnInstanceDescriptor> =
    wgpu::SType(WGPUSType_DawnInstanceDescriptor);

template <>
struct AdditionalExtensions<InstanceDescriptor> {
    using List = AdditionalExtensionsList<const DawnInstanceDescriptor*>;
};

template <>
constexpr inline wgpu::SType STypeForImpl<d3d::RequestAdapterOptionsLUID> =
    wgpu::SType(WGPUSType_RequestAdapterOptionsLUID);

template <>
constexpr inline wgpu::SType STypeForImpl<d3d11::RequestAdapterOptionsD3D11Device> =
    wgpu::SType(WGPUSType_RequestAdapterOptionsD3D11Device);

template <>
constexpr inline wgpu::SType STypeForImpl<opengl::RequestAdapterOptionsGetGLProc> =
    wgpu::SType(WGPUSType_RequestAdapterOptionsGetGLProc);

template <>
struct AdditionalExtensions<RequestAdapterOptions> {
    using List = AdditionalExtensionsList<const d3d::RequestAdapterOptionsLUID*,
                                          const d3d11::RequestAdapterOptionsD3D11Device*,
                                          const opengl::RequestAdapterOptionsGetGLProc*>;
};

template <>
constexpr inline wgpu::SType STypeForImpl<d3d11::SharedTextureMemoryD3D11Texture2DDescriptor> =
    wgpu::SType(WGPUSType_SharedTextureMemoryD3D11Texture2DDescriptor);

template <>
struct AdditionalExtensions<SharedTextureMemoryDescriptor> {
    using List =
        AdditionalExtensionsList<const d3d11::SharedTextureMemoryD3D11Texture2DDescriptor*>;
};

template <>
constexpr inline wgpu::SType STypeForImpl<d3d12::SharedBufferMemoryD3D12ResourceDescriptor> =
    wgpu::SType(WGPUSType_SharedBufferMemoryD3D12ResourceDescriptor);

template <>
struct AdditionalExtensions<SharedBufferMemoryDescriptor> {
    using List =
        AdditionalExtensionsList<const d3d12::SharedBufferMemoryD3D12ResourceDescriptor*>;
};

}  // namespace detail
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CHAINUTILSIMPL_INL_
