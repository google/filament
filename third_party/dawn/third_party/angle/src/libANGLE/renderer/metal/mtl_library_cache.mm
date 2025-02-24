//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_library_cache.mm:
//    Defines classes for caching of mtl libraries
//

#include "libANGLE/renderer/metal/mtl_library_cache.h"

#include <stdio.h>

#include <limits>

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "common/hash_utils.h"
#include "common/mathutil.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/process.h"
#include "platform/PlatformMethods.h"

namespace rx
{
namespace mtl
{

LibraryCache::LibraryCache() : mCache(kMaxCachedLibraries) {}

AutoObjCPtr<id<MTLLibrary>> LibraryCache::get(const std::shared_ptr<const std::string> &source,
                                              const std::map<std::string, std::string> &macros,
                                              bool disableFastMath,
                                              bool usesInvariance)
{
    ASSERT(source != nullptr);
    LibraryCache::LibraryCacheEntry &entry =
        getCacheEntry(LibraryKey(source, macros, disableFastMath, usesInvariance));

    // Try to lock the entry and return the library if it exists. If we can't lock then it means
    // another thread is currently compiling.
    std::unique_lock<std::mutex> entryLockGuard(entry.lock, std::try_to_lock);
    if (entryLockGuard)
    {
        return entry.library;
    }
    else
    {
        return nil;
    }
}

namespace
{

// Reads a metallib file at the specified path.
angle::MemoryBuffer ReadMetallibFromFile(const std::string &path)
{
    // TODO: optimize this to avoid the unnecessary strings.
    std::string metallib;
    if (!angle::ReadFileToString(path, &metallib))
    {
        FATAL() << "Failed reading back metallib";
    }

    angle::MemoryBuffer buffer;
    if (!buffer.resize(metallib.size()))
    {
        FATAL() << "Failed to resize metallib buffer";
    }
    memcpy(buffer.data(), metallib.data(), metallib.size());
    return buffer;
}

// Generates a key for the BlobCache based on the specified params.
egl::BlobCache::Key GenerateBlobCacheKeyForShaderLibrary(
    const std::shared_ptr<const std::string> &source,
    const std::map<std::string, std::string> &macros,
    bool disableFastMath,
    bool usesInvariance)
{
    angle::base::SecureHashAlgorithm sha1;
    sha1.Update(source->c_str(), source->size());
    const size_t macro_count = macros.size();
    sha1.Update(&macro_count, sizeof(size_t));
    for (const auto &macro : macros)
    {
        sha1.Update(macro.first.c_str(), macro.first.size());
        sha1.Update(macro.second.c_str(), macro.second.size());
    }
    sha1.Update(&disableFastMath, sizeof(bool));
    sha1.Update(&usesInvariance, sizeof(bool));
    sha1.Final();
    return sha1.DigestAsArray();
}


}  // namespace

AutoObjCPtr<id<MTLLibrary>> LibraryCache::getOrCompileShaderLibrary(
    DisplayMtl *displayMtl,
    const std::shared_ptr<const std::string> &source,
    const std::map<std::string, std::string> &macros,
    bool disableFastMath,
    bool usesInvariance,
    AutoObjCPtr<NSError *> *errorOut)
{
    id<MTLDevice> metalDevice          = displayMtl->getMetalDevice();
    const angle::FeaturesMtl &features = displayMtl->getFeatures();
    if (!features.enableInMemoryMtlLibraryCache.enabled)
    {
        return CreateShaderLibrary(metalDevice, *source, macros, disableFastMath, usesInvariance,
                                   errorOut);
    }

    ASSERT(source != nullptr);
    LibraryCache::LibraryCacheEntry &entry =
        getCacheEntry(LibraryKey(source, macros, disableFastMath, usesInvariance));

    // Lock this cache entry while compiling the shader. This causes other threads calling this
    // function to wait and not duplicate the compilation.
    std::lock_guard<std::mutex> entryLockGuard(entry.lock);
    if (entry.library)
    {
        return entry.library;
    }

    if (features.printMetalShaders.enabled)
    {
        auto cache_key =
            GenerateBlobCacheKeyForShaderLibrary(source, macros, disableFastMath, usesInvariance);
        NSLog(@"Loading metal shader, key=%@ source=%s",
              [NSData dataWithBytes:cache_key.data() length:cache_key.size()], source -> c_str());
    }

    if (features.compileMetalShaders.enabled)
    {
        if (features.enableParallelMtlLibraryCompilation.enabled)
        {
            // When enableParallelMtlLibraryCompilation is enabled, compilation happens in the
            // background. Chrome's ProgramCache only saves to disk when called at certain points,
            // which are not present when compiling in the background.
            FATAL() << "EnableParallelMtlLibraryCompilation is not compatible with "
                       "compileMetalShdaders";
        }
        // Note: there does not seem to be a
        std::string metallib_filename =
            CompileShaderLibraryToFile(*source, macros, disableFastMath, usesInvariance);
        angle::MemoryBuffer memory_buffer = ReadMetallibFromFile(metallib_filename);
        AutoObjCPtr<NSError *> error;
        entry.library = CreateShaderLibraryFromBinary(metalDevice, memory_buffer.data(),
                                                      memory_buffer.size(), &error);
        auto cache_key =
            GenerateBlobCacheKeyForShaderLibrary(source, macros, disableFastMath, usesInvariance);
        displayMtl->getBlobCache()->put(nullptr, cache_key, std::move(memory_buffer));
        return entry.library;
    }

    if (features.loadMetalShadersFromBlobCache.enabled)
    {
        auto cache_key =
            GenerateBlobCacheKeyForShaderLibrary(source, macros, disableFastMath, usesInvariance);
        egl::BlobCache::Value value;
        angle::ScratchBuffer scratch_buffer;
        if (displayMtl->getBlobCache()->get(nullptr, &scratch_buffer, cache_key, &value))
        {
            AutoObjCPtr<NSError *> error;
            entry.library =
                CreateShaderLibraryFromBinary(metalDevice, value.data(), value.size(), &error);
        }
        ANGLE_HISTOGRAM_BOOLEAN("GPU.ANGLE.MetalShaderInBlobCache", entry.library);
        ANGLEPlatformCurrent()->recordShaderCacheUse(entry.library);
        if (entry.library)
        {
            return entry.library;
        }
    }

    entry.library = CreateShaderLibrary(metalDevice, *source, macros, disableFastMath,
                                        usesInvariance, errorOut);
    return entry.library;
}

LibraryCache::LibraryCacheEntry &LibraryCache::getCacheEntry(LibraryKey &&key)
{
    // Lock while searching or adding new items to the cache.
    std::lock_guard<std::mutex> cacheLockGuard(mCacheLock);

    auto iter = mCache.Get(key);
    if (iter != mCache.end())
    {
        return iter->second;
    }

    angle::TrimCache(kMaxCachedLibraries, kGCLimit, "metal library", &mCache);

    iter = mCache.Put(std::move(key), LibraryCacheEntry());
    return iter->second;
}

LibraryCache::LibraryKey::LibraryKey(const std::shared_ptr<const std::string> &sourceIn,
                                     const std::map<std::string, std::string> &macrosIn,
                                     bool disableFastMathIn,
                                     bool usesInvarianceIn)
    : source(sourceIn),
      macros(macrosIn),
      disableFastMath(disableFastMathIn),
      usesInvariance(usesInvarianceIn)
{}

bool LibraryCache::LibraryKey::operator==(const LibraryKey &other) const
{
    return std::tie(*source, macros, disableFastMath, usesInvariance) ==
           std::tie(*other.source, other.macros, other.disableFastMath, other.usesInvariance);
}

size_t LibraryCache::LibraryKeyHasher::operator()(const LibraryKey &k) const
{
    size_t hash = 0;
    angle::HashCombine(hash, *k.source);
    for (const auto &macro : k.macros)
    {
        angle::HashCombine(hash, macro.first);
        angle::HashCombine(hash, macro.second);
    }
    angle::HashCombine(hash, k.disableFastMath);
    angle::HashCombine(hash, k.usesInvariance);
    return hash;
}

LibraryCache::LibraryCacheEntry::~LibraryCacheEntry()
{
    // Lock the cache entry before deletion to ensure there is no other thread compiling and
    // preparing to write to the library. LibraryCacheEntry objects can only be deleted while the
    // mCacheLock is held so only one thread modifies mCache at a time.
    std::lock_guard<std::mutex> entryLockGuard(lock);
}

LibraryCache::LibraryCacheEntry::LibraryCacheEntry(LibraryCacheEntry &&moveFrom)
{
    // Lock the cache entry being moved from to make sure the library can be safely accessed.
    // Mutexes cannot be moved so a new one will be created in this entry
    std::lock_guard<std::mutex> entryLockGuard(moveFrom.lock);

    library          = std::move(moveFrom.library);
    moveFrom.library = nullptr;
}

}  // namespace mtl
}  // namespace rx
