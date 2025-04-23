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

#include "dawn/native/opengl/CommandBufferGL.h"

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupTracker.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/opengl/BindingPoint.h"
#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/ComputePipelineGL.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/Forward.h"
#include "dawn/native/opengl/PersistentPipelineStateGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/QuerySetGL.h"
#include "dawn/native/opengl/RenderPipelineGL.h"
#include "dawn/native/opengl/SamplerGL.h"
#include "dawn/native/opengl/TextureGL.h"
#include "dawn/native/opengl/UtilsGL.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::opengl {

namespace {

GLenum IndexFormatType(wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Uint16:
            return GL_UNSIGNED_SHORT;
        case wgpu::IndexFormat::Uint32:
            return GL_UNSIGNED_INT;
        case wgpu::IndexFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

GLenum VertexFormatType(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8:
        case wgpu::VertexFormat::Uint8x2:
        case wgpu::VertexFormat::Uint8x4:
        case wgpu::VertexFormat::Unorm8:
        case wgpu::VertexFormat::Unorm8x2:
        case wgpu::VertexFormat::Unorm8x4:
        case wgpu::VertexFormat::Unorm8x4BGRA:
            return GL_UNSIGNED_BYTE;
        case wgpu::VertexFormat::Sint8:
        case wgpu::VertexFormat::Sint8x2:
        case wgpu::VertexFormat::Sint8x4:
        case wgpu::VertexFormat::Snorm8:
        case wgpu::VertexFormat::Snorm8x2:
        case wgpu::VertexFormat::Snorm8x4:
            return GL_BYTE;
        case wgpu::VertexFormat::Uint16:
        case wgpu::VertexFormat::Uint16x2:
        case wgpu::VertexFormat::Uint16x4:
        case wgpu::VertexFormat::Unorm16:
        case wgpu::VertexFormat::Unorm16x2:
        case wgpu::VertexFormat::Unorm16x4:
            return GL_UNSIGNED_SHORT;
        case wgpu::VertexFormat::Sint16:
        case wgpu::VertexFormat::Sint16x2:
        case wgpu::VertexFormat::Sint16x4:
        case wgpu::VertexFormat::Snorm16:
        case wgpu::VertexFormat::Snorm16x2:
        case wgpu::VertexFormat::Snorm16x4:
            return GL_SHORT;
        case wgpu::VertexFormat::Float16:
        case wgpu::VertexFormat::Float16x2:
        case wgpu::VertexFormat::Float16x4:
            return GL_HALF_FLOAT;
        case wgpu::VertexFormat::Float32:
        case wgpu::VertexFormat::Float32x2:
        case wgpu::VertexFormat::Float32x3:
        case wgpu::VertexFormat::Float32x4:
            return GL_FLOAT;
        case wgpu::VertexFormat::Uint32:
        case wgpu::VertexFormat::Uint32x2:
        case wgpu::VertexFormat::Uint32x3:
        case wgpu::VertexFormat::Uint32x4:
            return GL_UNSIGNED_INT;
        case wgpu::VertexFormat::Sint32:
        case wgpu::VertexFormat::Sint32x2:
        case wgpu::VertexFormat::Sint32x3:
        case wgpu::VertexFormat::Sint32x4:
            return GL_INT;
        case wgpu::VertexFormat::Unorm10_10_10_2:
            return GL_UNSIGNED_INT_2_10_10_10_REV;
        default:
            DAWN_UNREACHABLE();
    }
}

GLboolean VertexFormatIsNormalized(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Unorm8:
        case wgpu::VertexFormat::Unorm8x2:
        case wgpu::VertexFormat::Unorm8x4:
        case wgpu::VertexFormat::Unorm8x4BGRA:
        case wgpu::VertexFormat::Snorm8:
        case wgpu::VertexFormat::Snorm8x2:
        case wgpu::VertexFormat::Snorm8x4:
        case wgpu::VertexFormat::Unorm16:
        case wgpu::VertexFormat::Unorm16x2:
        case wgpu::VertexFormat::Unorm16x4:
        case wgpu::VertexFormat::Snorm16:
        case wgpu::VertexFormat::Snorm16x2:
        case wgpu::VertexFormat::Snorm16x4:
        case wgpu::VertexFormat::Unorm10_10_10_2:
            return GL_TRUE;
        default:
            return GL_FALSE;
    }
}

bool VertexFormatIsInt(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8:
        case wgpu::VertexFormat::Uint8x2:
        case wgpu::VertexFormat::Uint8x4:
        case wgpu::VertexFormat::Sint8:
        case wgpu::VertexFormat::Sint8x2:
        case wgpu::VertexFormat::Sint8x4:
        case wgpu::VertexFormat::Uint16:
        case wgpu::VertexFormat::Uint16x2:
        case wgpu::VertexFormat::Uint16x4:
        case wgpu::VertexFormat::Sint16:
        case wgpu::VertexFormat::Sint16x2:
        case wgpu::VertexFormat::Sint16x4:
        case wgpu::VertexFormat::Uint32:
        case wgpu::VertexFormat::Uint32x2:
        case wgpu::VertexFormat::Uint32x3:
        case wgpu::VertexFormat::Uint32x4:
        case wgpu::VertexFormat::Sint32:
        case wgpu::VertexFormat::Sint32x2:
        case wgpu::VertexFormat::Sint32x3:
        case wgpu::VertexFormat::Sint32x4:
            return true;
        default:
            return false;
    }
}

// Vertex buffers and index buffers are implemented as part of an OpenGL VAO that
// corresponds to a VertexState. On the contrary in Dawn they are part of the global state.
// This means that we have to re-apply these buffers on a VertexState change.
class VertexStateBufferBindingTracker {
  public:
    void OnSetIndexBuffer(BufferBase* buffer) {
        mIndexBufferDirty = true;
        mIndexBuffer = ToBackend(buffer);
    }

    void OnSetVertexBuffer(VertexBufferSlot slot, BufferBase* buffer, uint64_t offset) {
        mVertexBuffers[slot] = ToBackend(buffer);
        mVertexBufferOffsets[slot] = offset;
        mDirtyVertexBuffers.set(slot);
    }

    void OnSetPipeline(RenderPipelineBase* pipeline) {
        if (mLastPipeline == pipeline) {
            return;
        }

        mIndexBufferDirty = true;
        mDirtyVertexBuffers |= pipeline->GetVertexBuffersUsed();

        mLastPipeline = pipeline;
    }

    MaybeError Apply(const OpenGLFunctions& gl, int32_t baseVertex, uint32_t firstInstance) {
        if (mBaseVertex != baseVertex) {
            mBaseVertex = baseVertex;
            mDirtyVertexBuffers |= mLastPipeline->GetVertexBuffersUsedAsVertexBuffer();
        }

        if (mFirstInstance != firstInstance) {
            mFirstInstance = firstInstance;
            mDirtyVertexBuffers |= mLastPipeline->GetVertexBuffersUsedAsInstanceBuffer();
        }

        if (mIndexBufferDirty && mIndexBuffer != nullptr) {
            DAWN_GL_TRY(gl, BindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer->GetHandle()));
            mIndexBufferDirty = false;
        }

        for (VertexBufferSlot slot :
             IterateBitSet(mDirtyVertexBuffers & mLastPipeline->GetVertexBuffersUsed())) {
            for (VertexAttributeLocation location :
                 IterateBitSet(ToBackend(mLastPipeline)->GetAttributesUsingVertexBuffer(slot))) {
                const VertexAttributeInfo& attribute = mLastPipeline->GetAttribute(location);

                GLuint attribIndex = static_cast<GLuint>(static_cast<uint8_t>(location));
                GLuint buffer = mVertexBuffers[slot]->GetHandle();
                intptr_t offset = mVertexBufferOffsets[slot];

                const VertexBufferInfo& vertexBuffer = mLastPipeline->GetVertexBuffer(slot);

                if (vertexBuffer.stepMode == wgpu::VertexStepMode::Vertex) {
                    offset += mBaseVertex * vertexBuffer.arrayStride;
                } else if (vertexBuffer.stepMode == wgpu::VertexStepMode::Instance) {
                    offset += mFirstInstance * vertexBuffer.arrayStride;
                }
                uint32_t components = GetVertexFormatInfo(attribute.format).componentCount;
                GLenum formatType = VertexFormatType(attribute.format);

                GLboolean normalized = VertexFormatIsNormalized(attribute.format);
                DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, buffer));
                if (VertexFormatIsInt(attribute.format)) {
                    DAWN_GL_TRY(
                        gl, VertexAttribIPointer(
                                attribIndex, components, formatType, vertexBuffer.arrayStride,
                                reinterpret_cast<void*>(offset +
                                                        static_cast<intptr_t>(attribute.offset))));
                } else {
                    DAWN_GL_TRY(gl, VertexAttribPointer(
                                        attribIndex, components, formatType, normalized,
                                        vertexBuffer.arrayStride,
                                        reinterpret_cast<void*>(
                                            offset + static_cast<intptr_t>(attribute.offset))));
                }
            }
        }

        mDirtyVertexBuffers.reset();
        return {};
    }

  private:
    bool mIndexBufferDirty = false;
    raw_ptr<Buffer> mIndexBuffer = nullptr;

    VertexBufferMask mDirtyVertexBuffers;
    PerVertexBuffer<Buffer*> mVertexBuffers;
    PerVertexBuffer<uint64_t> mVertexBufferOffsets;

    int32_t mBaseVertex = 0;
    uint32_t mFirstInstance = 0;
    raw_ptr<RenderPipelineBase> mLastPipeline = nullptr;
};

class BindGroupTracker : public BindGroupTrackerBase<false, uint64_t> {
  public:
    void OnSetPipeline(RenderPipeline* pipeline) {
        BindGroupTrackerBase::OnSetPipeline(pipeline);
        mPipeline = pipeline;
        ResetInternalUniformDataDirtyRange();
    }

    void OnSetPipeline(ComputePipeline* pipeline) {
        BindGroupTrackerBase::OnSetPipeline(pipeline);
        mPipeline = pipeline;
        ResetInternalUniformDataDirtyRange();
    }

    MaybeError Apply(const OpenGLFunctions& gl) {
        BeforeApply();
        for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
            DAWN_TRY(ApplyBindGroup(gl, index, mBindGroups[index], mDynamicOffsets[index]));
        }
        DAWN_TRY(ApplyInternalUniforms(gl));
        AfterApply();
        return {};
    }

  private:
    MaybeError BindSamplerAtIndex(const OpenGLFunctions& gl, SamplerBase* s, GLuint samplerIndex) {
        Sampler* sampler = ToBackend(s);

        for (PipelineGL::SamplerUnit unit : mPipeline->GetTextureUnitsForSampler(samplerIndex)) {
            // Only use filtering for certain texture units, because int
            // and uint texture are only complete without filtering
            if (unit.shouldUseFiltering) {
                DAWN_GL_TRY(gl, BindSampler(unit.unit, sampler->GetFilteringHandle()));
            } else {
                DAWN_GL_TRY(gl, BindSampler(unit.unit, sampler->GetNonFilteringHandle()));
            }
        }

        return {};
    }

    MaybeError ApplyBindGroup(const OpenGLFunctions& gl,
                              BindGroupIndex groupIndex,
                              BindGroupBase* group,
                              const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets) {
        const auto& indices = ToBackend(mPipelineLayout)->GetBindingIndexInfo()[groupIndex];

        for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
            const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
            DAWN_TRY(MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& layout) -> MaybeError {
                    BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);
                    GLuint buffer = ToBackend(binding.buffer)->GetHandle();
                    GLuint index = indices[bindingIndex];
                    GLuint offset = binding.offset;

                    if (layout.hasDynamicOffset) {
                        // Dynamic buffers are packed at the front of BindingIndices.
                        offset += dynamicOffsets[bindingIndex];
                    }

                    GLenum target;
                    switch (layout.type) {
                        case wgpu::BufferBindingType::Uniform:
                            target = GL_UNIFORM_BUFFER;
                            break;
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding:
                            target = GL_SHADER_STORAGE_BUFFER;
                            break;
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                    }

                    DAWN_GL_TRY(gl, BindBufferRange(target, index, buffer, offset, binding.size));
                    return {};
                },
                [&](const StaticSamplerBindingInfo& layout) -> MaybeError {
                    DAWN_TRY(BindSamplerAtIndex(gl, layout.sampler.Get(), indices[bindingIndex]));
                    return {};
                },
                [&](const SamplerBindingInfo&) -> MaybeError {
                    DAWN_TRY(BindSamplerAtIndex(gl, group->GetBindingAsSampler(bindingIndex),
                                                indices[bindingIndex]));
                    return {};
                },
                [&](const TextureBindingInfo&) -> MaybeError {
                    TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                    GLuint handle = view->GetHandle();
                    GLenum target = view->GetGLTarget();
                    GLuint viewIndex = indices[bindingIndex];

                    for (auto unit : mPipeline->GetTextureUnitsForTextureView(viewIndex)) {
                        DAWN_GL_TRY(gl, ActiveTexture(GL_TEXTURE0 + unit));
                        DAWN_GL_TRY(gl, BindTexture(target, handle));
                        if (ToBackend(view->GetTexture())->GetGLFormat().format ==
                            GL_DEPTH_STENCIL) {
                            Aspect aspect = view->GetAspects();
                            DAWN_ASSERT(HasOneBit(aspect));
                            switch (aspect) {
                                case Aspect::None:
                                case Aspect::Color:
                                case Aspect::CombinedDepthStencil:
                                case Aspect::Plane0:
                                case Aspect::Plane1:
                                case Aspect::Plane2:
                                    DAWN_UNREACHABLE();
                                case Aspect::Depth:
                                    DAWN_GL_TRY(gl,
                                                TexParameteri(target, GL_DEPTH_STENCIL_TEXTURE_MODE,
                                                              GL_DEPTH_COMPONENT));
                                    break;
                                case Aspect::Stencil:
                                    DAWN_GL_TRY(gl,
                                                TexParameteri(target, GL_DEPTH_STENCIL_TEXTURE_MODE,
                                                              GL_STENCIL_INDEX));
                                    break;
                            }
                        }
                        DAWN_GL_TRY(gl, TexParameteri(target, GL_TEXTURE_BASE_LEVEL,
                                                      view->GetBaseMipLevel()));
                        DAWN_GL_TRY(
                            gl, TexParameteri(target, GL_TEXTURE_MAX_LEVEL,
                                              view->GetBaseMipLevel() + view->GetLevelCount() - 1));
                    }

                    // Some texture builtin function data needs emulation to update into the
                    // internal uniform buffer.
                    UpdateTextureBuiltinsUniformData(gl, view, groupIndex, bindingIndex);
                    return {};
                },
                [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                    TextureView* view = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                    Texture* texture = ToBackend(view->GetTexture());
                    GLuint handle = texture->GetHandle();
                    GLuint imageIndex = indices[bindingIndex];

                    GLenum access;
                    switch (layout.access) {
                        case wgpu::StorageTextureAccess::WriteOnly:
                            access = GL_WRITE_ONLY;
                            break;
                        case wgpu::StorageTextureAccess::ReadWrite:
                            access = GL_READ_WRITE;
                            break;
                        case wgpu::StorageTextureAccess::ReadOnly:
                            access = GL_READ_ONLY;
                            break;
                        case wgpu::StorageTextureAccess::BindingNotUsed:
                        case wgpu::StorageTextureAccess::Undefined:
                            DAWN_UNREACHABLE();
                    }

                    // OpenGL ES only supports either binding a layer or the entire
                    // texture in glBindImageTexture().
                    GLboolean isLayered;
                    if (texture->GetArrayLayers() == view->GetLayerCount()) {
                        isLayered = GL_TRUE;
                    } else if (view->GetLayerCount() == 1) {
                        isLayered = GL_FALSE;
                    } else {
                        DAWN_UNREACHABLE();
                    }

                    DAWN_GL_TRY(gl, BindImageTexture(imageIndex, handle, view->GetBaseMipLevel(),
                                                     isLayered, view->GetBaseArrayLayer(), access,
                                                     texture->GetGLFormat().internalFormat));
                    return {};
                },
                [](const InputAttachmentBindingInfo&) -> MaybeError { DAWN_UNREACHABLE(); }));
        }

        return {};
    }

    void UpdateTextureBuiltinsUniformData(const OpenGLFunctions& gl,
                                          const TextureView* view,
                                          BindGroupIndex groupIndex,
                                          BindingIndex bindingIndex) {
        const auto& bindingInfo = mPipeline->GetBindingPointBuiltinDataInfo();
        if (bindingInfo.empty()) {
            return;
        }

        auto iter = bindingInfo.find(tint::BindingPoint{static_cast<uint32_t>(groupIndex),
                                                        static_cast<uint32_t>(bindingIndex)});
        if (iter == bindingInfo.end()) {
            return;
        }

        // Update data by retrieving information from texture view object.
        const BindPointFunction field = iter->second.first;
        const size_t byteOffset = static_cast<size_t>(iter->second.second);

        uint32_t data;
        switch (field) {
            case BindPointFunction::kTextureNumLevels:
                data = view->GetLevelCount();
                break;
            case BindPointFunction::kTextureNumSamples:
                data = view->GetTexture()->GetSampleCount();
                break;
        }

        if (byteOffset >= mInternalUniformBufferData.size()) {
            mInternalUniformBufferData.resize(byteOffset + sizeof(uint32_t));
        }
        memcpy(mInternalUniformBufferData.data() + byteOffset, &data, sizeof(uint32_t));

        // Updating dirty range of the data vector
        mDirtyRange.first = std::min(mDirtyRange.first, byteOffset);
        mDirtyRange.second = std::max(mDirtyRange.second, byteOffset + sizeof(uint32_t));
    }

    void ResetInternalUniformDataDirtyRange() {
        mDirtyRange = {mInternalUniformBufferData.size(), 0};
    }

    MaybeError ApplyInternalUniforms(const OpenGLFunctions& gl) {
        const Buffer* internalUniformBuffer = mPipeline->GetInternalUniformBuffer();
        if (!internalUniformBuffer) {
            return {};
        }

        GLuint internalUniformBufferHandle = internalUniformBuffer->GetHandle();
        if (mDirtyRange.first >= mDirtyRange.second) {
            // Early return if no dirty uniform range needs updating.
            return {};
        }

        DAWN_GL_TRY(gl, BindBuffer(GL_UNIFORM_BUFFER, internalUniformBufferHandle));
        DAWN_GL_TRY(gl, BufferSubData(GL_UNIFORM_BUFFER, mDirtyRange.first,
                                      mDirtyRange.second - mDirtyRange.first,
                                      mInternalUniformBufferData.data() + mDirtyRange.first));
        DAWN_GL_TRY(gl, BindBuffer(GL_UNIFORM_BUFFER, 0));

        ResetInternalUniformDataDirtyRange();

        return {};
    }

    raw_ptr<PipelineGL> mPipeline = nullptr;

    // The data used for mPipeline's internal uniform buffer from current bind group.
    // Expecting no more than 4 texture bindings called as textureNumLevels/textureNumSamples
    // argument in a pipeline. Initialize the vector to this size to avoid frequent resizing.
    std::vector<uint8_t> mInternalUniformBufferData = std::vector<uint8_t>(4 * sizeof(uint32_t));
    // Tracking dirty byte range of the mInternalUniformBufferData that needs to call bufferSubData
    // to update to the internal uniform buffer of mPipeline. Range it represents: [first, second)
    std::pair<size_t, size_t> mDirtyRange;
};

MaybeError ResolveMultisampledRenderTargets(const OpenGLFunctions& gl,
                                            const BeginRenderPassCmd* renderPass) {
    DAWN_ASSERT(renderPass != nullptr);

    // Reset state that may affect glBlitFramebuffer().
    DAWN_GL_TRY(gl, Disable(GL_SCISSOR_TEST));

    GLuint readFbo = 0;
    GLuint writeFbo = 0;

    for (auto i : IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
        if (renderPass->colorAttachments[i].resolveTarget != nullptr) {
            if (readFbo == 0) {
                DAWN_ASSERT(writeFbo == 0);
                DAWN_GL_TRY(gl, GenFramebuffers(1, &readFbo));
                DAWN_GL_TRY(gl, GenFramebuffers(1, &writeFbo));
            }

            TextureView* colorView = ToBackend(renderPass->colorAttachments[i].view.Get());

            DAWN_GL_TRY(gl, BindFramebuffer(GL_READ_FRAMEBUFFER, readFbo));
            DAWN_TRY(colorView->BindToFramebuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));

            TextureView* resolveView =
                ToBackend(renderPass->colorAttachments[i].resolveTarget.Get());
            DAWN_GL_TRY(gl, BindFramebuffer(GL_DRAW_FRAMEBUFFER, writeFbo));
            DAWN_TRY(resolveView->BindToFramebuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0));
            DAWN_GL_TRY(gl, BlitFramebuffer(0, 0, renderPass->width, renderPass->height, 0, 0,
                                            renderPass->width, renderPass->height,
                                            GL_COLOR_BUFFER_BIT, GL_NEAREST));
        }
    }

    DAWN_GL_TRY(gl, Enable(GL_SCISSOR_TEST));
    DAWN_GL_TRY(gl, DeleteFramebuffers(1, &readFbo));
    DAWN_GL_TRY(gl, DeleteFramebuffers(1, &writeFbo));
    return {};
}

// OpenGL SPEC requires the source/destination region muPst be a region that is contained
// within srcImage/dstImage. Here the size of the image refers to the virtual size, while
// Dawn validates texture copy extent with the physical size, so we need to re-calculate the
// texture copy extent to ensure it should fit in the virtual size of the subresource.
Extent3D ComputeTextureCopyExtent(const TextureCopy& textureCopy, const Extent3D& copySize) {
    Extent3D validTextureCopyExtent = copySize;
    const TextureBase* texture = textureCopy.texture.Get();
    Extent3D virtualSizeAtLevel =
        texture->GetMipLevelSingleSubresourceVirtualSize(textureCopy.mipLevel, textureCopy.aspect);
    DAWN_ASSERT(textureCopy.origin.x <= virtualSizeAtLevel.width);
    DAWN_ASSERT(textureCopy.origin.y <= virtualSizeAtLevel.height);
    if (copySize.width > virtualSizeAtLevel.width - textureCopy.origin.x) {
        DAWN_ASSERT(texture->GetFormat().isCompressed);
        validTextureCopyExtent.width = virtualSizeAtLevel.width - textureCopy.origin.x;
    }
    if (copySize.height > virtualSizeAtLevel.height - textureCopy.origin.y) {
        DAWN_ASSERT(texture->GetFormat().isCompressed);
        validTextureCopyExtent.height = virtualSizeAtLevel.height - textureCopy.origin.y;
    }

    return validTextureCopyExtent;
}

}  // namespace

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {}

MaybeError CommandBuffer::Execute() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    auto LazyClearSyncScope = [](const SyncScopeResourceUsage& scope) -> MaybeError {
        for (size_t i = 0; i < scope.textures.size(); i++) {
            Texture* texture = ToBackend(scope.textures[i]);

            // Clear subresources that are not render attachments. Render attachments will be
            // cleared in RecordBeginRenderPass by setting the loadop to clear when the texture
            // subresource has not been initialized before the render pass.
            DAWN_TRY(scope.textureSyncInfos[i].Iterate(
                [&](const SubresourceRange& range, const TextureSyncInfo& syncInfo) -> MaybeError {
                    if (syncInfo.usage & ~wgpu::TextureUsage::RenderAttachment) {
                        DAWN_TRY(texture->EnsureSubresourceContentInitialized(range));
                    }
                    return {};
                }));
        }

        for (BufferBase* bufferBase : scope.buffers) {
            DAWN_TRY(ToBackend(bufferBase)->EnsureDataInitialized());
        }
        return {};
    };

    size_t nextComputePassNumber = 0;
    size_t nextRenderPassNumber = 0;

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                mCommands.NextCommand<BeginComputePassCmd>();
                for (TextureBase* texture :
                     GetResourceUsages().computePasses[nextComputePassNumber].referencedTextures) {
                    DAWN_TRY(ToBackend(texture)->SynchronizeTextureBeforeUse());
                }
                for (const SyncScopeResourceUsage& scope :
                     GetResourceUsages().computePasses[nextComputePassNumber].dispatchUsages) {
                    DAWN_TRY(LazyClearSyncScope(scope));
                }
                DAWN_TRY(ExecuteComputePass());

                nextComputePassNumber++;
                break;
            }

            case Command::BeginRenderPass: {
                auto* cmd = mCommands.NextCommand<BeginRenderPassCmd>();
                for (TextureBase* texture :
                     this->GetResourceUsages().renderPasses[nextRenderPassNumber].textures) {
                    DAWN_TRY(ToBackend(texture)->SynchronizeTextureBeforeUse());
                }
                DAWN_TRY(
                    LazyClearSyncScope(GetResourceUsages().renderPasses[nextRenderPassNumber]));
                LazyClearRenderPassAttachments(cmd);
                DAWN_TRY(ExecuteRenderPass(cmd));

                nextRenderPassNumber++;
                break;
            }

            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                if (copy->size == 0) {
                    // Skip no-op copies.
                    break;
                }

                DAWN_TRY(ToBackend(copy->source)->EnsureDataInitialized());
                DAWN_TRY(
                    ToBackend(copy->destination)
                        ->EnsureDataInitializedAsDestination(copy->destinationOffset, copy->size));

                DAWN_GL_TRY(gl,
                            BindBuffer(GL_PIXEL_PACK_BUFFER, ToBackend(copy->source)->GetHandle()));
                DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_UNPACK_BUFFER,
                                           ToBackend(copy->destination)->GetHandle()));
                DAWN_GL_TRY(
                    gl, CopyBufferSubData(GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER,
                                          copy->sourceOffset, copy->destinationOffset, copy->size));

                DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_PACK_BUFFER, 0));
                DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

                ToBackend(copy->source)->TrackUsage();
                ToBackend(copy->destination)->TrackUsage();
                break;
            }

            case Command::CopyBufferToTexture: {
                CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                auto& src = copy->source;
                auto& dst = copy->destination;
                Buffer* buffer = ToBackend(src.buffer.Get());
                Texture* texture = ToBackend(dst.texture.Get());

                DAWN_TRY(texture->SynchronizeTextureBeforeUse());

                DAWN_TRY(buffer->EnsureDataInitialized());
                SubresourceRange range = GetSubresourcesAffectedByCopy(dst, copy->copySize);
                if (IsCompleteSubresourceCopiedTo(texture, copy->copySize, dst.mipLevel,
                                                  dst.aspect)) {
                    texture->SetIsSubresourceContentInitialized(true, range);
                } else {
                    DAWN_TRY(texture->EnsureSubresourceContentInitialized(range));
                }

                DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->GetHandle()));

                TexelCopyBufferLayout dataLayout;
                dataLayout.offset = 0;
                dataLayout.bytesPerRow = src.bytesPerRow;
                dataLayout.rowsPerImage = src.rowsPerImage;

                DAWN_TRY(DoTexSubImage(gl, dst, reinterpret_cast<void*>(src.offset), dataLayout,
                                       copy->copySize));
                DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

                buffer->TrackUsage();
                break;
            }

            case Command::CopyTextureToBuffer: {
                CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                auto& src = copy->source;
                auto& dst = copy->destination;
                auto& copySize = copy->copySize;
                Texture* texture = ToBackend(src.texture.Get());
                Buffer* buffer = ToBackend(dst.buffer.Get());
                const Format& formatInfo = texture->GetFormat();
                const GLFormat& format = texture->GetGLFormat();
                GLenum target = texture->GetGLTarget();

                if (formatInfo.isCompressed) {
                    DAWN_UNREACHABLE();
                }

                DAWN_TRY(buffer->EnsureDataInitializedAsDestination(copy));
                DAWN_TRY(texture->SynchronizeTextureBeforeUse());

                SubresourceRange subresources = GetSubresourcesAffectedByCopy(src, copy->copySize);
                DAWN_TRY(texture->EnsureSubresourceContentInitialized(subresources));
                // The only way to move data from a texture to a buffer in GL is via
                // glReadPixels with a pack buffer. Create a temporary FBO for the copy.
                DAWN_GL_TRY(gl, BindTexture(target, texture->GetHandle()));

                GLuint readFBO = 0;
                DAWN_GL_TRY(gl, GenFramebuffers(1, &readFBO));
                DAWN_GL_TRY(gl, BindFramebuffer(GL_READ_FRAMEBUFFER, readFBO));

                const TexelBlockInfo& blockInfo = formatInfo.GetAspectInfo(src.aspect).block;

                DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_PACK_BUFFER, buffer->GetHandle()));
                DAWN_GL_TRY(gl, PixelStorei(GL_PACK_ALIGNMENT, std::min(8u, blockInfo.byteSize)));
                DAWN_GL_TRY(gl,
                            PixelStorei(GL_PACK_ROW_LENGTH, dst.bytesPerRow / blockInfo.byteSize));

                GLenum glAttachment;
                GLenum glFormat;
                GLenum glType;
                switch (src.aspect) {
                    case Aspect::Color:
                        glAttachment = GL_COLOR_ATTACHMENT0;
                        glFormat = format.format;
                        glType = format.type;
                        break;
                    case Aspect::Depth:
                        glAttachment = GL_DEPTH_ATTACHMENT;
                        glFormat = GL_DEPTH_COMPONENT;
                        glType = GL_FLOAT;
                        break;
                    case Aspect::Stencil:
                        glAttachment = GL_STENCIL_ATTACHMENT;
                        glFormat = GL_STENCIL_INDEX;
                        glType = GL_UNSIGNED_BYTE;
                        break;

                    case Aspect::CombinedDepthStencil:
                    case Aspect::None:
                    case Aspect::Plane0:
                    case Aspect::Plane1:
                    case Aspect::Plane2:
                        DAWN_UNREACHABLE();
                }

                uint8_t* offset = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(dst.offset));
                switch (texture->GetDimension()) {
                    case wgpu::TextureDimension::Undefined:
                        DAWN_UNREACHABLE();
                    case wgpu::TextureDimension::e1D:
                    case wgpu::TextureDimension::e2D: {
                        if (target == GL_TEXTURE_2D) {
                            DAWN_ASSERT(texture->GetArrayLayers() == 1);
                            DAWN_GL_TRY(
                                gl, FramebufferTexture2D(GL_READ_FRAMEBUFFER, glAttachment, target,
                                                         texture->GetHandle(), src.mipLevel));
                            DAWN_GL_TRY(gl, ReadPixels(src.origin.x, src.origin.y, copySize.width,
                                                       copySize.height, glFormat, glType, offset));
                            break;
                        } else if (target == GL_TEXTURE_CUBE_MAP) {
                            DAWN_ASSERT(texture->GetArrayLayers() == 6);
                            const uint64_t bytesPerImage = dst.bytesPerRow * dst.rowsPerImage;
                            for (uint32_t z = 0; z < copySize.depthOrArrayLayers; ++z) {
                                GLenum cubeMapTarget =
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + z + src.origin.z;
                                DAWN_GL_TRY(
                                    gl, FramebufferTexture2D(GL_READ_FRAMEBUFFER, glAttachment,
                                                             cubeMapTarget, texture->GetHandle(),
                                                             src.mipLevel));
                                DAWN_GL_TRY(gl,
                                            ReadPixels(src.origin.x, src.origin.y, copySize.width,
                                                       copySize.height, glFormat, glType, offset));
                                offset += bytesPerImage;
                            }
                            break;
                        }
                        // Implementation for 2D array is the same as 3D.
                        [[fallthrough]];
                    }

                    case wgpu::TextureDimension::e3D: {
                        const uint64_t bytesPerImage = dst.bytesPerRow * dst.rowsPerImage;
                        for (uint32_t z = 0; z < copySize.depthOrArrayLayers; ++z) {
                            DAWN_GL_TRY(gl,
                                        FramebufferTextureLayer(GL_READ_FRAMEBUFFER, glAttachment,
                                                                texture->GetHandle(), src.mipLevel,
                                                                src.origin.z + z));
                            DAWN_GL_TRY(gl, ReadPixels(src.origin.x, src.origin.y, copySize.width,
                                                       copySize.height, glFormat, glType, offset));

                            offset += bytesPerImage;
                        }
                        break;
                    }
                }

                DAWN_GL_TRY(gl, PixelStorei(GL_PACK_ROW_LENGTH, 0));
                DAWN_GL_TRY(gl, PixelStorei(GL_PACK_ALIGNMENT, 4));  // Reset to default
                DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_PACK_BUFFER, 0));
                DAWN_GL_TRY(gl, DeleteFramebuffers(1, &readFBO));

                buffer->TrackUsage();
                break;
            }

            case Command::CopyTextureToTexture: {
                CopyTextureToTextureCmd* copy = mCommands.NextCommand<CopyTextureToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                auto& src = copy->source;
                auto& dst = copy->destination;

                // TODO(crbug.com/dawn/817): add workaround for the case that imageExtentSrc
                // is not equal to imageExtentDst. For example when copySize fits in the virtual
                // size of the source image but does not fit in the one of the destination
                // image.
                Extent3D copySize = ComputeTextureCopyExtent(dst, copy->copySize);
                Texture* srcTexture = ToBackend(src.texture.Get());
                Texture* dstTexture = ToBackend(dst.texture.Get());

                DAWN_TRY(srcTexture->SynchronizeTextureBeforeUse());
                DAWN_TRY(dstTexture->SynchronizeTextureBeforeUse());

                SubresourceRange srcRange = GetSubresourcesAffectedByCopy(src, copy->copySize);
                SubresourceRange dstRange = GetSubresourcesAffectedByCopy(dst, copy->copySize);

                DAWN_TRY(srcTexture->EnsureSubresourceContentInitialized(srcRange));
                if (IsCompleteSubresourceCopiedTo(dstTexture, copySize, dst.mipLevel, dst.aspect)) {
                    dstTexture->SetIsSubresourceContentInitialized(true, dstRange);
                } else {
                    DAWN_TRY(dstTexture->EnsureSubresourceContentInitialized(dstRange));
                }
                DAWN_TRY(CopyImageSubData(gl, src.aspect, srcTexture->GetHandle(),
                                          srcTexture->GetGLTarget(), src.mipLevel, src.origin,
                                          dstTexture->GetHandle(), dstTexture->GetGLTarget(),
                                          dst.mipLevel, dst.origin, copySize));
                break;
            }

            case Command::ClearBuffer: {
                ClearBufferCmd* cmd = mCommands.NextCommand<ClearBufferCmd>();
                if (cmd->size == 0) {
                    // Skip no-op fills.
                    break;
                }
                Buffer* dstBuffer = ToBackend(cmd->buffer.Get());

                bool clearedToZero = false;
                DAWN_TRY(dstBuffer->EnsureDataInitializedAsDestination(cmd->offset, cmd->size,
                                                                       &clearedToZero));

                if (!clearedToZero) {
                    const std::vector<uint8_t> clearValues(cmd->size, 0u);
                    DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, dstBuffer->GetHandle()));
                    DAWN_GL_TRY(gl, BufferSubData(GL_ARRAY_BUFFER, cmd->offset, cmd->size,
                                                  clearValues.data()));
                }

                dstBuffer->TrackUsage();
                break;
            }

            case Command::ResolveQuerySet: {
                ResolveQuerySetCmd* cmd = mCommands.NextCommand<ResolveQuerySetCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                Buffer* destination = ToBackend(cmd->destination.Get());

                size_t size = cmd->queryCount * sizeof(uint64_t);
                DAWN_TRY(
                    destination->EnsureDataInitializedAsDestination(cmd->destinationOffset, size));

                std::vector<uint64_t> values(cmd->queryCount);
                auto availability = querySet->GetQueryAvailability();

                for (uint32_t i = 0; i < cmd->queryCount; ++i) {
                    if (!availability[cmd->firstQuery + i]) {
                        values[i] = 0;
                        continue;
                    }
                    uint32_t query = querySet->Get(cmd->firstQuery + i);
                    GLuint value;
                    DAWN_GL_TRY(gl, GetQueryObjectuiv(query, GL_QUERY_RESULT, &value));
                    values[i] = value;
                }

                DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, destination->GetHandle()));
                DAWN_GL_TRY(gl, BufferSubData(GL_ARRAY_BUFFER, cmd->destinationOffset, size,
                                              values.data()));

                break;
            }

            case Command::WriteTimestamp: {
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                // Due to lack of linux driver support for GL_EXT_debug_marker
                // extension these functions are skipped.
                SkipCommand(&mCommands, type);
                break;
            }

            case Command::WriteBuffer: {
                WriteBufferCmd* write = mCommands.NextCommand<WriteBufferCmd>();
                uint64_t offset = write->offset;
                uint64_t size = write->size;
                if (size == 0) {
                    continue;
                }

                Buffer* dstBuffer = ToBackend(write->buffer.Get());
                uint8_t* data = mCommands.NextData<uint8_t>(size);
                DAWN_TRY(dstBuffer->EnsureDataInitializedAsDestination(offset, size));

                DAWN_GL_TRY(gl, BindBuffer(GL_ARRAY_BUFFER, dstBuffer->GetHandle()));
                DAWN_GL_TRY(gl, BufferSubData(GL_ARRAY_BUFFER, offset, size, data));

                dstBuffer->TrackUsage();
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }
    }

    return {};
}

MaybeError CommandBuffer::ExecuteComputePass() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    ComputePipeline* lastPipeline = nullptr;
    BindGroupTracker bindGroupTracker = {};

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                mCommands.NextCommand<EndComputePassCmd>();
                return {};
            }

            case Command::Dispatch: {
                DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();
                DAWN_TRY(bindGroupTracker.Apply(gl));

                DAWN_GL_TRY(gl, DispatchCompute(dispatch->x, dispatch->y, dispatch->z));
                DAWN_GL_TRY(gl, MemoryBarrier(GL_ALL_BARRIER_BITS));
                break;
            }

            case Command::DispatchIndirect: {
                DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();
                DAWN_TRY(bindGroupTracker.Apply(gl));

                uint64_t indirectBufferOffset = dispatch->indirectOffset;
                Buffer* indirectBuffer = ToBackend(dispatch->indirectBuffer.Get());

                DAWN_GL_TRY(gl,
                            BindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirectBuffer->GetHandle()));
                DAWN_GL_TRY(gl,
                            DispatchComputeIndirect(static_cast<GLintptr>(indirectBufferOffset)));
                DAWN_GL_TRY(gl, MemoryBarrier(GL_ALL_BARRIER_BITS));

                indirectBuffer->TrackUsage();
                break;
            }

            case Command::SetComputePipeline: {
                SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                lastPipeline = ToBackend(cmd->pipeline).Get();
                DAWN_TRY(lastPipeline->ApplyNow());

                bindGroupTracker.OnSetPipeline(lastPipeline);
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = mCommands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                bindGroupTracker.OnSetBindGroup(cmd->index, cmd->group.Get(),
                                                cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                // Due to lack of linux driver support for GL_EXT_debug_marker
                // extension these functions are skipped.
                SkipCommand(&mCommands, type);
                break;
            }

            case Command::WriteTimestamp: {
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");
            }

            case Command::SetImmediateData:
                return DAWN_UNIMPLEMENTED_ERROR("SetImmediateData unimplemented");

            default:
                DAWN_UNREACHABLE();
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
}

MaybeError CommandBuffer::ExecuteRenderPass(BeginRenderPassCmd* renderPass) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    GLuint fbo = 0;

    // Create the framebuffer used for this render pass and calls the correct glDrawBuffers
    {
        // TODO(kainino@chromium.org): This is added to possibly work around an issue seen on
        // Windows/Intel. It should break any feedback loop before the clears, even if there
        // shouldn't be any negative effects from this. Investigate whether it's actually
        // needed.
        DAWN_GL_TRY(gl, BindFramebuffer(GL_READ_FRAMEBUFFER, 0));
        // TODO(kainino@chromium.org): possible future optimization: create these framebuffers
        // at Framebuffer build time (or maybe CommandBuffer build time) so they don't have to
        // be created and destroyed at draw time.
        DAWN_GL_TRY(gl, GenFramebuffers(1, &fbo));
        DAWN_GL_TRY(gl, BindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo));

        // Mapping from attachmentSlot to GL framebuffer attachment points. Defaults to zero
        // (GL_NONE).
        PerColorAttachment<GLenum> drawBuffers = {};

        // Construct GL framebuffer

        ColorAttachmentIndex attachmentCount{};
        for (auto i : IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
            TextureView* textureView = ToBackend(renderPass->colorAttachments[i].view.Get());
            GLenum glAttachment = GL_COLOR_ATTACHMENT0 + static_cast<uint8_t>(i);

            // Attach color buffers.
            DAWN_TRY(textureView->BindToFramebuffer(GL_DRAW_FRAMEBUFFER, glAttachment,
                                                    renderPass->colorAttachments[i].depthSlice));
            drawBuffers[i] = glAttachment;
            attachmentCount = ityp::PlusOne(i);
        }
        DAWN_GL_TRY(gl, DrawBuffers(static_cast<uint8_t>(attachmentCount), drawBuffers.data()));

        if (renderPass->attachmentState->HasDepthStencilAttachment()) {
            TextureView* textureView = ToBackend(renderPass->depthStencilAttachment.view.Get());
            const Format& format = textureView->GetTexture()->GetFormat();

            // Attach depth/stencil buffer.
            GLenum glAttachment = 0;
            if (format.aspects == (Aspect::Depth | Aspect::Stencil)) {
                glAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
            } else if (format.aspects == Aspect::Depth) {
                glAttachment = GL_DEPTH_ATTACHMENT;
            } else if (format.aspects == Aspect::Stencil) {
                glAttachment = GL_STENCIL_ATTACHMENT;
            } else {
                DAWN_UNREACHABLE();
            }

            DAWN_TRY(textureView->BindToFramebuffer(GL_DRAW_FRAMEBUFFER, glAttachment));
        }
    }

    DAWN_ASSERT(gl.CheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    // Set defaults for dynamic state before executing clears and commands.
    PersistentPipelineState persistentPipelineState;
    DAWN_TRY(persistentPipelineState.SetDefaultState(gl));
    DAWN_GL_TRY(gl, BlendColor(0, 0, 0, 0));
    DAWN_GL_TRY(gl, Viewport(0, 0, renderPass->width, renderPass->height));
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
    DAWN_GL_TRY(gl, DepthRangef(minDepth, maxDepth));

    DAWN_GL_TRY(gl, Scissor(0, 0, renderPass->width, renderPass->height));

    // Clear framebuffer attachments as needed
    {
        for (auto index : IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
            uint8_t i = static_cast<uint8_t>(index);
            auto* attachmentInfo = &renderPass->colorAttachments[index];

            // Load op - color
            if (attachmentInfo->loadOp == wgpu::LoadOp::Clear) {
                DAWN_GL_TRY(gl, ColorMask(true, true, true, true));

                TextureComponentType baseType =
                    attachmentInfo->view->GetFormat().GetAspectInfo(Aspect::Color).baseType;
                switch (baseType) {
                    case TextureComponentType::Float: {
                        const std::array<float, 4> appliedClearColor =
                            ConvertToFloatColor(attachmentInfo->clearColor);
                        DAWN_GL_TRY(gl, ClearBufferfv(GL_COLOR, i, appliedClearColor.data()));
                        break;
                    }
                    case TextureComponentType::Uint: {
                        const std::array<uint32_t, 4> appliedClearColor =
                            ConvertToUnsignedIntegerColor(attachmentInfo->clearColor);
                        DAWN_GL_TRY(gl, ClearBufferuiv(GL_COLOR, i, appliedClearColor.data()));
                        break;
                    }
                    case TextureComponentType::Sint: {
                        const std::array<int32_t, 4> appliedClearColor =
                            ConvertToSignedIntegerColor(attachmentInfo->clearColor);
                        DAWN_GL_TRY(gl, ClearBufferiv(GL_COLOR, i, appliedClearColor.data()));
                        break;
                    }
                }
            }

            if (attachmentInfo->storeOp == wgpu::StoreOp::Discard) {
                // TODO(natlee@microsoft.com): call glDiscard to do optimization
            }
        }

        if (renderPass->attachmentState->HasDepthStencilAttachment()) {
            auto* attachmentInfo = &renderPass->depthStencilAttachment;
            const Format& attachmentFormat = attachmentInfo->view->GetTexture()->GetFormat();

            // Load op - depth/stencil
            bool doDepthClear =
                attachmentFormat.HasDepth() && (attachmentInfo->depthLoadOp == wgpu::LoadOp::Clear);
            bool doStencilClear = attachmentFormat.HasStencil() &&
                                  (attachmentInfo->stencilLoadOp == wgpu::LoadOp::Clear);

            if (doDepthClear) {
                DAWN_GL_TRY(gl, DepthMask(GL_TRUE));
            }
            if (doStencilClear) {
                DAWN_GL_TRY(gl,
                            StencilMask(GetStencilMaskFromStencilFormat(attachmentFormat.format)));
            }

            if (doDepthClear && doStencilClear) {
                DAWN_GL_TRY(gl, ClearBufferfi(GL_DEPTH_STENCIL, 0, attachmentInfo->clearDepth,
                                              attachmentInfo->clearStencil));
            } else if (doDepthClear) {
                DAWN_GL_TRY(gl, ClearBufferfv(GL_DEPTH, 0, &attachmentInfo->clearDepth));
            } else if (doStencilClear) {
                const GLint clearStencil = attachmentInfo->clearStencil;
                DAWN_GL_TRY(gl, ClearBufferiv(GL_STENCIL, 0, &clearStencil));
            }
        }
    }

    RenderPipeline* lastPipeline = nullptr;
    uint64_t indexBufferBaseOffset = 0;
    GLenum indexBufferFormat;
    uint32_t indexFormatSize;

    VertexStateBufferBindingTracker vertexStateBufferBindingTracker;
    BindGroupTracker bindGroupTracker = {};

    auto DoRenderBundleCommand = [&](CommandIterator* iter, Command type) -> MaybeError {
        switch (type) {
            case Command::Draw: {
                DrawCmd* draw = iter->NextCommand<DrawCmd>();
                DAWN_TRY(vertexStateBufferBindingTracker.Apply(gl, 0, draw->firstInstance));
                DAWN_TRY(bindGroupTracker.Apply(gl));

                if (lastPipeline->UsesInstanceIndex()) {
                    DAWN_GL_TRY(gl, Uniform1ui(PipelineLayout::PushConstantLocation::FirstInstance,
                                               draw->firstInstance));
                }
                DAWN_GL_TRY(gl, DrawArraysInstanced(lastPipeline->GetGLPrimitiveTopology(),
                                                    draw->firstVertex, draw->vertexCount,
                                                    draw->instanceCount));
                break;
            }

            case Command::DrawIndexed: {
                DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();
                DAWN_TRY(vertexStateBufferBindingTracker.Apply(gl, draw->baseVertex,
                                                               draw->firstInstance));
                DAWN_TRY(bindGroupTracker.Apply(gl));

                const auto topology = lastPipeline->GetGLPrimitiveTopology();
                if (topology == GL_LINE_STRIP || topology == GL_TRIANGLE_STRIP) {
                    DAWN_GL_TRY(gl, Enable(GL_PRIMITIVE_RESTART_FIXED_INDEX));
                } else {
                    DAWN_GL_TRY(gl, Disable(GL_PRIMITIVE_RESTART_FIXED_INDEX));
                }

                if (lastPipeline->UsesVertexIndex()) {
                    DAWN_GL_TRY(gl, Uniform1ui(PipelineLayout::PushConstantLocation::FirstVertex,
                                               draw->baseVertex));
                }
                if (lastPipeline->UsesInstanceIndex()) {
                    DAWN_GL_TRY(gl, Uniform1ui(PipelineLayout::PushConstantLocation::FirstInstance,
                                               draw->firstInstance));
                }
                DAWN_GL_TRY(gl, DrawElementsInstanced(
                                    topology, draw->indexCount, indexBufferFormat,
                                    reinterpret_cast<void*>(draw->firstIndex * indexFormatSize +
                                                            indexBufferBaseOffset),
                                    draw->instanceCount));
                break;
            }

            case Command::DrawIndirect: {
                DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();
                if (lastPipeline->UsesInstanceIndex()) {
                    DAWN_GL_TRY(gl,
                                Uniform1ui(PipelineLayout::PushConstantLocation::FirstInstance, 0));
                }
                DAWN_TRY(vertexStateBufferBindingTracker.Apply(gl, 0, 0));
                DAWN_TRY(bindGroupTracker.Apply(gl));

                uint64_t indirectBufferOffset = draw->indirectOffset;
                Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());

                DAWN_GL_TRY(gl, BindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer->GetHandle()));
                DAWN_GL_TRY(
                    gl, DrawArraysIndirect(
                            lastPipeline->GetGLPrimitiveTopology(),
                            reinterpret_cast<void*>(static_cast<intptr_t>(indirectBufferOffset))));
                indirectBuffer->TrackUsage();
                break;
            }

            case Command::DrawIndexedIndirect: {
                DrawIndexedIndirectCmd* draw = iter->NextCommand<DrawIndexedIndirectCmd>();

                if (lastPipeline->UsesInstanceIndex()) {
                    DAWN_GL_TRY(gl,
                                Uniform1ui(PipelineLayout::PushConstantLocation::FirstInstance, 0));
                }
                DAWN_TRY(vertexStateBufferBindingTracker.Apply(gl, 0, 0));
                DAWN_TRY(bindGroupTracker.Apply(gl));

                Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());
                DAWN_ASSERT(indirectBuffer != nullptr);

                const auto topology = lastPipeline->GetGLPrimitiveTopology();
                if (topology == GL_LINE_STRIP || topology == GL_TRIANGLE_STRIP) {
                    DAWN_GL_TRY(gl, Enable(GL_PRIMITIVE_RESTART_FIXED_INDEX));
                } else {
                    DAWN_GL_TRY(gl, Disable(GL_PRIMITIVE_RESTART_FIXED_INDEX));
                }

                DAWN_GL_TRY(gl, BindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer->GetHandle()));
                DAWN_GL_TRY(
                    gl, DrawElementsIndirect(
                            topology, indexBufferFormat,
                            reinterpret_cast<void*>(static_cast<intptr_t>(draw->indirectOffset))));
                indirectBuffer->TrackUsage();
                break;
            }

            case Command::InsertDebugMarker:
            case Command::PopDebugGroup:
            case Command::PushDebugGroup: {
                // Due to lack of linux driver support for GL_EXT_debug_marker
                // extension these functions are skipped.
                SkipCommand(iter, type);
                break;
            }

            case Command::SetRenderPipeline: {
                SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();
                lastPipeline = ToBackend(cmd->pipeline).Get();
                DAWN_TRY(lastPipeline->ApplyNow(persistentPipelineState));

                vertexStateBufferBindingTracker.OnSetPipeline(lastPipeline);
                bindGroupTracker.OnSetPipeline(lastPipeline);
                if (lastPipeline->UsesFragDepth()) {
                    DAWN_GL_TRY(
                        gl, Uniform1f(PipelineLayout::PushConstantLocation::MinDepth, minDepth));
                    DAWN_GL_TRY(
                        gl, Uniform1f(PipelineLayout::PushConstantLocation::MaxDepth, maxDepth));
                }
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = iter->NextData<uint32_t>(cmd->dynamicOffsetCount);
                }
                bindGroupTracker.OnSetBindGroup(cmd->index, cmd->group.Get(),
                                                cmd->dynamicOffsetCount, dynamicOffsets);
                break;
            }

            case Command::SetIndexBuffer: {
                SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();

                indexBufferBaseOffset = cmd->offset;
                indexBufferFormat = IndexFormatType(cmd->format);
                indexFormatSize = IndexFormatSize(cmd->format);
                vertexStateBufferBindingTracker.OnSetIndexBuffer(cmd->buffer.Get());
                ToBackend(cmd->buffer)->TrackUsage();
                break;
            }

            case Command::SetVertexBuffer: {
                SetVertexBufferCmd* cmd = iter->NextCommand<SetVertexBufferCmd>();
                vertexStateBufferBindingTracker.OnSetVertexBuffer(cmd->slot, cmd->buffer.Get(),
                                                                  cmd->offset);
                ToBackend(cmd->buffer)->TrackUsage();
                break;
            }

            default:
                DAWN_UNREACHABLE();
                break;
        }

        return {};
    };

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                mCommands.NextCommand<EndRenderPassCmd>();

                if (renderPass->attachmentState->GetSampleCount() > 1) {
                    DAWN_TRY(ResolveMultisampledRenderTargets(gl, renderPass));
                }
                DAWN_GL_TRY(gl, DeleteFramebuffers(1, &fbo));
                return {};
            }

            case Command::SetStencilReference: {
                SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                DAWN_TRY(persistentPipelineState.SetStencilReference(gl, cmd->reference));
                break;
            }

            case Command::SetViewport: {
                SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();
                if (gl.IsAtLeastGL(4, 1)) {
                    DAWN_GL_TRY(gl, ViewportIndexedf(0, cmd->x, cmd->y, cmd->width, cmd->height));
                } else {
                    // Floating-point viewport coords are unsupported on OpenGL ES, but
                    // truncation is ok because other APIs do not guarantee subpixel precision
                    // either.
                    DAWN_GL_TRY(
                        gl, Viewport(static_cast<int>(cmd->x), static_cast<int>(cmd->y),
                                     static_cast<int>(cmd->width), static_cast<int>(cmd->height)));
                }
                minDepth = cmd->minDepth;
                maxDepth = cmd->maxDepth;
                DAWN_GL_TRY(gl, DepthRangef(minDepth, maxDepth));
                if (lastPipeline && lastPipeline->UsesFragDepth()) {
                    DAWN_GL_TRY(
                        gl, Uniform1f(PipelineLayout::PushConstantLocation::MinDepth, minDepth));
                    DAWN_GL_TRY(
                        gl, Uniform1f(PipelineLayout::PushConstantLocation::MaxDepth, maxDepth));
                }
                break;
            }

            case Command::SetScissorRect: {
                SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                DAWN_GL_TRY(gl, Scissor(cmd->x, cmd->y, cmd->width, cmd->height));
                break;
            }

            case Command::SetBlendConstant: {
                SetBlendConstantCmd* cmd = mCommands.NextCommand<SetBlendConstantCmd>();
                const std::array<float, 4> blendColor = ConvertToFloatColor(cmd->color);
                DAWN_GL_TRY(gl,
                            BlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]));
                break;
            }

            case Command::ExecuteBundles: {
                ExecuteBundlesCmd* cmd = mCommands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = mCommands.NextData<Ref<RenderBundleBase>>(cmd->count);

                for (uint32_t i = 0; i < cmd->count; ++i) {
                    CommandIterator* iter = bundles[i]->GetCommands();
                    iter->Reset();
                    while (iter->NextCommandId(&type)) {
                        DAWN_TRY(DoRenderBundleCommand(iter, type));
                    }
                }
                break;
            }

            case Command::BeginOcclusionQuery: {
                BeginOcclusionQueryCmd* cmd = mCommands.NextCommand<BeginOcclusionQueryCmd>();
                QuerySet* querySet = ToBackend(renderPass->occlusionQuerySet.Get());
                DAWN_GL_TRY(gl, BeginQuery(GL_ANY_SAMPLES_PASSED, querySet->Get(cmd->queryIndex)));
                break;
            }

            case Command::EndOcclusionQuery: {
                mCommands.NextCommand<EndOcclusionQueryCmd>();
                DAWN_GL_TRY(gl, EndQuery(GL_ANY_SAMPLES_PASSED));
                break;
            }

            case Command::WriteTimestamp:
                return DAWN_UNIMPLEMENTED_ERROR("WriteTimestamp unimplemented");

            case Command::SetImmediateData:
                return DAWN_UNIMPLEMENTED_ERROR("SetImmediateData unimplemented");

            default: {
                DAWN_TRY(DoRenderBundleCommand(&mCommands, type));
                break;
            }
        }
    }

    // EndRenderPass should have been called
    DAWN_UNREACHABLE();
}

MaybeError DoTexSubImage(const OpenGLFunctions& gl,
                         const TextureCopy& destination,
                         const void* data,
                         const TexelCopyBufferLayout& dataLayout,
                         const Extent3D& copySize) {
    Texture* texture = ToBackend(destination.texture.Get());

    const GLFormat& format = texture->GetGLFormat();
    GLenum target = texture->GetGLTarget();
    data = static_cast<const uint8_t*>(data) + dataLayout.offset;
    DAWN_GL_TRY(gl, ActiveTexture(GL_TEXTURE0));
    DAWN_GL_TRY(gl, BindTexture(target, texture->GetHandle()));
    const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(destination.aspect).block;

    uint32_t x = destination.origin.x;
    uint32_t y = destination.origin.y;
    uint32_t z = destination.origin.z;
    if (texture->GetFormat().isCompressed) {
        size_t rowSize = copySize.width / blockInfo.width * blockInfo.byteSize;
        Extent3D virtSize = texture->GetMipLevelSingleSubresourceVirtualSize(destination.mipLevel,
                                                                             destination.aspect);
        uint32_t width = std::min(copySize.width, virtSize.width - x);

        // In GLES glPixelStorei() doesn't affect CompressedTexSubImage*D() and
        // GL_UNPACK_COMPRESSED_BLOCK_* isn't defined, so we have to workaround
        // this limitation by copying the compressed texture data once per row.
        // See OpenGL ES 3.2 SPEC Chapter 8.4.1, "Pixel Storage Modes and Pixel
        // Buffer Objects" for more details. For Desktop GL, we use row-by-row
        // copies only for uploads where bytesPerRow is not a multiple of byteSize.
        if (dataLayout.bytesPerRow % blockInfo.byteSize == 0 && gl.GetVersion().IsDesktop()) {
            size_t imageSize =
                rowSize * (copySize.height / blockInfo.height) * copySize.depthOrArrayLayers;

            uint32_t height = std::min(copySize.height, virtSize.height - y);

            DAWN_GL_TRY(gl,
                        PixelStorei(GL_UNPACK_ROW_LENGTH,
                                    dataLayout.bytesPerRow / blockInfo.byteSize * blockInfo.width));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_SIZE, blockInfo.byteSize));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_WIDTH, blockInfo.width));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_HEIGHT, blockInfo.height));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_DEPTH, 1));

            if (target == GL_TEXTURE_2D) {
                DAWN_GL_TRY(
                    gl, CompressedTexSubImage2D(target, destination.mipLevel, x, y, width, height,
                                                format.internalFormat, imageSize, data));
            } else if (target == GL_TEXTURE_CUBE_MAP) {
                DAWN_ASSERT(texture->GetArrayLayers() == 6);
                const uint8_t* pointer = static_cast<const uint8_t*>(data);
                uint32_t baseLayer = destination.origin.z;
                for (uint32_t l = 0; l < copySize.depthOrArrayLayers; ++l) {
                    GLenum cubeMapTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + baseLayer + l;
                    DAWN_GL_TRY(gl, CompressedTexSubImage2D(cubeMapTarget, destination.mipLevel, x,
                                                            y, width, height, format.internalFormat,
                                                            imageSize, pointer));
                    pointer += dataLayout.rowsPerImage * dataLayout.bytesPerRow;
                }
            } else {
                DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_IMAGE_HEIGHT,
                                            dataLayout.rowsPerImage * blockInfo.height));
                DAWN_GL_TRY(gl, CompressedTexSubImage3D(target, destination.mipLevel, x, y, z,
                                                        width, height, copySize.depthOrArrayLayers,
                                                        format.internalFormat, imageSize, data));
                DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0));
            }

            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_ROW_LENGTH, 0));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_SIZE, 0));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_WIDTH, 0));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_HEIGHT, 0));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_COMPRESSED_BLOCK_DEPTH, 0));
        } else {
            if (target == GL_TEXTURE_2D) {
                const uint8_t* d = static_cast<const uint8_t*>(data);

                for (; y < destination.origin.y + copySize.height; y += blockInfo.height) {
                    uint32_t height = std::min(blockInfo.height, virtSize.height - y);
                    DAWN_GL_TRY(gl,
                                CompressedTexSubImage2D(target, destination.mipLevel, x, y, width,
                                                        height, format.internalFormat, rowSize, d));
                    d += dataLayout.bytesPerRow;
                }
            } else if (target == GL_TEXTURE_CUBE_MAP) {
                DAWN_ASSERT(texture->GetArrayLayers() == 6);
                const uint8_t* pointer = static_cast<const uint8_t*>(data);
                uint32_t baseLayer = destination.origin.z;
                for (uint32_t l = 0; l < copySize.depthOrArrayLayers; ++l) {
                    const uint8_t* d =
                        pointer + l * dataLayout.rowsPerImage * dataLayout.bytesPerRow;
                    GLenum cubeMapTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + baseLayer + l;
                    for (y = destination.origin.y; y < destination.origin.y + copySize.height;
                         y += blockInfo.height) {
                        uint32_t height = std::min(blockInfo.height, virtSize.height - y);
                        DAWN_GL_TRY(gl, CompressedTexSubImage2D(cubeMapTarget, destination.mipLevel,
                                                                x, y, width, height,
                                                                format.internalFormat, rowSize, d));
                        d += dataLayout.bytesPerRow;
                    }
                }
            } else {
                DAWN_ASSERT(target == GL_TEXTURE_3D || target == GL_TEXTURE_2D_ARRAY ||
                            target == GL_TEXTURE_CUBE_MAP_ARRAY);
                const uint8_t* slice = static_cast<const uint8_t*>(data);

                for (; z < destination.origin.z + copySize.depthOrArrayLayers; ++z) {
                    const uint8_t* d = slice;

                    for (y = destination.origin.y; y < destination.origin.y + copySize.height;
                         y += blockInfo.height) {
                        uint32_t height = std::min(blockInfo.height, virtSize.height - y);
                        DAWN_GL_TRY(gl, CompressedTexSubImage3D(target, destination.mipLevel, x, y,
                                                                z, width, height, 1,
                                                                format.internalFormat, rowSize, d));
                        d += dataLayout.bytesPerRow;
                    }

                    slice += dataLayout.rowsPerImage * dataLayout.bytesPerRow;
                }
            }
        }
    } else {
        uint32_t width = copySize.width;
        uint32_t height = copySize.height;
        GLenum adjustedFormat = format.format;
        if (format.format == GL_STENCIL) {
            DAWN_ASSERT(gl.GetVersion().IsDesktop() ||
                        gl.IsGLExtensionSupported("GL_OES_texture_stencil8"));
            adjustedFormat = GL_STENCIL_INDEX;
        }
        if (dataLayout.bytesPerRow % blockInfo.byteSize == 0) {
            // Valid values for GL_UNPACK_ALIGNMENT are 1, 2, 4, 8
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_ALIGNMENT, std::min(8u, blockInfo.byteSize)));
            DAWN_GL_TRY(gl,
                        PixelStorei(GL_UNPACK_ROW_LENGTH,
                                    dataLayout.bytesPerRow / blockInfo.byteSize * blockInfo.width));
            if (target == GL_TEXTURE_2D) {
                DAWN_GL_TRY(gl, TexSubImage2D(target, destination.mipLevel, x, y, width, height,
                                              adjustedFormat, format.type, data));
            } else if (target == GL_TEXTURE_CUBE_MAP) {
                DAWN_ASSERT(texture->GetArrayLayers() == 6);
                const uint8_t* pointer = static_cast<const uint8_t*>(data);
                uint32_t baseLayer = destination.origin.z;
                for (uint32_t l = 0; l < copySize.depthOrArrayLayers; ++l) {
                    GLenum cubeMapTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + baseLayer + l;
                    DAWN_GL_TRY(gl, TexSubImage2D(cubeMapTarget, destination.mipLevel, x, y, width,
                                                  height, adjustedFormat, format.type, pointer));
                    pointer += dataLayout.rowsPerImage * dataLayout.bytesPerRow;
                }
            } else {
                DAWN_ASSERT(target == GL_TEXTURE_3D || target == GL_TEXTURE_2D_ARRAY ||
                            target == GL_TEXTURE_CUBE_MAP_ARRAY);
                DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_IMAGE_HEIGHT,
                                            dataLayout.rowsPerImage * blockInfo.height));
                DAWN_GL_TRY(gl, TexSubImage3D(target, destination.mipLevel, x, y, z, width, height,
                                              copySize.depthOrArrayLayers, adjustedFormat,
                                              format.type, data));
                DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0));
            }
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_ROW_LENGTH, 0));
            DAWN_GL_TRY(gl, PixelStorei(GL_UNPACK_ALIGNMENT, 4));  // Reset to default
        } else {
            if (target == GL_TEXTURE_2D) {
                const uint8_t* d = static_cast<const uint8_t*>(data);
                for (; y < destination.origin.y + height; ++y) {
                    DAWN_GL_TRY(gl, TexSubImage2D(target, destination.mipLevel, x, y, width, 1,
                                                  adjustedFormat, format.type, d));
                    d += dataLayout.bytesPerRow;
                }
            } else if (target == GL_TEXTURE_CUBE_MAP) {
                DAWN_ASSERT(texture->GetArrayLayers() == 6);
                const uint8_t* pointer = static_cast<const uint8_t*>(data);
                uint32_t baseLayer = destination.origin.z;
                for (uint32_t l = 0; l < copySize.depthOrArrayLayers; ++l) {
                    const uint8_t* d = pointer;
                    GLenum cubeMapTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + baseLayer + l;
                    for (y = destination.origin.y; y < destination.origin.y + height; ++y) {
                        DAWN_GL_TRY(gl, TexSubImage2D(cubeMapTarget, destination.mipLevel, x, y,
                                                      width, 1, adjustedFormat, format.type, d));
                        d += dataLayout.bytesPerRow;
                    }
                    pointer += dataLayout.rowsPerImage * dataLayout.bytesPerRow;
                }
            } else {
                DAWN_ASSERT(target == GL_TEXTURE_3D || target == GL_TEXTURE_2D_ARRAY ||
                            target == GL_TEXTURE_CUBE_MAP_ARRAY);
                const uint8_t* slice = static_cast<const uint8_t*>(data);
                for (; z < destination.origin.z + copySize.depthOrArrayLayers; ++z) {
                    const uint8_t* d = slice;
                    for (y = destination.origin.y; y < destination.origin.y + height; ++y) {
                        DAWN_GL_TRY(gl, TexSubImage3D(target, destination.mipLevel, x, y, z, width,
                                                      1, 1, adjustedFormat, format.type, d));
                        d += dataLayout.bytesPerRow;
                    }
                    slice += dataLayout.rowsPerImage * dataLayout.bytesPerRow;
                }
            }
        }
    }

    return {};
}

}  // namespace dawn::native::opengl
