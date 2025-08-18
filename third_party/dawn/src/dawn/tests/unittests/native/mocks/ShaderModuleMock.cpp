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

#include "dawn/tests/unittests/native/mocks/ShaderModuleMock.h"

#include <memory>
#include <utility>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/ShaderModuleParseRequest.h"

namespace dawn::native {

using ::testing::NiceMock;

ShaderModuleMock::ShaderModuleMock(DeviceMock* device,
                                   const UnpackedPtr<ShaderModuleDescriptor>& descriptor)
    : ShaderModuleBase(device, descriptor, {}) {
    ON_CALL(*this, DestroyImpl).WillByDefault([this] { this->ShaderModuleBase::DestroyImpl(); });

    SetContentHash(ComputeContentHash());
}

ShaderModuleMock::~ShaderModuleMock() = default;

// static
Ref<ShaderModuleMock> ShaderModuleMock::Create(
    DeviceMock* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor) {

    Ref<ShaderModuleMock> shaderModule =
        AcquireRef(new NiceMock<ShaderModuleMock>(device, descriptor));

    ShaderModuleParseResult parseResult =
        ParseShaderModule(BuildShaderModuleParseRequest(device, shaderModule->GetHash(), descriptor,
                                                        {},
                                                        /* needReflection*/ true))
            .AcquireSuccess();

    shaderModule->InitializeBase(&parseResult).AcquireSuccess();
    return shaderModule;
}

// static
Ref<ShaderModuleMock> ShaderModuleMock::Create(DeviceMock* device,
                                               const ShaderModuleDescriptor* descriptor) {
    return Create(device, Unpack(descriptor));
}

// static
Ref<ShaderModuleMock> ShaderModuleMock::Create(DeviceMock* device, const char* source) {
    ShaderSourceWGSL wgslDesc = {};
    wgslDesc.code = source;
    ShaderModuleDescriptor desc = {};
    desc.nextInChain = &wgslDesc;
    return Create(device, &desc);
}

}  // namespace dawn::native
