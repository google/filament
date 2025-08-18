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

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    Ref<Sampler> sampler = AcquireRef(new Sampler(device, descriptor));
    DAWN_TRY(sampler->Initialize(descriptor));
    return sampler;
}

Sampler::Sampler(Device* device, const SamplerDescriptor* descriptor)
    : SamplerBase(device, descriptor) {}

Sampler::~Sampler() = default;

void Sampler::DestroyImpl() {
    SamplerBase::DestroyImpl();
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    DAWN_GL_TRY_IGNORE_ERRORS(gl, DeleteSamplers(1, &mHandle));
}

MaybeError Sampler::Initialize(const SamplerDescriptor* descriptor) {
    Device* device = ToBackend(GetDevice());
    const OpenGLFunctions& gl = device->GetGL();

    DAWN_GL_TRY(gl, GenSamplers(1, &mHandle));

    DAWN_GL_TRY(gl, SamplerParameteri(mHandle, GL_TEXTURE_MAG_FILTER,
                                      MagFilterMode(descriptor->magFilter)));
    DAWN_GL_TRY(gl,
                SamplerParameteri(mHandle, GL_TEXTURE_MIN_FILTER,
                                  MinFilterMode(descriptor->minFilter, descriptor->mipmapFilter)));

    DAWN_GL_TRY(gl,
                SamplerParameteri(mHandle, GL_TEXTURE_WRAP_R, WrapMode(descriptor->addressModeW)));
    DAWN_GL_TRY(gl,
                SamplerParameteri(mHandle, GL_TEXTURE_WRAP_S, WrapMode(descriptor->addressModeU)));
    DAWN_GL_TRY(gl,
                SamplerParameteri(mHandle, GL_TEXTURE_WRAP_T, WrapMode(descriptor->addressModeV)));

    DAWN_GL_TRY(gl, SamplerParameterf(mHandle, GL_TEXTURE_MIN_LOD, descriptor->lodMinClamp));
    DAWN_GL_TRY(gl, SamplerParameterf(mHandle, GL_TEXTURE_MAX_LOD, descriptor->lodMaxClamp));

    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        DAWN_GL_TRY(gl,
                    SamplerParameteri(mHandle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
        DAWN_GL_TRY(gl, SamplerParameteri(mHandle, GL_TEXTURE_COMPARE_FUNC,
                                          ToOpenGLCompareFunction(descriptor->compare)));
    }

    if (HasAnisotropicFiltering(gl)) {
        uint16_t value =
            std::min<uint16_t>(GetMaxAnisotropy(), device->GetMaxTextureMaxAnisotropy());
        DAWN_GL_TRY(gl, SamplerParameteri(mHandle, GL_TEXTURE_MAX_ANISOTROPY, value));
    }

    return {};
}

GLuint Sampler::GetHandle() const {
    return mHandle;
}

}  // namespace dawn::native::opengl
