//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourceManager11:
//   Centralized point of allocation for all D3D11 Resources.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_RESOURCEFACTORY11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_RESOURCEFACTORY11_H_

#include <array>
#include <atomic>
#include <memory>

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/serial_utils.h"

namespace rx
{
// These two methods are declared here to prevent circular includes.
namespace d3d11
{
HRESULT SetDebugName(ID3D11DeviceChild *resource,
                     const char *internalName,
                     const std::string *khrDebugName);

template <typename T>
HRESULT SetDebugName(angle::ComPtr<T> &resource,
                     const char *internalName,
                     const std::string *khrDebugName)
{
    return SetDebugName(resource.Get(), internalName, khrDebugName);
}
}  // namespace d3d11

namespace d3d
{
class Context;
}  // namespace d3d

class Renderer11;
class ResourceManager11;
template <typename T>
class SharedResource11;
class TextureHelper11;

using InputElementArray = WrappedArray<D3D11_INPUT_ELEMENT_DESC>;
using ShaderData        = WrappedArray<uint8_t>;

// Format: ResourceType, D3D11 type, DESC type, init data type.
#define ANGLE_RESOURCE_TYPE_OP(NAME, OP)                                                       \
    OP(NAME, BlendState, ID3D11BlendState, D3D11_BLEND_DESC, void)                             \
    OP(NAME, Buffer, ID3D11Buffer, D3D11_BUFFER_DESC, const D3D11_SUBRESOURCE_DATA)            \
    OP(NAME, ComputeShader, ID3D11ComputeShader, ShaderData, void)                             \
    OP(NAME, DepthStencilState, ID3D11DepthStencilState, D3D11_DEPTH_STENCIL_DESC, void)       \
    OP(NAME, DepthStencilView, ID3D11DepthStencilView, D3D11_DEPTH_STENCIL_VIEW_DESC,          \
       ID3D11Resource)                                                                         \
    OP(NAME, GeometryShader, ID3D11GeometryShader, ShaderData,                                 \
       const std::vector<D3D11_SO_DECLARATION_ENTRY>)                                          \
    OP(NAME, InputLayout, ID3D11InputLayout, InputElementArray, const ShaderData)              \
    OP(NAME, PixelShader, ID3D11PixelShader, ShaderData, void)                                 \
    OP(NAME, Query, ID3D11Query, D3D11_QUERY_DESC, void)                                       \
    OP(NAME, RasterizerState, ID3D11RasterizerState, D3D11_RASTERIZER_DESC, void)              \
    OP(NAME, RenderTargetView, ID3D11RenderTargetView, D3D11_RENDER_TARGET_VIEW_DESC,          \
       ID3D11Resource)                                                                         \
    OP(NAME, SamplerState, ID3D11SamplerState, D3D11_SAMPLER_DESC, void)                       \
    OP(NAME, ShaderResourceView, ID3D11ShaderResourceView, D3D11_SHADER_RESOURCE_VIEW_DESC,    \
       ID3D11Resource)                                                                         \
    OP(NAME, UnorderedAccessView, ID3D11UnorderedAccessView, D3D11_UNORDERED_ACCESS_VIEW_DESC, \
       ID3D11Resource)                                                                         \
    OP(NAME, Texture2D, ID3D11Texture2D, D3D11_TEXTURE2D_DESC, const D3D11_SUBRESOURCE_DATA)   \
    OP(NAME, Texture3D, ID3D11Texture3D, D3D11_TEXTURE3D_DESC, const D3D11_SUBRESOURCE_DATA)   \
    OP(NAME, VertexShader, ID3D11VertexShader, ShaderData, void)

#define ANGLE_RESOURCE_TYPE_LIST(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) RESTYPE,

enum class ResourceType
{
    ANGLE_RESOURCE_TYPE_OP(List, ANGLE_RESOURCE_TYPE_LIST) Last
};

#undef ANGLE_RESOURCE_TYPE_LIST

constexpr size_t ResourceTypeIndex(ResourceType resourceType)
{
    return static_cast<size_t>(resourceType);
}

constexpr size_t NumResourceTypes = ResourceTypeIndex(ResourceType::Last);

#define ANGLE_RESOURCE_TYPE_TO_D3D11(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) \
                                                                                       \
    template <>                                                                        \
    struct NAME<ResourceType::RESTYPE>                                                 \
    {                                                                                  \
        using Value = D3D11TYPE;                                                       \
    };

#define ANGLE_RESOURCE_TYPE_TO_DESC(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) \
                                                                                      \
    template <>                                                                       \
    struct NAME<ResourceType::RESTYPE>                                                \
    {                                                                                 \
        using Value = DESCTYPE;                                                       \
    };

#define ANGLE_RESOURCE_TYPE_TO_INIT_DATA(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) \
                                                                                           \
    template <>                                                                            \
    struct NAME<ResourceType::RESTYPE>                                                     \
    {                                                                                      \
        using Value = INITDATATYPE;                                                        \
    };

#define ANGLE_RESOURCE_TYPE_TO_TYPE(NAME, OP) \
    template <ResourceType Param>             \
    struct NAME;                              \
    ANGLE_RESOURCE_TYPE_OP(NAME, OP)          \
                                              \
    template <ResourceType Param>             \
    struct NAME                               \
    {};                                       \
                                              \
    template <ResourceType Param>             \
    using Get##NAME = typename NAME<Param>::Value;

ANGLE_RESOURCE_TYPE_TO_TYPE(D3D11Type, ANGLE_RESOURCE_TYPE_TO_D3D11)
ANGLE_RESOURCE_TYPE_TO_TYPE(DescType, ANGLE_RESOURCE_TYPE_TO_DESC)
ANGLE_RESOURCE_TYPE_TO_TYPE(InitDataType, ANGLE_RESOURCE_TYPE_TO_INIT_DATA)

#undef ANGLE_RESOURCE_TYPE_TO_D3D11
#undef ANGLE_RESOURCE_TYPE_TO_DESC
#undef ANGLE_RESOURCE_TYPE_TO_INIT_DATA
#undef ANGLE_RESOURCE_TYPE_TO_TYPE

#define ANGLE_TYPE_TO_RESOURCE_TYPE(NAME, OP) \
    template <typename Param>                 \
    struct NAME;                              \
    ANGLE_RESOURCE_TYPE_OP(NAME, OP)          \
                                              \
    template <typename Param>                 \
    struct NAME                               \
    {};                                       \
                                              \
    template <typename Param>                 \
    constexpr ResourceType Get##NAME()        \
    {                                         \
        return NAME<Param>::Value;            \
    }

#define ANGLE_D3D11_TO_RESOURCE_TYPE(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) \
                                                                                       \
    template <>                                                                        \
    struct NAME<D3D11TYPE>                                                             \
    {                                                                                  \
        static constexpr ResourceType Value = ResourceType::RESTYPE;                   \
    };

ANGLE_TYPE_TO_RESOURCE_TYPE(ResourceTypeFromD3D11, ANGLE_D3D11_TO_RESOURCE_TYPE)

#undef ANGLE_D3D11_TO_RESOURCE_TYPE
#undef ANGLE_TYPE_TO_RESOURCE_TYPE

template <typename T>
using GetDescFromD3D11 = GetDescType<ResourceTypeFromD3D11<T>::Value>;

template <typename T>
using GetInitDataFromD3D11 = GetInitDataType<ResourceTypeFromD3D11<T>::Value>;

template <typename T>
constexpr size_t ResourceTypeIndex()
{
    return static_cast<size_t>(GetResourceTypeFromD3D11<T>());
}

template <typename T>
struct TypedData
{
    TypedData() {}
    ~TypedData();

    T *object                  = nullptr;
    ResourceManager11 *manager = nullptr;
};

// Smart pointer type. Wraps the resource and a factory for safe deletion.
template <typename T, template <class> class Pointer, typename DataT>
class Resource11Base : angle::NonCopyable
{
  public:
    T *get() const { return mData->object; }
    T *const *getPointer() const { return &mData->object; }

    void setInternalName(const char *name)
    {
        mInternalDebugName = name;
        UpdateDebugNameWithD3D();
    }

    void setKHRDebugLabel(const std::string *label)
    {
        mKhrDebugName = label;
        UpdateDebugNameWithD3D();
    }

    void setLabels(const char *name, const std::string *label)
    {
        mInternalDebugName = name;
        mKhrDebugName      = label;
        UpdateDebugNameWithD3D();
    }

    void set(T *object)
    {
        ASSERT(!valid());
        mData->object = object;
    }

    bool valid() const { return (mData->object != nullptr); }

    void reset()
    {
        if (valid())
            mData.reset(new DataT());
    }

    ResourceSerial getSerial() const
    {
        return ResourceSerial(reinterpret_cast<uintptr_t>(mData->object));
    }

  protected:
    friend class TextureHelper11;

    Resource11Base() : mData(new DataT()) {}

    Resource11Base(Resource11Base &&movedObj) : mData(new DataT())
    {
        std::swap(mData, movedObj.mData);
    }

    virtual ~Resource11Base() { mData.reset(); }

    Resource11Base &operator=(Resource11Base &&movedObj)
    {
        std::swap(mData, movedObj.mData);
        return *this;
    }

    Pointer<DataT> mData;

  private:
    void UpdateDebugNameWithD3D()
    {
        d3d11::SetDebugName(mData->object, mInternalDebugName, mKhrDebugName);
    }

    const std::string *mKhrDebugName = nullptr;
    const char *mInternalDebugName   = nullptr;
};

template <typename T>
using UniquePtr = typename std::unique_ptr<T, std::default_delete<T>>;

template <typename ResourceT>
class Resource11 : public Resource11Base<ResourceT, UniquePtr, TypedData<ResourceT>>
{
  public:
    Resource11() {}
    Resource11(Resource11 &&other)
        : Resource11Base<ResourceT, UniquePtr, TypedData<ResourceT>>(std::move(other))
    {}
    Resource11 &operator=(Resource11 &&other)
    {
        std::swap(this->mData, other.mData);
        return *this;
    }

  private:
    template <typename T>
    friend class SharedResource11;
    friend class ResourceManager11;

    Resource11(ResourceT *object, ResourceManager11 *manager)
    {
        this->mData->object  = object;
        this->mData->manager = manager;
    }
};

template <typename T>
class SharedResource11 : public Resource11Base<T, std::shared_ptr, TypedData<T>>
{
  public:
    SharedResource11() {}
    SharedResource11(SharedResource11 &&movedObj)
        : Resource11Base<T, std::shared_ptr, TypedData<T>>(std::move(movedObj))
    {}

    SharedResource11 &operator=(SharedResource11 &&other)
    {
        std::swap(this->mData, other.mData);
        return *this;
    }

    SharedResource11 makeCopy() const
    {
        SharedResource11 copy;
        copy.mData = this->mData;
        return std::move(copy);
    }

  private:
    friend class ResourceManager11;
    SharedResource11(Resource11<T> &&obj) : Resource11Base<T, std::shared_ptr, TypedData<T>>()
    {
        std::swap(this->mData->manager, obj.mData->manager);

        // Can't use std::swap because of ID3D11Resource.
        auto temp           = this->mData->object;
        this->mData->object = obj.mData->object;
        obj.mData->object   = static_cast<T *>(temp);
    }
};

class ResourceManager11 final : angle::NonCopyable
{
  public:
    ResourceManager11();
    ~ResourceManager11();

    template <typename T>
    angle::Result allocate(d3d::Context *context,
                           Renderer11 *renderer,
                           const GetDescFromD3D11<T> *desc,
                           GetInitDataFromD3D11<T> *initData,
                           Resource11<T> *resourceOut);

    template <typename T>
    angle::Result allocate(d3d::Context *context,
                           Renderer11 *renderer,
                           const GetDescFromD3D11<T> *desc,
                           GetInitDataFromD3D11<T> *initData,
                           SharedResource11<T> *sharedRes)
    {
        Resource11<T> res;
        ANGLE_TRY(allocate(context, renderer, desc, initData, &res));
        *sharedRes = std::move(res);
        return angle::Result::Continue;
    }

    template <typename T>
    void onRelease(T *resource)
    {
        onReleaseGeneric(GetResourceTypeFromD3D11<T>(), resource);
    }

    void onReleaseGeneric(ResourceType resourceType, ID3D11DeviceChild *resource);

    void setAllocationsInitialized(bool initialize);

  private:
    void incrResource(ResourceType resourceType, uint64_t memorySize);
    void decrResource(ResourceType resourceType, uint64_t memorySize);

    template <typename T>
    GetInitDataFromD3D11<T> *createInitDataIfNeeded(const GetDescFromD3D11<T> *desc);

    bool mInitializeAllocations;

    std::array<std::atomic_size_t, NumResourceTypes> mAllocatedResourceCounts;
    std::array<std::atomic_uint64_t, NumResourceTypes> mAllocatedResourceDeviceMemory;
    angle::MemoryBuffer mZeroMemory;

    std::vector<D3D11_SUBRESOURCE_DATA> mShadowInitData;
};

template <typename ResourceT>
TypedData<ResourceT>::~TypedData()
{
    if (object)
    {
        // We can have a nullptr factory when holding passed-in resources.
        if (manager)
        {
            manager->onRelease(object);
        }
        object->Release();
    }
}

#define ANGLE_RESOURCE_TYPE_CLASS(NAME, RESTYPE, D3D11TYPE, DESCTYPE, INITDATATYPE) \
    using RESTYPE = Resource11<D3D11TYPE>;

namespace d3d11
{
ANGLE_RESOURCE_TYPE_OP(ClassList, ANGLE_RESOURCE_TYPE_CLASS)

using SharedSRV = SharedResource11<ID3D11ShaderResourceView>;
using SharedUAV = SharedResource11<ID3D11UnorderedAccessView>;
}  // namespace d3d11

#undef ANGLE_RESOURCE_TYPE_CLASS

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_RESOURCEFACTORY11_H_
