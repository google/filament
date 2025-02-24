//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlobCache: Stores compiled and linked programs in memory so they don't
//   always have to be re-compiled. Can be used in conjunction with the platform
//   layer to warm up the cache from disk.

#ifndef LIBANGLE_BLOB_CACHE_H_
#define LIBANGLE_BLOB_CACHE_H_

#include <array>
#include <cstring>

#include "common/SimpleMutex.h"
#include "libANGLE/Error.h"
#include "libANGLE/SizedMRUCache.h"
#include "libANGLE/angletypes.h"

namespace gl
{
class Context;
}  // namespace gl

namespace egl
{

// Used by MemoryProgramCache and MemoryShaderCache, this result indicates whether program/shader
// cache load from blob was successful.
enum class CacheGetResult
{
    // Binary blob was found and is valid
    Success,
    // Binary blob was not found
    NotFound,
    // Binary blob was found, but was rejected due to errors (corruption, version mismatch, etc)
    Rejected,
};

class BlobCache final : angle::NonCopyable
{
  public:
    // 160-bit SHA-1 hash key used for hasing a program.  BlobCache opts in using fixed keys for
    // simplicity and efficiency.
    static constexpr size_t kKeyLength = angle::kBlobCacheKeyLength;
    using Key                          = angle::BlobCacheKey;
    using Value                        = angle::BlobCacheValue;
    enum class CacheSource
    {
        Memory,
        Disk,
    };

    explicit BlobCache(size_t maxCacheSizeBytes);
    ~BlobCache();

    // Store a key-blob pair in the cache.  If application callbacks are set, the application cache
    // will be used.  Otherwise the value is cached in this object.
    void put(const gl::Context *context, const BlobCache::Key &key, angle::MemoryBuffer &&value);

    // Store a key-blob pair in the cache, but compress the blob before insertion. Returns false if
    // compression fails, returns true otherwise.
    bool compressAndPut(const gl::Context *context,
                        const BlobCache::Key &key,
                        angle::MemoryBuffer &&uncompressedValue,
                        size_t *compressedSize);

    // Store a key-blob pair in the application cache, only if application callbacks are set.
    void putApplication(const gl::Context *context,
                        const BlobCache::Key &key,
                        const angle::MemoryBuffer &value);

    // Store a key-blob pair in the cache without making callbacks to the application.  This is used
    // to repopulate this object's cache on startup without generating callback calls.
    void populate(const BlobCache::Key &key,
                  angle::MemoryBuffer &&value,
                  CacheSource source = CacheSource::Disk);

    // Check if the cache contains the blob corresponding to this key.  If application callbacks are
    // set, those will be used.  Otherwise they key is looked up in this object's cache.
    [[nodiscard]] bool get(const gl::Context *context,
                           angle::ScratchBuffer *scratchBuffer,
                           const BlobCache::Key &key,
                           BlobCache::Value *valueOut);

    // For querying the contents of the cache.
    [[nodiscard]] bool getAt(size_t index,
                             const BlobCache::Key **keyOut,
                             BlobCache::Value *valueOut);

    enum class GetAndDecompressResult
    {
        Success,
        NotFound,
        DecompressFailure,
    };
    [[nodiscard]] GetAndDecompressResult getAndDecompress(
        const gl::Context *context,
        angle::ScratchBuffer *scratchBuffer,
        const BlobCache::Key &key,
        size_t maxUncompressedDataSize,
        angle::MemoryBuffer *uncompressedValueOut);

    // Evict a blob from the binary cache.
    void remove(const BlobCache::Key &key);

    // Empty the cache.
    void clear() { mBlobCache.clear(); }

    // Resize the cache. Discards current contents.
    void resize(size_t maxCacheSizeBytes) { mBlobCache.resize(maxCacheSizeBytes); }

    // Returns the number of entries in the cache.
    size_t entryCount() const { return mBlobCache.entryCount(); }

    // Reduces the current cache size and returns the number of bytes freed.
    size_t trim(size_t limit) { return mBlobCache.shrinkToSize(limit); }

    // Returns the current cache size in bytes.
    size_t size() const { return mBlobCache.size(); }

    // Returns whether the cache is empty
    bool empty() const { return mBlobCache.empty(); }

    // Returns the maximum cache size in bytes.
    size_t maxSize() const { return mBlobCache.maxSize(); }

    void setBlobCacheFuncs(EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get);

    bool areBlobCacheFuncsSet() const;

    bool isCachingEnabled(const gl::Context *context) const;

    angle::SimpleMutex &getMutex() { return mBlobCacheMutex; }

  private:
    size_t callBlobGetCallback(const gl::Context *context,
                               const void *key,
                               size_t keySize,
                               void *value,
                               size_t valueSize);

    // This internal cache is used only if the application is not providing caching callbacks
    using CacheEntry = std::pair<angle::MemoryBuffer, CacheSource>;

    mutable angle::SimpleMutex mBlobCacheMutex;
    angle::SizedMRUCache<BlobCache::Key, CacheEntry> mBlobCache;

    EGLSetBlobFuncANDROID mSetBlobFunc;
    EGLGetBlobFuncANDROID mGetBlobFunc;
};

}  // namespace egl

#endif  // LIBANGLE_MEMORY_PROGRAM_CACHE_H_
