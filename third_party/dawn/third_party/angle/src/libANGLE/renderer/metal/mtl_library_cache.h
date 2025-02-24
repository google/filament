//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_library_cache.h:
//    Defines classes for caching of mtl libraries
//

#ifndef LIBANGLE_RENDERER_METAL_MTL_LIBRARY_CACHE_H_
#define LIBANGLE_RENDERER_METAL_MTL_LIBRARY_CACHE_H_

#include "libANGLE/renderer/metal/mtl_utils.h"

#include <type_traits>

namespace rx
{
namespace mtl
{

class LibraryCache : angle::NonCopyable
{
  public:
    LibraryCache();

    AutoObjCPtr<id<MTLLibrary>> get(const std::shared_ptr<const std::string> &source,
                                    const std::map<std::string, std::string> &macros,
                                    bool disableFastMath,
                                    bool usesInvariance);
    AutoObjCPtr<id<MTLLibrary>> getOrCompileShaderLibrary(
        DisplayMtl *displayMtl,
        const std::shared_ptr<const std::string> &source,
        const std::map<std::string, std::string> &macros,
        bool disableFastMath,
        bool usesInvariance,
        AutoObjCPtr<NSError *> *errorOut);

  private:
    struct LibraryKey
    {
        LibraryKey() = default;
        LibraryKey(const std::shared_ptr<const std::string> &source,
                   const std::map<std::string, std::string> &macros,
                   bool disableFastMath,
                   bool usesInvariance);

        std::shared_ptr<const std::string> source;
        std::map<std::string, std::string> macros;
        bool disableFastMath;
        bool usesInvariance;

        bool operator==(const LibraryKey &other) const;
    };

    struct LibraryKeyHasher
    {
        // Hash function
        size_t operator()(const LibraryKey &k) const;

        // Comparison
        bool operator()(const LibraryKey &a, const LibraryKey &b) const;
    };

    struct LibraryCacheEntry : angle::NonCopyable
    {
        LibraryCacheEntry() = default;
        ~LibraryCacheEntry();
        LibraryCacheEntry(LibraryCacheEntry &&moveFrom);

        // library can only go from the null -> not null state. It is safe to check if the library
        // already exists without locking.
        AutoObjCPtr<id<MTLLibrary>> library;

        // Lock for this specific library to avoid multiple threads compiling the same shader at
        // once.
        std::mutex lock;
    };

    LibraryCacheEntry &getCacheEntry(LibraryKey &&key);

    static constexpr unsigned int kMaxCachedLibraries = 128;

    // The cache tries to clean up this many states at once.
    static constexpr unsigned int kGCLimit = 32;

    // Lock for searching and adding new entries to the cache
    std::mutex mCacheLock;

    using CacheMap = angle::base::HashingMRUCache<LibraryKey, LibraryCacheEntry, LibraryKeyHasher>;
    CacheMap mCache;
};

}  // namespace mtl
}  // namespace rx

#endif  // LIBANGLE_RENDERER_METAL_MTL_LIBRARY_CACHE_H_
