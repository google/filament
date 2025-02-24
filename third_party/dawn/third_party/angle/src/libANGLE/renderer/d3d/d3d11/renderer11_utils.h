//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer11_utils.h: Conversion functions and other utility routines
// specific to the D3D11 renderer.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_RENDERER11_UTILS_H_
#define LIBANGLE_RENDERER_D3D_D3D11_RENDERER11_UTILS_H_

#include <array>
#include <functional>
#include <vector>

#include "common/Color.h"

#include "libANGLE/Caps.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

namespace gl
{
class FramebufferAttachment;
}

namespace rx
{
class Context11;
class Renderer11;
class RenderTarget11;
struct Renderer11DeviceCaps;

using RTVArray = std::array<ID3D11RenderTargetView *, gl::IMPLEMENTATION_MAX_DRAW_BUFFERS>;

namespace gl_d3d11
{

D3D11_BLEND ConvertBlendFunc(gl::BlendFactorType glBlend, bool isAlpha);
D3D11_BLEND_OP ConvertBlendOp(gl::BlendEquationType glBlendOp);
UINT8 ConvertColorMask(bool maskRed, bool maskGreen, bool maskBlue, bool maskAlpha);

D3D11_CULL_MODE ConvertCullMode(bool cullEnabled, gl::CullFaceMode cullMode);

D3D11_COMPARISON_FUNC ConvertComparison(GLenum comparison);
D3D11_DEPTH_WRITE_MASK ConvertDepthMask(bool depthWriteEnabled);
UINT8 ConvertStencilMask(GLuint stencilmask);
D3D11_STENCIL_OP ConvertStencilOp(GLenum stencilOp);

D3D11_FILTER ConvertFilter(GLenum minFilter,
                           GLenum magFilter,
                           float maxAnisotropy,
                           GLenum comparisonMode);
D3D11_TEXTURE_ADDRESS_MODE ConvertTextureWrap(GLenum wrap);
UINT ConvertMaxAnisotropy(float maxAnisotropy, D3D_FEATURE_LEVEL featureLevel);

D3D11_QUERY ConvertQueryType(gl::QueryType type);

UINT8 GetColorMask(const gl::InternalFormat &formatInfo);

}  // namespace gl_d3d11

namespace d3d11_gl
{

unsigned int GetReservedVertexUniformVectors(D3D_FEATURE_LEVEL featureLevel);

unsigned int GetReservedFragmentUniformVectors(D3D_FEATURE_LEVEL featureLevel);

gl::Version GetMaximumClientVersion(const Renderer11DeviceCaps &caps);
void GenerateCaps(ID3D11Device *device,
                  ID3D11DeviceContext *deviceContext,
                  const Renderer11DeviceCaps &renderer11DeviceCaps,
                  const angle::FeaturesD3D &features,
                  const char *description,
                  gl::Caps *caps,
                  gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions,
                  gl::Limitations *limitations,
                  ShPixelLocalStorageOptions *);

D3D_FEATURE_LEVEL GetMinimumFeatureLevelForES31();

}  // namespace d3d11_gl

namespace d3d11
{

enum ANGLED3D11DeviceType
{
    ANGLE_D3D11_DEVICE_TYPE_UNKNOWN,
    ANGLE_D3D11_DEVICE_TYPE_HARDWARE,
    ANGLE_D3D11_DEVICE_TYPE_SOFTWARE_REF_OR_NULL,
    ANGLE_D3D11_DEVICE_TYPE_WARP,
};

ANGLED3D11DeviceType GetDeviceType(ID3D11Device *device);

void MakeValidSize(bool isImage,
                   DXGI_FORMAT format,
                   GLsizei *requestWidth,
                   GLsizei *requestHeight,
                   int *levelOffset);

angle::Result GenerateInitialTextureData(
    const gl::Context *context,
    GLint internalFormat,
    const Renderer11DeviceCaps &renderer11DeviceCaps,
    GLuint width,
    GLuint height,
    GLuint depth,
    GLuint mipLevels,
    gl::TexLevelArray<D3D11_SUBRESOURCE_DATA> *outSubresourceData);

UINT GetPrimitiveRestartIndex();

struct PositionTexCoordVertex
{
    float x, y;
    float u, v;
};
void SetPositionTexCoordVertex(PositionTexCoordVertex *vertex, float x, float y, float u, float v);

struct PositionLayerTexCoord3DVertex
{
    float x, y;
    unsigned int l;
    float u, v, s;
};
void SetPositionLayerTexCoord3DVertex(PositionLayerTexCoord3DVertex *vertex,
                                      float x,
                                      float y,
                                      unsigned int layer,
                                      float u,
                                      float v,
                                      float s);

struct PositionVertex
{
    float x, y, z, w;
};

struct BlendStateKey final
{
    // This will zero-initialize the struct, including padding.
    BlendStateKey();
    BlendStateKey(const BlendStateKey &other);

    gl::BlendStateExt blendStateExt;

    // Use two 16-bit ints to round the struct nicely.
    uint16_t rtvMax;
    uint16_t sampleAlphaToCoverage;
};

bool operator==(const BlendStateKey &a, const BlendStateKey &b);
bool operator!=(const BlendStateKey &a, const BlendStateKey &b);

struct RasterizerStateKey final
{
    // This will zero-initialize the struct, including padding.
    RasterizerStateKey();

    gl::RasterizerState rasterizerState;

    // Use a 32-bit int to round the struct nicely.
    uint32_t scissorEnabled;
};

bool operator==(const RasterizerStateKey &a, const RasterizerStateKey &b);
bool operator!=(const RasterizerStateKey &a, const RasterizerStateKey &b);

template <typename outType>
outType *DynamicCastComObject(IUnknown *object)
{
    outType *outObject = nullptr;
    HRESULT result =
        object->QueryInterface(__uuidof(outType), reinterpret_cast<void **>(&outObject));
    if (SUCCEEDED(result))
    {
        return outObject;
    }
    else
    {
        SafeRelease(outObject);
        return nullptr;
    }
}

inline bool isDeviceLostError(HRESULT errorCode)
{
    switch (errorCode)
    {
        case DXGI_ERROR_DEVICE_HUNG:
        case DXGI_ERROR_DEVICE_REMOVED:
        case DXGI_ERROR_DEVICE_RESET:
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
        case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
            return true;
        default:
            return false;
    }
}

template <ResourceType ResourceT>
class LazyResource : angle::NonCopyable
{
  public:
    constexpr LazyResource() : mResource() {}
    virtual ~LazyResource() {}

    virtual angle::Result resolve(d3d::Context *context, Renderer11 *renderer) = 0;
    void reset() { mResource.reset(); }
    GetD3D11Type<ResourceT> *get() const
    {
        ASSERT(mResource.valid());
        return mResource.get();
    }

    const Resource11<GetD3D11Type<ResourceT>> &getObj() const { return mResource; }

  protected:
    LazyResource(LazyResource &&other) : mResource(std::move(other.mResource)) {}

    // Specialized in the cpp file to avoid MSVS/Clang specific code.
    angle::Result resolveImpl(d3d::Context *context,
                              Renderer11 *renderer,
                              const GetDescType<ResourceT> &desc,
                              GetInitDataType<ResourceT> *initData,
                              const char *name);

    Resource11<GetD3D11Type<ResourceT>> mResource;
};

template <typename D3D11ShaderType>
class LazyShader final : public LazyResource<GetResourceTypeFromD3D11<D3D11ShaderType>()>
{
  public:
    // All parameters must be constexpr. Not supported in VS2013.
    constexpr LazyShader(const BYTE *byteCode, size_t byteCodeSize, const char *name)
        : mByteCode(byteCode, byteCodeSize), mName(name)
    {}

    constexpr LazyShader(LazyShader &&shader)
        : LazyResource<GetResourceTypeFromD3D11<D3D11ShaderType>()>(std::move(shader)),
          mByteCode(std::move(shader.mByteCode)),
          mName(shader.mName)
    {}

    angle::Result resolve(d3d::Context *context, Renderer11 *renderer) override
    {
        return this->resolveImpl(context, renderer, mByteCode, nullptr, mName);
    }

  private:
    ShaderData mByteCode;
    const char *mName;
};

class LazyInputLayout final : public LazyResource<ResourceType::InputLayout>
{
  public:
    LazyInputLayout(const D3D11_INPUT_ELEMENT_DESC *inputDesc,
                    size_t inputDescLen,
                    const BYTE *byteCode,
                    size_t byteCodeLen,
                    const char *debugName);
    ~LazyInputLayout() override;

    angle::Result resolve(d3d::Context *context, Renderer11 *renderer) override;

  private:
    InputElementArray mInputDesc;
    ShaderData mByteCode;
    const char *mDebugName;
};

class LazyBlendState final : public LazyResource<ResourceType::BlendState>
{
  public:
    LazyBlendState(const D3D11_BLEND_DESC &desc, const char *debugName);

    angle::Result resolve(d3d::Context *context, Renderer11 *renderer) override;

  private:
    D3D11_BLEND_DESC mDesc;
    const char *mDebugName;
};

// Copy data to small D3D11 buffers, such as for small constant buffers, which use one struct to
// represent an entire buffer.
template <class T>
void SetBufferData(ID3D11DeviceContext *context, ID3D11Buffer *constantBuffer, const T &value)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource = {};
    HRESULT result = context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    ASSERT(SUCCEEDED(result));
    if (SUCCEEDED(result))
    {
        memcpy(mappedResource.pData, &value, sizeof(T));
        context->Unmap(constantBuffer, 0);
    }
}

void InitializeFeatures(const Renderer11DeviceCaps &deviceCaps,
                        const DXGI_ADAPTER_DESC &adapterDesc,
                        angle::FeaturesD3D *features);

void InitializeFrontendFeatures(const DXGI_ADAPTER_DESC &adapterDesc,
                                angle::FrontendFeatures *features);

enum ReservedConstantBufferSlot
{
    RESERVED_CONSTANT_BUFFER_SLOT_DEFAULT_UNIFORM_BLOCK = 0,
    RESERVED_CONSTANT_BUFFER_SLOT_DRIVER                = 1,

    RESERVED_CONSTANT_BUFFER_SLOT_COUNT = 2
};

void InitConstantBufferDesc(D3D11_BUFFER_DESC *constantBufferDescription, size_t byteWidth);

// Helper class for RAII patterning.
template <typename T>
class [[nodiscard]] ScopedUnmapper final : angle::NonCopyable
{
  public:
    ScopedUnmapper(T *object) : mObject(object) {}
    ~ScopedUnmapper() { mObject->unmap(); }

  private:
    T *mObject;
};
}  // namespace d3d11

struct GenericData
{
    GenericData() {}
    ~GenericData()
    {
        if (object)
        {
            // We can have a nullptr factory when holding passed-in resources.
            if (manager)
            {
                manager->onReleaseGeneric(resourceType, object);
                manager = nullptr;
            }
            object->Release();
            object = nullptr;
        }
    }

    ResourceType resourceType  = ResourceType::Last;
    ID3D11Resource *object     = nullptr;
    ResourceManager11 *manager = nullptr;
};

// A helper class which wraps a 2D or 3D texture.
class TextureHelper11 : public Resource11Base<ID3D11Resource, std::shared_ptr, GenericData>
{
  public:
    TextureHelper11();
    TextureHelper11(TextureHelper11 &&other);
    TextureHelper11(const TextureHelper11 &other);
    ~TextureHelper11() override;
    TextureHelper11 &operator=(TextureHelper11 &&other);
    TextureHelper11 &operator=(const TextureHelper11 &other);

    bool isBuffer() const { return mData->resourceType == ResourceType::Buffer; }
    bool is2D() const { return mData->resourceType == ResourceType::Texture2D; }
    bool is3D() const { return mData->resourceType == ResourceType::Texture3D; }
    ResourceType getTextureType() const { return mData->resourceType; }
    gl::Extents getExtents() const { return mExtents; }
    DXGI_FORMAT getFormat() const { return mFormatSet->texFormat; }
    const d3d11::Format &getFormatSet() const { return *mFormatSet; }
    int getSampleCount() const { return mSampleCount; }

    template <typename DescT, typename ResourceT>
    void init(Resource11<ResourceT> &&texture, const DescT &desc, const d3d11::Format &format)
    {
        std::swap(mData->manager, texture.mData->manager);

        // Can't use std::swap because texture is typed, and here we use ID3D11Resource.
        ID3D11Resource *temp  = mData->object;
        mData->object         = texture.mData->object;
        texture.mData->object = static_cast<ResourceT *>(temp);

        mFormatSet = &format;
        initDesc(desc);
    }

    template <typename ResourceT>
    void set(ResourceT *object, const d3d11::Format &format)
    {
        ASSERT(!valid());

        mFormatSet     = &format;
        mData->object  = object;
        mData->manager = nullptr;

        GetDescFromD3D11<ResourceT> desc;
        getDesc(&desc);
        initDesc(desc);
    }

    bool operator==(const TextureHelper11 &other) const;
    bool operator!=(const TextureHelper11 &other) const;

    void getDesc(D3D11_TEXTURE2D_DESC *desc) const;
    void getDesc(D3D11_TEXTURE3D_DESC *desc) const;
    void getDesc(D3D11_BUFFER_DESC *desc) const;

  private:
    void initDesc(const D3D11_TEXTURE2D_DESC &desc2D);
    void initDesc(const D3D11_TEXTURE3D_DESC &desc3D);
    void initDesc(const D3D11_BUFFER_DESC &descBuffer);

    const d3d11::Format *mFormatSet;
    gl::Extents mExtents;
    int mSampleCount;
};

enum class StagingAccess
{
    READ,
    READ_WRITE,
};

bool UsePresentPathFast(const Renderer11 *renderer, const gl::FramebufferAttachment *colorbuffer);
bool UsePrimitiveRestartWorkaround(bool primitiveRestartFixedIndexEnabled,
                                   gl::DrawElementsType type);

enum class IndexStorageType
{
    // Dynamic indexes are re-streamed every frame. They come from a client data pointer or
    // from buffers that are updated frequently.
    Dynamic,

    // Static indexes are translated from the original storage once, and re-used multiple times.
    Static,

    // Direct indexes are never transated and are used directly from the source buffer. They are
    // the fastest available path.
    Direct,

    // Not a real storage type.
    Invalid,
};

IndexStorageType ClassifyIndexStorage(const gl::State &glState,
                                      const gl::Buffer *elementArrayBuffer,
                                      gl::DrawElementsType elementType,
                                      gl::DrawElementsType destElementType,
                                      unsigned int offset);

bool SwizzleRequired(const gl::TextureState &textureState);
gl::SwizzleState GetEffectiveSwizzle(const gl::TextureState &textureState);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_RENDERER11_UTILS_H_
