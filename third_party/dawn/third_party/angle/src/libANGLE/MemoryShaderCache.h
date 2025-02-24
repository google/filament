//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryShaderCache: Stores compiled shaders in memory so they don't
//   always have to be re-compiled. Can be used in conjunction with the platform
//   layer to warm up the cache from disk.

#ifndef LIBANGLE_MEMORY_SHADER_CACHE_H_
#define LIBANGLE_MEMORY_SHADER_CACHE_H_

#include <array>

#include "GLSLANG/ShaderLang.h"
#include "common/MemoryBuffer.h"
#include "libANGLE/BlobCache.h"
#include "libANGLE/Error.h"

namespace gl
{
class Context;
class Shader;
class ShaderState;
class ShCompilerInstance;

class MemoryShaderCache final : angle::NonCopyable
{
  public:
    explicit MemoryShaderCache(egl::BlobCache &blobCache);
    ~MemoryShaderCache();

    // Helper method that serializes a shader.
    angle::Result putShader(const Context *context,
                            const egl::BlobCache::Key &shaderHash,
                            const Shader *shader);

    // Check the cache, and deserialize and load the shader if found. Evict existing hash if load
    // fails.
    egl::CacheGetResult getShader(const Context *context,
                                  Shader *shader,
                                  const egl::BlobCache::Key &shaderHash,
                                  angle::JobResultExpectancy resultExpectancy);

    // Empty the cache.
    void clear();

    // Returns the maximum cache size in bytes.
    size_t maxSize() const;

  private:
    egl::BlobCache &mBlobCache;
};

}  // namespace gl

#endif  // LIBANGLE_MEMORY_SHADER_CACHE_H_
