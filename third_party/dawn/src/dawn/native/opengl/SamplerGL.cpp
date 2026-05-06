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

void Sampler::DestroyImpl(DestroyReason reason) {
    SamplerBase::DestroyImpl(reason);
    IgnoreErrors(ToBackend(GetDevice())
                     ->EnqueueDestroyGL(this, &Sampler::GetHandle, reason,
                                        [](const OpenGLFunctions& gl, GLuint handle) -> MaybeError {
                                            DAWN_GL_TRY_IGNORE_ERRORS(gl,
                                                                      DeleteSamplers(1, &handle));
                                            return {};
                                        }));
}

MaybeError Sampler::Initialize(const SamplerDescriptor* descriptor) {
    Device* device = ToBackend(GetDevice());

    return device->EnqueueGL([self = Ref<Sampler>(this)](const OpenGLFunctions& gl) -> MaybeError {
        GLuint handle;
        DAWN_GL_TRY(gl, GenSamplers(1, &handle));

        self->mHandle = handle;

        DAWN_GL_TRY(gl, SamplerParameteri(handle, GL_TEXTURE_MAG_FILTER,
                                          MagFilterMode(self->GetMagFilter())));
        DAWN_GL_TRY(
            gl, SamplerParameteri(handle, GL_TEXTURE_MIN_FILTER,
                                  MinFilterMode(self->GetMinFilter(), self->GetMipmapFilter())));

        DAWN_GL_TRY(
            gl, SamplerParameteri(handle, GL_TEXTURE_WRAP_R, WrapMode(self->GetAddressModeW())));
        DAWN_GL_TRY(
            gl, SamplerParameteri(handle, GL_TEXTURE_WRAP_S, WrapMode(self->GetAddressModeU())));
        DAWN_GL_TRY(
            gl, SamplerParameteri(handle, GL_TEXTURE_WRAP_T, WrapMode(self->GetAddressModeV())));

        DAWN_GL_TRY(gl, SamplerParameterf(handle, GL_TEXTURE_MIN_LOD, self->GetLodMinClamp()));
        DAWN_GL_TRY(gl, SamplerParameterf(handle, GL_TEXTURE_MAX_LOD, self->GetLodMaxClamp()));

        if (self->GetCompareFunction() != wgpu::CompareFunction::Undefined) {
            DAWN_GL_TRY(
                gl, SamplerParameteri(handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
            DAWN_GL_TRY(gl, SamplerParameteri(handle, GL_TEXTURE_COMPARE_FUNC,
                                              ToOpenGLCompareFunction(self->GetCompareFunction())));
        }

        if (HasAnisotropicFiltering(gl)) {
            auto maxAnisotropy =
                std::min<uint16_t>(self->GetMaxAnisotropy(),
                                   ToBackend(self->GetDevice())->GetMaxTextureMaxAnisotropy());

            DAWN_GL_TRY(gl, SamplerParameteri(handle, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy));
        }

        return {};
    });
}

GLuint Sampler::GetHandle() const {
    return mHandle;
}

}  // namespace dawn::native::opengl
