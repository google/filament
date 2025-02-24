//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexDataManager.h: Defines the IndexDataManager, a class that
// runs the Buffer translation process for index buffers.

#ifndef LIBANGLE_INDEXDATAMANAGER_H_
#define LIBANGLE_INDEXDATAMANAGER_H_

#include <GLES2/gl2.h>

#include "common/angleutils.h"
#include "common/mathutil.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace
{
enum
{
    INITIAL_INDEX_BUFFER_SIZE = 4096 * sizeof(GLuint)
};
}

namespace gl
{
class Buffer;
}

namespace rx
{
class IndexBufferInterface;
class StaticIndexBufferInterface;
class StreamingIndexBufferInterface;
class IndexBuffer;
class BufferD3D;
class RendererD3D;

struct SourceIndexData
{
    BufferD3D *srcBuffer;
    const void *srcIndices;
    unsigned int srcCount;
    gl::DrawElementsType srcIndexType;
    bool srcIndicesChanged;
};

struct TranslatedIndexData
{
    unsigned int startIndex;
    unsigned int startOffset;  // In bytes

    IndexBuffer *indexBuffer;
    BufferD3D *storage;
    gl::DrawElementsType indexType;
    unsigned int serial;

    SourceIndexData srcIndexData;
};

class IndexDataManager : angle::NonCopyable
{
  public:
    explicit IndexDataManager(BufferFactoryD3D *factory);
    virtual ~IndexDataManager();

    void deinitialize();

    angle::Result prepareIndexData(const gl::Context *context,
                                   gl::DrawElementsType srcType,
                                   gl::DrawElementsType dstType,
                                   GLsizei count,
                                   gl::Buffer *glBuffer,
                                   const void *indices,
                                   TranslatedIndexData *translated);

  private:
    angle::Result streamIndexData(const gl::Context *context,
                                  const void *data,
                                  unsigned int count,
                                  gl::DrawElementsType srcType,
                                  gl::DrawElementsType dstType,
                                  bool usePrimitiveRestartFixedIndex,
                                  TranslatedIndexData *translated);
    angle::Result getStreamingIndexBuffer(const gl::Context *context,
                                          gl::DrawElementsType destinationIndexType,
                                          IndexBufferInterface **outBuffer);

    using StreamingBuffer = std::unique_ptr<StreamingIndexBufferInterface>;

    BufferFactoryD3D *const mFactory;
    std::unique_ptr<StreamingIndexBufferInterface> mStreamingBufferShort;
    std::unique_ptr<StreamingIndexBufferInterface> mStreamingBufferInt;
};

angle::Result GetIndexTranslationDestType(const gl::Context *context,
                                          GLsizei indexCount,
                                          gl::DrawElementsType indexType,
                                          const void *indices,
                                          bool usePrimitiveRestartWorkaround,
                                          gl::DrawElementsType *destTypeOut);

ANGLE_INLINE bool IsOffsetAligned(gl::DrawElementsType elementType, unsigned int offset)
{
    return (offset % gl::GetDrawElementsTypeSize(elementType) == 0);
}
}  // namespace rx

#endif  // LIBANGLE_INDEXDATAMANAGER_H_
