//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InputLayoutCache.h: Defines InputLayoutCache, a class that builds and caches
// D3D11 input layouts.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_
#define LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_

#include <GLES2/gl2.h>

#include <cstddef>

#include <array>
#include <map>

#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/SizedMRUCache.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"

namespace rx
{
struct PackedAttributeLayout
{
    PackedAttributeLayout();
    PackedAttributeLayout(const PackedAttributeLayout &other);

    void addAttributeData(GLenum glType,
                          UINT semanticIndex,
                          angle::FormatID vertexFormatID,
                          unsigned int divisor);

    bool operator==(const PackedAttributeLayout &other) const;

    uint32_t numAttributes;
    gl::AttribArray<uint64_t> attributeData;
};
}  // namespace rx

namespace std
{
template <>
struct hash<rx::PackedAttributeLayout>
{
    size_t operator()(const rx::PackedAttributeLayout &value) const
    {
        return angle::ComputeGenericHash(value);
    }
};
}  // namespace std

namespace gl
{
class Program;
}  // namespace gl

namespace rx
{
class Context11;
struct TranslatedAttribute;
struct TranslatedIndexData;
struct SourceIndexData;
class ProgramD3D;
class Renderer11;

class InputLayoutCache : angle::NonCopyable
{
  public:
    InputLayoutCache();
    ~InputLayoutCache();

    void clear();

    // Useful for testing
    void setCacheSize(size_t newCacheSize);

    angle::Result getInputLayout(Context11 *context,
                                 const gl::State &state,
                                 const std::vector<const TranslatedAttribute *> &currentAttributes,
                                 const AttribIndexArray &sortedSemanticIndices,
                                 gl::PrimitiveMode mode,
                                 GLsizei vertexCount,
                                 GLsizei instances,
                                 const d3d11::InputLayout **inputLayoutOut);

  private:
    angle::Result createInputLayout(
        Context11 *context11,
        const AttribIndexArray &sortedSemanticIndices,
        const std::vector<const TranslatedAttribute *> &currentAttributes,
        gl::PrimitiveMode mode,
        GLsizei vertexCount,
        GLsizei instances,
        d3d11::InputLayout *inputLayoutOut);

    // Starting cache size.
    static constexpr size_t kDefaultCacheSize = 1024;

    // The cache tries to clean up this many states at once.
    static constexpr size_t kGCLimit = 128;

    using LayoutCache = angle::base::HashingMRUCache<PackedAttributeLayout, d3d11::InputLayout>;
    LayoutCache mLayoutCache;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_
