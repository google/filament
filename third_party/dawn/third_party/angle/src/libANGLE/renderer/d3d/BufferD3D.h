//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferD3D.h: Defines the rx::BufferD3D class, an implementation of BufferImpl.

#ifndef LIBANGLE_RENDERER_D3D_BUFFERD3D_H_
#define LIBANGLE_RENDERER_D3D_BUFFERD3D_H_

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/BufferImpl.h"

#include <stdint.h>
#include <vector>

namespace gl
{
struct VertexAttribute;
class VertexBinding;
}  // namespace gl

namespace rx
{
class BufferFactoryD3D;
class StaticIndexBufferInterface;
class StaticVertexBufferInterface;

enum class D3DBufferUsage
{
    STATIC,
    DYNAMIC,
};

class BufferD3D : public BufferImpl
{
  public:
    BufferD3D(const gl::BufferState &state, BufferFactoryD3D *factory);
    ~BufferD3D() override;

    unsigned int getSerial() const { return mSerial; }

    virtual size_t getSize() const                                                     = 0;
    virtual bool supportsDirectBinding() const                                         = 0;
    virtual angle::Result markTransformFeedbackUsage(const gl::Context *context)       = 0;
    virtual angle::Result getData(const gl::Context *context, const uint8_t **outData) = 0;

    // Warning: you should ensure binding really matches attrib.bindingIndex before using this
    // function.
    StaticVertexBufferInterface *getStaticVertexBuffer(const gl::VertexAttribute &attribute,
                                                       const gl::VertexBinding &binding);
    StaticIndexBufferInterface *getStaticIndexBuffer();

    virtual void initializeStaticData(const gl::Context *context);
    virtual void invalidateStaticData(const gl::Context *context);

    void promoteStaticUsage(const gl::Context *context, size_t dataSize);

    angle::Result getIndexRange(const gl::Context *context,
                                gl::DrawElementsType type,
                                size_t offset,
                                size_t count,
                                bool primitiveRestartEnabled,
                                gl::IndexRange *outRange) override;

    BufferFactoryD3D *getFactory() const { return mFactory; }
    D3DBufferUsage getUsage() const { return mUsage; }

  protected:
    void updateSerial();
    void updateD3DBufferUsage(const gl::Context *context, gl::BufferUsage usage);
    void emptyStaticBufferCache();

    BufferFactoryD3D *mFactory;
    unsigned int mSerial;
    static unsigned int mNextSerial;

    std::vector<std::unique_ptr<StaticVertexBufferInterface>> mStaticVertexBuffers;
    StaticIndexBufferInterface *mStaticIndexBuffer;
    unsigned int mStaticBufferCacheTotalSize;
    unsigned int mStaticVertexBufferOutOfDate;
    size_t mUnmodifiedDataUse;
    D3DBufferUsage mUsage;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_BUFFERD3D_H_
