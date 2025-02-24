//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourceManager11:
//   Centralized point of allocation for all D3D11 Resources.

#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"

#include "common/debug.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"

namespace rx
{

namespace
{

constexpr uint8_t kDebugInitTextureDataValue = 0x48;
constexpr FLOAT kDebugColorInitClearValue[4] = {0.3f, 0.5f, 0.7f, 0.5f};
constexpr FLOAT kDebugDepthInitValue         = 0.2f;
constexpr UINT8 kDebugStencilInitValue       = 3;

// A hard limit on buffer size. This works around a problem in the NVIDIA drivers where buffer sizes
// close to MAX_UINT would give undefined results. The limit of MAX_UINT/2 should be generous enough
// for almost any demanding application.
constexpr UINT kMaximumBufferSizeHardLimit = std::numeric_limits<UINT>::max() >> 1;

uint64_t ComputeMippedMemoryUsage(unsigned int width,
                                  unsigned int height,
                                  unsigned int depth,
                                  uint64_t pixelSize,
                                  unsigned int mipLevels)
{
    uint64_t sizeSum = 0;

    for (unsigned int level = 0; level < mipLevels; ++level)
    {
        unsigned int mipWidth  = std::max(width >> level, 1u);
        unsigned int mipHeight = std::max(height >> level, 1u);
        unsigned int mipDepth  = std::max(depth >> level, 1u);
        sizeSum += static_cast<uint64_t>(mipWidth * mipHeight * mipDepth) * pixelSize;
    }

    return sizeSum;
}

uint64_t ComputeMemoryUsage(const D3D11_TEXTURE2D_DESC *desc)
{
    ASSERT(desc);
    uint64_t pixelBytes =
        static_cast<uint64_t>(d3d11::GetDXGIFormatSizeInfo(desc->Format).pixelBytes);
    return ComputeMippedMemoryUsage(desc->Width, desc->Height, 1, pixelBytes, desc->MipLevels);
}

uint64_t ComputeMemoryUsage(const D3D11_TEXTURE3D_DESC *desc)
{
    ASSERT(desc);
    uint64_t pixelBytes =
        static_cast<uint64_t>(d3d11::GetDXGIFormatSizeInfo(desc->Format).pixelBytes);
    return ComputeMippedMemoryUsage(desc->Width, desc->Height, desc->Depth, pixelBytes,
                                    desc->MipLevels);
}

uint64_t ComputeMemoryUsage(const D3D11_BUFFER_DESC *desc)
{
    ASSERT(desc);
    return static_cast<uint64_t>(desc->ByteWidth);
}

template <typename T>
uint64_t ComputeMemoryUsage(const T *desc)
{
    return 0;
}

template <ResourceType ResourceT>
uint64_t ComputeGenericMemoryUsage(ID3D11DeviceChild *genericResource)
{
    auto *typedResource = static_cast<GetD3D11Type<ResourceT> *>(genericResource);
    GetDescType<ResourceT> desc;
    typedResource->GetDesc(&desc);
    return ComputeMemoryUsage(&desc);
}

uint64_t ComputeGenericMemoryUsage(ResourceType resourceType, ID3D11DeviceChild *resource)
{
    switch (resourceType)
    {
        case ResourceType::Texture2D:
            return ComputeGenericMemoryUsage<ResourceType::Texture2D>(resource);
        case ResourceType::Texture3D:
            return ComputeGenericMemoryUsage<ResourceType::Texture3D>(resource);
        case ResourceType::Buffer:
            return ComputeGenericMemoryUsage<ResourceType::Buffer>(resource);

        default:
            return 0;
    }
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_BLEND_DESC *desc,
                       void * /*initData*/,
                       ID3D11BlendState **blendState)
{
    return device->CreateBlendState(desc, blendState);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_BUFFER_DESC *desc,
                       const D3D11_SUBRESOURCE_DATA *initData,
                       ID3D11Buffer **buffer)
{
    // Force buffers to be limited to a fixed max size.
    if (desc->ByteWidth > kMaximumBufferSizeHardLimit)
    {
        return E_OUTOFMEMORY;
    }

    return device->CreateBuffer(desc, initData, buffer);
}

HRESULT CreateResource(ID3D11Device *device,
                       const ShaderData *desc,
                       void * /*initData*/,
                       ID3D11ComputeShader **resourceOut)
{
    return device->CreateComputeShader(desc->get(), desc->size(), nullptr, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_DEPTH_STENCIL_DESC *desc,
                       void * /*initData*/,
                       ID3D11DepthStencilState **resourceOut)
{
    return device->CreateDepthStencilState(desc, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_DEPTH_STENCIL_VIEW_DESC *desc,
                       ID3D11Resource *resource,
                       ID3D11DepthStencilView **resourceOut)
{
    return device->CreateDepthStencilView(resource, desc, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const ShaderData *desc,
                       const std::vector<D3D11_SO_DECLARATION_ENTRY> *initData,
                       ID3D11GeometryShader **resourceOut)
{
    if (initData)
    {
        return device->CreateGeometryShaderWithStreamOutput(
            desc->get(), desc->size(), initData->data(), static_cast<UINT>(initData->size()),
            nullptr, 0, 0, nullptr, resourceOut);
    }
    else
    {
        return device->CreateGeometryShader(desc->get(), desc->size(), nullptr, resourceOut);
    }
}

HRESULT CreateResource(ID3D11Device *device,
                       const InputElementArray *desc,
                       const ShaderData *initData,
                       ID3D11InputLayout **resourceOut)
{
    return device->CreateInputLayout(desc->get(), static_cast<UINT>(desc->size()), initData->get(),
                                     initData->size(), resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const ShaderData *desc,
                       void * /*initData*/,
                       ID3D11PixelShader **resourceOut)
{
    return device->CreatePixelShader(desc->get(), desc->size(), nullptr, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_QUERY_DESC *desc,
                       void * /*initData*/,
                       ID3D11Query **resourceOut)
{
    return device->CreateQuery(desc, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_RASTERIZER_DESC *desc,
                       void * /*initData*/,
                       ID3D11RasterizerState **rasterizerState)
{
    return device->CreateRasterizerState(desc, rasterizerState);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_RENDER_TARGET_VIEW_DESC *desc,
                       ID3D11Resource *resource,
                       ID3D11RenderTargetView **renderTargetView)
{
    return device->CreateRenderTargetView(resource, desc, renderTargetView);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_SAMPLER_DESC *desc,
                       void * /*initData*/,
                       ID3D11SamplerState **resourceOut)
{
    return device->CreateSamplerState(desc, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_SHADER_RESOURCE_VIEW_DESC *desc,
                       ID3D11Resource *resource,
                       ID3D11ShaderResourceView **resourceOut)
{
    return device->CreateShaderResourceView(resource, desc, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_UNORDERED_ACCESS_VIEW_DESC *desc,
                       ID3D11Resource *resource,
                       ID3D11UnorderedAccessView **resourceOut)
{
    return device->CreateUnorderedAccessView(resource, desc, resourceOut);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_TEXTURE2D_DESC *desc,
                       const D3D11_SUBRESOURCE_DATA *initData,
                       ID3D11Texture2D **texture)
{
    return device->CreateTexture2D(desc, initData, texture);
}

HRESULT CreateResource(ID3D11Device *device,
                       const D3D11_TEXTURE3D_DESC *desc,
                       const D3D11_SUBRESOURCE_DATA *initData,
                       ID3D11Texture3D **texture)
{
    return device->CreateTexture3D(desc, initData, texture);
}

HRESULT CreateResource(ID3D11Device *device,
                       const ShaderData *desc,
                       void * /*initData*/,
                       ID3D11VertexShader **resourceOut)
{
    return device->CreateVertexShader(desc->get(), desc->size(), nullptr, resourceOut);
}

DXGI_FORMAT GetTypedDepthStencilFormat(DXGI_FORMAT dxgiFormat)
{
    switch (dxgiFormat)
    {
        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_D16_UNORM;
        case DXGI_FORMAT_R24G8_TYPELESS:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_D32_FLOAT;
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return dxgiFormat;
    }
}

DXGI_FORMAT GetTypedColorFormatForClearing(DXGI_FORMAT dxgiFormat)
{
    switch (dxgiFormat)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case DXGI_FORMAT_R32G32B32_TYPELESS:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case DXGI_FORMAT_R32G32_TYPELESS:
            return DXGI_FORMAT_R32G32_FLOAT;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_R16G16_TYPELESS:
            return DXGI_FORMAT_R16G16_FLOAT;
        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_R32_FLOAT;
        case DXGI_FORMAT_R8G8_TYPELESS:
            return DXGI_FORMAT_R8G8_UNORM;
        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_R16_FLOAT;
        case DXGI_FORMAT_R8_TYPELESS:
            return DXGI_FORMAT_R8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            return DXGI_FORMAT_B8G8R8X8_UNORM;
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            UNREACHABLE();
            [[fallthrough]];
        default:
            return dxgiFormat;
    }
}

template <typename DescT, typename ResourceT>
angle::Result ClearResource(d3d::Context *context,
                            Renderer11 *renderer,
                            const DescT *desc,
                            ResourceT *texture)
{
    // No-op.
    return angle::Result::Continue;
}

template <>
angle::Result ClearResource(d3d::Context *context,
                            Renderer11 *renderer,
                            const D3D11_TEXTURE2D_DESC *desc,
                            ID3D11Texture2D *texture)
{
    ID3D11DeviceContext *deviceContext = renderer->getDeviceContext();

    if ((desc->BindFlags & D3D11_BIND_DEPTH_STENCIL) != 0)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags  = 0;
        dsvDesc.Format = GetTypedDepthStencilFormat(desc->Format);

        const auto &format = d3d11_angle::GetFormat(dsvDesc.Format);
        UINT clearFlags    = (format.depthBits > 0 ? D3D11_CLEAR_DEPTH : 0) |
                          (format.stencilBits > 0 ? D3D11_CLEAR_STENCIL : 0);

        // Must process each mip level individually.
        for (UINT mipLevel = 0; mipLevel < desc->MipLevels; ++mipLevel)
        {
            if (desc->SampleDesc.Count == 0)
            {
                dsvDesc.Texture2D.MipSlice = mipLevel;
                dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
            }
            else
            {
                dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            }

            d3d11::DepthStencilView dsv;
            ANGLE_TRY(renderer->allocateResource(context, dsvDesc, texture, &dsv));

            deviceContext->ClearDepthStencilView(dsv.get(), clearFlags, kDebugDepthInitValue,
                                                 kDebugStencilInitValue);
        }
    }
    else
    {
        ASSERT((desc->BindFlags & D3D11_BIND_RENDER_TARGET) != 0);
        DXGI_FORMAT formatForClearing = GetTypedColorFormatForClearing(desc->Format);
        if (formatForClearing != desc->Format || desc->MipLevels > 1)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = formatForClearing;
            if (desc->SampleDesc.Count <= 1)
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            }
            else
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
                ASSERT(desc->MipLevels == 1);
            }
            for (UINT mipLevel = 0; mipLevel < desc->MipLevels; ++mipLevel)
            {
                d3d11::RenderTargetView rtv;
                if (rtvDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D)
                {
                    rtvDesc.Texture2D.MipSlice = mipLevel;
                }
                ANGLE_TRY(renderer->allocateResource(context, rtvDesc, texture, &rtv));
                deviceContext->ClearRenderTargetView(rtv.get(), kDebugColorInitClearValue);
            }
        }
        else
        {
            d3d11::RenderTargetView rtv;
            ANGLE_TRY(renderer->allocateResourceNoDesc(context, texture, &rtv));
            deviceContext->ClearRenderTargetView(rtv.get(), kDebugColorInitClearValue);
        }
    }

    return angle::Result::Continue;
}

template <>
angle::Result ClearResource(d3d::Context *context,
                            Renderer11 *renderer,
                            const D3D11_TEXTURE3D_DESC *desc,
                            ID3D11Texture3D *texture)
{
    ID3D11DeviceContext *deviceContext = renderer->getDeviceContext();

    ASSERT((desc->BindFlags & D3D11_BIND_DEPTH_STENCIL) == 0);
    ASSERT((desc->BindFlags & D3D11_BIND_RENDER_TARGET) != 0);

    DXGI_FORMAT formatForClearing = GetTypedColorFormatForClearing(desc->Format);
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format          = formatForClearing;
    rtvDesc.ViewDimension   = D3D11_RTV_DIMENSION_TEXTURE3D;
    rtvDesc.Texture3D.WSize = 1;
    UINT depth              = desc->Depth;
    for (UINT mipLevel = 0; mipLevel < desc->MipLevels; ++mipLevel)
    {
        rtvDesc.Texture3D.MipSlice = mipLevel;
        for (UINT w = 0; w < depth; ++w)
        {
            rtvDesc.Texture3D.FirstWSlice = w;
            d3d11::RenderTargetView rtv;
            ANGLE_TRY(renderer->allocateResource(context, rtvDesc, texture, &rtv));
            deviceContext->ClearRenderTargetView(rtv.get(), kDebugColorInitClearValue);
        }
        depth /= 2;
    }

    return angle::Result::Continue;
}

#define ANGLE_RESOURCE_STRINGIFY_OP(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) \
    "Error allocating " #RESTYPE,

constexpr std::array<const char *, NumResourceTypes> kResourceTypeErrors = {
    {ANGLE_RESOURCE_TYPE_OP(Stringify, ANGLE_RESOURCE_STRINGIFY_OP)}};
static_assert(kResourceTypeErrors[NumResourceTypes - 1] != nullptr,
              "All members must be initialized.");

}  // anonymous namespace

// ResourceManager11 Implementation.
ResourceManager11::ResourceManager11() : mInitializeAllocations(false)
{
    for (auto &count : mAllocatedResourceCounts)
    {
        count = 0;
    }
    for (auto &memorySize : mAllocatedResourceDeviceMemory)
    {
        memorySize = 0;
    }
}

ResourceManager11::~ResourceManager11()
{
    for (size_t count : mAllocatedResourceCounts)
    {
        ASSERT(count == 0);
    }

    for (uint64_t memorySize : mAllocatedResourceDeviceMemory)
    {
        ASSERT(memorySize == 0);
    }
}

template <typename T>
angle::Result ResourceManager11::allocate(d3d::Context *context,
                                          Renderer11 *renderer,
                                          const GetDescFromD3D11<T> *desc,
                                          GetInitDataFromD3D11<T> *initData,
                                          Resource11<T> *resourceOut)
{
    ID3D11Device *device = renderer->getDevice();
    T *resource          = nullptr;

    GetInitDataFromD3D11<T> *shadowInitData = initData;
    if (!shadowInitData && mInitializeAllocations)
    {
        shadowInitData = createInitDataIfNeeded<T>(desc);
    }

    HRESULT hr = CreateResource(device, desc, shadowInitData, &resource);
    ANGLE_TRY_HR(context, hr, kResourceTypeErrors[ResourceTypeIndex<T>()]);

    if (!shadowInitData && mInitializeAllocations)
    {
        ANGLE_TRY(ClearResource(context, renderer, desc, resource));
    }

    ASSERT(resource);
    incrResource(GetResourceTypeFromD3D11<T>(), ComputeMemoryUsage(desc));
    *resourceOut = std::move(Resource11<T>(resource, this));
    return angle::Result::Continue;
}

void ResourceManager11::incrResource(ResourceType resourceType, uint64_t memorySize)
{
    size_t typeIndex = ResourceTypeIndex(resourceType);

    mAllocatedResourceCounts[typeIndex]++;
    mAllocatedResourceDeviceMemory[typeIndex] += memorySize;

    // This checks for integer overflow.
    ASSERT(mAllocatedResourceCounts[typeIndex] > 0);
    ASSERT(mAllocatedResourceDeviceMemory[typeIndex] >= memorySize);
}

void ResourceManager11::decrResource(ResourceType resourceType, uint64_t memorySize)
{
    size_t typeIndex = ResourceTypeIndex(resourceType);

    ASSERT(mAllocatedResourceCounts[typeIndex] > 0);
    mAllocatedResourceCounts[typeIndex]--;
    ASSERT(mAllocatedResourceDeviceMemory[typeIndex] >= memorySize);
    mAllocatedResourceDeviceMemory[typeIndex] -= memorySize;
}

void ResourceManager11::onReleaseGeneric(ResourceType resourceType, ID3D11DeviceChild *resource)
{
    ASSERT(resource);
    decrResource(resourceType, ComputeGenericMemoryUsage(resourceType, resource));
}

template <>
const D3D11_SUBRESOURCE_DATA *ResourceManager11::createInitDataIfNeeded<ID3D11Texture2D>(
    const D3D11_TEXTURE2D_DESC *desc)
{
    ASSERT(desc);

    if ((desc->BindFlags & (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_RENDER_TARGET)) != 0)
    {
        // This will be done using ClearView methods.
        return nullptr;
    }

    size_t requiredSize = static_cast<size_t>(ComputeMemoryUsage(desc));
    if (mZeroMemory.size() < requiredSize)
    {
        if (!mZeroMemory.resize(requiredSize))
        {
            ERR() << "Failed to allocate D3D texture initialization data.";
            return nullptr;
        }
        mZeroMemory.fill(kDebugInitTextureDataValue);
    }

    const auto &formatSizeInfo = d3d11::GetDXGIFormatSizeInfo(desc->Format);

    UINT subresourceCount = desc->MipLevels * desc->ArraySize;
    if (mShadowInitData.size() < subresourceCount)
    {
        mShadowInitData.resize(subresourceCount);
    }

    for (UINT mipLevel = 0; mipLevel < desc->MipLevels; ++mipLevel)
    {
        for (UINT arrayIndex = 0; arrayIndex < desc->ArraySize; ++arrayIndex)
        {
            UINT subresourceIndex = D3D11CalcSubresource(mipLevel, arrayIndex, desc->MipLevels);
            D3D11_SUBRESOURCE_DATA *data = &mShadowInitData[subresourceIndex];

            UINT levelWidth  = std::max(desc->Width >> mipLevel, 1u);
            UINT levelHeight = std::max(desc->Height >> mipLevel, 1u);

            data->SysMemPitch      = levelWidth * formatSizeInfo.pixelBytes;
            data->SysMemSlicePitch = data->SysMemPitch * levelHeight;
            data->pSysMem          = mZeroMemory.data();
        }
    }

    return mShadowInitData.data();
}

template <>
const D3D11_SUBRESOURCE_DATA *ResourceManager11::createInitDataIfNeeded<ID3D11Texture3D>(
    const D3D11_TEXTURE3D_DESC *desc)
{
    ASSERT(desc);

    if ((desc->BindFlags & D3D11_BIND_RENDER_TARGET) != 0)
    {
        // This will be done using ClearView methods.
        return nullptr;
    }

    size_t requiredSize = static_cast<size_t>(ComputeMemoryUsage(desc));
    if (mZeroMemory.size() < requiredSize)
    {
        if (!mZeroMemory.resize(requiredSize))
        {
            ERR() << "Failed to allocate D3D texture initialization data.";
            return nullptr;
        }
        mZeroMemory.fill(kDebugInitTextureDataValue);
    }

    const auto &formatSizeInfo = d3d11::GetDXGIFormatSizeInfo(desc->Format);

    UINT subresourceCount = desc->MipLevels;
    if (mShadowInitData.size() < subresourceCount)
    {
        mShadowInitData.resize(subresourceCount);
    }

    for (UINT mipLevel = 0; mipLevel < desc->MipLevels; ++mipLevel)
    {
        UINT subresourceIndex        = D3D11CalcSubresource(mipLevel, 0, desc->MipLevels);
        D3D11_SUBRESOURCE_DATA *data = &mShadowInitData[subresourceIndex];

        UINT levelWidth  = std::max(desc->Width >> mipLevel, 1u);
        UINT levelHeight = std::max(desc->Height >> mipLevel, 1u);

        data->SysMemPitch      = levelWidth * formatSizeInfo.pixelBytes;
        data->SysMemSlicePitch = data->SysMemPitch * levelHeight;
        data->pSysMem          = mZeroMemory.data();
    }

    return mShadowInitData.data();
}

template <typename T>
GetInitDataFromD3D11<T> *ResourceManager11::createInitDataIfNeeded(const GetDescFromD3D11<T> *desc)
{
    // No-op.
    return nullptr;
}

void ResourceManager11::setAllocationsInitialized(bool initialize)
{
    mInitializeAllocations = initialize;
}

#define ANGLE_INSTANTIATE_OP(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) \
                                                                               \
    template angle::Result ResourceManager11::allocate(                        \
        d3d::Context *, Renderer11 *, const DESCTYPE *, INITDATATYPE *, Resource11<D3D11TYPE> *);

ANGLE_RESOURCE_TYPE_OP(Instantitate, ANGLE_INSTANTIATE_OP)
}  // namespace rx
