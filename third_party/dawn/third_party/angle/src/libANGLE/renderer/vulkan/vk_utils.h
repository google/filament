//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Renderer.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_UTILS_H_

#include <atomic>
#include <limits>
#include <queue>

#include "GLSLANG/ShaderLang.h"
#include "common/FixedVector.h"
#include "common/Optional.h"
#include "common/PackedEnums.h"
#include "common/SimpleMutex.h"
#include "common/WorkerThread.h"
#include "common/backtrace_utils.h"
#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/Observer.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/serial_utils.h"
#include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"
#include "libANGLE/renderer/vulkan/SecondaryCommandPool.h"
#include "libANGLE/renderer/vulkan/VulkanSecondaryCommandBuffer.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"
#include "platform/autogen/FeaturesVk_autogen.h"
#include "vulkan/vulkan_fuchsia_ext.h"

#define ANGLE_GL_OBJECTS_X(PROC) \
    PROC(Buffer)                 \
    PROC(Context)                \
    PROC(Framebuffer)            \
    PROC(MemoryObject)           \
    PROC(Overlay)                \
    PROC(Program)                \
    PROC(ProgramExecutable)      \
    PROC(ProgramPipeline)        \
    PROC(Query)                  \
    PROC(Renderbuffer)           \
    PROC(Sampler)                \
    PROC(Semaphore)              \
    PROC(Texture)                \
    PROC(TransformFeedback)      \
    PROC(VertexArray)

#define ANGLE_PRE_DECLARE_OBJECT(OBJ) class OBJ;

namespace egl
{
class Display;
class Image;
class ShareGroup;
}  // namespace egl

namespace gl
{
class MockOverlay;
class ProgramExecutable;
struct RasterizerState;
struct SwizzleState;
struct VertexAttribute;
class VertexBinding;

ANGLE_GL_OBJECTS_X(ANGLE_PRE_DECLARE_OBJECT)
}  // namespace gl

#define ANGLE_PRE_DECLARE_VK_OBJECT(OBJ) class OBJ##Vk;

namespace rx
{
class DisplayVk;
class ImageVk;
class ProgramExecutableVk;
class RenderbufferVk;
class RenderTargetVk;
class RenderPassCache;
class ShareGroupVk;
}  // namespace rx

namespace angle
{
egl::Error ToEGL(Result result, EGLint errorCode);
}  // namespace angle

namespace rx
{
ANGLE_GL_OBJECTS_X(ANGLE_PRE_DECLARE_VK_OBJECT)

const char *VulkanResultString(VkResult result);

constexpr size_t kMaxVulkanLayers = 20;
using VulkanLayerVector           = angle::FixedVector<const char *, kMaxVulkanLayers>;

// Verify that validation layers are available.
bool GetAvailableValidationLayers(const std::vector<VkLayerProperties> &layerProps,
                                  bool mustHaveLayers,
                                  VulkanLayerVector *enabledLayerNames);

enum class TextureDimension
{
    TEX_2D,
    TEX_CUBE,
    TEX_3D,
    TEX_2D_ARRAY,
};

enum class BufferUsageType
{
    Static      = 0,
    Dynamic     = 1,
    InvalidEnum = 2,
    EnumCount   = InvalidEnum,
};

// A maximum offset of 4096 covers almost every Vulkan driver on desktop (80%) and mobile (99%). The
// next highest values to meet native drivers are 16 bits or 32 bits.
constexpr uint32_t kAttributeOffsetMaxBits = 15;
constexpr uint32_t kInvalidMemoryTypeIndex = UINT32_MAX;
constexpr uint32_t kInvalidMemoryHeapIndex = UINT32_MAX;

namespace vk
{
class Renderer;

// Used for memory allocation tracking.
enum class MemoryAllocationType;

enum class MemoryHostVisibility
{
    NonVisible,
    Visible
};

// Encapsulate the graphics family index and VkQueue index (as seen in vkGetDeviceQueue API
// arguments) into one integer so that we can easily pass around without introduce extra overhead..
class DeviceQueueIndex final
{
  public:
    constexpr DeviceQueueIndex()
        : mFamilyIndex(kInvalidQueueFamilyIndex), mQueueIndex(kInvalidQueueIndex)
    {}
    constexpr DeviceQueueIndex(uint32_t familyIndex)
        : mFamilyIndex((int8_t)familyIndex), mQueueIndex(kInvalidQueueIndex)
    {
        ASSERT(static_cast<uint32_t>(mFamilyIndex) == familyIndex);
    }
    DeviceQueueIndex(uint32_t familyIndex, uint32_t queueIndex)
        : mFamilyIndex((int8_t)familyIndex), mQueueIndex((int8_t)queueIndex)
    {
        // Ensure the value we actually don't truncate the useful bits.
        ASSERT(static_cast<uint32_t>(mFamilyIndex) == familyIndex);
        ASSERT(static_cast<uint32_t>(mQueueIndex) == queueIndex);
    }
    DeviceQueueIndex(const DeviceQueueIndex &other) { *this = other; }

    DeviceQueueIndex &operator=(const DeviceQueueIndex &other)
    {
        mValue = other.mValue;
        return *this;
    }

    constexpr uint32_t familyIndex() const { return mFamilyIndex; }
    constexpr uint32_t queueIndex() const { return mQueueIndex; }

    bool operator==(const DeviceQueueIndex &other) const { return mValue == other.mValue; }
    bool operator!=(const DeviceQueueIndex &other) const { return mValue != other.mValue; }

  private:
    static constexpr int8_t kInvalidQueueFamilyIndex = -1;
    static constexpr int8_t kInvalidQueueIndex       = -1;
    // The expectation is that these indices are small numbers that could easily fit into int8_t.
    // int8_t is used instead of uint8_t because we need to handle VK_QUEUE_FAMILY_FOREIGN_EXT and
    // VK_QUEUE_FAMILY_EXTERNAL properly which are essentially are negative values.
    union
    {
        struct
        {
            int8_t mFamilyIndex;
            int8_t mQueueIndex;
        };
        uint16_t mValue;
    };
};
static constexpr DeviceQueueIndex kInvalidDeviceQueueIndex = DeviceQueueIndex();
static constexpr DeviceQueueIndex kForeignDeviceQueueIndex =
    DeviceQueueIndex(VK_QUEUE_FAMILY_FOREIGN_EXT);
static constexpr DeviceQueueIndex kExternalDeviceQueueIndex =
    DeviceQueueIndex(VK_QUEUE_FAMILY_EXTERNAL);
static_assert(kForeignDeviceQueueIndex.familyIndex() == VK_QUEUE_FAMILY_FOREIGN_EXT);
static_assert(kExternalDeviceQueueIndex.familyIndex() == VK_QUEUE_FAMILY_EXTERNAL);
static_assert(kInvalidDeviceQueueIndex.familyIndex() == VK_QUEUE_FAMILY_IGNORED);

// A packed attachment index interface with vulkan API
class PackedAttachmentIndex final
{
  public:
    explicit constexpr PackedAttachmentIndex(uint32_t index) : mAttachmentIndex(index) {}
    constexpr PackedAttachmentIndex(const PackedAttachmentIndex &other)            = default;
    constexpr PackedAttachmentIndex &operator=(const PackedAttachmentIndex &other) = default;

    constexpr uint32_t get() const { return mAttachmentIndex; }
    PackedAttachmentIndex &operator++()
    {
        ++mAttachmentIndex;
        return *this;
    }
    constexpr bool operator==(const PackedAttachmentIndex &other) const
    {
        return mAttachmentIndex == other.mAttachmentIndex;
    }
    constexpr bool operator!=(const PackedAttachmentIndex &other) const
    {
        return mAttachmentIndex != other.mAttachmentIndex;
    }
    constexpr bool operator<(const PackedAttachmentIndex &other) const
    {
        return mAttachmentIndex < other.mAttachmentIndex;
    }

  private:
    uint32_t mAttachmentIndex;
};
using PackedAttachmentCount                                    = PackedAttachmentIndex;
static constexpr PackedAttachmentIndex kAttachmentIndexInvalid = PackedAttachmentIndex(-1);
static constexpr PackedAttachmentIndex kAttachmentIndexZero    = PackedAttachmentIndex(0);

// Prepend ptr to the pNext chain at chainStart
template <typename VulkanStruct1, typename VulkanStruct2>
void AddToPNextChain(VulkanStruct1 *chainStart, VulkanStruct2 *ptr)
{
    // Catch bugs where this function is called with `&pointer` instead of `pointer`.
    static_assert(!std::is_pointer<VulkanStruct1>::value);
    static_assert(!std::is_pointer<VulkanStruct2>::value);

    ASSERT(ptr->pNext == nullptr);

    VkBaseOutStructure *localPtr = reinterpret_cast<VkBaseOutStructure *>(chainStart);
    ptr->pNext                   = localPtr->pNext;
    localPtr->pNext              = reinterpret_cast<VkBaseOutStructure *>(ptr);
}

// Append ptr to the end of the chain
template <typename VulkanStruct1, typename VulkanStruct2>
void AppendToPNextChain(VulkanStruct1 *chainStart, VulkanStruct2 *ptr)
{
    static_assert(!std::is_pointer<VulkanStruct1>::value);
    static_assert(!std::is_pointer<VulkanStruct2>::value);

    if (!ptr)
    {
        return;
    }

    VkBaseOutStructure *endPtr = reinterpret_cast<VkBaseOutStructure *>(chainStart);
    while (endPtr->pNext)
    {
        endPtr = endPtr->pNext;
    }
    endPtr->pNext = reinterpret_cast<VkBaseOutStructure *>(ptr);
}

class QueueSerialIndexAllocator final
{
  public:
    QueueSerialIndexAllocator() : mLargestIndexEverAllocated(kInvalidQueueSerialIndex)
    {
        // Start with every index is free
        mFreeIndexBitSetArray.set();
        ASSERT(mFreeIndexBitSetArray.all());
    }
    SerialIndex allocate()
    {
        std::lock_guard<angle::SimpleMutex> lock(mMutex);
        if (mFreeIndexBitSetArray.none())
        {
            ERR() << "Run out of queue serial index. All " << kMaxQueueSerialIndexCount
                  << " indices are used.";
            return kInvalidQueueSerialIndex;
        }
        SerialIndex index = static_cast<SerialIndex>(mFreeIndexBitSetArray.first());
        ASSERT(index < kMaxQueueSerialIndexCount);
        mFreeIndexBitSetArray.reset(index);
        mLargestIndexEverAllocated = (~mFreeIndexBitSetArray).last();
        return index;
    }

    void release(SerialIndex index)
    {
        std::lock_guard<angle::SimpleMutex> lock(mMutex);
        ASSERT(index <= mLargestIndexEverAllocated);
        ASSERT(!mFreeIndexBitSetArray.test(index));
        mFreeIndexBitSetArray.set(index);
        // mLargestIndexEverAllocated is for optimization. Even if we released queueIndex, we may
        // still have resources still have serial the index. Thus do not decrement
        // mLargestIndexEverAllocated here. The only downside is that we may get into slightly less
        // optimal code path in GetBatchCountUpToSerials.
    }

    size_t getLargestIndexEverAllocated() const
    {
        return mLargestIndexEverAllocated.load(std::memory_order_consume);
    }

  private:
    angle::BitSetArray<kMaxQueueSerialIndexCount> mFreeIndexBitSetArray;
    std::atomic<size_t> mLargestIndexEverAllocated;
    angle::SimpleMutex mMutex;
};

class [[nodiscard]] ScopedQueueSerialIndex final : angle::NonCopyable
{
  public:
    ScopedQueueSerialIndex() : mIndex(kInvalidQueueSerialIndex), mIndexAllocator(nullptr) {}
    ~ScopedQueueSerialIndex()
    {
        if (mIndex != kInvalidQueueSerialIndex)
        {
            ASSERT(mIndexAllocator != nullptr);
            mIndexAllocator->release(mIndex);
        }
    }

    void init(SerialIndex index, QueueSerialIndexAllocator *indexAllocator)
    {
        ASSERT(mIndex == kInvalidQueueSerialIndex);
        ASSERT(index != kInvalidQueueSerialIndex);
        ASSERT(indexAllocator != nullptr);
        mIndex          = index;
        mIndexAllocator = indexAllocator;
    }

    SerialIndex get() const { return mIndex; }

  private:
    SerialIndex mIndex;
    QueueSerialIndexAllocator *mIndexAllocator;
};

class RefCountedEventsGarbageRecycler;
// Abstracts error handling. Implemented by ContextVk for GL, DisplayVk for EGL, worker threads,
// CLContextVk etc.
class ErrorContext : angle::NonCopyable
{
  public:
    ErrorContext(Renderer *renderer);
    virtual ~ErrorContext();

    virtual void handleError(VkResult result,
                             const char *file,
                             const char *function,
                             unsigned int line) = 0;
    VkDevice getDevice() const;
    Renderer *getRenderer() const { return mRenderer; }
    const angle::FeaturesVk &getFeatures() const;

    const angle::VulkanPerfCounters &getPerfCounters() const { return mPerfCounters; }
    angle::VulkanPerfCounters &getPerfCounters() { return mPerfCounters; }
    const DeviceQueueIndex &getDeviceQueueIndex() const { return mDeviceQueueIndex; }

  protected:
    Renderer *const mRenderer;
    DeviceQueueIndex mDeviceQueueIndex;
    angle::VulkanPerfCounters mPerfCounters;
};

// Abstract global operations that are handled differently between EGL and OpenCL.
class GlobalOps : angle::NonCopyable
{
  public:
    virtual ~GlobalOps() = default;

    virtual void putBlob(const angle::BlobCacheKey &key, const angle::MemoryBuffer &value) = 0;
    virtual bool getBlob(const angle::BlobCacheKey &key, angle::BlobCacheValue *valueOut)  = 0;

    virtual std::shared_ptr<angle::WaitableEvent> postMultiThreadWorkerTask(
        const std::shared_ptr<angle::Closure> &task) = 0;

    virtual void notifyDeviceLost() = 0;
};

class RenderPassDesc;

#if ANGLE_USE_CUSTOM_VULKAN_OUTSIDE_RENDER_PASS_CMD_BUFFERS
using OutsideRenderPassCommandBuffer = priv::SecondaryCommandBuffer;
#else
using OutsideRenderPassCommandBuffer = VulkanSecondaryCommandBuffer;
#endif
#if ANGLE_USE_CUSTOM_VULKAN_RENDER_PASS_CMD_BUFFERS
using RenderPassCommandBuffer = priv::SecondaryCommandBuffer;
#else
using RenderPassCommandBuffer = VulkanSecondaryCommandBuffer;
#endif

struct SecondaryCommandPools
{
    SecondaryCommandPool outsideRenderPassPool;
    SecondaryCommandPool renderPassPool;
};

VkImageAspectFlags GetDepthStencilAspectFlags(const angle::Format &format);
VkImageAspectFlags GetFormatAspectFlags(const angle::Format &format);

template <typename T>
struct ImplTypeHelper;

// clang-format off
#define ANGLE_IMPL_TYPE_HELPER_GL(OBJ) \
template<>                             \
struct ImplTypeHelper<gl::OBJ>         \
{                                      \
    using ImplType = OBJ##Vk;          \
};
// clang-format on

ANGLE_GL_OBJECTS_X(ANGLE_IMPL_TYPE_HELPER_GL)

template <>
struct ImplTypeHelper<gl::MockOverlay>
{
    using ImplType = OverlayVk;
};

template <>
struct ImplTypeHelper<egl::Display>
{
    using ImplType = DisplayVk;
};

template <>
struct ImplTypeHelper<egl::Image>
{
    using ImplType = ImageVk;
};

template <>
struct ImplTypeHelper<egl::ShareGroup>
{
    using ImplType = ShareGroupVk;
};

template <typename T>
using GetImplType = typename ImplTypeHelper<T>::ImplType;

template <typename T>
GetImplType<T> *GetImpl(const T *glObject)
{
    return GetImplAs<GetImplType<T>>(glObject);
}

template <typename T>
GetImplType<T> *SafeGetImpl(const T *glObject)
{
    return SafeGetImplAs<GetImplType<T>>(glObject);
}

template <>
inline OverlayVk *GetImpl(const gl::MockOverlay *glObject)
{
    return nullptr;
}

// Reference to a deleted object. The object is due to be destroyed at some point in the future.
// |mHandleType| determines the type of the object and which destroy function should be called.
class GarbageObject
{
  public:
    GarbageObject();
    GarbageObject(GarbageObject &&other);
    GarbageObject &operator=(GarbageObject &&rhs);

    bool valid() const { return mHandle != VK_NULL_HANDLE; }
    void destroy(Renderer *renderer);

    template <typename DerivedT, typename HandleT>
    static GarbageObject Get(WrappedObject<DerivedT, HandleT> *object)
    {
        // Using c-style cast here to avoid conditional compile for MSVC 32-bit
        //  which fails to compile with reinterpret_cast, requiring static_cast.
        return GarbageObject(HandleTypeHelper<DerivedT>::kHandleType,
                             (GarbageHandle)(object->release()));
    }

  private:
    VK_DEFINE_NON_DISPATCHABLE_HANDLE(GarbageHandle)
    GarbageObject(HandleType handleType, GarbageHandle handle);

    HandleType mHandleType;
    GarbageHandle mHandle;
};

template <typename T>
GarbageObject GetGarbage(T *obj)
{
    return GarbageObject::Get(obj);
}

// A list of garbage objects. Has no object lifetime information.
using GarbageObjects = std::vector<GarbageObject>;

class MemoryProperties final : angle::NonCopyable
{
  public:
    MemoryProperties();

    void init(VkPhysicalDevice physicalDevice);
    bool hasLazilyAllocatedMemory() const;
    VkResult findCompatibleMemoryIndex(Renderer *renderer,
                                       const VkMemoryRequirements &memoryRequirements,
                                       VkMemoryPropertyFlags requestedMemoryPropertyFlags,
                                       bool isExternalMemory,
                                       VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                       uint32_t *indexOut) const;
    void destroy();

    uint32_t getHeapIndexForMemoryType(uint32_t memoryType) const
    {
        if (memoryType == kInvalidMemoryTypeIndex)
        {
            return kInvalidMemoryHeapIndex;
        }

        ASSERT(memoryType < getMemoryTypeCount());
        return mMemoryProperties.memoryTypes[memoryType].heapIndex;
    }

    VkDeviceSize getHeapSizeForMemoryType(uint32_t memoryType) const
    {
        uint32_t heapIndex = mMemoryProperties.memoryTypes[memoryType].heapIndex;
        return mMemoryProperties.memoryHeaps[heapIndex].size;
    }

    const VkMemoryType &getMemoryType(uint32_t i) const { return mMemoryProperties.memoryTypes[i]; }

    uint32_t getMemoryHeapCount() const { return mMemoryProperties.memoryHeapCount; }
    uint32_t getMemoryTypeCount() const { return mMemoryProperties.memoryTypeCount; }

  private:
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
};

// Similar to StagingImage, for Buffers.
class StagingBuffer final : angle::NonCopyable
{
  public:
    StagingBuffer();
    void release(ContextVk *contextVk);
    void collectGarbage(Renderer *renderer, const QueueSerial &queueSerial);
    void destroy(Renderer *renderer);

    angle::Result init(ErrorContext *context, VkDeviceSize size, StagingUsage usage);

    Buffer &getBuffer() { return mBuffer; }
    const Buffer &getBuffer() const { return mBuffer; }
    size_t getSize() const { return mSize; }

  private:
    Buffer mBuffer;
    Allocation mAllocation;
    size_t mSize;
};

angle::Result InitMappableAllocation(ErrorContext *context,
                                     const Allocator &allocator,
                                     Allocation *allocation,
                                     VkDeviceSize size,
                                     int value,
                                     VkMemoryPropertyFlags memoryPropertyFlags);

VkResult AllocateBufferMemory(ErrorContext *context,
                              vk::MemoryAllocationType memoryAllocationType,
                              VkMemoryPropertyFlags requestedMemoryPropertyFlags,
                              VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                              const void *extraAllocationInfo,
                              Buffer *buffer,
                              uint32_t *memoryTypeIndexOut,
                              DeviceMemory *deviceMemoryOut,
                              VkDeviceSize *sizeOut);

VkResult AllocateImageMemory(ErrorContext *context,
                             vk::MemoryAllocationType memoryAllocationType,
                             VkMemoryPropertyFlags memoryPropertyFlags,
                             VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                             const void *extraAllocationInfo,
                             Image *image,
                             uint32_t *memoryTypeIndexOut,
                             DeviceMemory *deviceMemoryOut,
                             VkDeviceSize *sizeOut);

VkResult AllocateImageMemoryWithRequirements(ErrorContext *context,
                                             vk::MemoryAllocationType memoryAllocationType,
                                             VkMemoryPropertyFlags memoryPropertyFlags,
                                             const VkMemoryRequirements &memoryRequirements,
                                             const void *extraAllocationInfo,
                                             const VkBindImagePlaneMemoryInfoKHR *extraBindInfo,
                                             Image *image,
                                             uint32_t *memoryTypeIndexOut,
                                             DeviceMemory *deviceMemoryOut);

VkResult AllocateBufferMemoryWithRequirements(ErrorContext *context,
                                              MemoryAllocationType memoryAllocationType,
                                              VkMemoryPropertyFlags memoryPropertyFlags,
                                              const VkMemoryRequirements &memoryRequirements,
                                              const void *extraAllocationInfo,
                                              Buffer *buffer,
                                              VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                              uint32_t *memoryTypeIndexOut,
                                              DeviceMemory *deviceMemoryOut);

gl::TextureType Get2DTextureType(uint32_t layerCount, GLint samples);

enum class RecordingMode
{
    Start,
    Append,
};

// Helper class to handle RAII patterns for initialization. Requires that T have a destroy method
// that takes a VkDevice and returns void.
template <typename T>
class [[nodiscard]] DeviceScoped final : angle::NonCopyable
{
  public:
    explicit DeviceScoped(VkDevice device) : mDevice(device) {}
    DeviceScoped(DeviceScoped &&other) : mDevice(other.mDevice), mVar(std::move(other.mVar)) {}
    ~DeviceScoped() { mVar.destroy(mDevice); }

    const T &get() const { return mVar; }
    T &get() { return mVar; }

    T &&release() { return std::move(mVar); }

  private:
    VkDevice mDevice;
    T mVar;
};

template <typename T>
class [[nodiscard]] AllocatorScoped final : angle::NonCopyable
{
  public:
    AllocatorScoped(const Allocator &allocator) : mAllocator(allocator) {}
    ~AllocatorScoped() { mVar.destroy(mAllocator); }

    const T &get() const { return mVar; }
    T &get() { return mVar; }

    T &&release() { return std::move(mVar); }

  private:
    const Allocator &mAllocator;
    T mVar;
};

// Similar to DeviceScoped, but releases objects instead of destroying them. Requires that T have a
// release method that takes a ContextVk * and returns void.
template <typename T>
class [[nodiscard]] ContextScoped final : angle::NonCopyable
{
  public:
    ContextScoped(ContextVk *contextVk) : mContextVk(contextVk) {}
    ~ContextScoped() { mVar.release(mContextVk); }

    const T &get() const { return mVar; }
    T &get() { return mVar; }

    T &&release() { return std::move(mVar); }

  private:
    ContextVk *mContextVk;
    T mVar;
};

template <typename T>
class [[nodiscard]] RendererScoped final : angle::NonCopyable
{
  public:
    RendererScoped(Renderer *renderer) : mRenderer(renderer) {}
    ~RendererScoped() { mVar.release(mRenderer); }

    const T &get() const { return mVar; }
    T &get() { return mVar; }

    T &&release() { return std::move(mVar); }

  private:
    Renderer *mRenderer;
    T mVar;
};

// This is a very simple RefCount class that has no autoreleasing.
template <typename T>
class RefCounted : angle::NonCopyable
{
  public:
    RefCounted() : mRefCount(0) {}
    template <class... Args>
    explicit RefCounted(Args &&...args) : mRefCount(0), mObject(std::forward<Args>(args)...)
    {}
    explicit RefCounted(T &&newObject) : mRefCount(0), mObject(std::move(newObject)) {}
    ~RefCounted() { ASSERT(mRefCount == 0 && !mObject.valid()); }

    RefCounted(RefCounted &&copy) : mRefCount(copy.mRefCount), mObject(std::move(copy.mObject))
    {
        ASSERT(this != &copy);
        copy.mRefCount = 0;
    }

    RefCounted &operator=(RefCounted &&rhs)
    {
        std::swap(mRefCount, rhs.mRefCount);
        mObject = std::move(rhs.mObject);
        return *this;
    }

    void addRef()
    {
        ASSERT(mRefCount != std::numeric_limits<uint32_t>::max());
        mRefCount++;
    }

    void releaseRef()
    {
        ASSERT(isReferenced());
        mRefCount--;
    }

    uint32_t getAndReleaseRef()
    {
        ASSERT(isReferenced());
        return mRefCount--;
    }

    bool isReferenced() const { return mRefCount != 0; }
    uint32_t getRefCount() const { return mRefCount; }
    bool isLastReferenceCount() const { return mRefCount == 1; }

    T &get() { return mObject; }
    const T &get() const { return mObject; }

    // A debug function to validate that the reference count is as expected used for assertions.
    bool isRefCountAsExpected(uint32_t expectedRefCount) { return mRefCount == expectedRefCount; }

  private:
    uint32_t mRefCount;
    T mObject;
};

// Atomic version of RefCounted.  Used in the descriptor set and pipeline layout caches, which are
// accessed by link jobs.  No std::move is allowed due to the atomic ref count.
template <typename T>
class AtomicRefCounted : angle::NonCopyable
{
  public:
    AtomicRefCounted() : mRefCount(0) {}
    explicit AtomicRefCounted(T &&newObject) : mRefCount(0), mObject(std::move(newObject)) {}
    ~AtomicRefCounted() { ASSERT(mRefCount == 0 && !mObject.valid()); }

    void addRef()
    {
        ASSERT(mRefCount != std::numeric_limits<uint32_t>::max());
        mRefCount.fetch_add(1, std::memory_order_relaxed);
    }

    // Warning: method does not perform any synchronization, therefore can not be used along with
    // following `!isReferenced()` call to check if object is not longer accessed by other threads.
    // Use `getAndReleaseRef()` instead, when synchronization is required.
    void releaseRef()
    {
        ASSERT(isReferenced());
        mRefCount.fetch_sub(1, std::memory_order_relaxed);
    }

    // Performs acquire-release memory synchronization. When result is "1", the object is
    // guaranteed to be no longer in use by other threads, and may be safely destroyed or updated.
    // Warning: do not mix this method and the unsynchronized `releaseRef()` call.
    unsigned int getAndReleaseRef()
    {
        ASSERT(isReferenced());
        return mRefCount.fetch_sub(1, std::memory_order_acq_rel);
    }

    // Making decisions based on reference count is not thread safe, so it should not used in
    // release build.
#if defined(ANGLE_ENABLE_ASSERTS)
    // Warning: method does not perform any synchronization.  See `releaseRef()` for details.
    // Method may be only used after external synchronization.
    bool isReferenced() const { return mRefCount.load(std::memory_order_relaxed) != 0; }
    uint32_t getRefCount() const { return mRefCount.load(std::memory_order_relaxed); }
    // This is used by SharedPtr::unique, so needs strong ordering.
    bool isLastReferenceCount() const { return mRefCount.load(std::memory_order_acquire) == 1; }
#else
    // Compiler still compile but should never actually produce code.
    bool isReferenced() const
    {
        UNREACHABLE();
        return false;
    }
    uint32_t getRefCount() const
    {
        UNREACHABLE();
        return 0;
    }
    bool isLastReferenceCount() const
    {
        UNREACHABLE();
        return false;
    }
#endif

    T &get() { return mObject; }
    const T &get() const { return mObject; }

  private:
    std::atomic_uint mRefCount;
    T mObject;
};

// This is intended to have same interface as std::shared_ptr except this must used in thread safe
// environment.
template <typename>
class WeakPtr;
template <typename T, class RefCountedStorage = RefCounted<T>>
class SharedPtr final
{
  public:
    SharedPtr() : mRefCounted(nullptr), mDevice(VK_NULL_HANDLE) {}
    SharedPtr(VkDevice device, T &&object) : mDevice(device)
    {
        mRefCounted = new RefCountedStorage(std::move(object));
        mRefCounted->addRef();
    }
    SharedPtr(VkDevice device, const WeakPtr<T> &other)
        : mRefCounted(other.mRefCounted), mDevice(device)
    {
        if (mRefCounted)
        {
            // There must already have another SharedPtr holding onto the underline object when
            // WeakPtr is valid.
            ASSERT(mRefCounted->isReferenced());
            mRefCounted->addRef();
        }
    }
    ~SharedPtr() { reset(); }

    SharedPtr(const SharedPtr &other) : mRefCounted(nullptr), mDevice(VK_NULL_HANDLE)
    {
        *this = other;
    }

    SharedPtr(SharedPtr &&other) : mRefCounted(nullptr), mDevice(VK_NULL_HANDLE)
    {
        *this = std::move(other);
    }

    template <class... Args>
    static SharedPtr<T, RefCountedStorage> MakeShared(VkDevice device, Args &&...args)
    {
        SharedPtr<T, RefCountedStorage> newObject;
        newObject.mRefCounted = new RefCountedStorage(std::forward<Args>(args)...);
        newObject.mRefCounted->addRef();
        newObject.mDevice = device;
        return newObject;
    }

    void reset()
    {
        if (mRefCounted)
        {
            releaseRef();
            mRefCounted = nullptr;
            mDevice     = VK_NULL_HANDLE;
        }
    }

    SharedPtr &operator=(SharedPtr &&other)
    {
        if (mRefCounted)
        {
            releaseRef();
        }
        mRefCounted       = other.mRefCounted;
        mDevice           = other.mDevice;
        other.mRefCounted = nullptr;
        other.mDevice     = VK_NULL_HANDLE;
        return *this;
    }

    SharedPtr &operator=(const SharedPtr &other)
    {
        if (mRefCounted)
        {
            releaseRef();
        }
        mRefCounted = other.mRefCounted;
        mDevice     = other.mDevice;
        if (mRefCounted)
        {
            mRefCounted->addRef();
        }
        return *this;
    }

    operator bool() const { return mRefCounted != nullptr; }

    T &operator*() const
    {
        ASSERT(mRefCounted != nullptr);
        return mRefCounted->get();
    }

    T *operator->() const { return get(); }

    T *get() const
    {
        ASSERT(mRefCounted != nullptr);
        return &mRefCounted->get();
    }

    bool unique() const
    {
        ASSERT(mRefCounted != nullptr);
        return mRefCounted->isLastReferenceCount();
    }

    bool owner_equal(const SharedPtr<T> &other) const { return mRefCounted == other.mRefCounted; }

    uint32_t getRefCount() const { return mRefCounted->getRefCount(); }

  private:
    void releaseRef()
    {
        ASSERT(mRefCounted != nullptr);
        unsigned int refCount = mRefCounted->getAndReleaseRef();
        if (refCount == 1)
        {
            mRefCounted->get().destroy(mDevice);
            SafeDelete(mRefCounted);
        }
    }

    friend class WeakPtr<T>;
    RefCountedStorage *mRefCounted;
    VkDevice mDevice;
};

template <typename T>
using AtomicSharedPtr = SharedPtr<T, AtomicRefCounted<T>>;

// This is intended to have same interface as std::weak_ptr
template <typename T>
class WeakPtr final
{
  public:
    using RefCountedStorage = RefCounted<T>;

    WeakPtr() : mRefCounted(nullptr) {}

    WeakPtr(const SharedPtr<T> &other) : mRefCounted(other.mRefCounted) {}

    void reset() { mRefCounted = nullptr; }

    operator bool() const
    {
        // There must have another SharedPtr holding onto the underline object when WeakPtr is
        // valid.
        ASSERT(mRefCounted == nullptr || mRefCounted->isReferenced());
        return mRefCounted != nullptr;
    }

    T *operator->() const { return get(); }

    T *get() const
    {
        ASSERT(mRefCounted != nullptr);
        ASSERT(mRefCounted->isReferenced());
        return &mRefCounted->get();
    }

    long use_count() const
    {
        ASSERT(mRefCounted != nullptr);
        // There must have another SharedPtr holding onto the underline object when WeakPtr is
        // valid.
        ASSERT(mRefCounted->isReferenced());
        return mRefCounted->getRefCount();
    }
    bool owner_equal(const SharedPtr<T> &other) const
    {
        // There must have another SharedPtr holding onto the underlying object when WeakPtr is
        // valid.
        ASSERT(mRefCounted == nullptr || mRefCounted->isReferenced());
        return mRefCounted == other.mRefCounted;
    }

  private:
    friend class SharedPtr<T>;
    RefCountedStorage *mRefCounted;
};

// Helper class to share ref-counted Vulkan objects.  Requires that T have a destroy method
// that takes a VkDevice and returns void.
template <typename T>
class Shared final : angle::NonCopyable
{
  public:
    Shared() : mRefCounted(nullptr) {}
    ~Shared() { ASSERT(mRefCounted == nullptr); }

    Shared(Shared &&other) { *this = std::move(other); }
    Shared &operator=(Shared &&other)
    {
        ASSERT(this != &other);
        mRefCounted       = other.mRefCounted;
        other.mRefCounted = nullptr;
        return *this;
    }

    void set(VkDevice device, RefCounted<T> *refCounted)
    {
        if (mRefCounted)
        {
            mRefCounted->releaseRef();
            if (!mRefCounted->isReferenced())
            {
                mRefCounted->get().destroy(device);
                SafeDelete(mRefCounted);
            }
        }

        mRefCounted = refCounted;

        if (mRefCounted)
        {
            mRefCounted->addRef();
        }
    }

    void setUnreferenced(RefCounted<T> *refCounted)
    {
        ASSERT(!mRefCounted);
        ASSERT(refCounted);

        mRefCounted = refCounted;
        mRefCounted->addRef();
    }

    void assign(VkDevice device, T &&newObject)
    {
        set(device, new RefCounted<T>(std::move(newObject)));
    }

    void copy(VkDevice device, const Shared<T> &other) { set(device, other.mRefCounted); }

    void copyUnreferenced(const Shared<T> &other) { setUnreferenced(other.mRefCounted); }

    void reset(VkDevice device) { set(device, nullptr); }

    template <typename RecyclerT>
    void resetAndRecycle(RecyclerT *recycler)
    {
        if (mRefCounted)
        {
            mRefCounted->releaseRef();
            if (!mRefCounted->isReferenced())
            {
                ASSERT(mRefCounted->get().valid());
                recycler->recycle(std::move(mRefCounted->get()));
                SafeDelete(mRefCounted);
            }

            mRefCounted = nullptr;
        }
    }

    template <typename OnRelease>
    void resetAndRelease(OnRelease *onRelease)
    {
        if (mRefCounted)
        {
            mRefCounted->releaseRef();
            if (!mRefCounted->isReferenced())
            {
                ASSERT(mRefCounted->get().valid());
                (*onRelease)(std::move(mRefCounted->get()));
                SafeDelete(mRefCounted);
            }

            mRefCounted = nullptr;
        }
    }

    bool isReferenced() const
    {
        // If reference is zero, the object should have been deleted.  I.e. if the object is not
        // nullptr, it should have a reference.
        ASSERT(!mRefCounted || mRefCounted->isReferenced());
        return mRefCounted != nullptr;
    }

    T &get()
    {
        ASSERT(mRefCounted && mRefCounted->isReferenced());
        return mRefCounted->get();
    }
    const T &get() const
    {
        ASSERT(mRefCounted && mRefCounted->isReferenced());
        return mRefCounted->get();
    }

  private:
    RefCounted<T> *mRefCounted;
};

template <typename T, typename StorageT = std::deque<T>>
class Recycler final : angle::NonCopyable
{
  public:
    Recycler() = default;
    Recycler(StorageT &&storage) { mObjectFreeList = std::move(storage); }

    void recycle(T &&garbageObject)
    {
        // Recycling invalid objects is pointless and potentially a bug.
        ASSERT(garbageObject.valid());
        mObjectFreeList.emplace_back(std::move(garbageObject));
    }

    void recycle(StorageT &&garbageObjects)
    {
        // Recycling invalid objects is pointless and potentially a bug.
        ASSERT(!garbageObjects.empty());
        mObjectFreeList.insert(mObjectFreeList.end(), garbageObjects.begin(), garbageObjects.end());
        ASSERT(garbageObjects.empty());
    }

    void refill(StorageT &&garbageObjects)
    {
        ASSERT(!garbageObjects.empty());
        ASSERT(mObjectFreeList.empty());
        mObjectFreeList.swap(garbageObjects);
    }

    void fetch(T *outObject)
    {
        ASSERT(!empty());
        *outObject = std::move(mObjectFreeList.back());
        mObjectFreeList.pop_back();
    }

    void destroy(VkDevice device)
    {
        while (!mObjectFreeList.empty())
        {
            T &object = mObjectFreeList.back();
            object.destroy(device);
            mObjectFreeList.pop_back();
        }
    }

    bool empty() const { return mObjectFreeList.empty(); }

  private:
    StorageT mObjectFreeList;
};

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
struct SpecializationConstants final
{
    VkBool32 surfaceRotation;
    uint32_t dither;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

template <typename T>
using SpecializationConstantMap = angle::PackedEnumMap<sh::vk::SpecializationConstantId, T>;

using ShaderModulePtr = SharedPtr<ShaderModule>;
using ShaderModuleMap = gl::ShaderMap<ShaderModulePtr>;

angle::Result InitShaderModule(ErrorContext *context,
                               ShaderModulePtr *shaderModulePtr,
                               const uint32_t *shaderCode,
                               size_t shaderCodeSize);

void MakeDebugUtilsLabel(GLenum source, const char *marker, VkDebugUtilsLabelEXT *label);

constexpr size_t kUnpackedDepthIndex   = gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
constexpr size_t kUnpackedStencilIndex = gl::IMPLEMENTATION_MAX_DRAW_BUFFERS + 1;
constexpr uint32_t kUnpackedColorBuffersMask =
    angle::BitMask<uint32_t>(gl::IMPLEMENTATION_MAX_DRAW_BUFFERS);

class ClearValuesArray final
{
  public:
    ClearValuesArray();
    ~ClearValuesArray();

    ClearValuesArray(const ClearValuesArray &other);
    ClearValuesArray &operator=(const ClearValuesArray &rhs);

    void store(uint32_t index, VkImageAspectFlags aspectFlags, const VkClearValue &clearValue);
    void storeNoDepthStencil(uint32_t index, const VkClearValue &clearValue);

    void reset(size_t index)
    {
        mValues[index] = {};
        mEnabled.reset(index);
    }

    bool test(size_t index) const { return mEnabled.test(index); }
    bool testDepth() const { return mEnabled.test(kUnpackedDepthIndex); }
    bool testStencil() const { return mEnabled.test(kUnpackedStencilIndex); }
    gl::DrawBufferMask getColorMask() const;

    const VkClearValue &operator[](size_t index) const { return mValues[index]; }

    float getDepthValue() const { return mValues[kUnpackedDepthIndex].depthStencil.depth; }
    uint32_t getStencilValue() const { return mValues[kUnpackedStencilIndex].depthStencil.stencil; }

    const VkClearValue *data() const { return mValues.data(); }
    bool empty() const { return mEnabled.none(); }
    bool any() const { return mEnabled.any(); }

  private:
    gl::AttachmentArray<VkClearValue> mValues;
    gl::AttachmentsMask mEnabled;
};

// Defines Serials for Vulkan objects.
#define ANGLE_VK_SERIAL_OP(X) \
    X(Buffer)                 \
    X(Image)                  \
    X(ImageOrBufferView)      \
    X(Sampler)

#define ANGLE_DEFINE_VK_SERIAL_TYPE(Type)                                     \
    class Type##Serial                                                        \
    {                                                                         \
      public:                                                                 \
        constexpr Type##Serial() : mSerial(kInvalid) {}                       \
        constexpr explicit Type##Serial(uint32_t serial) : mSerial(serial) {} \
                                                                              \
        constexpr bool operator==(const Type##Serial &other) const            \
        {                                                                     \
            ASSERT(mSerial != kInvalid || other.mSerial != kInvalid);         \
            return mSerial == other.mSerial;                                  \
        }                                                                     \
        constexpr bool operator!=(const Type##Serial &other) const            \
        {                                                                     \
            ASSERT(mSerial != kInvalid || other.mSerial != kInvalid);         \
            return mSerial != other.mSerial;                                  \
        }                                                                     \
        constexpr uint32_t getValue() const                                   \
        {                                                                     \
            return mSerial;                                                   \
        }                                                                     \
        constexpr bool valid() const                                          \
        {                                                                     \
            return mSerial != kInvalid;                                       \
        }                                                                     \
                                                                              \
      private:                                                                \
        uint32_t mSerial;                                                     \
        static constexpr uint32_t kInvalid = 0;                               \
    };                                                                        \
    static constexpr Type##Serial kInvalid##Type##Serial = Type##Serial();

ANGLE_VK_SERIAL_OP(ANGLE_DEFINE_VK_SERIAL_TYPE)

#define ANGLE_DECLARE_GEN_VK_SERIAL(Type) Type##Serial generate##Type##Serial();

class ResourceSerialFactory final : angle::NonCopyable
{
  public:
    ResourceSerialFactory();
    ~ResourceSerialFactory();

    ANGLE_VK_SERIAL_OP(ANGLE_DECLARE_GEN_VK_SERIAL)

  private:
    uint32_t issueSerial();

    // Kept atomic so it can be accessed from multiple Context threads at once.
    std::atomic<uint32_t> mCurrentUniqueSerial;
};

#if defined(ANGLE_ENABLE_PERF_COUNTER_OUTPUT)
constexpr bool kOutputCumulativePerfCounters = ANGLE_ENABLE_PERF_COUNTER_OUTPUT;
#else
constexpr bool kOutputCumulativePerfCounters = false;
#endif

// Performance and resource counters.
struct RenderPassPerfCounters
{
    // load/storeOps. Includes ops for resolve attachment. Maximum value = 2.
    uint8_t colorLoadOpClears;
    uint8_t colorLoadOpLoads;
    uint8_t colorLoadOpNones;
    uint8_t colorStoreOpStores;
    uint8_t colorStoreOpNones;
    uint8_t depthLoadOpClears;
    uint8_t depthLoadOpLoads;
    uint8_t depthLoadOpNones;
    uint8_t depthStoreOpStores;
    uint8_t depthStoreOpNones;
    uint8_t stencilLoadOpClears;
    uint8_t stencilLoadOpLoads;
    uint8_t stencilLoadOpNones;
    uint8_t stencilStoreOpStores;
    uint8_t stencilStoreOpNones;
    // Number of unresolve and resolve operations.  Maximum value for color =
    // gl::IMPLEMENTATION_MAX_DRAW_BUFFERS and for depth/stencil = 1 each.
    uint8_t colorAttachmentUnresolves;
    uint8_t colorAttachmentResolves;
    uint8_t depthAttachmentUnresolves;
    uint8_t depthAttachmentResolves;
    uint8_t stencilAttachmentUnresolves;
    uint8_t stencilAttachmentResolves;
    // Whether the depth/stencil attachment is using a read-only layout.
    uint8_t readOnlyDepthStencil;
};

// A Vulkan image level index.
using LevelIndex = gl::LevelIndexWrapper<uint32_t>;

// Ensure viewport is within Vulkan requirements
void ClampViewport(VkViewport *viewport);

constexpr bool IsDynamicDescriptor(VkDescriptorType descriptorType)
{
    switch (descriptorType)
    {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return true;
        default:
            return false;
    }
}

void ApplyPipelineCreationFeedback(ErrorContext *context,
                                   const VkPipelineCreationFeedback &feedback);

angle::Result SetDebugUtilsObjectName(ContextVk *contextVk,
                                      VkObjectType objectType,
                                      uint64_t handle,
                                      const std::string &label);

}  // namespace vk

#if !defined(ANGLE_SHARED_LIBVULKAN)
// Lazily load entry points for each extension as necessary.
void InitDebugUtilsEXTFunctions(VkInstance instance);
void InitTransformFeedbackEXTFunctions(VkDevice device);
void InitRenderPass2KHRFunctions(VkDevice device);

#    if defined(ANGLE_PLATFORM_FUCHSIA)
// VK_FUCHSIA_imagepipe_surface
void InitImagePipeSurfaceFUCHSIAFunctions(VkInstance instance);
#    endif

#    if defined(ANGLE_PLATFORM_ANDROID)
// VK_ANDROID_external_memory_android_hardware_buffer
void InitExternalMemoryHardwareBufferANDROIDFunctions(VkDevice device);
#    endif

#    if defined(ANGLE_PLATFORM_GGP)
// VK_GGP_stream_descriptor_surface
void InitGGPStreamDescriptorSurfaceFunctions(VkInstance instance);
#    endif  // defined(ANGLE_PLATFORM_GGP)

// VK_KHR_external_semaphore_fd
void InitExternalSemaphoreFdFunctions(VkDevice device);

// VK_EXT_host_query_reset
void InitHostQueryResetFunctions(VkDevice device);

// VK_KHR_external_fence_fd
void InitExternalFenceFdFunctions(VkDevice device);

// VK_KHR_shared_presentable_image
void InitGetSwapchainStatusKHRFunctions(VkDevice device);

// VK_EXT_extended_dynamic_state
void InitExtendedDynamicStateEXTFunctions(VkDevice device);

// VK_EXT_extended_dynamic_state2
void InitExtendedDynamicState2EXTFunctions(VkDevice device);

// VK_EXT_vertex_input_dynamic_state
void InitVertexInputDynamicStateEXTFunctions(VkDevice device);

// VK_KHR_dynamic_rendering
void InitDynamicRenderingFunctions(VkDevice device);

// VK_KHR_dynamic_rendering_local_read
void InitDynamicRenderingLocalReadFunctions(VkDevice device);

// VK_KHR_fragment_shading_rate
void InitFragmentShadingRateKHRInstanceFunction(VkInstance instance);
void InitFragmentShadingRateKHRDeviceFunction(VkDevice device);

// VK_GOOGLE_display_timing
void InitGetPastPresentationTimingGoogleFunction(VkDevice device);

// VK_EXT_host_image_copy
void InitHostImageCopyFunctions(VkDevice device);

// VK_KHR_Synchronization2
void InitSynchronization2Functions(VkDevice device);

#endif  // !defined(ANGLE_SHARED_LIBVULKAN)

// Promoted to Vulkan 1.1
void InitGetPhysicalDeviceProperties2KHRFunctionsFromCore();
void InitExternalFenceCapabilitiesFunctionsFromCore();
void InitExternalSemaphoreCapabilitiesFunctionsFromCore();
void InitSamplerYcbcrKHRFunctionsFromCore();
void InitGetMemoryRequirements2KHRFunctionsFromCore();
void InitBindMemory2KHRFunctionsFromCore();

GLenum CalculateGenerateMipmapFilter(ContextVk *contextVk, angle::FormatID formatID);

namespace gl_vk
{
inline VkRect2D GetRect(const gl::Rectangle &source)
{
    return {{source.x, source.y},
            {static_cast<uint32_t>(source.width), static_cast<uint32_t>(source.height)}};
}
VkFilter GetFilter(const GLenum filter);
VkSamplerMipmapMode GetSamplerMipmapMode(const GLenum filter);
VkSamplerAddressMode GetSamplerAddressMode(const GLenum wrap);
VkPrimitiveTopology GetPrimitiveTopology(gl::PrimitiveMode mode);
VkPolygonMode GetPolygonMode(const gl::PolygonMode polygonMode);
VkCullModeFlagBits GetCullMode(const gl::RasterizerState &rasterState);
VkFrontFace GetFrontFace(GLenum frontFace, bool invertCullFace);
VkSampleCountFlagBits GetSamples(GLint sampleCount, bool limitSampleCountTo2);
VkComponentSwizzle GetSwizzle(const GLenum swizzle);
VkCompareOp GetCompareOp(const GLenum compareFunc);
VkStencilOp GetStencilOp(const GLenum compareOp);
VkLogicOp GetLogicOp(const GLenum logicOp);

constexpr gl::ShaderMap<VkShaderStageFlagBits> kShaderStageMap = {
    {gl::ShaderType::Vertex, VK_SHADER_STAGE_VERTEX_BIT},
    {gl::ShaderType::TessControl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {gl::ShaderType::TessEvaluation, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {gl::ShaderType::Fragment, VK_SHADER_STAGE_FRAGMENT_BIT},
    {gl::ShaderType::Geometry, VK_SHADER_STAGE_GEOMETRY_BIT},
    {gl::ShaderType::Compute, VK_SHADER_STAGE_COMPUTE_BIT},
};

void GetOffset(const gl::Offset &glOffset, VkOffset3D *vkOffset);
void GetExtent(const gl::Extents &glExtent, VkExtent3D *vkExtent);
VkImageType GetImageType(gl::TextureType textureType);
VkImageViewType GetImageViewType(gl::TextureType textureType);
VkColorComponentFlags GetColorComponentFlags(bool red, bool green, bool blue, bool alpha);
VkShaderStageFlags GetShaderStageFlags(gl::ShaderBitSet activeShaders);

void GetViewport(const gl::Rectangle &viewport,
                 float nearPlane,
                 float farPlane,
                 bool invertViewport,
                 bool upperLeftOrigin,
                 GLint renderAreaHeight,
                 VkViewport *viewportOut);

void GetExtentsAndLayerCount(gl::TextureType textureType,
                             const gl::Extents &extents,
                             VkExtent3D *extentsOut,
                             uint32_t *layerCountOut);

vk::LevelIndex GetLevelIndex(gl::LevelIndex levelGL, gl::LevelIndex baseLevel);

VkImageTiling GetTilingMode(gl::TilingMode tilingMode);

VkImageCompressionFixedRateFlagsEXT ConvertEGLFixedRateToVkFixedRate(
    const EGLenum eglCompressionRate,
    const angle::FormatID actualFormatID);
}  // namespace gl_vk

namespace vk_gl
{
// The Vulkan back-end will not support a sample count of 1, because of a Vulkan specification
// restriction:
//
//   If the image was created with VkImageCreateInfo::samples equal to VK_SAMPLE_COUNT_1_BIT, the
//   instruction must: have MS = 0.
//
// This restriction was tracked in http://anglebug.com/42262827 and Khronos-private Vulkan
// specification issue https://gitlab.khronos.org/vulkan/vulkan/issues/1925.
//
// In addition, the Vulkan back-end will not support sample counts of 32 or 64, since there are no
// standard sample locations for those sample counts.
constexpr unsigned int kSupportedSampleCounts = (VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT |
                                                 VK_SAMPLE_COUNT_8_BIT | VK_SAMPLE_COUNT_16_BIT);

// Find set bits in sampleCounts and add the corresponding sample count to the set.
void AddSampleCounts(VkSampleCountFlags sampleCounts, gl::SupportedSampleSet *outSet);
// Return the maximum sample count with a bit set in |sampleCounts|.
GLuint GetMaxSampleCount(VkSampleCountFlags sampleCounts);
// Return a supported sample count that's at least as large as the requested one.
GLuint GetSampleCount(VkSampleCountFlags supportedCounts, GLuint requestedCount);

gl::LevelIndex GetLevelIndex(vk::LevelIndex levelVk, gl::LevelIndex baseLevel);

GLenum ConvertVkFixedRateToGLFixedRate(const VkImageCompressionFixedRateFlagsEXT vkCompressionRate);
GLint ConvertCompressionFlagsToGLFixedRates(
    VkImageCompressionFixedRateFlagsEXT imageCompressionFixedRateFlags,
    GLsizei bufSize,
    GLint *rates);

EGLenum ConvertVkFixedRateToEGLFixedRate(
    const VkImageCompressionFixedRateFlagsEXT vkCompressionRate);
std::vector<EGLint> ConvertCompressionFlagsToEGLFixedRate(
    VkImageCompressionFixedRateFlagsEXT imageCompressionFixedRateFlags,
    size_t rateSize);
}  // namespace vk_gl

enum class RenderPassClosureReason
{
    // Don't specify the reason (it should already be specified elsewhere)
    AlreadySpecifiedElsewhere,

    // Implicit closures due to flush/wait/etc.
    ContextDestruction,
    ContextChange,
    GLFlush,
    GLFinish,
    EGLSwapBuffers,
    EGLWaitClient,
    SurfaceUnMakeCurrent,

    // Closure due to switching rendering to another framebuffer.
    FramebufferBindingChange,
    FramebufferChange,
    NewRenderPass,

    // Incompatible use of resource in the same render pass
    BufferUseThenXfbWrite,
    XfbWriteThenVertexIndexBuffer,
    XfbWriteThenIndirectDrawBuffer,
    XfbResumeAfterDrawBasedClear,
    DepthStencilUseInFeedbackLoop,
    DepthStencilWriteAfterFeedbackLoop,
    PipelineBindWhileXfbActive,

    // Use of resource after render pass
    BufferWriteThenMap,
    BufferWriteThenOutOfRPRead,
    BufferUseThenOutOfRPWrite,
    ImageUseThenOutOfRPRead,
    ImageUseThenOutOfRPWrite,
    XfbWriteThenComputeRead,
    XfbWriteThenIndirectDispatchBuffer,
    ImageAttachmentThenComputeRead,
    GraphicsTextureImageAccessThenComputeAccess,
    GetQueryResult,
    BeginNonRenderPassQuery,
    EndNonRenderPassQuery,
    TimestampQuery,
    EndRenderPassQuery,
    GLReadPixels,

    // Synchronization
    BufferUseThenReleaseToExternal,
    ImageUseThenReleaseToExternal,
    BufferInUseWhenSynchronizedMap,
    GLMemoryBarrierThenStorageResource,
    StorageResourceUseThenGLMemoryBarrier,
    ExternalSemaphoreSignal,
    SyncObjectInit,
    SyncObjectWithFdInit,
    SyncObjectClientWait,
    SyncObjectServerWait,
    SyncObjectGetStatus,

    // Closures that ANGLE could have avoided, but doesn't for simplicity or optimization of more
    // common cases.
    XfbPause,
    FramebufferFetchEmulation,
    ColorBufferWithEmulatedAlphaInvalidate,
    GenerateMipmapOnCPU,
    CopyTextureOnCPU,
    TextureReformatToRenderable,
    DeviceLocalBufferMap,
    OutOfReservedQueueSerialForOutsideCommands,

    // UtilsVk
    GenerateMipmapWithDraw,
    PrepareForBlit,
    PrepareForImageCopy,
    TemporaryForClearTexture,
    TemporaryForImageClear,
    TemporaryForImageCopy,
    TemporaryForOverlayDraw,

    // LegacyDithering requires updating the render pass
    LegacyDithering,

    // In case of memory budget issues, pending garbage needs to be freed.
    ExcessivePendingGarbage,
    OutOfMemory,

    InvalidEnum,
    EnumCount = InvalidEnum,
};

// The scope of synchronization for a sync object.  Synchronization is done between the signal
// entity (src) and the entities waiting on the signal (dst)
//
// - For GL fence sync objects, src is the current context and dst is host / the rest of share
// group.
// - For EGL fence sync objects, src is the current context and dst is host / all other contexts.
// - For EGL global fence sync objects (which is an ANGLE extension), src is all contexts who have
//   previously made a submission to the queue used by the current context and dst is host / all
//   other contexts.
enum class SyncFenceScope
{
    CurrentContextToShareGroup,
    CurrentContextToAllContexts,
    AllContextsToAllContexts,
};

}  // namespace rx

#define ANGLE_VK_TRY(context, command)                                                   \
    do                                                                                   \
    {                                                                                    \
        auto ANGLE_LOCAL_VAR = command;                                                  \
        if (ANGLE_UNLIKELY(ANGLE_LOCAL_VAR != VK_SUCCESS))                               \
        {                                                                                \
            (context)->handleError(ANGLE_LOCAL_VAR, __FILE__, ANGLE_FUNCTION, __LINE__); \
            return angle::Result::Stop;                                                  \
        }                                                                                \
    } while (0)

#define ANGLE_VK_CHECK(context, test, error) ANGLE_VK_TRY(context, test ? VK_SUCCESS : error)

#define ANGLE_VK_CHECK_MATH(context, result) \
    ANGLE_VK_CHECK(context, result, VK_ERROR_VALIDATION_FAILED_EXT)

#define ANGLE_VK_CHECK_ALLOC(context, result) \
    ANGLE_VK_CHECK(context, result, VK_ERROR_OUT_OF_HOST_MEMORY)

#define ANGLE_VK_UNREACHABLE(context) \
    UNREACHABLE();                    \
    ANGLE_VK_CHECK(context, false, VK_ERROR_FEATURE_NOT_PRESENT)

// Returns VkResult in the case of an error.
#define VK_RESULT_TRY(command)                             \
    do                                                     \
    {                                                      \
        auto ANGLE_LOCAL_VAR = command;                    \
        if (ANGLE_UNLIKELY(ANGLE_LOCAL_VAR != VK_SUCCESS)) \
        {                                                  \
            return ANGLE_LOCAL_VAR;                        \
        }                                                  \
    } while (0)

#define VK_RESULT_CHECK(test, error) VK_RESULT_TRY((test) ? VK_SUCCESS : (error))

// NVIDIA uses special formatting for the driver version:
// Major: 10
// Minor: 8
// Sub-minor: 8
// patch: 6
#define ANGLE_VK_VERSION_MAJOR_NVIDIA(version) (((uint32_t)(version) >> 22) & 0x3ff)
#define ANGLE_VK_VERSION_MINOR_NVIDIA(version) (((uint32_t)(version) >> 14) & 0xff)
#define ANGLE_VK_VERSION_SUB_MINOR_NVIDIA(version) (((uint32_t)(version) >> 6) & 0xff)
#define ANGLE_VK_VERSION_PATCH_NVIDIA(version) ((uint32_t)(version) & 0x3f)

// Similarly for Intel on Windows:
// Major: 18
// Minor: 14
#define ANGLE_VK_VERSION_MAJOR_WIN_INTEL(version) (((uint32_t)(version) >> 14) & 0x3ffff)
#define ANGLE_VK_VERSION_MINOR_WIN_INTEL(version) ((uint32_t)(version) & 0x3fff)

#endif  // LIBANGLE_RENDERER_VULKAN_VK_UTILS_H_
