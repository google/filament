//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer11.h: Defines the rx::Buffer11 class which implements rx::BufferImpl via rx::BufferD3D.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_

#include <array>
#include <map>

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace gl
{
class FramebufferAttachment;
}

namespace rx
{
struct PackPixelsParams;
class Renderer11;
struct SourceIndexData;
struct TranslatedAttribute;

// The order of this enum governs priority of 'getLatestBufferStorage'.
enum BufferUsage
{
    BUFFER_USAGE_SYSTEM_MEMORY,
    BUFFER_USAGE_STAGING,
    BUFFER_USAGE_VERTEX_OR_TRANSFORM_FEEDBACK,
    BUFFER_USAGE_INDEX,
    BUFFER_USAGE_INDIRECT,
    BUFFER_USAGE_PIXEL_UNPACK,
    BUFFER_USAGE_PIXEL_PACK,
    BUFFER_USAGE_UNIFORM,
    BUFFER_USAGE_STRUCTURED,
    BUFFER_USAGE_RAW_UAV,
    BUFFER_USAGE_TYPED_UAV,

    BUFFER_USAGE_COUNT,
};

typedef size_t DataRevision;

class Buffer11 : public BufferD3D
{
  public:
    Buffer11(const gl::BufferState &state, Renderer11 *renderer);
    ~Buffer11() override;

    angle::Result getBuffer(const gl::Context *context,
                            BufferUsage usage,
                            ID3D11Buffer **bufferOut);
    angle::Result getConstantBufferRange(const gl::Context *context,
                                         GLintptr offset,
                                         GLsizeiptr size,
                                         const d3d11::Buffer **bufferOut,
                                         UINT *firstConstantOut,
                                         UINT *numConstantsOut);
    angle::Result getStructuredBufferRangeSRV(const gl::Context *context,
                                              unsigned int offset,
                                              unsigned int size,
                                              unsigned int structureByteStride,
                                              const d3d11::ShaderResourceView **srvOut);
    angle::Result getSRV(const gl::Context *context,
                         DXGI_FORMAT srvFormat,
                         const d3d11::ShaderResourceView **srvOut);
    angle::Result getRawUAVRange(const gl::Context *context,
                                 GLintptr offset,
                                 GLsizeiptr size,
                                 d3d11::UnorderedAccessView **uavOut);

    angle::Result getTypedUAVRange(const gl::Context *context,
                                   GLintptr offset,
                                   GLsizeiptr size,
                                   DXGI_FORMAT format,
                                   d3d11::UnorderedAccessView **uavOut);

    angle::Result markRawBufferUsage(const gl::Context *context);
    angle::Result markTypedBufferUsage(const gl::Context *context);
    bool isMapped() const { return mMappedStorage != nullptr; }
    angle::Result packPixels(const gl::Context *context,
                             const gl::FramebufferAttachment &readAttachment,
                             const PackPixelsParams &params);
    size_t getTotalCPUBufferMemoryBytes() const;

    // BufferD3D implementation
    size_t getSize() const override;
    bool supportsDirectBinding() const override;
    angle::Result getData(const gl::Context *context, const uint8_t **outData) override;
    void initializeStaticData(const gl::Context *context) override;
    void invalidateStaticData(const gl::Context *context) override;

    // BufferImpl implementation
    angle::Result setData(const gl::Context *context,
                          gl::BufferBinding target,
                          const void *data,
                          size_t size,
                          gl::BufferUsage usage) override;
    angle::Result setSubData(const gl::Context *context,
                             gl::BufferBinding target,
                             const void *data,
                             size_t size,
                             size_t offset) override;
    angle::Result copySubData(const gl::Context *context,
                              BufferImpl *source,
                              GLintptr sourceOffset,
                              GLintptr destOffset,
                              GLsizeiptr size) override;
    angle::Result map(const gl::Context *context, GLenum access, void **mapPtr) override;
    angle::Result mapRange(const gl::Context *context,
                           size_t offset,
                           size_t length,
                           GLbitfield access,
                           void **mapPtr) override;
    angle::Result unmap(const gl::Context *context, GLboolean *result) override;
    angle::Result markTransformFeedbackUsage(const gl::Context *context) override;

  private:
    class BufferStorage;
    class EmulatedIndexedStorage;
    class NativeStorage;
    class PackStorage;
    class SystemMemoryStorage;
    class StructuredBufferStorage;

    struct BufferCacheEntry
    {
        BufferCacheEntry() : storage(nullptr), lruCount(0) {}

        BufferStorage *storage;
        unsigned int lruCount;
    };

    struct StructuredBufferKey
    {
        StructuredBufferKey(unsigned int offsetIn, unsigned int structureByteStrideIn)
            : offset(offsetIn), structureByteStride(structureByteStrideIn)
        {}
        bool operator<(const StructuredBufferKey &rhs) const
        {
            return std::tie(offset, structureByteStride) <
                   std::tie(rhs.offset, rhs.structureByteStride);
        }
        unsigned int offset;
        unsigned int structureByteStride;
    };

    void markBufferUsage(BufferUsage usage);
    angle::Result markBufferUsage(const gl::Context *context, BufferUsage usage);
    angle::Result garbageCollection(const gl::Context *context, BufferUsage currentUsage);

    angle::Result updateBufferStorage(const gl::Context *context,
                                      BufferStorage *storage,
                                      size_t sourceOffset,
                                      size_t storageSize);

    angle::Result getNativeStorageForUAV(const gl::Context *context,
                                         Buffer11::NativeStorage **storageOut);

    template <typename StorageOutT>
    angle::Result getBufferStorage(const gl::Context *context,
                                   BufferUsage usage,
                                   StorageOutT **storageOut);

    template <typename StorageOutT>
    angle::Result getStagingStorage(const gl::Context *context, StorageOutT **storageOut);

    angle::Result getLatestBufferStorage(const gl::Context *context,
                                         BufferStorage **storageOut) const;

    angle::Result getConstantBufferRangeStorage(const gl::Context *context,
                                                GLintptr offset,
                                                GLsizeiptr size,
                                                NativeStorage **storageOut);

    BufferStorage *allocateStorage(BufferUsage usage);
    void updateDeallocThreshold(BufferUsage usage);

    // Free the storage if we decide it isn't being used very often.
    angle::Result checkForDeallocation(const gl::Context *context, BufferUsage usage);

    // For some cases of uniform buffer storage, we can't deallocate system memory storage.
    bool canDeallocateSystemMemory() const;

    // Updates data revisions and latest storage.
    void onCopyStorage(BufferStorage *dest, BufferStorage *source);
    void onStorageUpdate(BufferStorage *updatedStorage);

    Renderer11 *mRenderer;
    size_t mSize;

    BufferStorage *mMappedStorage;

    // Buffer storages are sorted by usage. It's important that the latest buffer storage picks
    // the lowest usage in the case where two storages are tied on data revision - this ensures
    // we never do anything dangerous like map a uniform buffer over a staging or system memory
    // copy.
    std::array<BufferStorage *, BUFFER_USAGE_COUNT> mBufferStorages;
    BufferStorage *mLatestBufferStorage;

    // These two arrays are used to track when to free unused storage.
    std::array<unsigned int, BUFFER_USAGE_COUNT> mDeallocThresholds;
    std::array<unsigned int, BUFFER_USAGE_COUNT> mIdleness;

    // Cache of D3D11 constant buffer for specific ranges of buffer data.
    // This is used to emulate UBO ranges on 11.0 devices.
    // Constant buffers are indexed by there start offset.
    typedef std::map<GLintptr /*offset*/, BufferCacheEntry> BufferCache;
    BufferCache mConstantBufferRangeStoragesCache;
    size_t mConstantBufferStorageAdditionalSize;
    unsigned int mMaxConstantBufferLruCount;

    typedef std::map<StructuredBufferKey, BufferCacheEntry> StructuredBufferCache;
    StructuredBufferCache mStructuredBufferRangeStoragesCache;
    size_t mStructuredBufferStorageAdditionalSize;
    unsigned int mMaxStructuredBufferLruCount;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_BUFFER11_H_
