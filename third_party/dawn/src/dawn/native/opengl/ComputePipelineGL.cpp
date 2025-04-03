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

#include "dawn/native/opengl/ComputePipelineGL.h"

#include "dawn/native/opengl/DeviceGL.h"

namespace dawn::native::opengl {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::DestroyImpl() {
    ComputePipelineBase::DestroyImpl();
    DeleteProgram(ToBackend(GetDevice())->GetGL());
}

MaybeError ComputePipeline::InitializeImpl() {
    return InitializeBase(ToBackend(GetDevice())->GetGL(), ToBackend(GetLayout()), GetAllStages(),
                          /* usesVertexIndex */ false, /* usesInstanceIndex */ false,
                          /* usesFragDepth */ false, /* bgraSwizzleAttributes */ {});
}

MaybeError ComputePipeline::ApplyNow() {
    DAWN_TRY(PipelineGL::ApplyNow(ToBackend(GetDevice())->GetGL()));
    return {};
}

}  // namespace dawn::native::opengl
