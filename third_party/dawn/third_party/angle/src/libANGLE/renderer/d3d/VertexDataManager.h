//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexDataManager.h: Defines the VertexDataManager, a class that
// runs the Buffer translation process.

#ifndef LIBANGLE_RENDERER_D3D_VERTEXDATAMANAGER_H_
#define LIBANGLE_RENDERER_D3D_VERTEXDATAMANAGER_H_

#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/VertexBuffer.h"

namespace gl
{
class State;
struct VertexAttribute;
class VertexBinding;
struct VertexAttribCurrentValueData;
}  // namespace gl

namespace rx
{
class BufferD3D;
class BufferFactoryD3D;
class StreamingVertexBufferInterface;
class VertexBuffer;

class VertexBufferBinding final
{
  public:
    VertexBufferBinding();
    VertexBufferBinding(const VertexBufferBinding &other);
    ~VertexBufferBinding();

    void set(VertexBuffer *vertexBuffer);
    VertexBuffer *get() const;
    VertexBufferBinding &operator=(const VertexBufferBinding &other);

  private:
    VertexBuffer *mBoundVertexBuffer;
};

struct TranslatedAttribute
{
    TranslatedAttribute();
    TranslatedAttribute(const TranslatedAttribute &other);

    // Computes the correct offset from baseOffset, usesFirstVertexOffset, stride and startVertex.
    // Can throw an error on integer overflow.
    angle::Result computeOffset(const gl::Context *context,
                                GLint startVertex,
                                unsigned int *offsetOut) const;

    bool active;

    const gl::VertexAttribute *attribute;
    const gl::VertexBinding *binding;
    gl::VertexAttribType currentValueType;
    unsigned int baseOffset;
    bool usesFirstVertexOffset;
    unsigned int stride;  // 0 means not to advance the read pointer at all

    VertexBufferBinding vertexBuffer;
    BufferD3D *storage;
    unsigned int serial;
    unsigned int divisor;
};

enum class VertexStorageType
{
    UNKNOWN,
    STATIC,         // Translate the vertex data once and re-use it.
    DYNAMIC,        // Translate the data every frame into a ring buffer.
    DIRECT,         // Bind a D3D buffer directly without any translation.
    CURRENT_VALUE,  // Use a single value for the attribute.
};

// Given a vertex attribute, return the type of storage it will use.
VertexStorageType ClassifyAttributeStorage(const gl::Context *context,
                                           const gl::VertexAttribute &attrib,
                                           const gl::VertexBinding &binding);

class VertexDataManager : angle::NonCopyable
{
  public:
    VertexDataManager(BufferFactoryD3D *factory);
    virtual ~VertexDataManager();

    angle::Result initialize(const gl::Context *context);
    void deinitialize();

    angle::Result prepareVertexData(const gl::Context *context,
                                    GLint start,
                                    GLsizei count,
                                    std::vector<TranslatedAttribute> *translatedAttribs,
                                    GLsizei instances);

    static void StoreDirectAttrib(const gl::Context *context, TranslatedAttribute *directAttrib);

    static angle::Result StoreStaticAttrib(const gl::Context *context,
                                           TranslatedAttribute *translated);

    angle::Result storeDynamicAttribs(const gl::Context *context,
                                      std::vector<TranslatedAttribute> *translatedAttribs,
                                      const gl::AttributesMask &dynamicAttribsMask,
                                      GLint start,
                                      size_t count,
                                      GLsizei instances,
                                      GLuint baseInstance);

    // Promote static usage of dynamic buffers.
    static void PromoteDynamicAttribs(const gl::Context *context,
                                      const std::vector<TranslatedAttribute> &translatedAttribs,
                                      const gl::AttributesMask &dynamicAttribsMask,
                                      size_t count);

    angle::Result storeCurrentValue(const gl::Context *context,
                                    const gl::VertexAttribCurrentValueData &currentValue,
                                    TranslatedAttribute *translated,
                                    size_t attribIndex);

  private:
    struct CurrentValueState final : angle::NonCopyable
    {
        CurrentValueState(BufferFactoryD3D *factory);
        CurrentValueState(CurrentValueState &&other);
        ~CurrentValueState();

        std::unique_ptr<StreamingVertexBufferInterface> buffer;
        gl::VertexAttribCurrentValueData data;
        size_t offset;
    };

    angle::Result reserveSpaceForAttrib(const gl::Context *context,
                                        const TranslatedAttribute &translatedAttrib,
                                        GLint start,
                                        size_t count,
                                        GLsizei instances,
                                        GLuint baseInstance);

    angle::Result storeDynamicAttrib(const gl::Context *context,
                                     TranslatedAttribute *translated,
                                     GLint start,
                                     size_t count,
                                     GLsizei instances,
                                     GLuint baseInstance);

    BufferFactoryD3D *const mFactory;

    StreamingVertexBufferInterface mStreamingBuffer;
    std::vector<CurrentValueState> mCurrentValueCache;
    gl::AttributesMask mDynamicAttribsMaskCache;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_VERTEXDATAMANAGER_H_
