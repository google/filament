

// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_WEBGPU_COMMANDBUFFERHELPERS_H_
#define SRC_DAWN_NATIVE_WEBGPU_COMMANDBUFFERHELPERS_H_

#include <vector>

#include "dawn/native/Commands.h"

namespace dawn::native {

class BindGroupBase;
class CommandIterator;
class ComputerPipelineBase;
class RenderBundleBase;
class RenderPipelineBase;

}  // namespace dawn::native

namespace dawn::native::webgpu {

class CaptureContext;

// Note: These are fine to be pointers and not Refs as this object
// does not outlast a CommandBuffer which itself uses Refs.
struct CommandBufferResourceUsages {
    std::vector<ComputePipelineBase*> computePipelines;
    std::vector<RenderPipelineBase*> renderPipelines;
    std::vector<BindGroupBase*> bindGroups;
    std::vector<RenderBundleBase*> renderBundles;
};

void CaptureSharedCommand(CaptureContext& captureContext, CommandIterator& commands, Command type);
void CaptureDebugCommand(CaptureContext& captureContext, CommandIterator& commands, Command type);

// Captures shared commands from both render passes and render bundles
MaybeError CaptureRenderCommand(CaptureContext& captureContext,
                                CommandIterator& commands,
                                Command type);
// Gathers resources from shared commands from both render passes and render bundles
MaybeError GatherReferencedResourcesFromRenderCommand(CaptureContext& captureContext,
                                                      CommandIterator& commands,
                                                      CommandBufferResourceUsages& usedResources,
                                                      Command type);
MaybeError AddUsedResources(CaptureContext& captureContext,
                            const CommandBufferResourceUsages& usedResources);

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_COMMANDBUFFERHELPERS_H_
