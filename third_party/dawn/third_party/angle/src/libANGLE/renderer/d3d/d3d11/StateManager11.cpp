//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManager11.cpp: Defines a class for caching D3D11 state

#include "libANGLE/renderer/d3d/d3d11/StateManager11.h"

#include "common/angleutils.h"
#include "common/bitset_utils.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Query.h"
#include "libANGLE/Surface.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/renderer/d3d/DisplayD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Framebuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/IndexBuffer11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/ShaderExecutable11.h"
#include "libANGLE/renderer/d3d/d3d11/TextureStorage11.h"
#include "libANGLE/renderer/d3d/d3d11/TransformFeedback11.h"
#include "libANGLE/renderer/d3d/d3d11/VertexArray11.h"
#include "libANGLE/renderer/d3d/d3d11/VertexBuffer11.h"

namespace rx
{

namespace
{
bool ImageIndexConflictsWithSRV(const gl::ImageIndex &index, D3D11_SHADER_RESOURCE_VIEW_DESC desc)
{
    unsigned mipLevel           = index.getLevelIndex();
    gl::TextureType textureType = index.getType();

    switch (desc.ViewDimension)
    {
        case D3D11_SRV_DIMENSION_TEXTURE2D:
        {
            bool allLevels         = (desc.Texture2D.MipLevels == std::numeric_limits<UINT>::max());
            unsigned int maxSrvMip = desc.Texture2D.MipLevels + desc.Texture2D.MostDetailedMip;
            maxSrvMip              = allLevels ? INT_MAX : maxSrvMip;

            unsigned mipMin = index.getLevelIndex();
            unsigned mipMax = INT_MAX;

            return textureType == gl::TextureType::_2D &&
                   gl::RangeUI(mipMin, mipMax)
                       .intersects(gl::RangeUI(desc.Texture2D.MostDetailedMip, maxSrvMip));
        }

        case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
        {
            GLint layerIndex = index.getLayerIndex();

            bool allLevels = (desc.Texture2DArray.MipLevels == std::numeric_limits<UINT>::max());
            unsigned int maxSrvMip =
                desc.Texture2DArray.MipLevels + desc.Texture2DArray.MostDetailedMip;
            maxSrvMip = allLevels ? INT_MAX : maxSrvMip;

            unsigned maxSlice = desc.Texture2DArray.FirstArraySlice + desc.Texture2DArray.ArraySize;

            // Cube maps can be mapped to Texture2DArray SRVs
            return (textureType == gl::TextureType::_2DArray ||
                    textureType == gl::TextureType::CubeMap) &&
                   desc.Texture2DArray.MostDetailedMip <= mipLevel && mipLevel < maxSrvMip &&
                   desc.Texture2DArray.FirstArraySlice <= static_cast<UINT>(layerIndex) &&
                   static_cast<UINT>(layerIndex) < maxSlice;
        }

        case D3D11_SRV_DIMENSION_TEXTURECUBE:
        {
            bool allLevels = (desc.TextureCube.MipLevels == std::numeric_limits<UINT>::max());
            unsigned int maxSrvMip = desc.TextureCube.MipLevels + desc.TextureCube.MostDetailedMip;
            maxSrvMip              = allLevels ? INT_MAX : maxSrvMip;

            return textureType == gl::TextureType::CubeMap &&
                   desc.TextureCube.MostDetailedMip <= mipLevel && mipLevel < maxSrvMip;
        }

        case D3D11_SRV_DIMENSION_TEXTURE3D:
        {
            bool allLevels         = (desc.Texture3D.MipLevels == std::numeric_limits<UINT>::max());
            unsigned int maxSrvMip = desc.Texture3D.MipLevels + desc.Texture3D.MostDetailedMip;
            maxSrvMip              = allLevels ? INT_MAX : maxSrvMip;

            return textureType == gl::TextureType::_3D &&
                   desc.Texture3D.MostDetailedMip <= mipLevel && mipLevel < maxSrvMip;
        }
        default:
            // We only handle the cases corresponding to valid image indexes
            UNIMPLEMENTED();
    }

    return false;
}

bool ImageIndexConflictsWithUAV(const gl::ImageIndex &index, D3D11_UNORDERED_ACCESS_VIEW_DESC desc)
{
    unsigned mipLevel           = index.getLevelIndex();
    gl::TextureType textureType = index.getType();

    switch (desc.ViewDimension)
    {
        case D3D11_UAV_DIMENSION_TEXTURE2D:
        {
            return textureType == gl::TextureType::_2D && mipLevel == desc.Texture2D.MipSlice;
        }

        case D3D11_UAV_DIMENSION_TEXTURE2DARRAY:
        {
            GLint layerIndex         = index.getLayerIndex();
            unsigned mipSlice        = desc.Texture2DArray.MipSlice;
            unsigned firstArraySlice = desc.Texture2DArray.FirstArraySlice;
            unsigned lastArraySlice  = firstArraySlice + desc.Texture2DArray.ArraySize;

            return (textureType == gl::TextureType::_2DArray ||
                    textureType == gl::TextureType::CubeMap) &&
                   (mipLevel == mipSlice && gl::RangeUI(firstArraySlice, lastArraySlice)
                                                .contains(static_cast<UINT>(layerIndex)));
        }

        case D3D11_UAV_DIMENSION_TEXTURE3D:
        {
            GLint layerIndex     = index.getLayerIndex();
            unsigned mipSlice    = desc.Texture3D.MipSlice;
            unsigned firstWSlice = desc.Texture3D.FirstWSlice;
            unsigned lastWSlice  = firstWSlice + desc.Texture3D.WSize;

            return textureType == gl::TextureType::_3D &&
                   (mipLevel == mipSlice &&
                    gl::RangeUI(firstWSlice, lastWSlice).contains(static_cast<UINT>(layerIndex)));
        }
        default:
            return false;
    }
}

// Does *not* increment the resource ref count!!
ID3D11Resource *GetViewResource(ID3D11View *view)
{
    ID3D11Resource *resource = nullptr;
    ASSERT(view);
    view->GetResource(&resource);
    resource->Release();
    return resource;
}

int GetWrapBits(GLenum wrap)
{
    const int wrapBits = gl_d3d11::ConvertTextureWrap(wrap);
    ASSERT(wrapBits >= 1 && wrapBits <= 5);
    return wrapBits;
}

Optional<size_t> FindFirstNonInstanced(
    const std::vector<const TranslatedAttribute *> &currentAttributes)
{
    for (size_t index = 0; index < currentAttributes.size(); ++index)
    {
        if (currentAttributes[index]->divisor == 0)
        {
            return Optional<size_t>(index);
        }
    }

    return Optional<size_t>::Invalid();
}

void SortAttributesByLayout(const ProgramExecutableD3D &executableD3D,
                            const std::vector<TranslatedAttribute> &vertexArrayAttribs,
                            const std::vector<TranslatedAttribute> &currentValueAttribs,
                            AttribIndexArray *sortedD3DSemanticsOut,
                            std::vector<const TranslatedAttribute *> *sortedAttributesOut)
{
    sortedAttributesOut->clear();

    const AttribIndexArray &locationToSemantic = executableD3D.getAttribLocationToD3DSemantics();
    const gl::ProgramExecutable *executable    = executableD3D.getExecutable();

    for (auto locationIndex : executable->getActiveAttribLocationsMask())
    {
        int d3dSemantic = locationToSemantic[locationIndex];
        if (sortedAttributesOut->size() <= static_cast<size_t>(d3dSemantic))
        {
            sortedAttributesOut->resize(d3dSemantic + 1);
        }

        (*sortedD3DSemanticsOut)[d3dSemantic] = d3dSemantic;

        const auto *arrayAttrib = &vertexArrayAttribs[locationIndex];
        if (arrayAttrib->attribute && arrayAttrib->attribute->enabled)
        {
            (*sortedAttributesOut)[d3dSemantic] = arrayAttrib;
        }
        else
        {
            ASSERT(currentValueAttribs[locationIndex].attribute);
            (*sortedAttributesOut)[d3dSemantic] = &currentValueAttribs[locationIndex];
        }
    }
}

void UpdateUniformBuffer(ID3D11DeviceContext *deviceContext,
                         UniformStorage11 *storage,
                         const d3d11::Buffer *buffer)
{
    deviceContext->UpdateSubresource(buffer->get(), 0, nullptr, storage->getDataPointer(0, 0), 0,
                                     0);
}

size_t GetReservedBufferCount(bool usesPointSpriteEmulation)
{
    return usesPointSpriteEmulation ? 1 : 0;
}

bool CullsEverything(const gl::State &glState)
{
    return (glState.getRasterizerState().cullFace &&
            glState.getRasterizerState().cullMode == gl::CullFaceMode::FrontAndBack);
}
}  // anonymous namespace

// StateManager11::ViewCache Implementation.
template <typename ViewType, typename DescType>
StateManager11::ViewCache<ViewType, DescType>::ViewCache() : mHighestUsedView(0)
{}

template <typename ViewType, typename DescType>
StateManager11::ViewCache<ViewType, DescType>::~ViewCache()
{}

template <typename ViewType, typename DescType>
void StateManager11::ViewCache<ViewType, DescType>::update(size_t resourceIndex, ViewType *view)
{
    ASSERT(resourceIndex < mCurrentViews.size());
    ViewRecord<DescType> *record = &mCurrentViews[resourceIndex];

    record->view = reinterpret_cast<uintptr_t>(view);
    if (view)
    {
        record->resource = reinterpret_cast<uintptr_t>(GetViewResource(view));
        view->GetDesc(&record->desc);
        mHighestUsedView = std::max(resourceIndex + 1, mHighestUsedView);
    }
    else
    {
        record->resource = 0;

        if (resourceIndex + 1 == mHighestUsedView)
        {
            do
            {
                --mHighestUsedView;
            } while (mHighestUsedView > 0 && mCurrentViews[mHighestUsedView].view == 0);
        }
    }
}

template <typename ViewType, typename DescType>
void StateManager11::ViewCache<ViewType, DescType>::clear()
{
    if (mCurrentViews.empty())
    {
        return;
    }

    memset(&mCurrentViews[0], 0, sizeof(ViewRecord<DescType>) * mCurrentViews.size());
    mHighestUsedView = 0;
}

StateManager11::SRVCache *StateManager11::getSRVCache(gl::ShaderType shaderType)
{
    ASSERT(shaderType != gl::ShaderType::InvalidEnum);
    return &mCurShaderSRVs[shaderType];
}

// ShaderConstants11 implementation
ShaderConstants11::ShaderConstants11() : mNumActiveShaderSamplers({})
{
    mShaderConstantsDirty.set();
}

ShaderConstants11::~ShaderConstants11() {}

void ShaderConstants11::init(const gl::Caps &caps)
{
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mShaderSamplerMetadata[shaderType].resize(caps.maxShaderTextureImageUnits[shaderType]);
        mShaderReadonlyImageMetadata[shaderType].resize(caps.maxShaderImageUniforms[shaderType]);
        mShaderImageMetadata[shaderType].resize(caps.maxShaderImageUniforms[shaderType]);
    }
}

size_t ShaderConstants11::GetShaderConstantsStructSize(gl::ShaderType shaderType)
{
    switch (shaderType)
    {
        case gl::ShaderType::Vertex:
            return sizeof(Vertex);
        case gl::ShaderType::Fragment:
            return sizeof(Pixel);
        case gl::ShaderType::Compute:
            return sizeof(Compute);

        // TODO(jiawei.shao@intel.com): return geometry shader constant struct size
        case gl::ShaderType::Geometry:
            return 0u;

        default:
            UNREACHABLE();
            return 0u;
    }
}

size_t ShaderConstants11::getRequiredBufferSize(gl::ShaderType shaderType) const
{
    ASSERT(shaderType != gl::ShaderType::InvalidEnum);
    return GetShaderConstantsStructSize(shaderType) +
           mShaderSamplerMetadata[shaderType].size() * sizeof(SamplerMetadata) +
           mShaderImageMetadata[shaderType].size() * sizeof(ImageMetadata) +
           mShaderReadonlyImageMetadata[shaderType].size() * sizeof(ImageMetadata);
}

void ShaderConstants11::markDirty()
{
    mShaderConstantsDirty.set();
    mNumActiveShaderSamplers.fill(0);
}

bool ShaderConstants11::updateSamplerMetadata(SamplerMetadata *data,
                                              const gl::Texture &texture,
                                              const gl::SamplerState &samplerState)
{
    bool dirty               = false;
    unsigned int baseLevel   = texture.getTextureState().getEffectiveBaseLevel();
    gl::TextureTarget target = (texture.getType() == gl::TextureType::CubeMap)
                                   ? gl::kCubeMapTextureTargetMin
                                   : gl::NonCubeTextureTypeToTarget(texture.getType());
    if (data->baseLevel != static_cast<int>(baseLevel))
    {
        data->baseLevel = static_cast<int>(baseLevel);
        dirty           = true;
    }

    // Some metadata is needed only for integer textures. We avoid updating the constant buffer
    // unnecessarily by changing the data only in case the texture is an integer texture and
    // the values have changed.
    const gl::InternalFormat &info = *texture.getFormat(target, baseLevel).info;
    if (!info.isInt() && !texture.getState().isStencilMode())
    {
        return dirty;
    }

    // Pack the wrap values into one integer so we can fit all the metadata in two 4-integer
    // vectors.
    const GLenum wrapS = samplerState.getWrapS();
    const GLenum wrapT = samplerState.getWrapT();
    const GLenum wrapR = samplerState.getWrapR();
    const int wrapModes =
        GetWrapBits(wrapS) | (GetWrapBits(wrapT) << 3) | (GetWrapBits(wrapR) << 6);
    if (data->wrapModes != wrapModes)
    {
        data->wrapModes = wrapModes;
        dirty           = true;
    }

    // Skip checking and syncing integer border color if it is not used
    if (wrapS != GL_CLAMP_TO_BORDER && wrapT != GL_CLAMP_TO_BORDER && wrapR != GL_CLAMP_TO_BORDER)
    {
        return dirty;
    }

    // Use the sampler state border color only if it is integer, initialize to zeros otherwise
    angle::ColorGeneric borderColor;
    ASSERT(borderColor.colorI.red == 0 && borderColor.colorI.green == 0 &&
           borderColor.colorI.blue == 0 && borderColor.colorI.alpha == 0);
    if (samplerState.getBorderColor().type != angle::ColorGeneric::Type::Float)
    {
        borderColor = samplerState.getBorderColor();
    }

    // Adjust the border color value to the texture format
    borderColor = AdjustBorderColor<false>(
        borderColor,
        angle::Format::Get(angle::Format::InternalFormatToID(info.sizedInternalFormat)),
        texture.getState().isStencilMode());

    ASSERT(static_cast<const void *>(borderColor.colorI.data()) ==
           static_cast<const void *>(borderColor.colorUI.data()));
    if (memcmp(data->intBorderColor, borderColor.colorI.data(), sizeof(data->intBorderColor)) != 0)
    {
        memcpy(data->intBorderColor, borderColor.colorI.data(), sizeof(data->intBorderColor));
        dirty = true;
    }

    return dirty;
}

bool ShaderConstants11::updateImageMetadata(ImageMetadata *data, const gl::ImageUnit &imageUnit)
{
    bool dirty = false;

    if (data->layer != static_cast<int>(imageUnit.layer))
    {
        data->layer = static_cast<int>(imageUnit.layer);
        dirty       = true;
    }

    if (data->level != static_cast<unsigned int>(imageUnit.level))
    {
        data->level = static_cast<unsigned int>(imageUnit.level);
        dirty       = true;
    }

    return dirty;
}

void ShaderConstants11::setComputeWorkGroups(GLuint numGroupsX,
                                             GLuint numGroupsY,
                                             GLuint numGroupsZ)
{
    mCompute.numWorkGroups[0] = numGroupsX;
    mCompute.numWorkGroups[1] = numGroupsY;
    mCompute.numWorkGroups[2] = numGroupsZ;
    mShaderConstantsDirty.set(gl::ShaderType::Compute);
}

void ShaderConstants11::onViewportChange(const gl::Rectangle &glViewport,
                                         const D3D11_VIEWPORT &dxViewport,
                                         const gl::Offset &glFragCoordOffset,
                                         bool is9_3,
                                         bool presentPathFast)
{
    mShaderConstantsDirty.set(gl::ShaderType::Vertex);
    mShaderConstantsDirty.set(gl::ShaderType::Fragment);

    // On Feature Level 9_*, we must emulate large and/or negative viewports in the shaders
    // using viewAdjust (like the D3D9 renderer).
    if (is9_3)
    {
        mVertex.viewAdjust[0] = static_cast<float>((glViewport.width - dxViewport.Width) +
                                                   2 * (glViewport.x - dxViewport.TopLeftX)) /
                                dxViewport.Width;
        mVertex.viewAdjust[1] = static_cast<float>((glViewport.height - dxViewport.Height) +
                                                   2 * (glViewport.y - dxViewport.TopLeftY)) /
                                dxViewport.Height;
        mVertex.viewAdjust[2] = static_cast<float>(glViewport.width) / dxViewport.Width;
        mVertex.viewAdjust[3] = static_cast<float>(glViewport.height) / dxViewport.Height;
    }

    mPixel.viewCoords[0] = glViewport.width * 0.5f;
    mPixel.viewCoords[1] = glViewport.height * 0.5f;
    mPixel.viewCoords[2] = glViewport.x + (glViewport.width * 0.5f);
    mPixel.viewCoords[3] = glViewport.y + (glViewport.height * 0.5f);

    // Instanced pointsprite emulation requires ViewCoords to be defined in the
    // the vertex shader.
    mVertex.viewCoords[0] = mPixel.viewCoords[0];
    mVertex.viewCoords[1] = mPixel.viewCoords[1];
    mVertex.viewCoords[2] = mPixel.viewCoords[2];
    mVertex.viewCoords[3] = mPixel.viewCoords[3];

    const float zNear = dxViewport.MinDepth;
    const float zFar  = dxViewport.MaxDepth;

    mPixel.depthFront[0] = (zFar - zNear) * 0.5f;
    mPixel.depthFront[1] = (zNear + zFar) * 0.5f;

    mVertex.depthRange[0] = zNear;
    mVertex.depthRange[1] = zFar;
    mVertex.depthRange[2] = zFar - zNear;

    mPixel.depthRange[0] = zNear;
    mPixel.depthRange[1] = zFar;
    mPixel.depthRange[2] = zFar - zNear;

    mPixel.viewScale[0] = 1.0f;
    mPixel.viewScale[1] = presentPathFast ? 1.0f : -1.0f;

    mVertex.viewScale[0] = mPixel.viewScale[0];
    mVertex.viewScale[1] = mPixel.viewScale[1];

    mPixel.fragCoordOffset[0] = static_cast<float>(glFragCoordOffset.x);
    mPixel.fragCoordOffset[1] = static_cast<float>(glFragCoordOffset.y);
}

// Update the ShaderConstants with a new first vertex and return whether the update dirties them.
ANGLE_INLINE bool ShaderConstants11::onFirstVertexChange(GLint firstVertex)
{
    // firstVertex should already include baseVertex, if any.
    uint32_t newFirstVertex = static_cast<uint32_t>(firstVertex);

    bool firstVertexDirty = (mVertex.firstVertex != newFirstVertex);
    if (firstVertexDirty)
    {
        mVertex.firstVertex = newFirstVertex;
        mShaderConstantsDirty.set(gl::ShaderType::Vertex);
    }
    return firstVertexDirty;
}

void ShaderConstants11::onSamplerChange(gl::ShaderType shaderType,
                                        unsigned int samplerIndex,
                                        const gl::Texture &texture,
                                        const gl::SamplerState &samplerState)
{
    ASSERT(shaderType != gl::ShaderType::InvalidEnum);
    if (updateSamplerMetadata(&mShaderSamplerMetadata[shaderType][samplerIndex], texture,
                              samplerState))
    {
        mNumActiveShaderSamplers[shaderType] = 0;
    }
}

bool ShaderConstants11::onImageChange(gl::ShaderType shaderType,
                                      unsigned int imageIndex,
                                      const gl::ImageUnit &imageUnit)
{
    ASSERT(shaderType != gl::ShaderType::InvalidEnum);
    bool dirty = false;
    if (imageUnit.access == GL_READ_ONLY)
    {
        if (updateImageMetadata(&mShaderReadonlyImageMetadata[shaderType][imageIndex], imageUnit))
        {
            mNumActiveShaderReadonlyImages[shaderType] = 0;
            dirty                                      = true;
        }
    }
    else
    {
        if (updateImageMetadata(&mShaderImageMetadata[shaderType][imageIndex], imageUnit))
        {
            mNumActiveShaderImages[shaderType] = 0;
            dirty                              = true;
        }
    }
    return dirty;
}

void ShaderConstants11::onClipOriginChange(bool lowerLeft)
{
    mVertex.clipControlOrigin = lowerLeft ? -1.0f : 1.0f;
    mShaderConstantsDirty.set(gl::ShaderType::Vertex);
}

bool ShaderConstants11::onClipDepthModeChange(bool zeroToOne)
{
    const float value             = static_cast<float>(zeroToOne);
    const bool clipDepthModeDirty = mVertex.clipControlZeroToOne != value;
    if (clipDepthModeDirty)
    {
        mVertex.clipControlZeroToOne = value;
        mShaderConstantsDirty.set(gl::ShaderType::Vertex);
    }
    return clipDepthModeDirty;
}

bool ShaderConstants11::onClipDistancesEnabledChange(const uint32_t value)
{
    ASSERT(value == (value & 0xFF));
    const bool clipDistancesEnabledDirty = (mVertex.clipDistancesEnabled != value);
    if (clipDistancesEnabledDirty)
    {
        mVertex.clipDistancesEnabled = value;
        mShaderConstantsDirty.set(gl::ShaderType::Vertex);
    }
    return clipDistancesEnabledDirty;
}

bool ShaderConstants11::onMultisamplingChange(bool multisampling)
{
    const bool multisamplingDirty =
        ((mPixel.misc & kPixelMiscMultisamplingMask) != 0) != multisampling;
    if (multisamplingDirty)
    {
        mPixel.misc ^= kPixelMiscMultisamplingMask;
        mShaderConstantsDirty.set(gl::ShaderType::Fragment);
    }
    return multisamplingDirty;
}

angle::Result ShaderConstants11::updateBuffer(const gl::Context *context,
                                              Renderer11 *renderer,
                                              gl::ShaderType shaderType,
                                              const ProgramExecutableD3D &executableD3D,
                                              const d3d11::Buffer &driverConstantBuffer)
{
    // Re-upload the sampler meta-data if the current program uses more samplers
    // than we previously uploaded.
    const int numSamplers       = executableD3D.getUsedSamplerRange(shaderType).length();
    const int numReadonlyImages = executableD3D.getUsedImageRange(shaderType, true).length();
    const int numImages         = executableD3D.getUsedImageRange(shaderType, false).length();

    const bool dirty = mShaderConstantsDirty[shaderType] ||
                       (mNumActiveShaderSamplers[shaderType] < numSamplers) ||
                       (mNumActiveShaderReadonlyImages[shaderType] < numReadonlyImages) ||
                       (mNumActiveShaderImages[shaderType] < numImages);

    const size_t dataSize = GetShaderConstantsStructSize(shaderType);
    const uint8_t *samplerData =
        reinterpret_cast<const uint8_t *>(mShaderSamplerMetadata[shaderType].data());
    const size_t samplerDataSize = sizeof(SamplerMetadata) * numSamplers;
    const uint8_t *readonlyImageData =
        reinterpret_cast<const uint8_t *>(mShaderReadonlyImageMetadata[shaderType].data());
    const size_t readonlyImageDataSize = sizeof(ImageMetadata) * numReadonlyImages;
    const uint8_t *imageData =
        reinterpret_cast<const uint8_t *>(mShaderImageMetadata[shaderType].data());
    const size_t imageDataSize = sizeof(ImageMetadata) * numImages;

    mNumActiveShaderSamplers[shaderType]       = numSamplers;
    mNumActiveShaderReadonlyImages[shaderType] = numReadonlyImages;
    mNumActiveShaderImages[shaderType]         = numImages;
    mShaderConstantsDirty.set(shaderType, false);

    const uint8_t *data = nullptr;
    switch (shaderType)
    {
        case gl::ShaderType::Vertex:
            data = reinterpret_cast<const uint8_t *>(&mVertex);
            break;
        case gl::ShaderType::Fragment:
            data = reinterpret_cast<const uint8_t *>(&mPixel);
            break;
        case gl::ShaderType::Compute:
            data = reinterpret_cast<const uint8_t *>(&mCompute);
            break;
        default:
            UNREACHABLE();
            break;
    }

    ASSERT(driverConstantBuffer.valid());

    if (!dirty)
    {
        return angle::Result::Continue;
    }

    // Previous buffer contents are discarded, so we need to refresh the whole buffer.
    D3D11_MAPPED_SUBRESOURCE mapping = {};
    ANGLE_TRY(renderer->mapResource(context, driverConstantBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD,
                                    0, &mapping));

    memcpy(mapping.pData, data, dataSize);
    memcpy(static_cast<uint8_t *>(mapping.pData) + dataSize, samplerData,
           sizeof(SamplerMetadata) * numSamplers);

    memcpy(static_cast<uint8_t *>(mapping.pData) + dataSize + samplerDataSize, readonlyImageData,
           readonlyImageDataSize);
    memcpy(
        static_cast<uint8_t *>(mapping.pData) + dataSize + samplerDataSize + readonlyImageDataSize,
        imageData, imageDataSize);
    renderer->getDeviceContext()->Unmap(driverConstantBuffer.get(), 0);

    return angle::Result::Continue;
}

StateManager11::StateManager11(Renderer11 *renderer)
    : mRenderer(renderer),
      mInternalDirtyBits(),
      mCurSampleAlphaToCoverage(false),
      mCurBlendStateExt(),
      mCurBlendColor(0, 0, 0, 0),
      mCurSampleMask(0),
      mCurStencilRef(0),
      mCurStencilBackRef(0),
      mCurStencilSize(0),
      mCurScissorEnabled(false),
      mCurScissorRect(),
      mCurViewport(),
      mCurNear(0.0f),
      mCurFar(0.0f),
      mViewportBounds(),
      mRenderTargetIsDirty(true),
      mCurPresentPathFastEnabled(false),
      mCurPresentPathFastColorBufferHeight(0),
      mDirtyCurrentValueAttribs(),
      mCurrentValueAttribs(),
      mCurrentInputLayout(),
      mDirtyVertexBufferRange(gl::MAX_VERTEX_ATTRIBS, 0),
      mCurrentPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED),
      mLastAppliedDrawMode(gl::PrimitiveMode::InvalidEnum),
      mCullEverything(false),
      mDirtySwizzles(false),
      mAppliedIB(nullptr),
      mAppliedIBFormat(DXGI_FORMAT_UNKNOWN),
      mAppliedIBOffset(0),
      mIndexBufferIsDirty(false),
      mVertexDataManager(renderer),
      mIndexDataManager(renderer),
      mIsMultiviewEnabled(false),
      mIndependentBlendStates(false),
      mEmptySerial(mRenderer->generateSerial()),
      mExecutableD3D(nullptr),
      mVertexArray11(nullptr),
      mFramebuffer11(nullptr)
{
    mCurDepthStencilState.depthTest                = false;
    mCurDepthStencilState.depthFunc                = GL_LESS;
    mCurDepthStencilState.depthMask                = true;
    mCurDepthStencilState.stencilTest              = false;
    mCurDepthStencilState.stencilMask              = true;
    mCurDepthStencilState.stencilFail              = GL_KEEP;
    mCurDepthStencilState.stencilPassDepthFail     = GL_KEEP;
    mCurDepthStencilState.stencilPassDepthPass     = GL_KEEP;
    mCurDepthStencilState.stencilWritemask         = static_cast<GLuint>(-1);
    mCurDepthStencilState.stencilBackFunc          = GL_ALWAYS;
    mCurDepthStencilState.stencilBackMask          = static_cast<GLuint>(-1);
    mCurDepthStencilState.stencilBackFail          = GL_KEEP;
    mCurDepthStencilState.stencilBackPassDepthFail = GL_KEEP;
    mCurDepthStencilState.stencilBackPassDepthPass = GL_KEEP;
    mCurDepthStencilState.stencilBackWritemask     = static_cast<GLuint>(-1);

    mCurRasterState.cullFace            = false;
    mCurRasterState.cullMode            = gl::CullFaceMode::Back;
    mCurRasterState.frontFace           = GL_CCW;
    mCurRasterState.polygonMode         = gl::PolygonMode::Fill;
    mCurRasterState.polygonOffsetPoint  = false;
    mCurRasterState.polygonOffsetLine   = false;
    mCurRasterState.polygonOffsetFill   = false;
    mCurRasterState.polygonOffsetFactor = 0.0f;
    mCurRasterState.polygonOffsetUnits  = 0.0f;
    mCurRasterState.polygonOffsetClamp  = 0.0f;
    mCurRasterState.depthClamp          = false;
    mCurRasterState.pointDrawMode       = false;
    mCurRasterState.multiSample         = false;
    mCurRasterState.rasterizerDiscard   = false;
    mCurRasterState.dither              = false;

    // Start with all internal dirty bits set except the SRV and UAV bits.
    mInternalDirtyBits.set();
    mInternalDirtyBits.reset(DIRTY_BIT_GRAPHICS_SRV_STATE);
    mInternalDirtyBits.reset(DIRTY_BIT_GRAPHICS_UAV_STATE);
    mInternalDirtyBits.reset(DIRTY_BIT_COMPUTE_SRV_STATE);
    mInternalDirtyBits.reset(DIRTY_BIT_COMPUTE_UAV_STATE);

    mGraphicsDirtyBitsMask.set();
    mGraphicsDirtyBitsMask.reset(DIRTY_BIT_COMPUTE_SRV_STATE);
    mGraphicsDirtyBitsMask.reset(DIRTY_BIT_COMPUTE_UAV_STATE);
    mComputeDirtyBitsMask.set(DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE);
    mComputeDirtyBitsMask.set(DIRTY_BIT_PROGRAM_UNIFORMS);
    mComputeDirtyBitsMask.set(DIRTY_BIT_DRIVER_UNIFORMS);
    mComputeDirtyBitsMask.set(DIRTY_BIT_PROGRAM_UNIFORM_BUFFERS);
    mComputeDirtyBitsMask.set(DIRTY_BIT_SHADERS);
    mComputeDirtyBitsMask.set(DIRTY_BIT_COMPUTE_SRV_STATE);
    mComputeDirtyBitsMask.set(DIRTY_BIT_COMPUTE_UAV_STATE);

    // Initially all current value attributes must be updated on first use.
    mDirtyCurrentValueAttribs.set();

    mCurrentVertexBuffers.fill(nullptr);
    mCurrentVertexStrides.fill(std::numeric_limits<UINT>::max());
    mCurrentVertexOffsets.fill(std::numeric_limits<UINT>::max());
}

StateManager11::~StateManager11() {}

template <typename SRVType>
void StateManager11::setShaderResourceInternal(gl::ShaderType shaderType,
                                               UINT resourceSlot,
                                               const SRVType *srv)
{
    auto *currentSRVs = getSRVCache(shaderType);
    ASSERT(static_cast<size_t>(resourceSlot) < currentSRVs->size());
    const ViewRecord<D3D11_SHADER_RESOURCE_VIEW_DESC> &record = (*currentSRVs)[resourceSlot];

    if (record.view != reinterpret_cast<uintptr_t>(srv))
    {
        ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
        ID3D11ShaderResourceView *srvPtr   = srv ? srv->get() : nullptr;
        if (srvPtr)
        {
            uintptr_t resource = reinterpret_cast<uintptr_t>(GetViewResource(srvPtr));
            unsetConflictingUAVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Compute,
                                 resource, nullptr);
        }

        switch (shaderType)
        {
            case gl::ShaderType::Vertex:
                deviceContext->VSSetShaderResources(resourceSlot, 1, &srvPtr);
                break;
            case gl::ShaderType::Fragment:
                deviceContext->PSSetShaderResources(resourceSlot, 1, &srvPtr);
                break;
            case gl::ShaderType::Compute:
            {
                if (srvPtr)
                {
                    uintptr_t resource = reinterpret_cast<uintptr_t>(GetViewResource(srvPtr));
                    unsetConflictingRTVs(resource);
                }
                deviceContext->CSSetShaderResources(resourceSlot, 1, &srvPtr);
                break;
            }
            default:
                UNREACHABLE();
        }

        currentSRVs->update(resourceSlot, srvPtr);
    }
}

template <typename UAVType>
void StateManager11::setUnorderedAccessViewInternal(UINT resourceSlot,
                                                    const UAVType *uav,
                                                    UAVList *uavList)
{
    ASSERT(static_cast<size_t>(resourceSlot) < mCurComputeUAVs.size());
    const ViewRecord<D3D11_UNORDERED_ACCESS_VIEW_DESC> &record = mCurComputeUAVs[resourceSlot];

    if (record.view != reinterpret_cast<uintptr_t>(uav))
    {
        ID3D11UnorderedAccessView *uavPtr = uav ? uav->get() : nullptr;
        // We need to make sure that resource being set to UnorderedAccessView slot |resourceSlot|
        // is not bound on SRV.
        if (uavPtr)
        {
            uintptr_t resource = reinterpret_cast<uintptr_t>(GetViewResource(uavPtr));
            unsetConflictingSRVs(gl::PipelineType::ComputePipeline, gl::ShaderType::Vertex,
                                 resource, nullptr, false);
            unsetConflictingSRVs(gl::PipelineType::ComputePipeline, gl::ShaderType::Fragment,
                                 resource, nullptr, false);
            unsetConflictingSRVs(gl::PipelineType::ComputePipeline, gl::ShaderType::Compute,
                                 resource, nullptr, false);
        }
        uavList->data[resourceSlot] = uavPtr;
        if (static_cast<int>(resourceSlot) > uavList->highestUsed)
        {
            uavList->highestUsed = resourceSlot;
        }

        mCurComputeUAVs.update(resourceSlot, uavPtr);
    }
}

void StateManager11::updateStencilSizeIfChanged(bool depthStencilInitialized,
                                                unsigned int stencilSize)
{
    if (!depthStencilInitialized || stencilSize != mCurStencilSize)
    {
        mCurStencilSize = stencilSize;
        mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
    }
}

void StateManager11::checkPresentPath(const gl::Context *context)
{
    const auto *framebuffer          = context->getState().getDrawFramebuffer();
    const auto *firstColorAttachment = framebuffer->getFirstColorAttachment();
    const bool clipSpaceOriginUpperLeft =
        context->getState().getClipOrigin() == gl::ClipOrigin::UpperLeft;
    const bool presentPathFastActive =
        UsePresentPathFast(mRenderer, firstColorAttachment) || clipSpaceOriginUpperLeft;

    const int colorBufferHeight = firstColorAttachment ? firstColorAttachment->getSize().height : 0;

    if ((mCurPresentPathFastEnabled != presentPathFastActive) ||
        (presentPathFastActive && (colorBufferHeight != mCurPresentPathFastColorBufferHeight)))
    {
        mCurPresentPathFastEnabled           = presentPathFastActive;
        mCurPresentPathFastColorBufferHeight = colorBufferHeight;

        // Scissor rect may need to be vertically inverted
        mInternalDirtyBits.set(DIRTY_BIT_SCISSOR_STATE);

        // Cull Mode may need to be inverted
        mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);

        // Viewport may need to be vertically inverted
        invalidateViewport(context);
    }
}

angle::Result StateManager11::updateStateForCompute(const gl::Context *context,
                                                    GLuint numGroupsX,
                                                    GLuint numGroupsY,
                                                    GLuint numGroupsZ)
{
    mShaderConstants.setComputeWorkGroups(numGroupsX, numGroupsY, numGroupsZ);

    if (mExecutableD3D->isSamplerMappingDirty())
    {
        mExecutableD3D->updateSamplerMapping();
        invalidateTexturesAndSamplers();
    }

    if (mDirtySwizzles)
    {
        ANGLE_TRY(generateSwizzlesForShader(context, gl::ShaderType::Compute));
        mDirtySwizzles = false;
    }

    if (mExecutableD3D->anyShaderUniformsDirty())
    {
        mInternalDirtyBits.set(DIRTY_BIT_PROGRAM_UNIFORMS);
    }

    auto dirtyBitsCopy = mInternalDirtyBits & mComputeDirtyBitsMask;
    mInternalDirtyBits &= ~mComputeDirtyBitsMask;

    for (auto iter = dirtyBitsCopy.begin(), end = dirtyBitsCopy.end(); iter != end; ++iter)
    {
        switch (*iter)
        {
            case DIRTY_BIT_COMPUTE_SRV_STATE:
                // Avoid to call syncTexturesForCompute function two times.
                iter.resetLaterBit(DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE);
                ANGLE_TRY(syncTexturesForCompute(context));
                break;
            case DIRTY_BIT_COMPUTE_UAV_STATE:
                ANGLE_TRY(syncUAVsForCompute(context));
                break;
            case DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE:
                ANGLE_TRY(syncTexturesForCompute(context));
                break;
            case DIRTY_BIT_PROGRAM_UNIFORMS:
            case DIRTY_BIT_DRIVER_UNIFORMS:
                ANGLE_TRY(applyComputeUniforms(context, mExecutableD3D));
                break;
            case DIRTY_BIT_PROGRAM_UNIFORM_BUFFERS:
                ANGLE_TRY(syncUniformBuffers(context));
                break;
            case DIRTY_BIT_SHADERS:
                ANGLE_TRY(syncProgramForCompute(context));
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    return angle::Result::Continue;
}

void StateManager11::syncState(const gl::Context *context,
                               const gl::state::DirtyBits &dirtyBits,
                               const gl::state::ExtendedDirtyBits &extendedDirtyBits,
                               gl::Command command)
{
    if (!dirtyBits.any())
    {
        return;
    }

    const gl::State &state = context->getState();

    for (auto iter = dirtyBits.begin(), endIter = dirtyBits.end(); iter != endIter; ++iter)
    {
        size_t dirtyBit = *iter;
        switch (dirtyBit)
        {
            case gl::state::DIRTY_BIT_BLEND_EQUATIONS:
            {
                const gl::BlendStateExt &blendStateExt = state.getBlendStateExt();
                ASSERT(mCurBlendStateExt.getDrawBufferCount() ==
                       blendStateExt.getDrawBufferCount());
                // Compare blend equations only for buffers with blending enabled because
                // subsequent sync stages enforce default values for buffers with blending disabled.
                if ((blendStateExt.getEnabledMask() &
                     mCurBlendStateExt.compareEquations(blendStateExt))
                        .any())
                {
                    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
                }
                break;
            }
            case gl::state::DIRTY_BIT_BLEND_FUNCS:
            {
                const gl::BlendStateExt &blendStateExt = state.getBlendStateExt();
                ASSERT(mCurBlendStateExt.getDrawBufferCount() ==
                       blendStateExt.getDrawBufferCount());
                // Compare blend factors only for buffers with blending enabled because
                // subsequent sync stages enforce default values for buffers with blending disabled.
                if ((blendStateExt.getEnabledMask() &
                     mCurBlendStateExt.compareFactors(blendStateExt))
                        .any())
                {
                    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
                }
                break;
            }
            case gl::state::DIRTY_BIT_BLEND_ENABLED:
            {
                if (state.getBlendStateExt().getEnabledMask() != mCurBlendStateExt.getEnabledMask())
                {
                    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
                }
                break;
            }
            case gl::state::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
                if (state.isSampleAlphaToCoverageEnabled() != mCurSampleAlphaToCoverage)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
                }
                break;
            case gl::state::DIRTY_BIT_DITHER_ENABLED:
                if (state.getRasterizerState().dither != mCurRasterState.dither)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                }
                break;
            case gl::state::DIRTY_BIT_COLOR_MASK:
            {
                if (state.getBlendStateExt().getColorMaskBits() !=
                    mCurBlendStateExt.getColorMaskBits())
                {
                    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
                }
                break;
            }
            case gl::state::DIRTY_BIT_BLEND_COLOR:
                if (state.getBlendColor() != mCurBlendColor)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
                }
                break;
            // Depth and stencil redundant state changes are guarded in the
            // frontend so for related cases here just set the dirty bit.
            case gl::state::DIRTY_BIT_DEPTH_MASK:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_DEPTH_TEST_ENABLED:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_DEPTH_FUNC:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_STENCIL_TEST_ENABLED:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_FRONT:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_STENCIL_FUNCS_BACK:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_STENCIL_OPS_FRONT:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;
            case gl::state::DIRTY_BIT_STENCIL_OPS_BACK:
                mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
                break;

            case gl::state::DIRTY_BIT_CULL_FACE_ENABLED:
                if (state.getRasterizerState().cullFace != mCurRasterState.cullFace)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                    mInternalDirtyBits.set(DIRTY_BIT_PRIMITIVE_TOPOLOGY);
                }
                break;
            case gl::state::DIRTY_BIT_CULL_FACE:
                if (state.getRasterizerState().cullMode != mCurRasterState.cullMode)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                    mInternalDirtyBits.set(DIRTY_BIT_PRIMITIVE_TOPOLOGY);
                }
                break;
            case gl::state::DIRTY_BIT_FRONT_FACE:
                if (state.getRasterizerState().frontFace != mCurRasterState.frontFace)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                    mInternalDirtyBits.set(DIRTY_BIT_PRIMITIVE_TOPOLOGY);
                }
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED:
                if (state.getRasterizerState().polygonOffsetFill !=
                    mCurRasterState.polygonOffsetFill)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                }
                break;
            case gl::state::DIRTY_BIT_POLYGON_OFFSET:
            {
                const gl::RasterizerState &rasterState = state.getRasterizerState();
                if (rasterState.polygonOffsetFactor != mCurRasterState.polygonOffsetFactor ||
                    rasterState.polygonOffsetUnits != mCurRasterState.polygonOffsetUnits ||
                    rasterState.polygonOffsetClamp != mCurRasterState.polygonOffsetClamp)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                }
                break;
            }
            case gl::state::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED:
                if (state.getRasterizerState().rasterizerDiscard !=
                    mCurRasterState.rasterizerDiscard)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);

                    // Enabling/disabling rasterizer discard affects the pixel shader.
                    invalidateShaders();
                }
                break;
            case gl::state::DIRTY_BIT_SCISSOR:
                if (state.getScissor() != mCurScissorRect)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_SCISSOR_STATE);
                }
                break;
            case gl::state::DIRTY_BIT_SCISSOR_TEST_ENABLED:
                if (state.isScissorTestEnabled() != mCurScissorEnabled)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_SCISSOR_STATE);
                    // Rasterizer state update needs mCurScissorsEnabled and updates when it changes
                    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                }
                break;
            case gl::state::DIRTY_BIT_DEPTH_RANGE:
                invalidateViewport(context);
                break;
            case gl::state::DIRTY_BIT_VIEWPORT:
                if (state.getViewport() != mCurViewport)
                {
                    invalidateViewport(context);
                }
                break;
            case gl::state::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING:
                invalidateRenderTarget();
                mFramebuffer11 = GetImplAs<Framebuffer11>(state.getDrawFramebuffer());
                if (mShaderConstants.onMultisamplingChange(
                        state.getDrawFramebuffer()->getSamples(context) != 0))
                {
                    invalidateDriverUniforms();
                }
                break;
            case gl::state::DIRTY_BIT_VERTEX_ARRAY_BINDING:
                invalidateVertexBuffer();
                // Force invalidate the current value attributes, since the VertexArray11 keeps an
                // internal cache of TranslatedAttributes, and they CurrentValue attributes are
                // owned by the StateManager11/Context.
                mDirtyCurrentValueAttribs.set();
                // Invalidate the cached index buffer.
                invalidateIndexBuffer();
                mVertexArray11 = GetImplAs<VertexArray11>(state.getVertexArray());
                break;
            case gl::state::DIRTY_BIT_UNIFORM_BUFFER_BINDINGS:
                invalidateProgramUniformBuffers();
                break;
            case gl::state::DIRTY_BIT_ATOMIC_COUNTER_BUFFER_BINDING:
                invalidateProgramAtomicCounterBuffers();
                break;
            case gl::state::DIRTY_BIT_SHADER_STORAGE_BUFFER_BINDING:
                invalidateProgramShaderStorageBuffers();
                break;
            case gl::state::DIRTY_BIT_TEXTURE_BINDINGS:
                invalidateTexturesAndSamplers();
                break;
            case gl::state::DIRTY_BIT_SAMPLER_BINDINGS:
                invalidateTexturesAndSamplers();
                break;
            case gl::state::DIRTY_BIT_IMAGE_BINDINGS:
                invalidateImageBindings();
                break;
            case gl::state::DIRTY_BIT_TRANSFORM_FEEDBACK_BINDING:
                invalidateTransformFeedback();
                break;
            case gl::state::DIRTY_BIT_PROGRAM_BINDING:
                static_assert(
                    gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE > gl::state::DIRTY_BIT_PROGRAM_BINDING,
                    "Dirty bit order");
                iter.setLaterBit(gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE);
                break;
            case gl::state::DIRTY_BIT_PROGRAM_EXECUTABLE:
            {
                invalidateShaders();
                invalidateTexturesAndSamplers();
                invalidateProgramUniforms();
                invalidateProgramUniformBuffers();
                invalidateProgramAtomicCounterBuffers();
                invalidateProgramShaderStorageBuffers();
                invalidateDriverUniforms();
                const gl::ProgramExecutable *executable = state.getProgramExecutable();
                ASSERT(executable);
                mExecutableD3D = GetImplAs<ProgramExecutableD3D>(executable);
                if (!executable || command != gl::Command::Dispatch)
                {
                    mInternalDirtyBits.set(DIRTY_BIT_PRIMITIVE_TOPOLOGY);
                    invalidateVertexBuffer();
                    invalidateRenderTarget();
                    // If OVR_multiview2 is enabled, the attribute divisor has to be updated for
                    // each binding. When using compute, there could be no vertex array.
                    if (mIsMultiviewEnabled && mVertexArray11)
                    {
                        ASSERT(mExecutableD3D);
                        ASSERT(mVertexArray11 == GetImplAs<VertexArray11>(state.getVertexArray()));
                        int numViews = executable->usesMultiview() ? executable->getNumViews() : 1;
                        mVertexArray11->markAllAttributeDivisorsForAdjustment(numViews);
                    }
                }
                break;
            }
            case gl::state::DIRTY_BIT_CURRENT_VALUES:
            {
                for (auto attribIndex : state.getAndResetDirtyCurrentValues())
                {
                    invalidateCurrentValueAttrib(attribIndex);
                }
                break;
            }
            case gl::state::DIRTY_BIT_PROVOKING_VERTEX:
                invalidateShaders();
                break;
            case gl::state::DIRTY_BIT_EXTENDED:
            {
                for (size_t extendedDirtyBit : extendedDirtyBits)
                {
                    switch (extendedDirtyBit)
                    {
                        case gl::state::EXTENDED_DIRTY_BIT_CLIP_CONTROL:
                            checkPresentPath(context);
                            if (mShaderConstants.onClipDepthModeChange(
                                    state.isClipDepthModeZeroToOne()))
                            {
                                invalidateDriverUniforms();
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_CLIP_DISTANCES:
                            if (mShaderConstants.onClipDistancesEnabledChange(
                                    state.getEnabledClipDistances().bits()))
                            {
                                mInternalDirtyBits.set(DIRTY_BIT_DRIVER_UNIFORMS);
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_DEPTH_CLAMP_ENABLED:
                            if (state.getRasterizerState().depthClamp != mCurRasterState.depthClamp)
                            {
                                mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_MODE:
                            if (state.getRasterizerState().polygonMode !=
                                mCurRasterState.polygonMode)
                            {
                                mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                            }
                            break;
                        case gl::state::EXTENDED_DIRTY_BIT_POLYGON_OFFSET_LINE_ENABLED:
                            if (state.getRasterizerState().polygonOffsetLine !=
                                mCurRasterState.polygonOffsetLine)
                            {
                                mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
                            }
                            break;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    // TODO(jmadill): Input layout and vertex buffer state.
}

angle::Result StateManager11::syncBlendState(const gl::Context *context,
                                             const gl::BlendStateExt &blendStateExt,
                                             const gl::ColorF &blendColor,
                                             unsigned int sampleMask,
                                             bool sampleAlphaToCoverage,
                                             bool emulateConstantAlpha)
{
    const d3d11::BlendState *dxBlendState = nullptr;
    const d3d11::BlendStateKey &key       = RenderStateCache::GetBlendStateKey(
        context, mFramebuffer11, blendStateExt, sampleAlphaToCoverage);

    ANGLE_TRY(mRenderer->getBlendState(context, key, &dxBlendState));

    ASSERT(dxBlendState != nullptr);

    // D3D11 does not support CONSTANT_ALPHA as source or destination color factor, so ANGLE sets
    // the factor to CONSTANT_COLOR and swizzles the color value to aaaa. For this reason, it's
    // impossible to simultaneously use CONSTANT_ALPHA and CONSTANT_COLOR as source or destination
    // color factors in the same blend state. This is enforced in the validation layer.
    float blendColors[4] = {0.0f};
    blendColors[0]       = emulateConstantAlpha ? blendColor.alpha : blendColor.red;
    blendColors[1]       = emulateConstantAlpha ? blendColor.alpha : blendColor.green;
    blendColors[2]       = emulateConstantAlpha ? blendColor.alpha : blendColor.blue;
    blendColors[3]       = blendColor.alpha;

    mRenderer->getDeviceContext()->OMSetBlendState(dxBlendState->get(), blendColors, sampleMask);

    mCurBlendStateExt         = blendStateExt;
    mCurBlendColor            = blendColor;
    mCurSampleMask            = sampleMask;
    mCurSampleAlphaToCoverage = sampleAlphaToCoverage;

    return angle::Result::Continue;
}

angle::Result StateManager11::syncDepthStencilState(const gl::Context *context)
{
    const gl::State &glState = context->getState();

    mCurDepthStencilState = glState.getDepthStencilState();
    mCurStencilRef        = glState.getStencilRef();
    mCurStencilBackRef    = glState.getStencilBackRef();

    // get the maximum size of the stencil ref
    unsigned int maxStencil = 0;
    if (mCurDepthStencilState.stencilTest && mCurStencilSize > 0)
    {
        maxStencil = (1 << mCurStencilSize) - 1;
    }
    ASSERT((mCurDepthStencilState.stencilWritemask & maxStencil) ==
           (mCurDepthStencilState.stencilBackWritemask & maxStencil));
    ASSERT(gl::clamp(mCurStencilRef, 0, static_cast<int>(maxStencil)) ==
           gl::clamp(mCurStencilBackRef, 0, static_cast<int>(maxStencil)));
    ASSERT((mCurDepthStencilState.stencilMask & maxStencil) ==
           (mCurDepthStencilState.stencilBackMask & maxStencil));

    gl::DepthStencilState modifiedGLState = glState.getDepthStencilState();

    ASSERT(mCurDisableDepth.valid() && mCurDisableStencil.valid());

    if (mCurDisableDepth.value())
    {
        modifiedGLState.depthTest = false;
        modifiedGLState.depthMask = false;
    }

    if (mCurDisableStencil.value())
    {
        modifiedGLState.stencilTest = false;
    }
    if (!modifiedGLState.stencilTest)
    {
        modifiedGLState.stencilWritemask     = 0;
        modifiedGLState.stencilBackWritemask = 0;
    }

    // If STENCIL_TEST is disabled in glState, stencil testing and writing should be disabled.
    // Verify that's true in the modifiedGLState so it is propagated to d3dState.
    ASSERT(glState.getDepthStencilState().stencilTest ||
           (!modifiedGLState.stencilTest && modifiedGLState.stencilWritemask == 0 &&
            modifiedGLState.stencilBackWritemask == 0));

    const d3d11::DepthStencilState *d3dState = nullptr;
    ANGLE_TRY(mRenderer->getDepthStencilState(context, modifiedGLState, &d3dState));
    ASSERT(d3dState);

    // Max D3D11 stencil reference value is 0xFF,
    // corresponding to the max 8 bits in a stencil buffer
    // GL specifies we should clamp the ref value to the
    // nearest bit depth when doing stencil ops
    static_assert(D3D11_DEFAULT_STENCIL_READ_MASK == 0xFF,
                  "Unexpected value of D3D11_DEFAULT_STENCIL_READ_MASK");
    static_assert(D3D11_DEFAULT_STENCIL_WRITE_MASK == 0xFF,
                  "Unexpected value of D3D11_DEFAULT_STENCIL_WRITE_MASK");
    UINT dxStencilRef = static_cast<UINT>(gl::clamp(mCurStencilRef, 0, 0xFF));

    mRenderer->getDeviceContext()->OMSetDepthStencilState(d3dState->get(), dxStencilRef);

    return angle::Result::Continue;
}

angle::Result StateManager11::syncRasterizerState(const gl::Context *context,
                                                  gl::PrimitiveMode mode)
{
    // TODO: Remove pointDrawMode and multiSample from gl::RasterizerState.
    gl::RasterizerState rasterState = context->getState().getRasterizerState();
    rasterState.pointDrawMode       = (mode == gl::PrimitiveMode::Points);
    rasterState.multiSample         = mCurRasterState.multiSample;

    ID3D11RasterizerState *dxRasterState = nullptr;

    if (mCurPresentPathFastEnabled)
    {
        gl::RasterizerState modifiedRasterState = rasterState;

        // If prseent path fast is active then we need invert the front face state.
        // This ensures that both gl_FrontFacing is correct, and front/back culling
        // is performed correctly.
        if (modifiedRasterState.frontFace == GL_CCW)
        {
            modifiedRasterState.frontFace = GL_CW;
        }
        else
        {
            ASSERT(modifiedRasterState.frontFace == GL_CW);
            modifiedRasterState.frontFace = GL_CCW;
        }

        ANGLE_TRY(mRenderer->getRasterizerState(context, modifiedRasterState, mCurScissorEnabled,
                                                &dxRasterState));
    }
    else
    {
        ANGLE_TRY(mRenderer->getRasterizerState(context, rasterState, mCurScissorEnabled,
                                                &dxRasterState));
    }

    mRenderer->getDeviceContext()->RSSetState(dxRasterState);

    mCurRasterState = rasterState;

    return angle::Result::Continue;
}

void StateManager11::syncScissorRectangle(const gl::Context *context)
{
    const auto &glState          = context->getState();
    gl::Framebuffer *framebuffer = glState.getDrawFramebuffer();
    const gl::Rectangle &scissor = glState.getScissor();
    const bool enabled           = glState.isScissorTestEnabled();

    mCurScissorOffset = framebuffer->getSurfaceTextureOffset();

    int scissorX = scissor.x + mCurScissorOffset.x;
    int scissorY = scissor.y + mCurScissorOffset.y;

    if (mCurPresentPathFastEnabled && glState.getClipOrigin() == gl::ClipOrigin::LowerLeft)
    {
        scissorY = mCurPresentPathFastColorBufferHeight - scissor.height - scissor.y;
    }

    if (enabled)
    {
        D3D11_RECT rect;
        int x       = scissorX;
        int y       = scissorY;
        rect.left   = std::max(0, x);
        rect.top    = std::max(0, y);
        rect.right  = x + std::max(0, scissor.width);
        rect.bottom = y + std::max(0, scissor.height);
        mRenderer->getDeviceContext()->RSSetScissorRects(1, &rect);
    }

    mCurScissorRect    = scissor;
    mCurScissorEnabled = enabled;
}

void StateManager11::syncViewport(const gl::Context *context)
{
    const auto &glState          = context->getState();
    gl::Framebuffer *framebuffer = glState.getDrawFramebuffer();
    float actualZNear            = gl::clamp01(glState.getNearPlane());
    float actualZFar             = gl::clamp01(glState.getFarPlane());

    const auto &caps         = context->getCaps();
    int dxMaxViewportBoundsX = caps.maxViewportWidth;
    int dxMaxViewportBoundsY = caps.maxViewportHeight;
    int dxMinViewportBoundsX = -dxMaxViewportBoundsX;
    int dxMinViewportBoundsY = -dxMaxViewportBoundsY;

    bool is9_3 = mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3;

    if (is9_3)
    {
        // Feature Level 9 viewports shouldn't exceed the dimensions of the rendertarget.
        dxMaxViewportBoundsX = static_cast<int>(mViewportBounds.width);
        dxMaxViewportBoundsY = static_cast<int>(mViewportBounds.height);
        dxMinViewportBoundsX = 0;
        dxMinViewportBoundsY = 0;
    }

    bool clipSpaceOriginLowerLeft = glState.getClipOrigin() == gl::ClipOrigin::LowerLeft;
    mShaderConstants.onClipOriginChange(clipSpaceOriginLowerLeft);

    const auto &viewport = glState.getViewport();

    int dxViewportTopLeftX = 0;
    int dxViewportTopLeftY = 0;
    int dxViewportWidth    = 0;
    int dxViewportHeight   = 0;

    mCurViewportOffset = framebuffer->getSurfaceTextureOffset();

    dxViewportTopLeftX =
        gl::clamp(viewport.x + mCurViewportOffset.x, dxMinViewportBoundsX, dxMaxViewportBoundsX);
    dxViewportTopLeftY =
        gl::clamp(viewport.y + mCurViewportOffset.y, dxMinViewportBoundsY, dxMaxViewportBoundsY);
    dxViewportWidth  = gl::clamp(viewport.width, 0, dxMaxViewportBoundsX - dxViewportTopLeftX);
    dxViewportHeight = gl::clamp(viewport.height, 0, dxMaxViewportBoundsY - dxViewportTopLeftY);

    D3D11_VIEWPORT dxViewport;
    dxViewport.TopLeftX = static_cast<float>(dxViewportTopLeftX);
    if (mCurPresentPathFastEnabled && clipSpaceOriginLowerLeft)
    {
        // When present path fast is active and we're rendering to framebuffer 0, we must invert
        // the viewport in Y-axis.
        // NOTE: We delay the inversion until right before the call to RSSetViewports, and leave
        // dxViewportTopLeftY unchanged. This allows us to calculate viewAdjust below using the
        // unaltered dxViewportTopLeftY value.
        dxViewport.TopLeftY = static_cast<float>(mCurPresentPathFastColorBufferHeight -
                                                 dxViewportTopLeftY - dxViewportHeight);
    }
    else
    {
        dxViewport.TopLeftY = static_cast<float>(dxViewportTopLeftY);
    }

    // The es 3.1 spec section 9.2 states that, "If there are no attachments, rendering
    // will be limited to a rectangle having a lower left of (0, 0) and an upper right of
    // (width, height), where width and height are the framebuffer object's default width
    // and height." See http://anglebug.com/42260558
    // If the Framebuffer has no color attachment and the default width or height is smaller
    // than the current viewport, use the smaller of the two sizes.
    // If framebuffer default width or height is 0, the params should not set.
    if (!framebuffer->getFirstNonNullAttachment() &&
        (framebuffer->getDefaultWidth() || framebuffer->getDefaultHeight()))
    {
        dxViewport.Width =
            static_cast<GLfloat>(std::min(viewport.width, framebuffer->getDefaultWidth()));
        dxViewport.Height =
            static_cast<GLfloat>(std::min(viewport.height, framebuffer->getDefaultHeight()));
    }
    else
    {
        dxViewport.Width  = static_cast<float>(dxViewportWidth);
        dxViewport.Height = static_cast<float>(dxViewportHeight);
    }
    dxViewport.MinDepth = actualZNear;
    dxViewport.MaxDepth = actualZFar;

    mRenderer->getDeviceContext()->RSSetViewports(1, &dxViewport);

    mCurViewport = viewport;
    mCurNear     = actualZNear;
    mCurFar      = actualZFar;

    const D3D11_VIEWPORT adjustViewport = {static_cast<FLOAT>(dxViewportTopLeftX),
                                           static_cast<FLOAT>(dxViewportTopLeftY),
                                           static_cast<FLOAT>(dxViewportWidth),
                                           static_cast<FLOAT>(dxViewportHeight),
                                           actualZNear,
                                           actualZFar};
    mShaderConstants.onViewportChange(viewport, adjustViewport, mCurViewportOffset, is9_3,
                                      mCurPresentPathFastEnabled);
}

void StateManager11::invalidateRenderTarget()
{
    mRenderTargetIsDirty = true;
}

void StateManager11::processFramebufferInvalidation(const gl::Context *context)
{
    ASSERT(mRenderTargetIsDirty);
    ASSERT(context);

    mInternalDirtyBits.set(DIRTY_BIT_RENDER_TARGET);

    // The pixel shader is dependent on the output layout.
    invalidateShaders();

    // The D3D11 blend state is heavily dependent on the current render target.
    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);

    gl::Framebuffer *fbo = context->getState().getDrawFramebuffer();
    ASSERT(fbo);

    // Dirty scissor and viewport because surface texture offset might have changed.
    if (mCurViewportOffset != fbo->getSurfaceTextureOffset())
    {
        mInternalDirtyBits.set(DIRTY_BIT_VIEWPORT_STATE);
    }
    if (mCurScissorOffset != fbo->getSurfaceTextureOffset())
    {
        mInternalDirtyBits.set(DIRTY_BIT_SCISSOR_STATE);
    }

    // Disable the depth test/depth write if we are using a stencil-only attachment.
    // This is because ANGLE emulates stencil-only with D24S8 on D3D11 - we should neither read
    // nor write to the unused depth part of this emulated texture.
    bool disableDepth = (!fbo->hasDepth() && fbo->hasStencil());

    // Similarly we disable the stencil portion of the DS attachment if the app only binds depth.
    bool disableStencil = (fbo->hasDepth() && !fbo->hasStencil());

    if (!mCurDisableDepth.valid() || disableDepth != mCurDisableDepth.value() ||
        !mCurDisableStencil.valid() || disableStencil != mCurDisableStencil.value())
    {
        mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
        mCurDisableDepth   = disableDepth;
        mCurDisableStencil = disableStencil;
    }

    bool multiSample = (fbo->getSamples(context) != 0);
    if (multiSample != mCurRasterState.multiSample)
    {
        mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
        mCurRasterState.multiSample = multiSample;
    }

    checkPresentPath(context);

    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        const auto *firstAttachment = fbo->getFirstNonNullAttachment();
        if (firstAttachment)
        {
            const auto &size = firstAttachment->getSize();
            if (mViewportBounds.width != size.width || mViewportBounds.height != size.height)
            {
                mViewportBounds = gl::Extents(size.width, size.height, 1);
                invalidateViewport(context);
            }
        }
    }
}

void StateManager11::invalidateBoundViews()
{
    for (SRVCache &curShaderSRV : mCurShaderSRVs)
    {
        curShaderSRV.clear();
    }

    invalidateRenderTarget();
}

void StateManager11::invalidateVertexBuffer()
{
    unsigned int limit      = std::min<unsigned int>(mRenderer->getNativeCaps().maxVertexAttributes,
                                                     gl::MAX_VERTEX_ATTRIBS);
    mDirtyVertexBufferRange = gl::RangeUI(0, limit);
    invalidateInputLayout();
    invalidateShaders();
    mInternalDirtyBits.set(DIRTY_BIT_CURRENT_VALUE_ATTRIBS);
}

void StateManager11::invalidateViewport(const gl::Context *context)
{
    mInternalDirtyBits.set(DIRTY_BIT_VIEWPORT_STATE);

    // Viewport affects the driver constants.
    invalidateDriverUniforms();
}

void StateManager11::invalidateTexturesAndSamplers()
{
    mInternalDirtyBits.set(DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE);
    invalidateSwizzles();

    // Texture state affects the driver uniforms (base level, etc).
    invalidateDriverUniforms();
}

void StateManager11::invalidateSwizzles()
{
    mDirtySwizzles = true;
}

void StateManager11::invalidateProgramUniforms()
{
    mInternalDirtyBits.set(DIRTY_BIT_PROGRAM_UNIFORMS);
}

void StateManager11::invalidateDriverUniforms()
{
    mInternalDirtyBits.set(DIRTY_BIT_DRIVER_UNIFORMS);
}

void StateManager11::invalidateProgramUniformBuffers()
{
    mInternalDirtyBits.set(DIRTY_BIT_PROGRAM_UNIFORM_BUFFERS);
}

void StateManager11::invalidateProgramAtomicCounterBuffers()
{
    mInternalDirtyBits.set(DIRTY_BIT_GRAPHICS_UAV_STATE);
    mInternalDirtyBits.set(DIRTY_BIT_COMPUTE_UAV_STATE);
}

void StateManager11::invalidateProgramShaderStorageBuffers()
{
    mInternalDirtyBits.set(DIRTY_BIT_GRAPHICS_UAV_STATE);
    mInternalDirtyBits.set(DIRTY_BIT_COMPUTE_UAV_STATE);
}

void StateManager11::invalidateImageBindings()
{
    mInternalDirtyBits.set(DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE);
    mInternalDirtyBits.set(DIRTY_BIT_GRAPHICS_SRV_STATE);
    mInternalDirtyBits.set(DIRTY_BIT_GRAPHICS_UAV_STATE);
    mInternalDirtyBits.set(DIRTY_BIT_COMPUTE_SRV_STATE);
    mInternalDirtyBits.set(DIRTY_BIT_COMPUTE_UAV_STATE);
    mInternalDirtyBits.set(DIRTY_BIT_DRIVER_UNIFORMS);
}

void StateManager11::invalidateConstantBuffer(unsigned int slot)
{
    if (slot == d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DRIVER)
    {
        invalidateDriverUniforms();
    }
    else if (slot == d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DEFAULT_UNIFORM_BLOCK)
    {
        invalidateProgramUniforms();
    }
    else
    {
        invalidateProgramUniformBuffers();
    }
}

void StateManager11::invalidateShaders()
{
    mInternalDirtyBits.set(DIRTY_BIT_SHADERS);
}

void StateManager11::invalidateTransformFeedback()
{
    // Transform feedback affects the stream-out geometry shader.
    invalidateShaders();
    mInternalDirtyBits.set(DIRTY_BIT_TRANSFORM_FEEDBACK);
    // syncPrimitiveTopology checks the transform feedback state.
    mInternalDirtyBits.set(DIRTY_BIT_PRIMITIVE_TOPOLOGY);
}

void StateManager11::invalidateInputLayout()
{
    mInternalDirtyBits.set(DIRTY_BIT_VERTEX_BUFFERS_AND_INPUT_LAYOUT);
}

void StateManager11::invalidateIndexBuffer()
{
    mIndexBufferIsDirty = true;
}

void StateManager11::setRenderTarget(ID3D11RenderTargetView *rtv, ID3D11DepthStencilView *dsv)
{
    if (rtv)
    {
        unsetConflictingView(gl::PipelineType::GraphicsPipeline, rtv, true);
    }

    if (dsv)
    {
        unsetConflictingView(gl::PipelineType::GraphicsPipeline, dsv, true);
    }

    mRenderer->getDeviceContext()->OMSetRenderTargets(1, &rtv, dsv);
    mCurRTVs.clear();
    mCurRTVs.update(0, rtv);
    mCurrentDSV.clear();
    mCurrentDSV.update(0, dsv);
    mInternalDirtyBits.set(DIRTY_BIT_RENDER_TARGET);
}

void StateManager11::setRenderTargets(ID3D11RenderTargetView **rtvs,
                                      UINT numRTVs,
                                      ID3D11DepthStencilView *dsv)
{
    for (UINT rtvIndex = 0; rtvIndex < numRTVs; ++rtvIndex)
    {
        unsetConflictingView(gl::PipelineType::GraphicsPipeline, rtvs[rtvIndex], true);
    }

    if (dsv)
    {
        unsetConflictingView(gl::PipelineType::GraphicsPipeline, dsv, true);
    }

    mRenderer->getDeviceContext()->OMSetRenderTargets(numRTVs, (numRTVs > 0) ? rtvs : nullptr, dsv);
    mCurRTVs.clear();
    for (UINT i = 0; i < numRTVs; i++)
    {
        mCurRTVs.update(i, rtvs[i]);
    }
    mCurrentDSV.clear();
    mCurrentDSV.update(0, dsv);
    mInternalDirtyBits.set(DIRTY_BIT_RENDER_TARGET);
}

void StateManager11::onBeginQuery(Query11 *query)
{
    mCurrentQueries.insert(query);
}

void StateManager11::onDeleteQueryObject(Query11 *query)
{
    mCurrentQueries.erase(query);
}

angle::Result StateManager11::onMakeCurrent(const gl::Context *context)
{
    ANGLE_TRY(ensureInitialized(context));

    const gl::State &state = context->getState();

    Context11 *context11 = GetImplAs<Context11>(context);

    for (Query11 *query : mCurrentQueries)
    {
        ANGLE_TRY(query->pause(context11));
    }
    mCurrentQueries.clear();

    for (gl::QueryType type : angle::AllEnums<gl::QueryType>())
    {
        gl::Query *query = state.getActiveQuery(type);
        if (query != nullptr)
        {
            Query11 *query11 = GetImplAs<Query11>(query);
            ANGLE_TRY(query11->resume(context11));
            mCurrentQueries.insert(query11);
        }
    }

    // Reset the cache objects.
    mExecutableD3D = nullptr;
    mVertexArray11 = nullptr;
    mFramebuffer11 = nullptr;

    return angle::Result::Continue;
}

void StateManager11::unsetConflictingView(gl::PipelineType pipeline,
                                          ID3D11View *view,
                                          bool isRenderTarget)
{
    uintptr_t resource = reinterpret_cast<uintptr_t>(GetViewResource(view));

    unsetConflictingSRVs(pipeline, gl::ShaderType::Vertex, resource, nullptr, isRenderTarget);
    unsetConflictingSRVs(pipeline, gl::ShaderType::Fragment, resource, nullptr, isRenderTarget);
    unsetConflictingSRVs(pipeline, gl::ShaderType::Compute, resource, nullptr, isRenderTarget);
    unsetConflictingUAVs(pipeline, gl::ShaderType::Compute, resource, nullptr);
}

void StateManager11::unsetConflictingSRVs(gl::PipelineType pipeline,
                                          gl::ShaderType shaderType,
                                          uintptr_t resource,
                                          const gl::ImageIndex *index,
                                          bool isRenderTarget)
{
    auto *currentSRVs                 = getSRVCache(shaderType);
    gl::PipelineType conflictPipeline = gl::GetPipelineType(shaderType);
    bool foundOne                     = false;
    size_t count = std::min(currentSRVs->size(), currentSRVs->highestUsed() + 1);
    for (size_t resourceIndex = 0; resourceIndex < count; ++resourceIndex)
    {
        auto &record = (*currentSRVs)[resourceIndex];

        if (record.view && record.resource == resource &&
            (!index || ImageIndexConflictsWithSRV(*index, record.desc)))
        {
            setShaderResourceInternal<d3d11::ShaderResourceView>(
                shaderType, static_cast<UINT>(resourceIndex), nullptr);
            foundOne = true;
        }
    }

    if (foundOne && (pipeline != conflictPipeline || isRenderTarget))
    {
        switch (conflictPipeline)
        {
            case gl::PipelineType::GraphicsPipeline:
                mInternalDirtyBits.set(DIRTY_BIT_GRAPHICS_SRV_STATE);
                break;
            case gl::PipelineType::ComputePipeline:
                mInternalDirtyBits.set(DIRTY_BIT_COMPUTE_SRV_STATE);
                break;
            default:
                UNREACHABLE();
        }
    }
}

void StateManager11::unsetConflictingUAVs(gl::PipelineType pipeline,
                                          gl::ShaderType shaderType,
                                          uintptr_t resource,
                                          const gl::ImageIndex *index)
{
    ASSERT(shaderType == gl::ShaderType::Compute);
    bool foundOne = false;

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    size_t count = std::min(mCurComputeUAVs.size(), mCurComputeUAVs.highestUsed() + 1);
    for (size_t resourceIndex = 0; resourceIndex < count; ++resourceIndex)
    {
        auto &record = mCurComputeUAVs[resourceIndex];

        if (record.view && record.resource == resource &&
            (!index || ImageIndexConflictsWithUAV(*index, record.desc)))
        {
            deviceContext->CSSetUnorderedAccessViews(static_cast<UINT>(resourceIndex), 1,
                                                     &mNullUAVs[0], nullptr);
            mCurComputeUAVs.update(resourceIndex, nullptr);
            foundOne = true;
        }
    }

    if (foundOne && pipeline == gl::PipelineType::GraphicsPipeline)
    {
        mInternalDirtyBits.set(DIRTY_BIT_COMPUTE_UAV_STATE);
    }
}

template <typename CacheType>
void StateManager11::unsetConflictingRTVs(uintptr_t resource, CacheType &viewCache)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    size_t count = std::min(viewCache.size(), viewCache.highestUsed() + 1);
    for (size_t resourceIndex = 0; resourceIndex < count; ++resourceIndex)
    {
        auto &record = viewCache[resourceIndex];

        if (record.view && record.resource == resource)
        {
            deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
            mCurRTVs.clear();
            mCurrentDSV.clear();
            mInternalDirtyBits.set(DIRTY_BIT_RENDER_TARGET);
            return;
        }
    }
}

void StateManager11::unsetConflictingRTVs(uintptr_t resource)
{
    unsetConflictingRTVs(resource, mCurRTVs);
    unsetConflictingRTVs(resource, mCurrentDSV);
}

void StateManager11::unsetConflictingAttachmentResources(
    const gl::FramebufferAttachment &attachment,
    ID3D11Resource *resource)
{
    // Unbind render target SRVs from the shader here to prevent D3D11 warnings.
    if (attachment.type() == GL_TEXTURE)
    {
        uintptr_t resourcePtr       = reinterpret_cast<uintptr_t>(resource);
        const gl::ImageIndex &index = attachment.getTextureImageIndex();
        // The index doesn't need to be corrected for the small compressed texture workaround
        // because a rendertarget is never compressed.
        unsetConflictingSRVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Vertex,
                             resourcePtr, &index, false);
        unsetConflictingSRVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Fragment,
                             resourcePtr, &index, false);
        unsetConflictingSRVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Compute,
                             resourcePtr, &index, false);
        unsetConflictingUAVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Compute,
                             resourcePtr, &index);
    }
    else if (attachment.type() == GL_FRAMEBUFFER_DEFAULT)
    {
        uintptr_t resourcePtr = reinterpret_cast<uintptr_t>(resource);
        unsetConflictingSRVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Vertex,
                             resourcePtr, nullptr, false);
        unsetConflictingSRVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Fragment,
                             resourcePtr, nullptr, false);
        unsetConflictingSRVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Compute,
                             resourcePtr, nullptr, false);
        unsetConflictingUAVs(gl::PipelineType::GraphicsPipeline, gl::ShaderType::Compute,
                             resourcePtr, nullptr);
    }
}

angle::Result StateManager11::ensureInitialized(const gl::Context *context)
{
    Renderer11 *renderer = GetImplAs<Context11>(context)->getRenderer();

    const gl::Caps &caps             = renderer->getNativeCaps();
    const gl::Extensions &extensions = renderer->getNativeExtensions();

    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const GLuint maxShaderTextureImageUnits =
            static_cast<GLuint>(caps.maxShaderTextureImageUnits[shaderType]);

        mCurShaderSRVs[shaderType].initialize(maxShaderTextureImageUnits);
        mForceSetShaderSamplerStates[shaderType].resize(maxShaderTextureImageUnits, true);
        mCurShaderSamplerStates[shaderType].resize(maxShaderTextureImageUnits);
    }
    mCurRTVs.initialize(caps.maxColorAttachments);
    mCurrentDSV.initialize(1);
    mCurComputeUAVs.initialize(caps.maxImageUnits);

    // Initialize cached NULL SRV block
    mNullSRVs.resize(caps.maxShaderTextureImageUnits[gl::ShaderType::Fragment], nullptr);

    mNullUAVs.resize(caps.maxImageUnits, nullptr);

    mCurrentValueAttribs.resize(caps.maxVertexAttributes);

    mShaderConstants.init(caps);

    mIsMultiviewEnabled = extensions.multiviewOVR || extensions.multiview2OVR;

    mIndependentBlendStates = extensions.drawBuffersIndexedAny();  // requires FL10_1

    // ES3.1 context on FL11_0 is limited to 7
    mCurBlendStateExt =
        gl::BlendStateExt(GetImplAs<Context11>(context)->getNativeCaps().maxDrawBuffers);

    ANGLE_TRY(mVertexDataManager.initialize(context));

    mCurrentAttributes.reserve(gl::MAX_VERTEX_ATTRIBS);

    return angle::Result::Continue;
}

void StateManager11::deinitialize()
{
    mCurrentValueAttribs.clear();
    mInputLayoutCache.clear();
    mVertexDataManager.deinitialize();
    mIndexDataManager.deinitialize();

    for (d3d11::Buffer &ShaderDriverConstantBuffer : mShaderDriverConstantBuffers)
    {
        ShaderDriverConstantBuffer.reset();
    }
}

// Applies the render target surface, depth stencil surface, viewport rectangle and
// scissor rectangle to the renderer
angle::Result StateManager11::syncFramebuffer(const gl::Context *context)
{
    // Check for zero-sized default framebuffer, which is a special case.
    // in this case we do not wish to modify any state and just silently return false.
    // this will not report any gl error but will cause the calling method to return.
    if (mFramebuffer11->getState().isDefault())
    {
        RenderTarget11 *firstRT = mFramebuffer11->getFirstRenderTarget();
        const gl::Extents &size = firstRT->getExtents();
        if (size.empty())
        {
            return angle::Result::Continue;
        }
    }

    RTVArray framebufferRTVs = {{}};
    const auto &colorRTs     = mFramebuffer11->getCachedColorRenderTargets();

    size_t appliedRTIndex  = 0;
    bool skipInactiveRTs   = mRenderer->getFeatures().mrtPerfWorkaround.enabled;
    const auto &drawStates = mFramebuffer11->getState().getDrawBufferStates();
    gl::DrawBufferMask activeProgramOutputs =
        mExecutableD3D->getExecutable()->getActiveOutputVariablesMask();
    UINT maxExistingRT           = 0;
    const auto &colorAttachments = mFramebuffer11->getState().getColorAttachments();

    for (size_t rtIndex = 0; rtIndex < colorRTs.size(); ++rtIndex)
    {
        const RenderTarget11 *renderTarget = colorRTs[rtIndex];

        // Skip inactive rendertargets if the workaround is enabled.
        if (skipInactiveRTs &&
            (!renderTarget || drawStates[rtIndex] == GL_NONE || !activeProgramOutputs[rtIndex]))
        {
            continue;
        }

        if (renderTarget)
        {
            framebufferRTVs[appliedRTIndex] = renderTarget->getRenderTargetView().get();
            ASSERT(framebufferRTVs[appliedRTIndex]);
            maxExistingRT = static_cast<UINT>(appliedRTIndex) + 1;

            // Unset conflicting texture SRVs
            const gl::FramebufferAttachment &attachment = colorAttachments[rtIndex];
            ASSERT(attachment.isAttached());
            unsetConflictingAttachmentResources(attachment, renderTarget->getTexture().get());
        }

        appliedRTIndex++;
    }

    // Get the depth stencil buffers
    ID3D11DepthStencilView *framebufferDSV = nullptr;
    const auto *depthStencilRenderTarget   = mFramebuffer11->getCachedDepthStencilRenderTarget();
    if (depthStencilRenderTarget)
    {
        framebufferDSV = depthStencilRenderTarget->getDepthStencilView().get();
        ASSERT(framebufferDSV);

        // Unset conflicting texture SRVs
        const gl::FramebufferAttachment *attachment =
            mFramebuffer11->getState().getDepthOrStencilAttachment();
        ASSERT(attachment);
        unsetConflictingAttachmentResources(*attachment,
                                            depthStencilRenderTarget->getTexture().get());
    }

    ASSERT(maxExistingRT <= static_cast<UINT>(context->getCaps().maxDrawBuffers));

    // Apply the render target and depth stencil
    mRenderer->getDeviceContext()->OMSetRenderTargets(maxExistingRT, framebufferRTVs.data(),
                                                      framebufferDSV);
    mCurRTVs.clear();
    for (UINT i = 0; i < maxExistingRT; i++)
    {
        mCurRTVs.update(i, framebufferRTVs[i]);
    }
    mCurrentDSV.clear();
    mCurrentDSV.update(0, framebufferDSV);
    return angle::Result::Continue;
}

void StateManager11::invalidateCurrentValueAttrib(size_t attribIndex)
{
    mDirtyCurrentValueAttribs.set(attribIndex);
    mInternalDirtyBits.set(DIRTY_BIT_CURRENT_VALUE_ATTRIBS);
    invalidateInputLayout();
    invalidateShaders();
}

angle::Result StateManager11::syncCurrentValueAttribs(
    const gl::Context *context,
    const std::vector<gl::VertexAttribCurrentValueData> &currentValues)
{
    const gl::ProgramExecutable *executable = mExecutableD3D->getExecutable();
    const auto &activeAttribsMask           = executable->getActiveAttribLocationsMask();
    const auto &dirtyActiveAttribs          = (activeAttribsMask & mDirtyCurrentValueAttribs);

    if (!dirtyActiveAttribs.any())
    {
        return angle::Result::Continue;
    }

    const auto &vertexAttributes = mVertexArray11->getState().getVertexAttributes();
    const auto &vertexBindings   = mVertexArray11->getState().getVertexBindings();
    mDirtyCurrentValueAttribs    = (mDirtyCurrentValueAttribs & ~dirtyActiveAttribs);

    for (auto attribIndex : dirtyActiveAttribs)
    {
        if (vertexAttributes[attribIndex].enabled)
            continue;

        const auto *attrib                      = &vertexAttributes[attribIndex];
        const auto &currentValue                = currentValues[attribIndex];
        TranslatedAttribute *currentValueAttrib = &mCurrentValueAttribs[attribIndex];
        currentValueAttrib->currentValueType    = currentValue.Type;
        currentValueAttrib->attribute           = attrib;
        currentValueAttrib->binding             = &vertexBindings[attrib->bindingIndex];

        mDirtyVertexBufferRange.extend(static_cast<unsigned int>(attribIndex));

        ANGLE_TRY(mVertexDataManager.storeCurrentValue(context, currentValue, currentValueAttrib,
                                                       static_cast<size_t>(attribIndex)));
    }

    return angle::Result::Continue;
}

void StateManager11::setInputLayout(const d3d11::InputLayout *inputLayout)
{
    if (setInputLayoutInternal(inputLayout))
    {
        invalidateInputLayout();
    }
}

bool StateManager11::setInputLayoutInternal(const d3d11::InputLayout *inputLayout)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    if (inputLayout == nullptr)
    {
        if (!mCurrentInputLayout.empty())
        {
            deviceContext->IASetInputLayout(nullptr);
            mCurrentInputLayout.clear();
            return true;
        }
    }
    else if (inputLayout->getSerial() != mCurrentInputLayout)
    {
        deviceContext->IASetInputLayout(inputLayout->get());
        mCurrentInputLayout = inputLayout->getSerial();
        return true;
    }

    return false;
}

bool StateManager11::queueVertexBufferChange(size_t bufferIndex,
                                             ID3D11Buffer *buffer,
                                             UINT stride,
                                             UINT offset)
{
    if (buffer != mCurrentVertexBuffers[bufferIndex] ||
        stride != mCurrentVertexStrides[bufferIndex] ||
        offset != mCurrentVertexOffsets[bufferIndex])
    {
        mDirtyVertexBufferRange.extend(static_cast<unsigned int>(bufferIndex));

        mCurrentVertexBuffers[bufferIndex] = buffer;
        mCurrentVertexStrides[bufferIndex] = stride;
        mCurrentVertexOffsets[bufferIndex] = offset;
        return true;
    }

    return false;
}

void StateManager11::applyVertexBufferChanges()
{
    if (mDirtyVertexBufferRange.empty())
    {
        return;
    }

    ASSERT(mDirtyVertexBufferRange.high() <= gl::MAX_VERTEX_ATTRIBS);

    UINT start = static_cast<UINT>(mDirtyVertexBufferRange.low());

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    deviceContext->IASetVertexBuffers(start, static_cast<UINT>(mDirtyVertexBufferRange.length()),
                                      &mCurrentVertexBuffers[start], &mCurrentVertexStrides[start],
                                      &mCurrentVertexOffsets[start]);

    mDirtyVertexBufferRange = gl::RangeUI(gl::MAX_VERTEX_ATTRIBS, 0);
}

void StateManager11::setSingleVertexBuffer(const d3d11::Buffer *buffer, UINT stride, UINT offset)
{
    ID3D11Buffer *native = buffer ? buffer->get() : nullptr;
    if (queueVertexBufferChange(0, native, stride, offset))
    {
        invalidateInputLayout();
        applyVertexBufferChanges();
    }
}

angle::Result StateManager11::updateState(const gl::Context *context,
                                          gl::PrimitiveMode mode,
                                          GLint firstVertex,
                                          GLsizei vertexOrIndexCount,
                                          gl::DrawElementsType indexTypeOrInvalid,
                                          const void *indices,
                                          GLsizei instanceCount,
                                          GLint baseVertex,
                                          GLuint baseInstance,
                                          bool promoteDynamic)
{
    const gl::State &glState = context->getState();

    // TODO(jmadill): Use dirty bits.
    if (mRenderTargetIsDirty)
    {
        processFramebufferInvalidation(context);
        mRenderTargetIsDirty = false;
    }

    // TODO(jmadill): Use dirty bits.
    if (mExecutableD3D->isSamplerMappingDirty())
    {
        mExecutableD3D->updateSamplerMapping();
        invalidateTexturesAndSamplers();
    }

    // TODO(jmadill): Use dirty bits.
    if (mExecutableD3D->anyShaderUniformsDirty())
    {
        mInternalDirtyBits.set(DIRTY_BIT_PROGRAM_UNIFORMS);
    }

    // Swizzling can cause internal state changes with blit shaders.
    if (mDirtySwizzles)
    {
        ANGLE_TRY(generateSwizzles(context));
        mDirtySwizzles = false;
    }

    ANGLE_TRY(mFramebuffer11->markAttachmentsDirty(context));

    // TODO(jiawei.shao@intel.com): This can be recomputed only on framebuffer or multisample mask
    // state changes.
    RenderTarget11 *firstRT = mFramebuffer11->getFirstRenderTarget();
    const int samples       = (firstRT ? firstRT->getSamples() : 0);
    // Single-sampled rendering requires ignoring sample coverage and sample mask states.
    unsigned int sampleMask = (samples != 0) ? GetBlendSampleMask(glState, samples) : 0xFFFFFFFF;
    if (sampleMask != mCurSampleMask)
    {
        mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
    }

    ANGLE_TRY(mVertexArray11->syncStateForDraw(context, firstVertex, vertexOrIndexCount,
                                               indexTypeOrInvalid, indices, instanceCount,
                                               baseVertex, baseInstance, promoteDynamic));

    // Changes in the draw call can affect the vertex buffer translations.
    if (!mLastFirstVertex.valid() || mLastFirstVertex.value() != firstVertex)
    {
        mLastFirstVertex = firstVertex;
        invalidateInputLayout();
    }

    // The ShaderConstants only need to be updated when the program uses vertexID
    if (mExecutableD3D->usesVertexID())
    {
        GLint firstVertexOnChange = firstVertex + baseVertex;
        ASSERT(mVertexArray11);
        if (mVertexArray11->hasActiveDynamicAttrib(context) &&
            indexTypeOrInvalid != gl::DrawElementsType::InvalidEnum)
        {
            // drawElements with Dynamic attribute
            // the firstVertex is already including baseVertex when
            // doing ComputeStartVertex
            firstVertexOnChange = firstVertex;
        }

        if (mShaderConstants.onFirstVertexChange(firstVertexOnChange))
        {
            mInternalDirtyBits.set(DIRTY_BIT_DRIVER_UNIFORMS);
        }
    }

    if (indexTypeOrInvalid != gl::DrawElementsType::InvalidEnum)
    {
        ANGLE_TRY(applyIndexBuffer(context, vertexOrIndexCount, indexTypeOrInvalid, indices));
    }

    if (mLastAppliedDrawMode != mode)
    {
        mLastAppliedDrawMode = mode;
        mInternalDirtyBits.set(DIRTY_BIT_PRIMITIVE_TOPOLOGY);

        bool pointDrawMode = (mode == gl::PrimitiveMode::Points);
        if (pointDrawMode != mCurRasterState.pointDrawMode)
        {
            mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);

            // Changing from points to not points (or vice-versa) affects the geometry shader.
            invalidateShaders();
        }
    }

    auto dirtyBitsCopy = mInternalDirtyBits & mGraphicsDirtyBitsMask;

    for (auto iter = dirtyBitsCopy.begin(), end = dirtyBitsCopy.end(); iter != end; ++iter)
    {
        mInternalDirtyBits.reset(*iter);
        switch (*iter)
        {
            case DIRTY_BIT_RENDER_TARGET:
                ANGLE_TRY(syncFramebuffer(context));
                break;
            case DIRTY_BIT_VIEWPORT_STATE:
                syncViewport(context);
                break;
            case DIRTY_BIT_SCISSOR_STATE:
                syncScissorRectangle(context);
                break;
            case DIRTY_BIT_RASTERIZER_STATE:
                ANGLE_TRY(syncRasterizerState(context, mode));
                break;
            case DIRTY_BIT_BLEND_STATE:
                // Single-sampled rendering requires ignoring alpha-to-coverage state.
                ANGLE_TRY(syncBlendState(context, glState.getBlendStateExt(),
                                         glState.getBlendColor(), sampleMask,
                                         glState.isSampleAlphaToCoverageEnabled() && (samples != 0),
                                         glState.hasConstantAlphaBlendFunc()));
                break;
            case DIRTY_BIT_DEPTH_STENCIL_STATE:
                ANGLE_TRY(syncDepthStencilState(context));
                break;
            case DIRTY_BIT_GRAPHICS_SRV_STATE:
                ANGLE_TRY(syncTextures(context));
                break;
            case DIRTY_BIT_GRAPHICS_UAV_STATE:
                ANGLE_TRY(syncUAVsForGraphics(context));
                break;
            case DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE:
                // TODO(jmadill): More fine-grained update.
                ANGLE_TRY(syncTextures(context));
                break;
            case DIRTY_BIT_PROGRAM_UNIFORMS:
                ANGLE_TRY(applyUniforms(context));
                break;
            case DIRTY_BIT_DRIVER_UNIFORMS:
                // This must happen after viewport sync; the viewport affects builtin uniforms.
                ANGLE_TRY(applyDriverUniforms(context));
                break;
            case DIRTY_BIT_PROGRAM_UNIFORM_BUFFERS:
                ANGLE_TRY(syncUniformBuffers(context));
                break;
            case DIRTY_BIT_SHADERS:
                ANGLE_TRY(syncProgram(context, mode));
                break;
            case DIRTY_BIT_CURRENT_VALUE_ATTRIBS:
                ANGLE_TRY(syncCurrentValueAttribs(context, glState.getVertexAttribCurrentValues()));
                break;
            case DIRTY_BIT_TRANSFORM_FEEDBACK:
                ANGLE_TRY(syncTransformFeedbackBuffers(context));
                break;
            case DIRTY_BIT_VERTEX_BUFFERS_AND_INPUT_LAYOUT:
                ANGLE_TRY(syncVertexBuffersAndInputLayout(context, mode, firstVertex,
                                                          vertexOrIndexCount, indexTypeOrInvalid,
                                                          instanceCount));
                break;
            case DIRTY_BIT_PRIMITIVE_TOPOLOGY:
                syncPrimitiveTopology(glState, mode);
                break;
            default:
                UNREACHABLE();
                break;
        }
    }

    // Check that we haven't set any dirty bits in the flushing of the dirty bits loop, except
    // DIRTY_BIT_COMPUTE_SRVUAV_STATE dirty bit.
    ASSERT((mInternalDirtyBits & mGraphicsDirtyBitsMask).none());

    return angle::Result::Continue;
}

void StateManager11::setShaderResourceShared(gl::ShaderType shaderType,
                                             UINT resourceSlot,
                                             const d3d11::SharedSRV *srv)
{
    setShaderResourceInternal(shaderType, resourceSlot, srv);

    // TODO(jmadill): Narrower dirty region.
    mInternalDirtyBits.set(DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE);
}

void StateManager11::setShaderResource(gl::ShaderType shaderType,
                                       UINT resourceSlot,
                                       const d3d11::ShaderResourceView *srv)
{
    setShaderResourceInternal(shaderType, resourceSlot, srv);

    // TODO(jmadill): Narrower dirty region.
    mInternalDirtyBits.set(DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE);
}

void StateManager11::setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    if (setPrimitiveTopologyInternal(primitiveTopology))
    {
        mInternalDirtyBits.set(DIRTY_BIT_PRIMITIVE_TOPOLOGY);
    }
}

bool StateManager11::setPrimitiveTopologyInternal(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    if (primitiveTopology != mCurrentPrimitiveTopology)
    {
        mRenderer->getDeviceContext()->IASetPrimitiveTopology(primitiveTopology);
        mCurrentPrimitiveTopology = primitiveTopology;
        return true;
    }
    else
    {
        return false;
    }
}

void StateManager11::setDrawShaders(const d3d11::VertexShader *vertexShader,
                                    const d3d11::GeometryShader *geometryShader,
                                    const d3d11::PixelShader *pixelShader)
{
    setVertexShader(vertexShader);
    setGeometryShader(geometryShader);
    setPixelShader(pixelShader);
}

void StateManager11::setVertexShader(const d3d11::VertexShader *shader)
{
    ResourceSerial serial = shader ? shader->getSerial() : ResourceSerial(0);

    if (serial != mAppliedShaders[gl::ShaderType::Vertex])
    {
        ID3D11VertexShader *appliedShader = shader ? shader->get() : nullptr;
        mRenderer->getDeviceContext()->VSSetShader(appliedShader, nullptr, 0);
        mAppliedShaders[gl::ShaderType::Vertex] = serial;
        invalidateShaders();
    }
}

void StateManager11::setGeometryShader(const d3d11::GeometryShader *shader)
{
    ResourceSerial serial = shader ? shader->getSerial() : ResourceSerial(0);

    if (serial != mAppliedShaders[gl::ShaderType::Geometry])
    {
        ID3D11GeometryShader *appliedShader = shader ? shader->get() : nullptr;
        mRenderer->getDeviceContext()->GSSetShader(appliedShader, nullptr, 0);
        mAppliedShaders[gl::ShaderType::Geometry] = serial;
        invalidateShaders();
    }
}

void StateManager11::setPixelShader(const d3d11::PixelShader *shader)
{
    ResourceSerial serial = shader ? shader->getSerial() : ResourceSerial(0);

    if (serial != mAppliedShaders[gl::ShaderType::Fragment])
    {
        ID3D11PixelShader *appliedShader = shader ? shader->get() : nullptr;
        mRenderer->getDeviceContext()->PSSetShader(appliedShader, nullptr, 0);
        mAppliedShaders[gl::ShaderType::Fragment] = serial;
        invalidateShaders();
    }
}

void StateManager11::setComputeShader(const d3d11::ComputeShader *shader)
{
    ResourceSerial serial = shader ? shader->getSerial() : ResourceSerial(0);

    if (serial != mAppliedShaders[gl::ShaderType::Compute])
    {
        ID3D11ComputeShader *appliedShader = shader ? shader->get() : nullptr;
        mRenderer->getDeviceContext()->CSSetShader(appliedShader, nullptr, 0);
        mAppliedShaders[gl::ShaderType::Compute] = serial;
        invalidateShaders();
    }
}

void StateManager11::setVertexConstantBuffer(unsigned int slot, const d3d11::Buffer *buffer)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    auto &currentSerial                = mCurrentConstantBufferVS[slot];

    mCurrentConstantBufferVSOffset[slot] = 0;
    mCurrentConstantBufferVSSize[slot]   = 0;

    if (buffer)
    {
        if (currentSerial != buffer->getSerial())
        {
            deviceContext->VSSetConstantBuffers(slot, 1, buffer->getPointer());
            currentSerial = buffer->getSerial();
            invalidateConstantBuffer(slot);
        }
    }
    else
    {
        if (!currentSerial.empty())
        {
            ID3D11Buffer *nullBuffer = nullptr;
            deviceContext->VSSetConstantBuffers(slot, 1, &nullBuffer);
            currentSerial.clear();
            invalidateConstantBuffer(slot);
        }
    }
}

void StateManager11::setPixelConstantBuffer(unsigned int slot, const d3d11::Buffer *buffer)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    auto &currentSerial                = mCurrentConstantBufferPS[slot];

    mCurrentConstantBufferPSOffset[slot] = 0;
    mCurrentConstantBufferPSSize[slot]   = 0;

    if (buffer)
    {
        if (currentSerial != buffer->getSerial())
        {
            deviceContext->PSSetConstantBuffers(slot, 1, buffer->getPointer());
            currentSerial = buffer->getSerial();
            invalidateConstantBuffer(slot);
        }
    }
    else
    {
        if (!currentSerial.empty())
        {
            ID3D11Buffer *nullBuffer = nullptr;
            deviceContext->PSSetConstantBuffers(slot, 1, &nullBuffer);
            currentSerial.clear();
            invalidateConstantBuffer(slot);
        }
    }
}

void StateManager11::setDepthStencilState(const d3d11::DepthStencilState *depthStencilState,
                                          UINT stencilRef)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    if (depthStencilState)
    {
        deviceContext->OMSetDepthStencilState(depthStencilState->get(), stencilRef);
    }
    else
    {
        deviceContext->OMSetDepthStencilState(nullptr, stencilRef);
    }

    mInternalDirtyBits.set(DIRTY_BIT_DEPTH_STENCIL_STATE);
}

void StateManager11::setSimpleBlendState(const d3d11::BlendState *blendState)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    if (blendState)
    {
        deviceContext->OMSetBlendState(blendState->get(), nullptr, 0xFFFFFFFF);
    }
    else
    {
        deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    }

    mInternalDirtyBits.set(DIRTY_BIT_BLEND_STATE);
}

void StateManager11::setRasterizerState(const d3d11::RasterizerState *rasterizerState)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    if (rasterizerState)
    {
        deviceContext->RSSetState(rasterizerState->get());
    }
    else
    {
        deviceContext->RSSetState(nullptr);
    }

    mInternalDirtyBits.set(DIRTY_BIT_RASTERIZER_STATE);
}

void StateManager11::setSimpleViewport(const gl::Extents &extents)
{
    setSimpleViewport(extents.width, extents.height);
}

void StateManager11::setSimpleViewport(int width, int height)
{
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = static_cast<FLOAT>(width);
    viewport.Height   = static_cast<FLOAT>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    mRenderer->getDeviceContext()->RSSetViewports(1, &viewport);
    mInternalDirtyBits.set(DIRTY_BIT_VIEWPORT_STATE);
}

void StateManager11::setSimplePixelTextureAndSampler(const d3d11::SharedSRV &srv,
                                                     const d3d11::SamplerState &samplerState)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    setShaderResourceInternal(gl::ShaderType::Fragment, 0, &srv);
    deviceContext->PSSetSamplers(0, 1, samplerState.getPointer());

    mInternalDirtyBits.set(DIRTY_BIT_TEXTURE_AND_SAMPLER_STATE);
    mForceSetShaderSamplerStates[gl::ShaderType::Fragment][0] = true;
}

void StateManager11::setSimpleScissorRect(const gl::Rectangle &glRect)
{
    D3D11_RECT scissorRect;
    scissorRect.left   = glRect.x;
    scissorRect.right  = glRect.x + glRect.width;
    scissorRect.top    = glRect.y;
    scissorRect.bottom = glRect.y + glRect.height;
    setScissorRectD3D(scissorRect);
}

void StateManager11::setScissorRectD3D(const D3D11_RECT &d3dRect)
{
    mRenderer->getDeviceContext()->RSSetScissorRects(1, &d3dRect);
    mInternalDirtyBits.set(DIRTY_BIT_SCISSOR_STATE);
}

angle::Result StateManager11::syncTextures(const gl::Context *context)
{
    ANGLE_TRY(applyTexturesForSRVs(context, gl::ShaderType::Vertex));
    ANGLE_TRY(applyTexturesForSRVs(context, gl::ShaderType::Fragment));
    if (mExecutableD3D->hasShaderStage(gl::ShaderType::Geometry))
    {
        ANGLE_TRY(applyTexturesForSRVs(context, gl::ShaderType::Geometry));
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::setSamplerState(const gl::Context *context,
                                              gl::ShaderType type,
                                              int index,
                                              gl::Texture *texture,
                                              const gl::SamplerState &samplerState)
{
#if !defined(NDEBUG)
    // Storage should exist, texture should be complete. Only verified in Debug.
    TextureD3D *textureD3D  = GetImplAs<TextureD3D>(texture);
    TextureStorage *storage = nullptr;
    ANGLE_TRY(textureD3D->getNativeTexture(context, &storage));
    ASSERT(storage);
#endif  // !defined(NDEBUG)

    auto *deviceContext = mRenderer->getDeviceContext();

    ASSERT(index < mRenderer->getNativeCaps().maxShaderTextureImageUnits[type]);

    // When border color is used, its value may need to be readjusted based on the texture format.
    const bool usesBorderColor = samplerState.usesBorderColor();

    if (mForceSetShaderSamplerStates[type][index] || usesBorderColor ||
        memcmp(&samplerState, &mCurShaderSamplerStates[type][index], sizeof(gl::SamplerState)) != 0)
    {
        // When clamp-to-border mode is used and a floating-point border color is set, the color
        // value must be adjusted based on the texture format. Reset it to zero in all other cases
        // to reduce the number of cached sampler entries. Address modes for integer texture
        // formats are emulated in shaders and do not rely on this state.
        angle::ColorGeneric borderColor;
        if (usesBorderColor)
        {
            if (samplerState.getBorderColor().type == angle::ColorGeneric::Type::Float)
            {
                borderColor = samplerState.getBorderColor();
            }
            const uint32_t baseLevel       = texture->getTextureState().getEffectiveBaseLevel();
            const gl::TextureTarget target = TextureTypeToTarget(texture->getType(), 0);
            const angle::Format &format    = angle::Format::Get(angle::Format::InternalFormatToID(
                texture->getFormat(target, baseLevel).info->sizedInternalFormat));

            borderColor = AdjustBorderColor<false>(borderColor, format, false);
        }
        gl::SamplerState adjustedSamplerState(samplerState);
        adjustedSamplerState.setBorderColor(borderColor.colorF);

        ID3D11SamplerState *dxSamplerState = nullptr;
        ANGLE_TRY(mRenderer->getSamplerState(context, adjustedSamplerState, &dxSamplerState));

        ASSERT(dxSamplerState != nullptr);

        switch (type)
        {
            case gl::ShaderType::Vertex:
                deviceContext->VSSetSamplers(index, 1, &dxSamplerState);
                break;
            case gl::ShaderType::Fragment:
                deviceContext->PSSetSamplers(index, 1, &dxSamplerState);
                break;
            case gl::ShaderType::Compute:
                deviceContext->CSSetSamplers(index, 1, &dxSamplerState);
                break;
            case gl::ShaderType::Geometry:
                deviceContext->GSSetSamplers(index, 1, &dxSamplerState);
                break;
            default:
                UNREACHABLE();
                break;
        }

        mCurShaderSamplerStates[type][index] = samplerState;
    }

    mForceSetShaderSamplerStates[type][index] = false;

    // Sampler metadata that's passed to shaders in uniforms is stored separately from rest of the
    // sampler state since having it in contiguous memory makes it possible to memcpy to a constant
    // buffer, and it doesn't affect the state set by
    // PSSetSamplers/VSSetSamplers/CSSetSamplers/GSSetSamplers.
    mShaderConstants.onSamplerChange(type, index, *texture, samplerState);

    return angle::Result::Continue;
}

angle::Result StateManager11::setTextureForSampler(const gl::Context *context,
                                                   gl::ShaderType type,
                                                   int index,
                                                   gl::Texture *texture,
                                                   const gl::SamplerState &sampler)
{
    const d3d11::SharedSRV *textureSRV = nullptr;

    if (texture)
    {
        TextureD3D *textureImpl = GetImplAs<TextureD3D>(texture);

        TextureStorage *texStorage = nullptr;
        ANGLE_TRY(textureImpl->getNativeTexture(context, &texStorage));

        // Texture should be complete and have a storage
        ASSERT(texStorage);

        TextureStorage11 *storage11 = GetAs<TextureStorage11>(texStorage);

        ANGLE_TRY(
            storage11->getSRVForSampler(context, texture->getTextureState(), sampler, &textureSRV));

        // If we get an invalid SRV here, something went wrong in the texture class and we're
        // unexpectedly missing the shader resource view.
        ASSERT(textureSRV->valid());

        textureImpl->resetDirty();
    }

    ASSERT(
        (type == gl::ShaderType::Fragment &&
         index < mRenderer->getNativeCaps().maxShaderTextureImageUnits[gl::ShaderType::Fragment]) ||
        (type == gl::ShaderType::Vertex &&
         index < mRenderer->getNativeCaps().maxShaderTextureImageUnits[gl::ShaderType::Vertex]) ||
        (type == gl::ShaderType::Compute &&
         index < mRenderer->getNativeCaps().maxShaderTextureImageUnits[gl::ShaderType::Compute]));

    setShaderResourceInternal(type, index, textureSRV);
    return angle::Result::Continue;
}

angle::Result StateManager11::setImageState(const gl::Context *context,
                                            gl::ShaderType type,
                                            int index,
                                            const gl::ImageUnit &imageUnit)
{
    ASSERT(index < mRenderer->getNativeCaps().maxShaderImageUniforms[type]);

    if (mShaderConstants.onImageChange(type, index, imageUnit))
    {
        invalidateDriverUniforms();
    }

    return angle::Result::Continue;
}

// For each Direct3D sampler of either the pixel or vertex stage,
// looks up the corresponding OpenGL texture image unit and texture type,
// and sets the texture and its addressing/filtering state (or NULL when inactive).
// Sampler mapping needs to be up-to-date on the program object before this is called.
angle::Result StateManager11::applyTexturesForSRVs(const gl::Context *context,
                                                   gl::ShaderType shaderType)
{
    const auto &glState = context->getState();
    const auto &caps    = context->getCaps();

    ASSERT(!mExecutableD3D->isSamplerMappingDirty());

    // TODO(jmadill): Use the Program's sampler bindings.
    const gl::ActiveTexturesCache &completeTextures = glState.getActiveTexturesCache();

    const gl::RangeUI samplerRange = mExecutableD3D->getUsedSamplerRange(shaderType);
    for (unsigned int samplerIndex = samplerRange.low(); samplerIndex < samplerRange.high();
         samplerIndex++)
    {
        GLint textureUnit = mExecutableD3D->getSamplerMapping(shaderType, samplerIndex, caps);
        ASSERT(textureUnit != -1);
        gl::Texture *texture = completeTextures[textureUnit];

        // A nullptr texture indicates incomplete.
        if (texture)
        {
            gl::Sampler *samplerObject = glState.getSampler(textureUnit);

            const gl::SamplerState &samplerState =
                samplerObject ? samplerObject->getSamplerState() : texture->getSamplerState();

            ANGLE_TRY(setSamplerState(context, shaderType, samplerIndex, texture, samplerState));
            ANGLE_TRY(
                setTextureForSampler(context, shaderType, samplerIndex, texture, samplerState));
        }
        else
        {
            gl::TextureType textureType =
                mExecutableD3D->getSamplerTextureType(shaderType, samplerIndex);

            // Texture is not sampler complete or it is in use by the framebuffer.  Bind the
            // incomplete texture.
            gl::Texture *incompleteTexture = nullptr;
            ANGLE_TRY(mRenderer->getIncompleteTexture(context, textureType, &incompleteTexture));
            ANGLE_TRY(setSamplerState(context, shaderType, samplerIndex, incompleteTexture,
                                      incompleteTexture->getSamplerState()));
            ANGLE_TRY(setTextureForSampler(context, shaderType, samplerIndex, incompleteTexture,
                                           incompleteTexture->getSamplerState()));
        }
    }

    const gl::RangeUI readonlyImageRange = mExecutableD3D->getUsedImageRange(shaderType, true);
    for (unsigned int readonlyImageIndex = readonlyImageRange.low();
         readonlyImageIndex < readonlyImageRange.high(); readonlyImageIndex++)
    {
        GLint imageUnitIndex =
            mExecutableD3D->getImageMapping(shaderType, readonlyImageIndex, true, caps);
        ASSERT(imageUnitIndex != -1);
        const gl::ImageUnit &imageUnit = glState.getImageUnit(imageUnitIndex);
        if (!imageUnit.layered)
        {
            ANGLE_TRY(setImageState(context, gl::ShaderType::Compute,
                                    readonlyImageIndex - readonlyImageRange.low(), imageUnit));
        }
        ANGLE_TRY(setTextureForImage(context, shaderType, readonlyImageIndex, imageUnit));
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::getUAVsForRWImages(const gl::Context *context,
                                                 gl::ShaderType shaderType,
                                                 UAVList *uavList)
{
    const auto &glState = context->getState();
    const auto &caps    = context->getCaps();

    const gl::RangeUI imageRange = mExecutableD3D->getUsedImageRange(shaderType, false);
    for (unsigned int imageIndex = imageRange.low(); imageIndex < imageRange.high(); imageIndex++)
    {
        GLint imageUnitIndex = mExecutableD3D->getImageMapping(shaderType, imageIndex, false, caps);
        ASSERT(imageUnitIndex != -1);
        const gl::ImageUnit &imageUnit = glState.getImageUnit(imageUnitIndex);
        if (!imageUnit.layered)
        {
            ANGLE_TRY(setImageState(context, shaderType, imageIndex - imageRange.low(), imageUnit));
        }
        ANGLE_TRY(getUAVForRWImage(context, shaderType, imageIndex, imageUnit, uavList));
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::syncTexturesForCompute(const gl::Context *context)
{
    ANGLE_TRY(applyTexturesForSRVs(context, gl::ShaderType::Compute));
    return angle::Result::Continue;
}

angle::Result StateManager11::setTextureForImage(const gl::Context *context,
                                                 gl::ShaderType type,
                                                 int index,
                                                 const gl::ImageUnit &imageUnit)
{
    TextureD3D *textureImpl = nullptr;
    if (!imageUnit.texture.get())
    {
        setShaderResourceInternal<d3d11::ShaderResourceView>(type, static_cast<UINT>(index),
                                                             nullptr);
        return angle::Result::Continue;
    }

    textureImpl = GetImplAs<TextureD3D>(imageUnit.texture.get());

    // Ensure that texture has unordered access; convert it if not.
    ANGLE_TRY(textureImpl->ensureUnorderedAccess(context));

    TextureStorage *texStorage = nullptr;
    ANGLE_TRY(textureImpl->getNativeTexture(context, &texStorage));
    // Texture should be complete and have a storage
    ASSERT(texStorage);
    TextureStorage11 *storage11 = GetAs<TextureStorage11>(texStorage);

    const d3d11::SharedSRV *textureSRV = nullptr;
    ANGLE_TRY(storage11->getSRVForImage(context, imageUnit, &textureSRV));
    // If we get an invalid SRV here, something went wrong in the texture class and we're
    // unexpectedly missing the shader resource view.
    ASSERT(textureSRV->valid());
    ASSERT((index < mRenderer->getNativeCaps().maxImageUnits));
    setShaderResourceInternal(type, index, textureSRV);

    textureImpl->resetDirty();
    return angle::Result::Continue;
}

angle::Result StateManager11::getUAVForRWImage(const gl::Context *context,
                                               gl::ShaderType type,
                                               int index,
                                               const gl::ImageUnit &imageUnit,
                                               UAVList *uavList)
{
    TextureD3D *textureImpl = nullptr;
    if (!imageUnit.texture.get())
    {
        setUnorderedAccessViewInternal<d3d11::UnorderedAccessView>(static_cast<UINT>(index),
                                                                   nullptr, uavList);
        return angle::Result::Continue;
    }

    textureImpl = GetImplAs<TextureD3D>(imageUnit.texture.get());

    // Ensure that texture has unordered access; convert it if not.
    ANGLE_TRY(textureImpl->ensureUnorderedAccess(context));

    TextureStorage *texStorage = nullptr;
    ANGLE_TRY(textureImpl->getNativeTexture(context, &texStorage));
    // Texture should be complete and have a storage
    ASSERT(texStorage);
    TextureStorage11 *storage11 = GetAs<TextureStorage11>(texStorage);

    const d3d11::SharedUAV *textureUAV = nullptr;
    ANGLE_TRY(storage11->getUAVForImage(context, imageUnit, &textureUAV));
    // If we get an invalid UAV here, something went wrong in the texture class and we're
    // unexpectedly missing the unordered access view.
    ASSERT(textureUAV->valid());
    ASSERT((index < mRenderer->getNativeCaps().maxImageUnits));
    setUnorderedAccessViewInternal(index, textureUAV, uavList);

    textureImpl->resetDirty();
    return angle::Result::Continue;
}

// Things that affect a program's dirtyness:
// 1. Directly changing the program executable -> triggered in StateManager11::syncState.
// 2. The vertex attribute layout              -> triggered in VertexArray11::syncState/signal.
// 3. The fragment shader's rendertargets      -> triggered in Framebuffer11::syncState/signal.
// 4. Enabling/disabling rasterizer discard.   -> triggered in StateManager11::syncState.
// 5. Enabling/disabling transform feedback.   -> checked in StateManager11::updateState.
// 6. An internal shader was used.             -> triggered in StateManager11::set*Shader.
// 7. Drawing with/without point sprites.      -> checked in StateManager11::updateState.
// TODO(jmadill): Use dirty bits for transform feedback.
angle::Result StateManager11::syncProgram(const gl::Context *context, gl::PrimitiveMode drawMode)
{
    Context11 *context11  = GetImplAs<Context11>(context);
    RendererD3D *renderer = context11->getRenderer();
    ANGLE_TRY(context11->triggerDrawCallProgramRecompilation(context, drawMode));

    const auto &glState = context->getState();

    // TODO: change mExecutableD3D to mExecutableD3D?
    mExecutableD3D->updateCachedInputLayout(context11->getRenderer(),
                                            mVertexArray11->getCurrentStateSerial(), glState);

    // Binaries must be compiled before the sync.
    ASSERT(mExecutableD3D->hasVertexExecutableForCachedInputLayout());
    ASSERT(mExecutableD3D->hasGeometryExecutableForPrimitiveType(context11->getRenderer(), glState,
                                                                 drawMode));
    ASSERT(mExecutableD3D->hasPixelExecutableForCachedOutputLayout());

    ShaderExecutableD3D *vertexExe = nullptr;
    ANGLE_TRY(mExecutableD3D->getVertexExecutableForCachedInputLayout(context11, renderer,
                                                                      &vertexExe, nullptr));

    ShaderExecutableD3D *pixelExe = nullptr;
    ANGLE_TRY(mExecutableD3D->getPixelExecutableForCachedOutputLayout(context11, renderer,
                                                                      &pixelExe, nullptr));

    ShaderExecutableD3D *geometryExe = nullptr;
    ANGLE_TRY(mExecutableD3D->getGeometryExecutableForPrimitiveType(
        context11, renderer, glState.getCaps(), glState.getProvokingVertex(), drawMode,
        &geometryExe, nullptr));

    const d3d11::VertexShader *vertexShader =
        (vertexExe ? &GetAs<ShaderExecutable11>(vertexExe)->getVertexShader() : nullptr);

    // Skip pixel shader if we're doing rasterizer discard.
    const d3d11::PixelShader *pixelShader = nullptr;
    if (!glState.getRasterizerState().rasterizerDiscard)
    {
        pixelShader = (pixelExe ? &GetAs<ShaderExecutable11>(pixelExe)->getPixelShader() : nullptr);
    }

    const d3d11::GeometryShader *geometryShader = nullptr;
    if (glState.isTransformFeedbackActiveUnpaused())
    {
        geometryShader =
            (vertexExe ? &GetAs<ShaderExecutable11>(vertexExe)->getStreamOutShader() : nullptr);
    }
    else
    {
        geometryShader =
            (geometryExe ? &GetAs<ShaderExecutable11>(geometryExe)->getGeometryShader() : nullptr);
    }

    setDrawShaders(vertexShader, geometryShader, pixelShader);

    // Explicitly clear the shaders dirty bit.
    mInternalDirtyBits.reset(DIRTY_BIT_SHADERS);

    return angle::Result::Continue;
}

angle::Result StateManager11::syncProgramForCompute(const gl::Context *context)
{
    Context11 *context11 = GetImplAs<Context11>(context);
    ANGLE_TRY(context11->triggerDispatchCallProgramRecompilation(context));

    mExecutableD3D->updateCachedImage2DBindLayout(context, gl::ShaderType::Compute);

    // Binaries must be compiled before the sync.
    ASSERT(mExecutableD3D->hasComputeExecutableForCachedImage2DBindLayout());

    ShaderExecutableD3D *computeExe = nullptr;
    ANGLE_TRY(mExecutableD3D->getComputeExecutableForImage2DBindLayout(
        context11, context11->getRenderer(), &computeExe, nullptr));

    const d3d11::ComputeShader *computeShader =
        (computeExe ? &GetAs<ShaderExecutable11>(computeExe)->getComputeShader() : nullptr);
    setComputeShader(computeShader);
    // Explicitly clear the shaders dirty bit.
    mInternalDirtyBits.reset(DIRTY_BIT_SHADERS);

    return angle::Result::Continue;
}

angle::Result StateManager11::syncVertexBuffersAndInputLayout(
    const gl::Context *context,
    gl::PrimitiveMode mode,
    GLint firstVertex,
    GLsizei vertexOrIndexCount,
    gl::DrawElementsType indexTypeOrInvalid,
    GLsizei instanceCount)
{
    const auto &vertexArrayAttribs = mVertexArray11->getTranslatedAttribs();

    // Sort the attributes according to ensure we re-use similar input layouts.
    AttribIndexArray sortedSemanticIndices;
    SortAttributesByLayout(*mExecutableD3D, vertexArrayAttribs, mCurrentValueAttribs,
                           &sortedSemanticIndices, &mCurrentAttributes);

    D3D_FEATURE_LEVEL featureLevel = mRenderer->getRenderer11DeviceCaps().featureLevel;

    // If we are using FL 9_3, make sure the first attribute is not instanced
    if (featureLevel <= D3D_FEATURE_LEVEL_9_3 && !mCurrentAttributes.empty())
    {
        if (mCurrentAttributes[0]->divisor > 0)
        {
            Optional<size_t> firstNonInstancedIndex = FindFirstNonInstanced(mCurrentAttributes);
            if (firstNonInstancedIndex.valid())
            {
                size_t index = firstNonInstancedIndex.value();
                std::swap(mCurrentAttributes[0], mCurrentAttributes[index]);
                std::swap(sortedSemanticIndices[0], sortedSemanticIndices[index]);
            }
        }
    }

    // Update the applied input layout by querying the cache.
    const gl::State &state                = context->getState();
    const d3d11::InputLayout *inputLayout = nullptr;
    ANGLE_TRY(mInputLayoutCache.getInputLayout(GetImplAs<Context11>(context), state,
                                               mCurrentAttributes, sortedSemanticIndices, mode,
                                               vertexOrIndexCount, instanceCount, &inputLayout));
    setInputLayoutInternal(inputLayout);

    // Update the applied vertex buffers.
    ANGLE_TRY(applyVertexBuffers(context, mode, indexTypeOrInvalid, firstVertex));

    return angle::Result::Continue;
}

angle::Result StateManager11::applyVertexBuffers(const gl::Context *context,
                                                 gl::PrimitiveMode mode,
                                                 gl::DrawElementsType indexTypeOrInvalid,
                                                 GLint firstVertex)
{
    for (size_t attribIndex = 0; attribIndex < gl::MAX_VERTEX_ATTRIBS; ++attribIndex)
    {
        ID3D11Buffer *buffer = nullptr;
        UINT vertexStride    = 0;
        UINT vertexOffset    = 0;

        if (attribIndex < mCurrentAttributes.size())
        {
            const TranslatedAttribute &attrib = *mCurrentAttributes[attribIndex];
            Buffer11 *bufferStorage = attrib.storage ? GetAs<Buffer11>(attrib.storage) : nullptr;

            // If indexed pointsprite emulation is active, then we need to take a less efficent code
            // path. Emulated indexed pointsprite rendering requires that the vertex buffers match
            // exactly to the indices passed by the caller.  This could expand or shrink the vertex
            // buffer depending on the number of points indicated by the index list or how many
            // duplicates are found on the index list.
            if (bufferStorage == nullptr)
            {
                ASSERT(attrib.vertexBuffer.get());
                buffer = GetAs<VertexBuffer11>(attrib.vertexBuffer.get())->getBuffer().get();
            }
            else
            {
                ANGLE_TRY(bufferStorage->getBuffer(
                    context, BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK, &buffer));
            }

            vertexStride = attrib.stride;
            ANGLE_TRY(attrib.computeOffset(context, firstVertex, &vertexOffset));
        }

        queueVertexBufferChange(attribIndex, buffer, vertexStride, vertexOffset);
    }

    applyVertexBufferChanges();
    return angle::Result::Continue;
}

angle::Result StateManager11::applyIndexBuffer(const gl::Context *context,
                                               GLsizei indexCount,
                                               gl::DrawElementsType indexType,
                                               const void *indices)
{
    if (!mIndexBufferIsDirty)
    {
        // No streaming or index buffer application necessary.
        return angle::Result::Continue;
    }

    gl::DrawElementsType destElementType = mVertexArray11->getCachedDestinationIndexType();
    gl::Buffer *elementArrayBuffer       = mVertexArray11->getState().getElementArrayBuffer();

    TranslatedIndexData indexInfo;
    ANGLE_TRY(mIndexDataManager.prepareIndexData(context, indexType, destElementType, indexCount,
                                                 elementArrayBuffer, indices, &indexInfo));

    ID3D11Buffer *buffer     = nullptr;
    DXGI_FORMAT bufferFormat = (indexInfo.indexType == gl::DrawElementsType::UnsignedInt)
                                   ? DXGI_FORMAT_R32_UINT
                                   : DXGI_FORMAT_R16_UINT;

    if (indexInfo.storage)
    {
        Buffer11 *storage = GetAs<Buffer11>(indexInfo.storage);
        ANGLE_TRY(storage->getBuffer(context, BUFFER_USAGE_INDEX, &buffer));
    }
    else
    {
        IndexBuffer11 *indexBuffer = GetAs<IndexBuffer11>(indexInfo.indexBuffer);
        buffer                     = indexBuffer->getBuffer().get();
    }

    // Track dirty indices in the index range cache.
    indexInfo.srcIndexData.srcIndicesChanged =
        syncIndexBuffer(buffer, bufferFormat, indexInfo.startOffset);

    mIndexBufferIsDirty = false;

    mVertexArray11->updateCachedIndexInfo(indexInfo);
    return angle::Result::Continue;
}

void StateManager11::setIndexBuffer(ID3D11Buffer *buffer,
                                    DXGI_FORMAT indexFormat,
                                    unsigned int offset)
{
    if (syncIndexBuffer(buffer, indexFormat, offset))
    {
        invalidateIndexBuffer();
    }
}

bool StateManager11::syncIndexBuffer(ID3D11Buffer *buffer,
                                     DXGI_FORMAT indexFormat,
                                     unsigned int offset)
{
    if (buffer != mAppliedIB || indexFormat != mAppliedIBFormat || offset != mAppliedIBOffset)
    {
        mRenderer->getDeviceContext()->IASetIndexBuffer(buffer, indexFormat, offset);

        mAppliedIB       = buffer;
        mAppliedIBFormat = indexFormat;
        mAppliedIBOffset = offset;
        return true;
    }

    return false;
}

// Vertex buffer is invalidated outside this function.
angle::Result StateManager11::updateVertexOffsetsForPointSpritesEmulation(
    const gl::Context *context,
    GLint startVertex,
    GLsizei emulatedInstanceId)
{
    size_t reservedBuffers = GetReservedBufferCount(true);
    for (size_t attribIndex = 0; attribIndex < mCurrentAttributes.size(); ++attribIndex)
    {
        const auto &attrib = *mCurrentAttributes[attribIndex];
        size_t bufferIndex = reservedBuffers + attribIndex;

        if (attrib.divisor > 0)
        {
            unsigned int offset = 0;
            ANGLE_TRY(attrib.computeOffset(context, startVertex, &offset));
            offset += (attrib.stride * (emulatedInstanceId / attrib.divisor));
            if (offset != mCurrentVertexOffsets[bufferIndex])
            {
                invalidateInputLayout();
                mDirtyVertexBufferRange.extend(static_cast<unsigned int>(bufferIndex));
                mCurrentVertexOffsets[bufferIndex] = offset;
            }
        }
    }

    applyVertexBufferChanges();
    return angle::Result::Continue;
}

angle::Result StateManager11::generateSwizzle(const gl::Context *context, gl::Texture *texture)
{
    if (!texture)
    {
        return angle::Result::Continue;
    }

    TextureD3D *textureD3D = GetImplAs<TextureD3D>(texture);
    ASSERT(textureD3D);

    TextureStorage *texStorage = nullptr;
    ANGLE_TRY(textureD3D->getNativeTexture(context, &texStorage));

    if (texStorage)
    {
        TextureStorage11 *storage11          = GetAs<TextureStorage11>(texStorage);
        const gl::TextureState &textureState = texture->getTextureState();
        ANGLE_TRY(storage11->generateSwizzles(context, textureState));
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::generateSwizzlesForShader(const gl::Context *context,
                                                        gl::ShaderType type)
{
    const gl::State &glState       = context->getState();
    const gl::RangeUI samplerRange = mExecutableD3D->getUsedSamplerRange(type);

    for (unsigned int i = samplerRange.low(); i < samplerRange.high(); i++)
    {
        gl::TextureType textureType = mExecutableD3D->getSamplerTextureType(type, i);
        GLint textureUnit = mExecutableD3D->getSamplerMapping(type, i, context->getCaps());
        if (textureUnit != -1)
        {
            gl::Texture *texture = glState.getSamplerTexture(textureUnit, textureType);
            ASSERT(texture);
            if (SwizzleRequired(texture->getTextureState()))
            {
                ANGLE_TRY(generateSwizzle(context, texture));
            }
        }
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::generateSwizzles(const gl::Context *context)
{
    ANGLE_TRY(generateSwizzlesForShader(context, gl::ShaderType::Vertex));
    ANGLE_TRY(generateSwizzlesForShader(context, gl::ShaderType::Fragment));
    return angle::Result::Continue;
}

angle::Result StateManager11::applyUniformsForShader(const gl::Context *context,
                                                     gl::ShaderType shaderType)
{
    UniformStorage11 *shaderUniformStorage =
        GetAs<UniformStorage11>(mExecutableD3D->getShaderUniformStorage(shaderType));
    ASSERT(shaderUniformStorage);

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    const d3d11::Buffer *shaderConstantBuffer = nullptr;
    ANGLE_TRY(shaderUniformStorage->getConstantBuffer(context, mRenderer, &shaderConstantBuffer));

    if (shaderUniformStorage->size() > 0 && mExecutableD3D->areShaderUniformsDirty(shaderType))
    {
        UpdateUniformBuffer(deviceContext, shaderUniformStorage, shaderConstantBuffer);
    }

    unsigned int slot = d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DEFAULT_UNIFORM_BLOCK;

    switch (shaderType)
    {
        case gl::ShaderType::Vertex:
            if (mCurrentConstantBufferVS[slot] != shaderConstantBuffer->getSerial())
            {
                deviceContext->VSSetConstantBuffers(slot, 1, shaderConstantBuffer->getPointer());
                mCurrentConstantBufferVS[slot]       = shaderConstantBuffer->getSerial();
                mCurrentConstantBufferVSOffset[slot] = 0;
                mCurrentConstantBufferVSSize[slot]   = 0;
            }
            break;

        case gl::ShaderType::Fragment:
            if (mCurrentConstantBufferPS[slot] != shaderConstantBuffer->getSerial())
            {
                deviceContext->PSSetConstantBuffers(slot, 1, shaderConstantBuffer->getPointer());
                mCurrentConstantBufferPS[slot]       = shaderConstantBuffer->getSerial();
                mCurrentConstantBufferPSOffset[slot] = 0;
                mCurrentConstantBufferPSSize[slot]   = 0;
            }
            break;

        // TODO(jiawei.shao@intel.com): apply geometry shader uniforms
        case gl::ShaderType::Geometry:
            UNIMPLEMENTED();
            break;

        default:
            UNREACHABLE();
            break;
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::applyUniforms(const gl::Context *context)
{
    ANGLE_TRY(applyUniformsForShader(context, gl::ShaderType::Vertex));
    ANGLE_TRY(applyUniformsForShader(context, gl::ShaderType::Fragment));
    if (mExecutableD3D->hasShaderStage(gl::ShaderType::Geometry))
    {
        ANGLE_TRY(applyUniformsForShader(context, gl::ShaderType::Geometry));
    }

    mExecutableD3D->markUniformsClean();

    return angle::Result::Continue;
}

angle::Result StateManager11::applyDriverUniformsForShader(const gl::Context *context,
                                                           gl::ShaderType shaderType)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    d3d11::Buffer &shaderDriverConstantBuffer = mShaderDriverConstantBuffers[shaderType];
    if (!shaderDriverConstantBuffer.valid())
    {
        size_t requiredSize = mShaderConstants.getRequiredBufferSize(shaderType);

        D3D11_BUFFER_DESC constantBufferDescription = {};
        d3d11::InitConstantBufferDesc(&constantBufferDescription, requiredSize);
        ANGLE_TRY(mRenderer->allocateResource(
            GetImplAs<Context11>(context), constantBufferDescription, &shaderDriverConstantBuffer));

        ID3D11Buffer *driverConstants = shaderDriverConstantBuffer.get();
        switch (shaderType)
        {
            case gl::ShaderType::Vertex:
                deviceContext->VSSetConstantBuffers(d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DRIVER, 1,
                                                    &driverConstants);
                break;

            case gl::ShaderType::Fragment:
                deviceContext->PSSetConstantBuffers(d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DRIVER, 1,
                                                    &driverConstants);
                break;

            case gl::ShaderType::Geometry:
                deviceContext->GSSetConstantBuffers(d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DRIVER, 1,
                                                    &driverConstants);
                break;

            default:
                UNREACHABLE();
                return angle::Result::Continue;
        }
    }

    // Sampler metadata and driver constants need to coexist in the same constant buffer to
    // conserve constant buffer slots. We update both in the constant buffer if needed.
    ANGLE_TRY(mShaderConstants.updateBuffer(context, mRenderer, shaderType, *mExecutableD3D,
                                            shaderDriverConstantBuffer));

    return angle::Result::Continue;
}

angle::Result StateManager11::applyDriverUniforms(const gl::Context *context)
{
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    ANGLE_TRY(applyDriverUniformsForShader(context, gl::ShaderType::Vertex));
    ANGLE_TRY(applyDriverUniformsForShader(context, gl::ShaderType::Fragment));
    if (mExecutableD3D->hasShaderStage(gl::ShaderType::Geometry))
    {
        ANGLE_TRY(applyDriverUniformsForShader(context, gl::ShaderType::Geometry));
    }

    // needed for the point sprite geometry shader
    // GSSetConstantBuffers triggers device removal on 9_3, so we should only call it for ES3.
    if (mRenderer->isES3Capable())
    {
        d3d11::Buffer &driverConstantBufferPS =
            mShaderDriverConstantBuffers[gl::ShaderType::Fragment];
        if (mCurrentGeometryConstantBuffer != driverConstantBufferPS.getSerial())
        {
            ASSERT(driverConstantBufferPS.valid());
            deviceContext->GSSetConstantBuffers(0, 1, driverConstantBufferPS.getPointer());
            mCurrentGeometryConstantBuffer = driverConstantBufferPS.getSerial();
        }
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::applyComputeUniforms(const gl::Context *context,
                                                   ProgramExecutableD3D *executableD3D)
{
    UniformStorage11 *computeUniformStorage =
        GetAs<UniformStorage11>(executableD3D->getShaderUniformStorage(gl::ShaderType::Compute));
    ASSERT(computeUniformStorage);

    const d3d11::Buffer *constantBuffer = nullptr;
    ANGLE_TRY(computeUniformStorage->getConstantBuffer(context, mRenderer, &constantBuffer));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    if (computeUniformStorage->size() > 0 &&
        executableD3D->areShaderUniformsDirty(gl::ShaderType::Compute))
    {
        UpdateUniformBuffer(deviceContext, computeUniformStorage, constantBuffer);
        executableD3D->markUniformsClean();
    }

    if (mCurrentComputeConstantBuffer != constantBuffer->getSerial())
    {
        deviceContext->CSSetConstantBuffers(
            d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DEFAULT_UNIFORM_BLOCK, 1,
            constantBuffer->getPointer());
        mCurrentComputeConstantBuffer = constantBuffer->getSerial();
    }

    if (!mShaderDriverConstantBuffers[gl::ShaderType::Compute].valid())
    {
        size_t requiredSize = mShaderConstants.getRequiredBufferSize(gl::ShaderType::Compute);

        D3D11_BUFFER_DESC constantBufferDescription = {};
        d3d11::InitConstantBufferDesc(&constantBufferDescription, requiredSize);
        ANGLE_TRY(
            mRenderer->allocateResource(GetImplAs<Context11>(context), constantBufferDescription,
                                        &mShaderDriverConstantBuffers[gl::ShaderType::Compute]));
        ID3D11Buffer *buffer = mShaderDriverConstantBuffers[gl::ShaderType::Compute].get();
        deviceContext->CSSetConstantBuffers(d3d11::RESERVED_CONSTANT_BUFFER_SLOT_DRIVER, 1,
                                            &buffer);
    }

    ANGLE_TRY(mShaderConstants.updateBuffer(context, mRenderer, gl::ShaderType::Compute,
                                            *executableD3D,
                                            mShaderDriverConstantBuffers[gl::ShaderType::Compute]));

    return angle::Result::Continue;
}

angle::Result StateManager11::syncUniformBuffersForShader(const gl::Context *context,
                                                          gl::ShaderType shaderType)
{
    const auto &glState                  = context->getState();
    ID3D11DeviceContext *deviceContext   = mRenderer->getDeviceContext();
    ID3D11DeviceContext1 *deviceContext1 = mRenderer->getDeviceContext1IfSupported();

    const auto &shaderUniformBuffers = mExecutableD3D->getShaderUniformBufferCache(shaderType);

    for (size_t bufferIndex = 0; bufferIndex < shaderUniformBuffers.size(); ++bufferIndex)
    {
        const D3DUBOCache cache = shaderUniformBuffers[bufferIndex];
        if (cache.binding == -1)
        {
            continue;
        }

        const auto &uniformBuffer          = glState.getIndexedUniformBuffer(cache.binding);
        const GLintptr uniformBufferOffset = uniformBuffer.getOffset();
        const GLsizeiptr uniformBufferSize = uniformBuffer.getSize();

        if (uniformBuffer.get() == nullptr)
        {
            continue;
        }

        Buffer11 *bufferStorage             = GetImplAs<Buffer11>(uniformBuffer.get());
        const d3d11::Buffer *constantBuffer = nullptr;
        UINT firstConstant                  = 0;
        UINT numConstants                   = 0;

        ANGLE_TRY(bufferStorage->getConstantBufferRange(context, uniformBufferOffset,
                                                        uniformBufferSize, &constantBuffer,
                                                        &firstConstant, &numConstants));
        ASSERT(constantBuffer);

        switch (shaderType)
        {
            case gl::ShaderType::Vertex:
            {
                if (mCurrentConstantBufferVS[cache.registerIndex] == constantBuffer->getSerial() &&
                    mCurrentConstantBufferVSOffset[cache.registerIndex] == uniformBufferOffset &&
                    mCurrentConstantBufferVSSize[cache.registerIndex] == uniformBufferSize)
                {
                    continue;
                }

                if (firstConstant != 0 && uniformBufferSize != 0)
                {
                    ASSERT(numConstants != 0);
                    deviceContext1->VSSetConstantBuffers1(cache.registerIndex, 1,
                                                          constantBuffer->getPointer(),
                                                          &firstConstant, &numConstants);
                }
                else
                {
                    deviceContext->VSSetConstantBuffers(cache.registerIndex, 1,
                                                        constantBuffer->getPointer());
                }

                mCurrentConstantBufferVS[cache.registerIndex]       = constantBuffer->getSerial();
                mCurrentConstantBufferVSOffset[cache.registerIndex] = uniformBufferOffset;
                mCurrentConstantBufferVSSize[cache.registerIndex]   = uniformBufferSize;
                break;
            }

            case gl::ShaderType::Fragment:
            {
                if (mCurrentConstantBufferPS[cache.registerIndex] == constantBuffer->getSerial() &&
                    mCurrentConstantBufferPSOffset[cache.registerIndex] == uniformBufferOffset &&
                    mCurrentConstantBufferPSSize[cache.registerIndex] == uniformBufferSize)
                {
                    continue;
                }

                if (firstConstant != 0 && uniformBufferSize != 0)
                {
                    deviceContext1->PSSetConstantBuffers1(cache.registerIndex, 1,
                                                          constantBuffer->getPointer(),
                                                          &firstConstant, &numConstants);
                }
                else
                {
                    deviceContext->PSSetConstantBuffers(cache.registerIndex, 1,
                                                        constantBuffer->getPointer());
                }

                mCurrentConstantBufferPS[cache.registerIndex]       = constantBuffer->getSerial();
                mCurrentConstantBufferPSOffset[cache.registerIndex] = uniformBufferOffset;
                mCurrentConstantBufferPSSize[cache.registerIndex]   = uniformBufferSize;
                break;
            }

            case gl::ShaderType::Compute:
            {
                if (mCurrentConstantBufferCS[bufferIndex] == constantBuffer->getSerial() &&
                    mCurrentConstantBufferCSOffset[bufferIndex] == uniformBufferOffset &&
                    mCurrentConstantBufferCSSize[bufferIndex] == uniformBufferSize)
                {
                    continue;
                }

                if (firstConstant != 0 && uniformBufferSize != 0)
                {
                    deviceContext1->CSSetConstantBuffers1(cache.registerIndex, 1,
                                                          constantBuffer->getPointer(),
                                                          &firstConstant, &numConstants);
                }
                else
                {
                    deviceContext->CSSetConstantBuffers(cache.registerIndex, 1,
                                                        constantBuffer->getPointer());
                }

                mCurrentConstantBufferCS[cache.registerIndex]       = constantBuffer->getSerial();
                mCurrentConstantBufferCSOffset[cache.registerIndex] = uniformBufferOffset;
                mCurrentConstantBufferCSSize[cache.registerIndex]   = uniformBufferSize;
                break;
            }

            // TODO(jiawei.shao@intel.com): update geometry shader uniform buffers.
            case gl::ShaderType::Geometry:
                UNIMPLEMENTED();
                break;

            default:
                UNREACHABLE();
        }
    }

    const auto &shaderUniformBuffersUseSB =
        mExecutableD3D->getShaderUniformBufferCacheUseSB(shaderType);
    for (size_t bufferIndex = 0; bufferIndex < shaderUniformBuffersUseSB.size(); ++bufferIndex)
    {
        const D3DUBOCacheUseSB cache = shaderUniformBuffersUseSB[bufferIndex];
        if (cache.binding == -1)
        {
            continue;
        }

        const auto &uniformBuffer = glState.getIndexedUniformBuffer(cache.binding);
        if (uniformBuffer.get() == nullptr)
        {
            continue;
        }
        const GLintptr uniformBufferOffset = uniformBuffer.getOffset();

        Buffer11 *bufferStorage                    = GetImplAs<Buffer11>(uniformBuffer.get());
        const d3d11::ShaderResourceView *bufferSRV = nullptr;
        ANGLE_TRY(bufferStorage->getStructuredBufferRangeSRV(
            context, static_cast<unsigned int>(uniformBufferOffset), cache.byteWidth,
            cache.structureByteStride, &bufferSRV));

        ASSERT(bufferSRV->valid());
        setShaderResourceInternal(shaderType, cache.registerIndex, bufferSRV);
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::getUAVsForShaderStorageBuffers(const gl::Context *context,
                                                             gl::ShaderType shaderType,
                                                             UAVList *uavList)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    angle::FixedVector<Buffer11 *, gl::IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS>
        previouslyBound;
    for (size_t blockIndex = 0; blockIndex < executable->getShaderStorageBlocks().size();
         blockIndex++)
    {
        GLuint binding = executable->getShaderStorageBlockBinding(static_cast<GLuint>(blockIndex));
        const unsigned int registerIndex = mExecutableD3D->getShaderStorageBufferRegisterIndex(
            static_cast<GLuint>(blockIndex), shaderType);
        // It means this block is active but not statically used.
        if (registerIndex == GL_INVALID_INDEX)
        {
            continue;
        }
        const auto &shaderStorageBuffer = glState.getIndexedShaderStorageBuffer(binding);
        if (shaderStorageBuffer.get() == nullptr)
        {
            // We didn't see a driver error like atomic buffer did. But theoretically, the same
            // thing should be done.
            setUnorderedAccessViewInternal<d3d11::UnorderedAccessView>(registerIndex, nullptr,
                                                                       uavList);
            continue;
        }

        Buffer11 *bufferStorage = GetImplAs<Buffer11>(shaderStorageBuffer.get());
        if (std::find(previouslyBound.begin(), previouslyBound.end(), bufferStorage) !=
            previouslyBound.end())
        {
            // D3D11 doesn't support binding a buffer multiple times
            // http://anglebug.com/42261718
            ERR() << "Writing to multiple blocks on the same buffer is not allowed.";
            return angle::Result::Stop;
        }
        previouslyBound.push_back(bufferStorage);

        d3d11::UnorderedAccessView *uavPtr = nullptr;
        GLsizeiptr viewSize                = 0;
        // Bindings only have a valid size if bound using glBindBufferRange
        if (shaderStorageBuffer.getSize() > 0)
        {
            viewSize = shaderStorageBuffer.getSize();
        }
        // We use the buffer size for glBindBufferBase
        else
        {
            viewSize = bufferStorage->getSize();
        }
        ANGLE_TRY(bufferStorage->getRawUAVRange(context, shaderStorageBuffer.getOffset(), viewSize,
                                                &uavPtr));

        setUnorderedAccessViewInternal(registerIndex, uavPtr, uavList);
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::syncUniformBuffers(const gl::Context *context)
{
    mExecutableD3D->updateUniformBufferCache(context->getCaps());

    if (mExecutableD3D->hasShaderStage(gl::ShaderType::Compute))
    {
        ANGLE_TRY(syncUniformBuffersForShader(context, gl::ShaderType::Compute));
    }
    else
    {
        ANGLE_TRY(syncUniformBuffersForShader(context, gl::ShaderType::Vertex));
        ANGLE_TRY(syncUniformBuffersForShader(context, gl::ShaderType::Fragment));
        if (mExecutableD3D->hasShaderStage(gl::ShaderType::Geometry))
        {
            ANGLE_TRY(syncUniformBuffersForShader(context, gl::ShaderType::Geometry));
        }
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::getUAVsForAtomicCounterBuffers(const gl::Context *context,
                                                             gl::ShaderType shaderType,
                                                             UAVList *uavList)
{
    const gl::State &glState                = context->getState();
    const gl::ProgramExecutable *executable = glState.getProgramExecutable();
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers =
        executable->getAtomicCounterBuffers();
    for (size_t index = 0; index < atomicCounterBuffers.size(); ++index)
    {
        const GLuint binding = executable->getAtomicCounterBufferBinding(index);
        const auto &buffer   = glState.getIndexedAtomicCounterBuffer(binding);
        const unsigned int registerIndex =
            mExecutableD3D->getAtomicCounterBufferRegisterIndex(binding, shaderType);
        ASSERT(registerIndex != GL_INVALID_INDEX);
        if (buffer.get() == nullptr)
        {
            // The atomic counter is used in shader. However, there is no buffer binding to it. We
            // should clear the corresponding UAV in case the previous view type is a texture not a
            // buffer. Otherwise, below error will be reported. The Unordered Access View dimension
            // declared in the shader code (BUFFER) does not match the view type bound to slot 0
            // of the Compute Shader unit (TEXTURE2D).
            setUnorderedAccessViewInternal<d3d11::UnorderedAccessView>(registerIndex, nullptr,
                                                                       uavList);
            continue;
        }

        Buffer11 *bufferStorage = GetImplAs<Buffer11>(buffer.get());
        // TODO(enrico.galli@intel.com): Check to make sure that we aren't binding the same buffer
        // multiple times, as this is unsupported by D3D11. http://anglebug.com/42261818

        // Bindings only have a valid size if bound using glBindBufferRange. Therefore, we use the
        // buffer size for glBindBufferBase
        GLsizeiptr viewSize = (buffer.getSize() > 0) ? buffer.getSize() : bufferStorage->getSize();
        d3d11::UnorderedAccessView *uavPtr = nullptr;
        ANGLE_TRY(bufferStorage->getRawUAVRange(context, buffer.getOffset(), viewSize, &uavPtr));

        setUnorderedAccessViewInternal(registerIndex, uavPtr, uavList);
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::getUAVsForShader(const gl::Context *context,
                                               gl::ShaderType shaderType,
                                               UAVList *uavList)
{
    ANGLE_TRY(getUAVsForShaderStorageBuffers(context, shaderType, uavList));
    ANGLE_TRY(getUAVsForRWImages(context, shaderType, uavList));
    ANGLE_TRY(getUAVsForAtomicCounterBuffers(context, shaderType, uavList));

    return angle::Result::Continue;
}

angle::Result StateManager11::syncUAVsForGraphics(const gl::Context *context)
{
    UAVList uavList(mRenderer->getNativeCaps().maxImageUnits);

    ANGLE_TRY(getUAVsForShader(context, gl::ShaderType::Fragment, &uavList));
    ANGLE_TRY(getUAVsForShader(context, gl::ShaderType::Vertex, &uavList));

    if (uavList.highestUsed >= 0)
    {
        ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
        UINT baseUAVRegister = static_cast<UINT>(mExecutableD3D->getPixelShaderKey().size());
        deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, baseUAVRegister,
            uavList.highestUsed + 1, uavList.data.data(), nullptr);
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::syncUAVsForCompute(const gl::Context *context)
{
    UAVList uavList(mRenderer->getNativeCaps().maxImageUnits);

    ANGLE_TRY(getUAVsForShader(context, gl::ShaderType::Compute, &uavList));

    if (uavList.highestUsed >= 0)
    {
        ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
        deviceContext->CSSetUnorderedAccessViews(0, uavList.highestUsed + 1, uavList.data.data(),
                                                 nullptr);
    }

    return angle::Result::Continue;
}

angle::Result StateManager11::syncTransformFeedbackBuffers(const gl::Context *context)
{
    const auto &glState = context->getState();

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // If transform feedback is not active, unbind all buffers
    if (!glState.isTransformFeedbackActiveUnpaused())
    {
        if (mAppliedTFSerial != mEmptySerial)
        {
            deviceContext->SOSetTargets(0, nullptr, nullptr);
            mAppliedTFSerial = mEmptySerial;
        }
        return angle::Result::Continue;
    }

    gl::TransformFeedback *transformFeedback = glState.getCurrentTransformFeedback();
    TransformFeedback11 *tf11                = GetImplAs<TransformFeedback11>(transformFeedback);
    if (mAppliedTFSerial == tf11->getSerial() && !tf11->isDirty())
    {
        return angle::Result::Continue;
    }

    const std::vector<ID3D11Buffer *> *soBuffers = nullptr;
    ANGLE_TRY(tf11->getSOBuffers(context, &soBuffers));
    const std::vector<UINT> &soOffsets = tf11->getSOBufferOffsets();

    deviceContext->SOSetTargets(tf11->getNumSOBuffers(), soBuffers->data(), soOffsets.data());

    mAppliedTFSerial = tf11->getSerial();
    tf11->onApply();

    return angle::Result::Continue;
}

void StateManager11::syncPrimitiveTopology(const gl::State &glState,
                                           gl::PrimitiveMode currentDrawMode)
{
    D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    // Don't cull everything by default, this also resets if we were previously culling
    mCullEverything = false;

    switch (currentDrawMode)
    {
        case gl::PrimitiveMode::Points:
        {
            bool usesPointSize = mExecutableD3D->usesPointSize();

            // ProgramBinary assumes non-point rendering if gl_PointSize isn't written,
            // which affects varying interpolation. Since the value of gl_PointSize is
            // undefined when not written, just skip drawing to avoid unexpected results.
            if (!usesPointSize && !glState.isTransformFeedbackActiveUnpaused())
            {
                // Notify developers of risking undefined behavior.
                WARN() << "Point rendering without writing to gl_PointSize.";
                mCullEverything = true;
                return;
            }

            primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        }
        case gl::PrimitiveMode::Lines:
            primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case gl::PrimitiveMode::LineLoop:
            primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case gl::PrimitiveMode::LineStrip:
            primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case gl::PrimitiveMode::Triangles:
            primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            mCullEverything   = CullsEverything(glState);
            break;
        case gl::PrimitiveMode::TriangleStrip:
            primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            mCullEverything   = CullsEverything(glState);
            break;
        // emulate fans via rewriting index buffer
        case gl::PrimitiveMode::TriangleFan:
            primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            mCullEverything   = CullsEverything(glState);
            break;
        default:
            UNREACHABLE();
            break;
    }

    setPrimitiveTopologyInternal(primitiveTopology);
}

}  // namespace rx
