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

#include "dawn/native/opengl/TextureGL.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Queue.h"
#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/CommandBufferGL.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/SharedFenceGL.h"
#include "dawn/native/opengl/SharedTextureMemoryGL.h"
#include "dawn/native/opengl/UtilsGL.h"

namespace dawn::native::opengl {

namespace {

GLenum TargetForTextureViewDimension(wgpu::TextureViewDimension dimension, uint32_t sampleCount) {
    switch (dimension) {
        case wgpu::TextureViewDimension::e1D:
        case wgpu::TextureViewDimension::e2D:
            return (sampleCount > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        case wgpu::TextureViewDimension::e2DArray:
            if (sampleCount > 1) {
                return GL_TEXTURE_2D_MULTISAMPLE;
            }
            DAWN_ASSERT(sampleCount == 1);
            return GL_TEXTURE_2D_ARRAY;
        case wgpu::TextureViewDimension::Cube:
            DAWN_ASSERT(sampleCount == 1);
            return GL_TEXTURE_CUBE_MAP;
        case wgpu::TextureViewDimension::CubeArray:
            DAWN_ASSERT(sampleCount == 1);
            return GL_TEXTURE_CUBE_MAP_ARRAY;
        case wgpu::TextureViewDimension::e3D:
            DAWN_ASSERT(sampleCount == 1);
            return GL_TEXTURE_3D;

        case wgpu::TextureViewDimension::Undefined:
        default:
            DAWN_UNREACHABLE();
    }
}

bool RequiresCreatingNewTextureView(
    const TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& textureViewDescriptor) {
    // Compatibility mode validation should prevent the need for creation of
    // new texture views.
    if (ToBackend(texture->GetDevice())->IsCompatibilityMode()) {
        return false;
    }

    constexpr wgpu::TextureUsage kShaderUsageNeedsView =
        wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
    constexpr wgpu::TextureUsage kUsageNeedsView =
        kShaderUsageNeedsView | wgpu::TextureUsage::RenderAttachment;
    if (!(texture->GetInternalUsage() & kUsageNeedsView)) {
        return false;
    }

    if (texture->GetFormat().format != textureViewDescriptor->format &&
        !texture->GetFormat().HasDepthOrStencil()) {
        // Color format reinterpretation required. Note: Depth/stencil formats don't support
        // reinterpretation.
        return true;
    }

    // Reinterpretation not required. Now, we only need a new view if the view dimension or
    // set of subresources for the shader is different from the base texture.
    if (!(texture->GetInternalUsage() & kShaderUsageNeedsView)) {
        return false;
    }

    if (texture->GetArrayLayers() != textureViewDescriptor->arrayLayerCount ||
        (texture->GetArrayLayers() == 1 && texture->GetDimension() == wgpu::TextureDimension::e2D &&
         textureViewDescriptor->dimension == wgpu::TextureViewDimension::e2DArray)) {
        // If the view has a different number of array layers, we need a new view.
        // And, if the original texture is a 2D texture with one array layer, we need a new
        // view to view it as a 2D array texture.
        return true;
    }

    if (ToBackend(texture)->GetGLFormat().format == GL_DEPTH_STENCIL &&
        (texture->GetUsage() & wgpu::TextureUsage::TextureBinding) &&
        textureViewDescriptor->aspect == wgpu::TextureAspect::StencilOnly) {
        // We need a separate view for one of the depth or stencil planes
        // because each glTextureView needs it's own handle to set
        // GL_DEPTH_STENCIL_TEXTURE_MODE. Choose the stencil aspect for the
        // extra handle since it is likely sampled less often.
        return true;
    }

    // TODO(414312052): Use TextureViewBase::UsesNonDefaultSwizzle() instead of
    // textureViewDescriptor.
    if (auto* swizzleDesc = textureViewDescriptor.Get<TextureComponentSwizzleDescriptor>()) {
        auto swizzle = swizzleDesc->swizzle.WithTrivialFrontendDefaults();
        if (swizzle.r != wgpu::ComponentSwizzle::R || swizzle.g != wgpu::ComponentSwizzle::G ||
            swizzle.b != wgpu::ComponentSwizzle::B || swizzle.a != wgpu::ComponentSwizzle::A) {
            return true;
        }
    }

    return false;
}

MaybeError AllocateTexture(const OpenGLFunctions& gl,
                           GLenum target,
                           GLsizei samples,
                           GLuint levels,
                           const GLFormat& format,
                           const Extent3D& size) {
    if (format.isSupportedForTextureStorage || target == GL_TEXTURE_2D_MULTISAMPLE) {
        // glTextureView() requires the value of GL_TEXTURE_IMMUTABLE_FORMAT for origtexture to
        // be GL_TRUE, so the storage of the texture must be allocated with glTexStorage*D.
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTextureView.xhtml

        // There is no fallback for multisampled textures. They must be allocated with
        // glTexStorage2DMultisample
        switch (target) {
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_CUBE_MAP_ARRAY:
            case GL_TEXTURE_3D:
                DAWN_GL_TRY_ALWAYS_CHECK(
                    gl, TexStorage3D(target, levels, format.internalFormat, size.width, size.height,
                                     size.depthOrArrayLayers));
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
                DAWN_GL_TRY_ALWAYS_CHECK(gl, TexStorage2D(target, levels, format.internalFormat,
                                                          size.width, size.height));
                break;
            case GL_TEXTURE_2D_MULTISAMPLE:
                DAWN_GL_TRY_ALWAYS_CHECK(
                    gl, TexStorage2DMultisample(target, samples, format.internalFormat, size.width,
                                                size.height, true));
                break;
            default:
                DAWN_UNREACHABLE();
        }
    } else {
        // Allocate the texture using multiple glTexImage. This should only happen in compat and the
        // resulting texture will not be usable with glTextureView

        // When using glTexImage, make sure there is no unpack buffer bound or data would be copied
        // from whatever buffer happens to be bound.
        DAWN_GL_TRY_ALWAYS_CHECK(gl, BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));

        // Array texture formats have the same depth for all mip levels.
        bool constantSizeForAllLevels =
            target == GL_TEXTURE_2D_ARRAY || target == GL_TEXTURE_CUBE_MAP_ARRAY;

        for (GLuint level = 0; level < levels; level++) {
            Extent3D levelSize{
                std::max(size.width >> level, 1u), std::max(size.height >> level, 1u),
                constantSizeForAllLevels ? size.depthOrArrayLayers
                                         : std::max(size.depthOrArrayLayers >> level, 1u)};

            switch (target) {
                case GL_TEXTURE_2D_ARRAY:
                case GL_TEXTURE_CUBE_MAP_ARRAY:
                case GL_TEXTURE_3D:
                    DAWN_GL_TRY_ALWAYS_CHECK(
                        gl, TexImage3D(target, level, format.format, levelSize.width,
                                       levelSize.height, levelSize.depthOrArrayLayers, 0,
                                       format.format, format.type, nullptr));
                    break;
                case GL_TEXTURE_2D:
                    DAWN_GL_TRY_ALWAYS_CHECK(
                        gl, TexImage2D(target, level, format.format, levelSize.width,
                                       levelSize.height, 0, format.format, format.type, nullptr));
                    break;
                case GL_TEXTURE_CUBE_MAP:
                    for (size_t faceIdx = 0; faceIdx < 6; faceIdx++) {
                        GLenum faceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx;
                        DAWN_GL_TRY_ALWAYS_CHECK(
                            gl,
                            TexImage2D(faceTarget, level, format.format, levelSize.width,
                                       levelSize.height, 0, format.format, format.type, nullptr));
                    }
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
        }
    }

    return {};
}

MaybeError FramebufferTextureHelper(const OpenGLFunctions& gl,
                                    GLenum textarget,
                                    GLenum target,
                                    GLenum attachment,
                                    GLuint textureHandle,
                                    GLuint mipLevel,
                                    GLuint arrayLayer) {
    switch (textarget) {
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
        case GL_TEXTURE_3D:
            DAWN_GL_TRY(gl, FramebufferTextureLayer(target, attachment, textureHandle, mipLevel,
                                                    arrayLayer));
            break;
        case GL_TEXTURE_CUBE_MAP: {
            GLenum cubeTexTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + arrayLayer;
            DAWN_GL_TRY(gl, FramebufferTexture2D(target, attachment, cubeTexTarget, textureHandle,
                                                 mipLevel));
            break;
        }
        default:
            DAWN_ASSERT(textarget == GL_TEXTURE_2D || textarget == GL_TEXTURE_2D_MULTISAMPLE);
            DAWN_GL_TRY(
                gl, FramebufferTexture2D(target, attachment, textarget, textureHandle, mipLevel));
            break;
    }
    return {};
}

}  // namespace

// Texture

// static
ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                            const UnpackedPtr<TextureDescriptor>& descriptor) {
    const OpenGLFunctions& gl = device->GetGL();

    GLuint handle = 0;
    DAWN_GL_TRY(gl, GenTextures(1, &handle));

    // Wrap the handle in a Texture class early so that it is deleted if initialization fails
    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor, handle, OwnsHandle::Yes));

    GLenum target = texture->GetGLTarget();
    uint32_t levels = descriptor->mipLevelCount;
    const GLFormat& glFormat = texture->GetGLFormat();

    DAWN_GL_TRY(gl, BindTexture(target, handle));
    DAWN_TRY(
        AllocateTexture(gl, target, descriptor->sampleCount, levels, glFormat, descriptor->size));

    // The texture is not complete if it uses mipmapping and not all levels up to
    // MAX_LEVEL have been defined.
    DAWN_GL_TRY(gl, TexParameteri(target, GL_TEXTURE_MAX_LEVEL, levels - 1));

    if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        DAWN_TRY(
            texture->ClearTexture(texture->GetAllSubresources(), TextureBase::ClearValue::NonZero));
    }
    return std::move(texture);
}

// static
ResultOrError<Ref<Texture>> Texture::CreateFromSharedTextureMemory(
    SharedTextureMemory* memory,
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    Device* device = ToBackend(memory->GetDevice());

    GLuint textureId = 0;
    DAWN_TRY_ASSIGN(textureId, memory->GenerateGLTexture());

    Ref<Texture> texture = AcquireRef(new Texture(device, descriptor, textureId, OwnsHandle::Yes));
    texture->mSharedResourceMemoryContents = memory->GetContents();
    return texture;
}

Texture::Texture(Device* device,
                 const UnpackedPtr<TextureDescriptor>& descriptor,
                 GLuint handle,
                 OwnsHandle ownsHandle)
    : TextureBase(device, descriptor), mHandle(handle), mOwnsHandle(ownsHandle) {
    mTarget = TargetForTextureViewDimension(GetCompatibilityTextureBindingViewDimension(),
                                            descriptor->sampleCount);
}

Texture::~Texture() {}

void Texture::DestroyImpl() {
    TextureBase::DestroyImpl();
    if (mOwnsHandle == OwnsHandle::Yes) {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
        DAWN_GL_TRY_IGNORE_ERRORS(gl, DeleteTextures(1, &mHandle));
        mHandle = 0;
    }
}

GLuint Texture::GetHandle() const {
    return mHandle;
}

GLenum Texture::GetGLTarget() const {
    return mTarget;
}

const GLFormat& Texture::GetGLFormat() const {
    return ToBackend(GetDevice())->GetGLFormat(GetFormat());
}

MaybeError Texture::ClearTexture(const SubresourceRange& range,
                                 TextureBase::ClearValue clearValue) {
    Device* device = ToBackend(GetDevice());
    const OpenGLFunctions& gl = device->GetGL();

    uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
    float fClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.f : 1.f;

    if (GetFormat().isRenderable) {
        if (range.aspects & (Aspect::Depth | Aspect::Stencil)) {
            GLfloat depth = fClearColor;
            GLint stencil = clearColor;
            if (range.aspects & Aspect::Depth) {
                DAWN_GL_TRY(gl, DepthMask(GL_TRUE));
            }
            if (range.aspects & Aspect::Stencil) {
                DAWN_GL_TRY(gl, StencilMask(GetStencilMaskFromStencilFormat(GetFormat().format)));
            }

            auto DoClear = [&](Aspect aspects) -> MaybeError {
                if (aspects == (Aspect::Depth | Aspect::Stencil)) {
                    DAWN_GL_TRY(gl, ClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil));
                } else if (aspects == Aspect::Depth) {
                    DAWN_GL_TRY(gl, ClearBufferfv(GL_DEPTH, 0, &depth));
                } else if (aspects == Aspect::Stencil) {
                    DAWN_GL_TRY(gl, ClearBufferiv(GL_STENCIL, 0, &stencil));
                } else {
                    DAWN_UNREACHABLE();
                }
                return {};
            };

            GLuint framebuffer = 0;
            DAWN_GL_TRY(gl, GenFramebuffers(1, &framebuffer));
            DAWN_GL_TRY(gl, BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer));
            DAWN_GL_TRY(gl, Disable(GL_SCISSOR_TEST));

            GLenum attachment;
            if (range.aspects == (Aspect::Depth | Aspect::Stencil)) {
                attachment = GL_DEPTH_STENCIL_ATTACHMENT;
            } else if (range.aspects == Aspect::Depth) {
                attachment = GL_DEPTH_ATTACHMENT;
            } else if (range.aspects == Aspect::Stencil) {
                attachment = GL_STENCIL_ATTACHMENT;
            } else {
                DAWN_UNREACHABLE();
            }

            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                for (uint32_t layer = range.baseArrayLayer;
                     layer < range.baseArrayLayer + range.layerCount; ++layer) {
                    Aspect aspectsToClear = Aspect::None;
                    for (Aspect aspect : IterateEnumMask(range.aspects)) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, layer, aspect))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }
                        aspectsToClear |= aspect;
                    }

                    if (aspectsToClear == Aspect::None) {
                        continue;
                    }
                    DAWN_TRY(FramebufferTextureHelper(gl, mTarget, GL_DRAW_FRAMEBUFFER, attachment,
                                                      GetHandle(), level, layer));
                    DAWN_TRY(DoClear(aspectsToClear));
                }
            }

            DAWN_GL_TRY(gl, Enable(GL_SCISSOR_TEST));
            DAWN_GL_TRY(gl, DeleteFramebuffers(1, &framebuffer));
        } else {
            DAWN_ASSERT(range.aspects == Aspect::Color);

            // For gl.ClearBufferiv/uiv calls
            constexpr std::array<GLuint, 4> kClearColorDataUint0 = {0u, 0u, 0u, 0u};
            constexpr std::array<GLuint, 4> kClearColorDataUint1 = {1u, 1u, 1u, 1u};
            std::array<GLuint, 4> clearColorData;
            clearColorData.fill((clearValue == TextureBase::ClearValue::Zero) ? 0u : 1u);

            // For gl.ClearBufferfv calls
            constexpr std::array<GLfloat, 4> kClearColorDataFloat0 = {0.f, 0.f, 0.f, 0.f};
            constexpr std::array<GLfloat, 4> kClearColorDataFloat1 = {1.f, 1.f, 1.f, 1.f};
            std::array<GLfloat, 4> fClearColorData;
            fClearColorData.fill((clearValue == TextureBase::ClearValue::Zero) ? 0.f : 1.f);

            static constexpr uint32_t MAX_TEXEL_SIZE = 16;
            const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(Aspect::Color).block;
            DAWN_ASSERT(blockInfo.byteSize <= MAX_TEXEL_SIZE);

            // For gl.ClearTexSubImage calls
            constexpr std::array<GLbyte, MAX_TEXEL_SIZE> kClearColorDataBytes0 = {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            constexpr std::array<GLbyte, MAX_TEXEL_SIZE> kClearColorDataBytes255 = {
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

            TextureComponentType baseType = GetFormat().GetAspectInfo(Aspect::Color).baseType;

            const GLFormat& glFormat = GetGLFormat();
            const auto dimension = GetDimension();
            for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
                 ++level) {
                Extent3D mipSize = GetMipLevelSingleSubresourcePhysicalSize(level, Aspect::Color);
                for (uint32_t layer = range.baseArrayLayer;
                     layer < range.baseArrayLayer + range.layerCount; ++layer) {
                    if (clearValue == TextureBase::ClearValue::Zero &&
                        IsSubresourceContentInitialized(
                            SubresourceRange::SingleMipAndLayer(level, layer, Aspect::Color))) {
                        // Skip lazy clears if already initialized.
                        continue;
                    }
                    if (gl.IsAtLeastGL(4, 4)) {
                        DAWN_GL_TRY(gl, ClearTexSubImage(mHandle, static_cast<GLint>(level), 0, 0,
                                                         static_cast<GLint>(layer), mipSize.width,
                                                         mipSize.height, mipSize.depthOrArrayLayers,
                                                         glFormat.format, glFormat.type,
                                                         clearValue == TextureBase::ClearValue::Zero
                                                             ? kClearColorDataBytes0.data()
                                                             : kClearColorDataBytes255.data()));
                        continue;
                    }

                    GLuint framebuffer = 0;
                    DAWN_GL_TRY(gl, GenFramebuffers(1, &framebuffer));
                    DAWN_GL_TRY(gl, BindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer));

                    GLenum attachment = GL_COLOR_ATTACHMENT0;
                    DAWN_GL_TRY(gl, DrawBuffers(1, &attachment));

                    DAWN_GL_TRY(gl, Disable(GL_SCISSOR_TEST));
                    DAWN_GL_TRY(gl, ColorMask(true, true, true, true));

                    auto DoClear = [&]() -> MaybeError {
                        switch (baseType) {
                            case TextureComponentType::Float: {
                                DAWN_GL_TRY(
                                    gl, ClearBufferfv(GL_COLOR, 0,
                                                      clearValue == TextureBase::ClearValue::Zero
                                                          ? kClearColorDataFloat0.data()
                                                          : kClearColorDataFloat1.data()));
                                break;
                            }
                            case TextureComponentType::Uint: {
                                DAWN_GL_TRY(
                                    gl, ClearBufferuiv(GL_COLOR, 0,
                                                       clearValue == TextureBase::ClearValue::Zero
                                                           ? kClearColorDataUint0.data()
                                                           : kClearColorDataUint1.data()));
                                break;
                            }
                            case TextureComponentType::Sint: {
                                DAWN_GL_TRY(gl, ClearBufferiv(
                                                    GL_COLOR, 0,
                                                    reinterpret_cast<const GLint*>(
                                                        clearValue == TextureBase::ClearValue::Zero
                                                            ? kClearColorDataUint0.data()
                                                            : kClearColorDataUint1.data())));
                                break;
                            }
                        }
                        return {};
                    };

                    if (dimension == wgpu::TextureDimension::e3D) {
                        uint32_t depth =
                            GetMipLevelSingleSubresourceVirtualSize(level, Aspect::Color)
                                .depthOrArrayLayers;
                        for (GLint z = 0; z < static_cast<GLint>(depth); ++z) {
                            DAWN_GL_TRY(gl, FramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachment,
                                                                    GetHandle(), level, z));
                            DAWN_TRY(DoClear());
                        }
                    } else {
                        DAWN_TRY(FramebufferTextureHelper(gl, mTarget, GL_DRAW_FRAMEBUFFER,
                                                          attachment, GetHandle(), level, layer));
                        DAWN_TRY(DoClear());
                    }

                    DAWN_GL_TRY(gl, Enable(GL_SCISSOR_TEST));
                    DAWN_GL_TRY(gl, DeleteFramebuffers(1, &framebuffer));
                    DAWN_GL_TRY(gl, BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
                }
            }
        }
    } else {
        DAWN_ASSERT(range.aspects == Aspect::Color);

        // create temp buffer with clear color to copy to the texture image
        const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(Aspect::Color).block;
        DAWN_ASSERT(kTextureBytesPerRowAlignment % blockInfo.byteSize == 0);

        Extent3D largestMipSize =
            GetMipLevelSingleSubresourcePhysicalSize(range.baseMipLevel, Aspect::Color);
        uint32_t bytesPerRow =
            Align((largestMipSize.width / blockInfo.width) * blockInfo.byteSize, 4);

        // Make sure that we are not rounding
        DAWN_ASSERT(bytesPerRow % blockInfo.byteSize == 0);
        DAWN_ASSERT(largestMipSize.height % blockInfo.height == 0);

        uint64_t bufferSize64 = static_cast<uint64_t>(bytesPerRow) *
                                (largestMipSize.height / blockInfo.height) *
                                largestMipSize.depthOrArrayLayers;
        if (bufferSize64 > std::numeric_limits<size_t>::max()) {
            return DAWN_OUT_OF_MEMORY_ERROR("Unable to allocate buffer.");
        }
        size_t bufferSize = static_cast<size_t>(bufferSize64);

        dawn::native::BufferDescriptor descriptor = {};
        descriptor.mappedAtCreation = true;
        descriptor.usage = wgpu::BufferUsage::CopySrc;
        descriptor.size = bufferSize;

        // We don't count the lazy clear of srcBuffer because it is an internal buffer.
        // TODO(natlee@microsoft.com): use Dynamic Uploader here for temp buffer
        Ref<Buffer> srcBuffer;
        DAWN_TRY_ASSIGN(srcBuffer, Buffer::CreateInternalBuffer(device, &descriptor, false));

        // Fill the buffer with clear color
        memset(srcBuffer->GetMappedRange(0, bufferSize), clearColor, bufferSize);
        DAWN_TRY(srcBuffer->Unmap());

        DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_UNPACK_BUFFER, srcBuffer->GetHandle()));
        for (uint32_t level = range.baseMipLevel; level < range.baseMipLevel + range.levelCount;
             ++level) {
            TextureCopy textureCopy;
            textureCopy.texture = this;
            textureCopy.mipLevel = level;
            textureCopy.origin = {};
            textureCopy.aspect = Aspect::Color;

            TexelCopyBufferLayout dataLayout;
            dataLayout.offset = 0;
            dataLayout.bytesPerRow = bytesPerRow;
            dataLayout.rowsPerImage = largestMipSize.height / blockInfo.height;

            Extent3D mipSize = GetMipLevelSingleSubresourcePhysicalSize(level, Aspect::Color);

            for (uint32_t layer = range.baseArrayLayer;
                 layer < range.baseArrayLayer + range.layerCount; ++layer) {
                if (clearValue == TextureBase::ClearValue::Zero &&
                    IsSubresourceContentInitialized(
                        SubresourceRange::SingleMipAndLayer(level, layer, Aspect::Color))) {
                    // Skip lazy clears if already initialized.
                    continue;
                }

                textureCopy.origin.z = layer;
                DAWN_TRY(DoTexSubImage(gl, textureCopy, 0, dataLayout, mipSize));
            }
        }
        DAWN_GL_TRY(gl, BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
    }
    if (clearValue == TextureBase::ClearValue::Zero) {
        SetIsSubresourceContentInitialized(true, range);
        device->IncrementLazyClearCountForTesting();
    }
    return {};
}

MaybeError Texture::EnsureSubresourceContentInitialized(const SubresourceRange& range) {
    if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        return {};
    }
    if (!IsSubresourceContentInitialized(range)) {
        DAWN_TRY(ClearTexture(range, TextureBase::ClearValue::Zero));
    }
    return {};
}

MaybeError Texture::SynchronizeTextureBeforeUse() {
    SharedTextureMemoryBase::PendingFenceList fences;
    SharedResourceMemoryContents* contents = GetSharedResourceMemoryContents();
    if (contents != nullptr) {
        contents->AcquirePendingFences(&fences);
    }
    for (const auto& fenceAndSignaledValue : fences) {
        SharedFence* fence = ToBackend(fenceAndSignaledValue.object).Get();
        DAWN_TRY(fence->ServerWait(fenceAndSignaledValue.signaledValue));
    }

    mLastSharedTextureMemoryUsageSerial = GetDevice()->GetQueue()->GetPendingCommandSerial();
    return {};
}

// TextureView

// static
ResultOrError<Ref<TextureView>> TextureView::Create(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    const OpenGLFunctions& gl = ToBackend(texture->GetDevice())->GetGL();
    GLuint handle = 0;
    OwnsHandle ownsHandle = OwnsHandle::No;

    // Texture could be destroyed by the time we make a view.
    if (texture->IsDestroyed()) {
        handle = 0;
    } else if (!RequiresCreatingNewTextureView(texture, descriptor)) {
        handle = ToBackend(texture)->GetHandle();
    } else {
        if (gl.IsAtLeastGL(4, 3)) {
            DAWN_GL_TRY(gl, GenTextures(1, &handle));
            ownsHandle = OwnsHandle::Yes;
        } else {
            handle = 0;
        }
    }

    // Wrap the handle in a TextureView class early so that it is deleted if initialization fails
    Ref<TextureView> view = AcquireRef(new TextureView(texture, descriptor, handle, ownsHandle));

    if (ownsHandle == OwnsHandle::Yes) {
        DAWN_GL_TRY(gl, TextureView(handle, view->GetGLTarget(), ToBackend(texture)->GetHandle(),
                                    view->GetInternalFormat(), descriptor->baseMipLevel,
                                    descriptor->mipLevelCount, descriptor->baseArrayLayer,
                                    descriptor->arrayLayerCount));
    }

    return view;
}

TextureView::TextureView(TextureBase* texture,
                         const UnpackedPtr<TextureViewDescriptor>& descriptor,
                         GLuint handle,
                         OwnsHandle ownsHandle)
    : TextureViewBase(texture, descriptor), mHandle(handle), mOwnsHandle(ownsHandle) {
    mTarget = TargetForTextureViewDimension(descriptor->dimension, texture->GetSampleCount());
}

TextureView::~TextureView() {}

void TextureView::DestroyImpl() {
    TextureViewBase::DestroyImpl();
    if (mOwnsHandle == OwnsHandle::Yes) {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
        DAWN_GL_TRY_IGNORE_ERRORS(gl, DeleteTextures(1, &mHandle));
    }
}

GLuint TextureView::GetHandle() const {
    DAWN_ASSERT(mHandle != 0);
    return mHandle;
}

GLenum TextureView::GetGLTarget() const {
    return mTarget;
}

MaybeError TextureView::BindToFramebuffer(GLenum target, GLenum attachment, GLuint depthSlice) {
    DAWN_ASSERT(depthSlice <
                static_cast<GLuint>(GetSingleSubresourceVirtualSize().depthOrArrayLayers));

    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    // Use the base texture where possible to minimize the amount of copying required on GLES.
    bool useOwnView = GetFormat().format != GetTexture()->GetFormat().format &&
                      !GetTexture()->GetFormat().HasDepthOrStencil();

    GLenum textarget;
    GLuint textureHandle, mipLevel, arrayLayer;
    if (useOwnView) {
        // Use our own texture handle and target which points to a subset of the texture's
        // subresources.
        textureHandle = GetHandle();
        textarget = GetGLTarget();
        mipLevel = 0;
        arrayLayer = 0;
    } else {
        // Use the texture's handle and target, with the view's base mip level and base array

        textureHandle = ToBackend(GetTexture())->GetHandle();
        textarget = ToBackend(GetTexture())->GetGLTarget();
        mipLevel = GetBaseMipLevel();
        // We have validated that the depthSlice in render pass's colorAttachments must be undefined
        // for 2d RTVs, which value is set to 0. For 3d RTVs, the baseArrayLayer must be 0. So here
        // we can simply use baseArrayLayer + depthSlice to specify the slice in RTVs without
        // checking the view's dimension.
        arrayLayer = GetBaseArrayLayer() + depthSlice;
    }

    DAWN_ASSERT(textureHandle != 0);

    return FramebufferTextureHelper(gl, textarget, target, attachment, textureHandle, mipLevel,
                                    arrayLayer);
}

GLenum TextureView::GetInternalFormat() const {
    // Depth/stencil don't support reinterpretation, and the aspect is specified at
    // bind time. In that case, we use the base texture format.
    const Format& format =
        GetFormat().HasDepthOrStencil() ? GetTexture()->GetFormat() : GetFormat();
    const GLFormat& glFormat = ToBackend(GetDevice())->GetGLFormat(format);
    return glFormat.internalFormat;
}

}  // namespace dawn::native::opengl
