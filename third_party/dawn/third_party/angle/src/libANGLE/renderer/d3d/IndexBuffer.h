//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBuffer.h: Defines the abstract IndexBuffer class and IndexBufferInterface
// class with derivations, classes that perform graphics API agnostic index buffer operations.

#ifndef LIBANGLE_RENDERER_D3D_INDEXBUFFER_H_
#define LIBANGLE_RENDERER_D3D_INDEXBUFFER_H_

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"

namespace gl
{
class Context;
}  // namespace gl

namespace rx
{
class BufferFactoryD3D;

class IndexBuffer : angle::NonCopyable
{
  public:
    IndexBuffer();
    virtual ~IndexBuffer();

    virtual angle::Result initialize(const gl::Context *context,
                                     unsigned int bufferSize,
                                     gl::DrawElementsType indexType,
                                     bool dynamic) = 0;

    virtual angle::Result mapBuffer(const gl::Context *context,
                                    unsigned int offset,
                                    unsigned int size,
                                    void **outMappedMemory)       = 0;
    virtual angle::Result unmapBuffer(const gl::Context *context) = 0;

    virtual angle::Result discard(const gl::Context *context) = 0;

    virtual gl::DrawElementsType getIndexType() const             = 0;
    virtual unsigned int getBufferSize() const                    = 0;
    virtual angle::Result setSize(const gl::Context *context,
                                  unsigned int bufferSize,
                                  gl::DrawElementsType indexType) = 0;

    unsigned int getSerial() const;

  protected:
    void updateSerial();

  private:
    unsigned int mSerial;
    static unsigned int mNextSerial;
};

class IndexBufferInterface : angle::NonCopyable
{
  public:
    IndexBufferInterface(BufferFactoryD3D *factory, bool dynamic);
    virtual ~IndexBufferInterface();

    virtual angle::Result reserveBufferSpace(const gl::Context *context,
                                             unsigned int size,
                                             gl::DrawElementsType indexType) = 0;

    gl::DrawElementsType getIndexType() const;
    unsigned int getBufferSize() const;

    unsigned int getSerial() const;

    angle::Result mapBuffer(const gl::Context *context,
                            unsigned int size,
                            void **outMappedMemory,
                            unsigned int *streamOffset);
    angle::Result unmapBuffer(const gl::Context *context);

    IndexBuffer *getIndexBuffer() const;

  protected:
    unsigned int getWritePosition() const;
    void setWritePosition(unsigned int writePosition);

    angle::Result discard(const gl::Context *context);

    angle::Result setBufferSize(const gl::Context *context,
                                unsigned int bufferSize,
                                gl::DrawElementsType indexType);

  private:
    IndexBuffer *mIndexBuffer;

    unsigned int mWritePosition;
    bool mDynamic;
};

class StreamingIndexBufferInterface : public IndexBufferInterface
{
  public:
    explicit StreamingIndexBufferInterface(BufferFactoryD3D *factory);
    ~StreamingIndexBufferInterface() override;

    angle::Result reserveBufferSpace(const gl::Context *context,
                                     unsigned int size,
                                     gl::DrawElementsType indexType) override;
};

class StaticIndexBufferInterface : public IndexBufferInterface
{
  public:
    explicit StaticIndexBufferInterface(BufferFactoryD3D *factory);
    ~StaticIndexBufferInterface() override;

    angle::Result reserveBufferSpace(const gl::Context *context,
                                     unsigned int size,
                                     gl::DrawElementsType indexType) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_INDEXBUFFER_H_
