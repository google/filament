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

#include "dawn/native/opengl/SamplerGL.h"

#include <algorithm>
#include <cstdint>

#include "dawn/common/Assert.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/UtilsGL.h"

namespace dawn::native::opengl {

namespace {
GLenum MagFilterMode(wgpu::FilterMode filter) {
    switch (filter) {
        case wgpu::FilterMode::Nearest:
            return GL_NEAREST;
        case wgpu::FilterMode::Linear:
            return GL_LINEAR;
        case wgpu::FilterMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

GLenum MinFilterMode(wgpu::FilterMode minFilter, wgpu::MipmapFilterMode mipMapFilter) {
    switch (minFilter) {
        case wgpu::FilterMode::Nearest:
            switch (mipMapFilter) {
                case wgpu::MipmapFilterMode::Nearest:
                    return GL_NEAREST_MIPMAP_NEAREST;
                case wgpu::MipmapFilterMode::Linear:
                    return GL_NEAREST_MIPMAP_LINEAR;
                case wgpu::MipmapFilterMode::Undefined:
                    DAWN_UNREACHABLE();
            }
        case wgpu::FilterMode::Linear:
            switch (mipMapFilter) {
                case wgpu::MipmapFilterMode::Nearest:
                    return GL_LINEAR_MIPMAP_NEAREST;
                case wgpu::MipmapFilterMode::Linear:
                    return GL_LINEAR_MIPMAP_LINEAR;
                case wgpu::MipmapFilterMode::Undefined:
                    DAWN_UNREACHABLE();
            }
        case wgpu::FilterMode::Undefined:
            DAWN_UNREACHABLE();
    }
    DAWN_UNREACHABLE();
}

GLenum WrapMode(wgpu::AddressMode mode) {
    switch (mode) {
        case wgpu::AddressMode::Repeat:
            return GL_REPEAT;
        case wgpu::AddressMode::MirrorRepeat:
            return GL_MIRRORED_REPEAT;
        case wgpu::AddressMode::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        case wgpu::AddressMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

}  // namespace

Sampler::Sampler(Device* device, const SamplerDescriptor* descriptor)
    : SamplerBase(device, descriptor) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    gl.GenSamplers(1, &mFilteringHandle);
    SetupGLSampler(mFilteringHandle, descriptor, false);

    gl.GenSamplers(1, &mNonFilteringHandle);
    SetupGLSampler(mNonFilteringHandle, descriptor, true);
}

Sampler::~Sampler() = default;

void Sampler::DestroyImpl() {
    SamplerBase::DestroyImpl();
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    gl.DeleteSamplers(1, &mFilteringHandle);
    gl.DeleteSamplers(1, &mNonFilteringHandle);
}

void Sampler::SetupGLSampler(GLuint sampler,
                             const SamplerDescriptor* descriptor,
                             bool forceNearest) {
    Device* device = ToBackend(GetDevice());
    const OpenGLFunctions& gl = device->GetGL();

    if (forceNearest) {
        gl.SamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl.SamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    } else {
        gl.SamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, MagFilterMode(descriptor->magFilter));
        gl.SamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER,
                             MinFilterMode(descriptor->minFilter, descriptor->mipmapFilter));
    }
    gl.SamplerParameteri(sampler, GL_TEXTURE_WRAP_R, WrapMode(descriptor->addressModeW));
    gl.SamplerParameteri(sampler, GL_TEXTURE_WRAP_S, WrapMode(descriptor->addressModeU));
    gl.SamplerParameteri(sampler, GL_TEXTURE_WRAP_T, WrapMode(descriptor->addressModeV));

    gl.SamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, descriptor->lodMinClamp);
    gl.SamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, descriptor->lodMaxClamp);

    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        gl.SamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        gl.SamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC,
                             ToOpenGLCompareFunction(descriptor->compare));
    }

    if (HasAnisotropicFiltering(gl)) {
        uint16_t value =
            std::min<uint16_t>(GetMaxAnisotropy(), device->GetMaxTextureMaxAnisotropy());
        gl.SamplerParameteri(sampler, GL_TEXTURE_MAX_ANISOTROPY, value);
    }
}

GLuint Sampler::GetFilteringHandle() const {
    return mFilteringHandle;
}

GLuint Sampler::GetNonFilteringHandle() const {
    return mNonFilteringHandle;
}

}  // namespace dawn::native::opengl
