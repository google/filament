// Copyright 2019 The Dawn & Tint Authors
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

#include <cmath>

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/WGPUHelpers.h"

namespace {

class SamplerValidationTest : public ValidationTest {};

// Test NaN and INFINITY values are not allowed
TEST_F(SamplerValidationTest, InvalidLOD) {
    { device.CreateSampler(); }
    {
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.lodMinClamp = NAN;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
    }
    {
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.lodMaxClamp = NAN;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
    }
    {
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.lodMaxClamp = INFINITY;
        device.CreateSampler(&samplerDesc);
    }
    {
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.lodMaxClamp = INFINITY;
        samplerDesc.lodMinClamp = INFINITY;
        device.CreateSampler(&samplerDesc);
    }
}

TEST_F(SamplerValidationTest, InvalidFilterAnisotropic) {
    wgpu::SamplerDescriptor kValidAnisoSamplerDesc = {};
    kValidAnisoSamplerDesc.maxAnisotropy = 2;
    kValidAnisoSamplerDesc.minFilter = wgpu::FilterMode::Linear;
    kValidAnisoSamplerDesc.magFilter = wgpu::FilterMode::Linear;
    kValidAnisoSamplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    {
        // when maxAnisotropy > 1, min, mag, mipmap filter should be linear
        device.CreateSampler(&kValidAnisoSamplerDesc);
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.maxAnisotropy = 0;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.minFilter = wgpu::FilterMode::Nearest;
        samplerDesc.magFilter = wgpu::FilterMode::Nearest;
        samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.minFilter = wgpu::FilterMode::Nearest;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.magFilter = wgpu::FilterMode::Nearest;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
        ASSERT_DEVICE_ERROR(device.CreateSampler(&samplerDesc));
    }
}

TEST_F(SamplerValidationTest, ValidFilterAnisotropic) {
    wgpu::SamplerDescriptor kValidAnisoSamplerDesc = {};
    kValidAnisoSamplerDesc.maxAnisotropy = 2;
    kValidAnisoSamplerDesc.minFilter = wgpu::FilterMode::Linear;
    kValidAnisoSamplerDesc.magFilter = wgpu::FilterMode::Linear;
    kValidAnisoSamplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    { device.CreateSampler(); }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.maxAnisotropy = 16;
        device.CreateSampler(&samplerDesc);
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.maxAnisotropy = 32;
        device.CreateSampler(&samplerDesc);
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.maxAnisotropy = 0x7FFF;
        device.CreateSampler(&samplerDesc);
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.maxAnisotropy = 0x8000;
        device.CreateSampler(&samplerDesc);
    }
    {
        wgpu::SamplerDescriptor samplerDesc = kValidAnisoSamplerDesc;
        samplerDesc.maxAnisotropy = 0xFFFF;
        device.CreateSampler(&samplerDesc);
    }
}

TEST_F(SamplerValidationTest, ValidFilterAnisotropicWithUndefined) {
    wgpu::SamplerDescriptor kValidAnisoSamplerDesc = {};
    kValidAnisoSamplerDesc.maxAnisotropy = 2;
    kValidAnisoSamplerDesc.minFilter = wgpu::FilterMode::Undefined;
    kValidAnisoSamplerDesc.magFilter = wgpu::FilterMode::Undefined;
    kValidAnisoSamplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Undefined;
    { device.CreateSampler(); }
}

}  // anonymous namespace
