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

#ifndef SRC_DAWN_NATIVE_TEXTURE_H_
#define SRC_DAWN_NATIVE_TEXTURE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/LRUCache.h"
#include "dawn/common/RefCountedWithExternalCount.h"
#include "dawn/common/WeakRef.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/BlockInfo.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/DeviceGuard.h"
#include "dawn/native/Error.h"
#include "dawn/native/Format.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedTextureMemory.h"
#include "dawn/native/Subresource.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ref.h"

namespace dawn::native {

class MemoryDump;
class ResourceTableBase;

enum class AllowMultiPlanarTextureFormat {
    No,
    SingleLayerOnly,
    Yes,
};

MaybeError ValidateTextureDescriptor(
    const DeviceBase* device,
    const UnpackedPtr<TextureDescriptor>& descriptor,
    AllowMultiPlanarTextureFormat allowMultiPlanar = AllowMultiPlanarTextureFormat::No,
    std::optional<wgpu::TextureUsage> allowedSharedTextureMemoryUsage = std::nullopt);
MaybeError ValidateTextureViewDescriptor(const DeviceBase* device,
                                         const TextureBase* texture,
                                         const UnpackedPtr<TextureViewDescriptor>& descriptor);
ResultOrError<TextureViewDescriptor> GetTextureViewDescriptorWithDefaults(
    const TextureBase* texture,
    const TextureViewDescriptor* descriptor);

bool IsValidSampleCount(uint32_t sampleCount);

// Computes a swizzle which, when applied, is equivalent to applying `firstSwizzle`
// then `secondSwizzle`, like the order of WGSL swizzles (`value.rgba.rgba`).
wgpu::TextureComponentSwizzle ComposeSwizzle(wgpu::TextureComponentSwizzle firstSwizzle,
                                             wgpu::TextureComponentSwizzle secondSwizzle);

// The default swizzle as defined by the WebGPU specification.
inline constexpr wgpu::TextureComponentSwizzle kRGBASwizzle = {
    wgpu::ComponentSwizzle::R,
    wgpu::ComponentSwizzle::G,
    wgpu::ComponentSwizzle::B,
    wgpu::ComponentSwizzle::A,
};

// The swizzle typically used for depth and stencil textures.
inline constexpr wgpu::TextureComponentSwizzle kR001Swizzle = {
    wgpu::ComponentSwizzle::R,
    wgpu::ComponentSwizzle::Zero,
    wgpu::ComponentSwizzle::Zero,
    wgpu::ComponentSwizzle::One,
};

inline constexpr wgpu::TextureUsage kReadOnlyTextureUsages =
    wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding | kReadOnlyRenderAttachment |
    kReadOnlyStorageTexture;

// Valid texture usages for a resolve texture that are loaded from at the beginning of a render
// pass.
inline constexpr wgpu::TextureUsage kResolveTextureLoadAndStoreUsages =
    kResolveAttachmentLoadingUsage | wgpu::TextureUsage::RenderAttachment;

inline constexpr wgpu::TextureUsage kShaderTextureUsages =
    wgpu::TextureUsage::TextureBinding | kReadOnlyStorageTexture |
    wgpu::TextureUsage::StorageBinding | kWriteOnlyStorageTexture;

// Usages that are used to validate operations that act on texture views.
inline constexpr wgpu::TextureUsage kTextureViewOnlyUsages =
    kShaderTextureUsages | kResolveTextureLoadAndStoreUsages |
    wgpu::TextureUsage::TransientAttachment | wgpu::TextureUsage::StorageAttachment;

// A flattened version of TextureViewDescriptor used to query the texture view cache.
struct TextureViewQuery {
    explicit TextureViewQuery(const UnpackedPtr<TextureViewDescriptor>& desc);
    bool operator==(const TextureViewQuery& b) const = default;

    // TextureViewDescriptor fields (label ignored)
    wgpu::TextureFormat format;
    wgpu::TextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    wgpu::TextureAspect aspect;
    wgpu::TextureUsage usage;

    // Update with fields from relevant chained structs as they are added.
    wgpu::ComponentSwizzle swizzleRed = wgpu::ComponentSwizzle::R;
    wgpu::ComponentSwizzle swizzleGreen = wgpu::ComponentSwizzle::G;
    wgpu::ComponentSwizzle swizzleBlue = wgpu::ComponentSwizzle::B;
    wgpu::ComponentSwizzle swizzleAlpha = wgpu::ComponentSwizzle::A;

    // Doesn't contain Vulkan YCbCr members as these views skip the cache.
};

static const size_t kDefaultTextureViewCacheCapacity = 4;
struct TextureViewCacheFuncs {
    size_t operator()(const TextureViewQuery& desc) const;
    bool operator()(const TextureViewQuery& a, const TextureViewQuery& b) const;
};

class TextureBase : public RefCountedWithExternalCount<SharedResource> {
  public:
    enum class ClearValue { Zero, NonZero };

    static Ref<TextureBase> MakeError(DeviceBase* device, const TextureDescriptor* descriptor);

    ObjectType GetType() const override;
    void FormatLabel(absl::FormatSink* s) const override;

    wgpu::TextureDimension GetDimension() const;
    wgpu::TextureViewDimension GetCompatibilityTextureBindingViewDimension() const;
    const Format& GetFormat() const;
    const FormatSet& GetViewFormats() const;

    // For multiplanar textures, base size is the size of plane 0. For other types of textures,
    // base size is the original size passed via TextureDescriptor.
    const Extent3D& GetBaseSize() const;
    Extent3D GetSize(Aspect aspect) const;
    Extent3D GetSize(wgpu::TextureAspect aspect) const;
    uint32_t GetWidth(Aspect aspect) const;
    uint32_t GetHeight(Aspect aspect) const;
    uint32_t GetDepth(Aspect aspect) const;
    uint32_t GetArrayLayers() const;
    uint32_t GetNumMipLevels() const;
    SubresourceRange GetAllSubresources() const;
    uint32_t GetSampleCount() const;
    uint32_t GetSubresourceCount() const;
    bool HasPinnedUsage() const;
    wgpu::TextureUsage GetPinnedUsage() const;

    // |GetUsage| returns the usage with which the texture was created using the base WebGPU
    // API. The dawn-internal-usages extension may add additional usages. |GetInternalUsage|
    // returns the union of base usage and the usages added by the extension.
    wgpu::TextureUsage GetUsage() const;
    wgpu::TextureUsage GetInternalUsage() const;

    // SharedResource implementation
    ExecutionSerial OnEndAccess() override;
    void OnBeginAccess() override;
    bool HasAccess() const override;
    bool IsDestroyed() const override;
    bool IsInitialized() const override;
    void SetInitialized(bool initialized) override;

    bool IsReadOnly() const;
    uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t arraySlice, Aspect aspect) const;
    bool IsSubresourceContentInitialized(const SubresourceRange& range) const;
    void SetIsSubresourceContentInitialized(bool isInitialized, const SubresourceRange& range);

    MaybeError ValidateCanUseInSubmitNow() const;

    bool IsMultisampledTexture() const;

    // Returns true if the size covers the whole subresource.
    bool CoversFullSubresource(uint32_t mipLevel, Aspect aspect, const TexelExtent3D& size) const;

    // For a texture with non-block-compressed texture format, its physical size is always equal
    // to its virtual size. For a texture with block compressed texture format, the physical
    // size is the one with paddings if necessary, which is always a multiple of the block size
    // and used in texture copying. The virtual size is the one without paddings, which is not
    // required to be a multiple of the block size and used in texture sampling.
    // TODO(crbug.com/424536624): Return BlockExtent3D for these functions.
    Extent3D GetMipLevelSingleSubresourcePhysicalSize(uint32_t level, Aspect aspect) const;
    Extent3D GetMipLevelSingleSubresourceVirtualSize(uint32_t level, Aspect aspect) const;
    // TODO(https://issues.chromium.org/424536624): Return TexelExtent3D and take typed
    // origin/extent.
    Extent3D ClampToMipLevelVirtualSize(uint32_t level,
                                        Aspect aspect,
                                        const Origin3D& origin,
                                        const Extent3D& extent) const;
    // For 2d-array textures, these 2 functions keeps the array layers in contrast to
    // GetMipLevelSingleSubresource(Physical/Virtual)Size.
    // TODO(https://issues.chromium.org/424536624): Return TexelExtent3D.
    Extent3D GetMipLevelSubresourcePhysicalSize(uint32_t level, Aspect aspect) const;
    Extent3D GetMipLevelSubresourceVirtualSize(uint32_t level, Aspect aspect) const;

    MaybeError Pin(wgpu::TextureUsage usage);
    void Unpin();
    void AddResourceTableSlotUse(ResourceTableBase* table, ResourceTableSlot slot);
    void RemoveResourceTableSlotUse(ResourceTableBase* table, ResourceTableSlot slot);

    ResultOrError<Ref<TextureViewBase>> CreateView(
        const TextureViewDescriptor* descriptor = nullptr);
    Ref<TextureViewBase> CreateErrorView(const TextureViewDescriptor* descriptor = nullptr);
    ApiObjectList* GetViewTrackingList();

    void DumpMemoryStatistics(dawn::native::MemoryDump* dump, const char* prefix) const;

    uint64_t ComputeEstimatedByteSize() const;

    template <typename CreateFn>
    ResultOrError<Ref<TextureViewBase>> GetOrCreateViewFromCache(
        const UnpackedPtr<TextureViewDescriptor>& desc,
        CreateFn createFn);

    // Dawn API
    TextureViewBase* APICreateView(const TextureViewDescriptor* descriptor = nullptr);
    TextureViewBase* APICreateErrorView(const TextureViewDescriptor* descriptor = nullptr);
    void APIDestroy();
    void APIPin(wgpu::TextureUsage usages);
    void APIUnpin();
    uint32_t APIGetWidth() const;
    uint32_t APIGetHeight() const;
    uint32_t APIGetDepthOrArrayLayers() const;
    uint32_t APIGetMipLevelCount() const;
    uint32_t APIGetSampleCount() const;
    wgpu::TextureDimension APIGetDimension() const;
    wgpu::TextureFormat APIGetFormat() const;
    wgpu::TextureUsage APIGetUsage() const;
    wgpu::TextureViewDimension APIGetTextureBindingViewDimension() const;
    void APISetOwnershipForMemoryDump(uint64_t ownerGuid);

  protected:
    TextureBase(DeviceBase* device, const UnpackedPtr<TextureDescriptor>& descriptor);
    ~TextureBase() override;

    void DestroyImpl(DestroyReason reason) override;
    void AddInternalUsage(wgpu::TextureUsage usage);
    void SetSharedResourceMemoryContentsForTesting(Ref<SharedResourceMemoryContents> contents);

    ExecutionSerial mLastSharedTextureMemoryUsageSerial{kBeginningOfGPUTime};

    virtual MaybeError PinImpl(wgpu::TextureUsage usage);
    virtual void UnpinImpl();

  private:
    struct TextureState {
        TextureState();

        // Indicates whether the texture may access by the GPU in a queue submit.
        bool hasAccess : 1;
        // Indicates whether the texture has been destroyed.
        bool destroyed : 1;
    };

    TextureBase(DeviceBase* device, const TextureDescriptor* descriptor, ObjectBase::ErrorTag tag);

    std::string GetSizeLabel() const;

    ResultOrError<Ref<TextureViewBase>> GetOrCreateDefaultView();

    MaybeError ValidatePin(wgpu::TextureUsage usages) const;
    MaybeError ValidateUnpin() const;

    void WillAddFirstExternalRef() override;
    void WillDropLastExternalRef() override;

    // TODO(crbug.com/481211676): Remove this once all backends' DestroyImpl methods are
    // thread-safe.
    virtual std::optional<DeviceGuard> UseDeviceGuardForDestroy();

    wgpu::TextureDimension mDimension;
    // Only used for compatibility mode
    wgpu::TextureViewDimension mCompatibilityTextureBindingViewDimension =
        wgpu::TextureViewDimension::Undefined;
    const raw_ref<const Format> mFormat;
    FormatSet mViewFormats;
    Extent3D mBaseSize;
    uint32_t mMipLevelCount;
    uint32_t mSampleCount;
    wgpu::TextureUsage mUsage = wgpu::TextureUsage::None;
    wgpu::TextureUsage mInternalUsage = wgpu::TextureUsage::None;
    wgpu::TextureUsage mPinnedUsage = wgpu::TextureUsage::None;  // None if not pinned.
    TextureState mState;
    wgpu::TextureFormat mFormatEnumForReflection;

    Ref<TextureViewBase> mDefaultView;
    // Textures track texture views created from them so that they can be destroyed when the texture
    // is destroyed.
    ApiObjectList mTextureViews;

    using TextureViewCache =
        LRUCache<TextureViewQuery, Ref<TextureViewBase>, TextureViewCacheFuncs>;
    std::unique_ptr<TextureViewCache> mTextureViewCache;

    // Keep a hash set of the places this texture is bound to in ResourceTables.
    struct ResourceTableSlotUse {
        WeakRef<ResourceTableBase> table;
        ResourceTableSlot slot;

        struct HashFuncs {
            size_t operator()(const ResourceTableSlotUse& query) const;
            bool operator()(const ResourceTableSlotUse& a, const ResourceTableSlotUse& b) const;
        };
    };
    absl::flat_hash_set<ResourceTableSlotUse,
                        ResourceTableSlotUse::HashFuncs,
                        ResourceTableSlotUse::HashFuncs>
        mResourceTableSlotUses;

    // TODO(crbug.com/dawn/845): Use a more optimized data structure to save space
    std::vector<bool> mIsSubresourceContentInitializedAtIndex;

    // Owner GUID for memory dump tracking, non-zero GUID will be reported via
    // MemoryDump::AddOwnerGUID, and 0 indicates no external ownership for this texture.
    uint64_t mOwnerGUIDForMemoryDump = 0u;
};

template <typename CreateFn>
ResultOrError<Ref<TextureViewBase>> TextureBase::GetOrCreateViewFromCache(
    const UnpackedPtr<TextureViewDescriptor>& desc,
    CreateFn createFn) {
    TextureViewQuery query(desc);

    // Skip the cache when not present, but also for Vulkan YCbCr textures that don't have all their
    // info unpacked in the TextureViewQuery.
    if (!mTextureViewCache || desc.Has<YCbCrVkDescriptor>()) {
        return createFn(query);
    }

    return mTextureViewCache->GetOrCreate(query, createFn);
}

class TextureViewBase : public ApiObjectBase {
  public:
    TextureViewBase(TextureBase* texture, const UnpackedPtr<TextureViewDescriptor>& descriptor);
    ~TextureViewBase() override;

    static Ref<TextureViewBase> MakeError(DeviceBase* device, StringView label = nullptr);

    ObjectType GetType() const override;
    void FormatLabel(absl::FormatSink* s) const override;

    const TextureBase* GetTexture() const;
    TextureBase* GetTexture();

    Aspect GetAspects() const;
    const Format& GetFormat() const;
    wgpu::TextureViewDimension GetDimension() const;
    uint32_t GetBaseMipLevel() const;
    uint32_t GetLevelCount() const;
    uint32_t GetBaseArrayLayer() const;
    uint32_t GetLayerCount() const;
    const SubresourceRange& GetSubresourceRange() const;

    // Returns the size of the texture's subresource at this view's base mip level and aspect.
    Extent3D GetSingleSubresourceVirtualSize() const;

    // |GetUsage| returns the usage with which the texture view was created using the base WebGPU
    // API. The dawn-internal-usages extension may add additional usages. |GetInternalUsage|
    // returns the union of base usage and the usages added by the extension.
    wgpu::TextureUsage GetUsage() const;
    wgpu::TextureUsage GetInternalUsage() const;

    wgpu::TextureComponentSwizzle GetSwizzle() const;
    bool IsSwizzleIdentity() const;

    bool IsYCbCr() const;
    // Valid to call only if `IsYCbCr()` is true.
    bool HasYCbCrDescriptor() const;
    virtual bool IsYCbCrFilterable() const;

  protected:
    void DestroyImpl(DestroyReason reason) override;

  private:
    TextureViewBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    ApiObjectList* GetObjectTrackingList() override;

    Ref<TextureBase> mTexture;

    const raw_ref<const Format> mFormat;
    wgpu::TextureViewDimension mDimension;
    SubresourceRange mRange;
    const wgpu::TextureUsage mUsage = wgpu::TextureUsage::None;
    const wgpu::TextureUsage mInternalUsage = wgpu::TextureUsage::None;
    wgpu::ComponentSwizzle mSwizzleRed = wgpu::ComponentSwizzle::R;
    wgpu::ComponentSwizzle mSwizzleGreen = wgpu::ComponentSwizzle::G;
    wgpu::ComponentSwizzle mSwizzleBlue = wgpu::ComponentSwizzle::B;
    wgpu::ComponentSwizzle mSwizzleAlpha = wgpu::ComponentSwizzle::A;
    bool mIsSwizzleIdentity = false;
    bool mHasYCbCrDescriptor = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TEXTURE_H_
