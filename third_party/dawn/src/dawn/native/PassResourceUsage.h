// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_PASSRESOURCEUSAGE_H_
#define SRC_DAWN_NATIVE_PASSRESOURCEUSAGE_H_

#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/native/SubresourceStorage.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

// This file declares various "ResourceUsage" structures. They are produced by the frontend
// while recording commands to be used for later validation and also some operations in the
// backends. The are produced by the "Encoder" objects that finalize them on "EndPass" or
// "Finish". Internally the "Encoder" may use the "StateTracker" to create them.

class BufferBase;
class QuerySetBase;
class TextureBase;

// Info about how a buffer is used and in which shader stages
struct BufferSyncInfo {
    wgpu::BufferUsage usage = wgpu::BufferUsage::None;
    wgpu::ShaderStage shaderStages = wgpu::ShaderStage::None;
};

// Info about how a texture is used and in which shader stages
// TODO(crbug.com/dawn/851): Optimize by merging into one u32?
struct TextureSyncInfo {
    wgpu::TextureUsage usage = wgpu::TextureUsage::None;
    wgpu::ShaderStage shaderStages = wgpu::ShaderStage::None;
    bool operator==(const TextureSyncInfo& b) const = default;
};

// The texture usage inside passes must be tracked per-subresource.
using TextureSubresourceSyncInfo = SubresourceStorage<TextureSyncInfo>;

// Which resources are used by a synchronization scope and how they are used. The command
// buffer validation pre-computes this information so that backends with explicit barriers
// don't have to re-compute it.
struct SyncScopeResourceUsage {
    std::vector<BufferBase*> buffers;
    std::vector<BufferSyncInfo> bufferSyncInfos;

    std::vector<TextureBase*> textures;
    std::vector<TextureSubresourceSyncInfo> textureSyncInfos;

    std::vector<ExternalTextureBase*> externalTextures;
};

// Contains all the resource usage data for a compute pass.
//
// Essentially a list of SyncScopeResourceUsage, one per Dispatch as required by the WebGPU
// specification. ComputePassResourceUsage also stores nline the set of all buffers and
// textures used, because some unused BindGroups may not be used at all in synchronization
// scope but their resources still need to be validated on Queue::Submit.
struct ComputePassResourceUsage {
    std::vector<SyncScopeResourceUsage> dispatchUsages;

    // All the resources referenced by this compute pass for validation in Queue::Submit.
    absl::flat_hash_set<BufferBase*> referencedBuffers;
    absl::flat_hash_set<TextureBase*> referencedTextures;
    absl::flat_hash_set<ExternalTextureBase*> referencedExternalTextures;
};

// Contains all the resource usage data for a render pass.
//
// In the WebGPU specification render passes are synchronization scopes but we also need to
// track additional data. It is stored for render passes used by a CommandBuffer, but also in
// RenderBundle so they can be merged into the render passes' usage on ExecuteBundles().
struct RenderPassResourceUsage : public SyncScopeResourceUsage {
    // Storage to track the occlusion queries used during the pass.
    std::vector<QuerySetBase*> querySets;
    std::vector<std::vector<bool>> queryAvailabilities;
};

using RenderPassUsages = std::vector<RenderPassResourceUsage>;
using ComputePassUsages = std::vector<ComputePassResourceUsage>;

// Contains a hierarchy of "ResourceUsage" that mirrors the hierarchy of the CommandBuffer and
// is used for validation and to produce barriers and lazy clears in the backends.
struct CommandBufferResourceUsage {
    RenderPassUsages renderPasses;
    ComputePassUsages computePasses;

    // Resources used in commands that aren't in a pass.
    absl::flat_hash_set<BufferBase*> topLevelBuffers;
    absl::flat_hash_set<TextureBase*> topLevelTextures;
    absl::flat_hash_set<QuerySetBase*> usedQuerySets;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PASSRESOURCEUSAGE_H_
