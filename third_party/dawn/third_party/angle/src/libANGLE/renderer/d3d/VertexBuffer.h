//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer.h: Defines the abstract VertexBuffer class and VertexBufferInterface
// class with derivations, classes that perform graphics API agnostic vertex buffer operations.

#ifndef LIBANGLE_RENDERER_D3D_VERTEXBUFFER_H_
#define LIBANGLE_RENDERER_D3D_VERTEXBUFFER_H_

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/Format.h"

#include <GLES2/gl2.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace gl
{
class Context;
struct VertexAttribute;
class VertexBinding;
struct VertexAttribCurrentValueData;
}  // namespace gl

namespace rx
{
class BufferFactoryD3D;

// Use a ref-counting scheme with self-deletion on release. We do this so that we can more
// easily manage the static buffer cache, without deleting currently bound buffers.
class VertexBuffer : angle::NonCopyable
{
  public:
    VertexBuffer();

    virtual angle::Result initialize(const gl::Context *context,
                                     unsigned int size,
                                     bool dynamicUsage) = 0;

    // Warning: you should ensure binding really matches attrib.bindingIndex before using this
    // function.
    virtual angle::Result storeVertexAttributes(const gl::Context *context,
                                                const gl::VertexAttribute &attrib,
                                                const gl::VertexBinding &binding,
                                                gl::VertexAttribType currentValueType,
                                                GLint start,
                                                size_t count,
                                                GLsizei instances,
                                                unsigned int offset,
                                                const uint8_t *sourceData) = 0;

    virtual unsigned int getBufferSize() const                                         = 0;
    virtual angle::Result setBufferSize(const gl::Context *context, unsigned int size) = 0;
    virtual angle::Result discard(const gl::Context *context)                          = 0;

    unsigned int getSerial() const;

    // This may be overridden (e.g. by VertexBuffer11) if necessary.
    virtual void hintUnmapResource() {}

    // Reference counting.
    void addRef();
    void release();

  protected:
    void updateSerial();
    virtual ~VertexBuffer();

  private:
    unsigned int mSerial;
    static unsigned int mNextSerial;
    unsigned int mRefCount;
};

class VertexBufferInterface : angle::NonCopyable
{
  public:
    VertexBufferInterface(BufferFactoryD3D *factory, bool dynamic);
    virtual ~VertexBufferInterface();

    unsigned int getBufferSize() const;
    bool empty() const { return getBufferSize() == 0; }

    unsigned int getSerial() const;

    VertexBuffer *getVertexBuffer() const;

  protected:
    angle::Result discard(const gl::Context *context);

    angle::Result setBufferSize(const gl::Context *context, unsigned int size);

    angle::Result getSpaceRequired(const gl::Context *context,
                                   const gl::VertexAttribute &attrib,
                                   const gl::VertexBinding &binding,
                                   size_t count,
                                   GLsizei instances,
                                   GLuint baseInstance,
                                   unsigned int *spaceInBytesOut) const;
    BufferFactoryD3D *const mFactory;
    VertexBuffer *mVertexBuffer;
    bool mDynamic;
};

class StreamingVertexBufferInterface : public VertexBufferInterface
{
  public:
    StreamingVertexBufferInterface(BufferFactoryD3D *factory);
    ~StreamingVertexBufferInterface() override;

    angle::Result initialize(const gl::Context *context, std::size_t initialSize);
    void reset();

    angle::Result storeDynamicAttribute(const gl::Context *context,
                                        const gl::VertexAttribute &attrib,
                                        const gl::VertexBinding &binding,
                                        gl::VertexAttribType currentValueType,
                                        GLint start,
                                        size_t count,
                                        GLsizei instances,
                                        GLuint baseInstance,
                                        unsigned int *outStreamOffset,
                                        const uint8_t *sourceData);

    angle::Result reserveVertexSpace(const gl::Context *context,
                                     const gl::VertexAttribute &attribute,
                                     const gl::VertexBinding &binding,
                                     size_t count,
                                     GLsizei instances,
                                     GLuint baseInstance);

  private:
    angle::Result reserveSpace(const gl::Context *context, unsigned int size);

    unsigned int mWritePosition;
    unsigned int mReservedSpace;
};

class StaticVertexBufferInterface : public VertexBufferInterface
{
  public:
    explicit StaticVertexBufferInterface(BufferFactoryD3D *factory);
    ~StaticVertexBufferInterface() override;

    // Warning: you should ensure binding really matches attrib.bindingIndex before using these
    // functions.
    angle::Result storeStaticAttribute(const gl::Context *context,
                                       const gl::VertexAttribute &attrib,
                                       const gl::VertexBinding &binding,
                                       GLint start,
                                       GLsizei count,
                                       GLsizei instances,
                                       const uint8_t *sourceData);

    bool matchesAttribute(const gl::VertexAttribute &attribute,
                          const gl::VertexBinding &binding) const;

    void setAttribute(const gl::VertexAttribute &attribute, const gl::VertexBinding &binding);

  private:
    class AttributeSignature final : angle::NonCopyable
    {
      public:
        AttributeSignature();

        bool matchesAttribute(const gl::VertexAttribute &attrib,
                              const gl::VertexBinding &binding) const;

        void set(const gl::VertexAttribute &attrib, const gl::VertexBinding &binding);

      private:
        angle::FormatID formatID;
        GLuint stride;
        size_t offset;
    };

    AttributeSignature mSignature;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_VERTEXBUFFER_H_
