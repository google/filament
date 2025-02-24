//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArray11.h: Defines the rx::VertexArray11 class which implements rx::VertexArrayImpl.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_

#include "libANGLE/Framebuffer.h"
#include "libANGLE/renderer/VertexArrayImpl.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{
class Renderer11;

class VertexArray11 : public VertexArrayImpl
{
  public:
    VertexArray11(const gl::VertexArrayState &data);
    ~VertexArray11() override;
    void destroy(const gl::Context *context) override;

    // Does not apply any state updates - these are done in syncStateForDraw which as access to
    // the draw call parameters.
    angle::Result syncState(const gl::Context *context,
                            const gl::VertexArray::DirtyBits &dirtyBits,
                            gl::VertexArray::DirtyAttribBitsArray *attribBits,
                            gl::VertexArray::DirtyBindingBitsArray *bindingBits) override;

    // Applied buffer pointers are updated here.
    angle::Result syncStateForDraw(const gl::Context *context,
                                   GLint firstVertex,
                                   GLsizei vertexOrIndexCount,
                                   gl::DrawElementsType indexTypeOrInvalid,
                                   const void *indices,
                                   GLsizei instances,
                                   GLint baseVertex,
                                   GLuint baseInstance,
                                   bool promoteDynamic);

    // This will check the dynamic attribs mask.
    bool hasActiveDynamicAttrib(const gl::Context *context);

    const std::vector<TranslatedAttribute> &getTranslatedAttribs() const;

    UniqueSerial getCurrentStateSerial() const { return mCurrentStateSerial; }

    // In case of a multi-view program change, we have to update all attributes so that the divisor
    // is adjusted.
    void markAllAttributeDivisorsForAdjustment(int numViews);

    const TranslatedIndexData &getCachedIndexInfo() const;
    void updateCachedIndexInfo(const TranslatedIndexData &indexInfo);
    bool isCachedIndexInfoValid() const;

    gl::DrawElementsType getCachedDestinationIndexType() const;

  private:
    void updateVertexAttribStorage(const gl::Context *context,
                                   StateManager11 *stateManager,
                                   size_t attribIndex);
    angle::Result updateDirtyAttribs(const gl::Context *context,
                                     const gl::AttributesMask &activeDirtyAttribs);
    angle::Result updateDynamicAttribs(const gl::Context *context,
                                       VertexDataManager *vertexDataManager,
                                       GLint firstVertex,
                                       GLsizei vertexOrIndexCount,
                                       gl::DrawElementsType indexTypeOrInvalid,
                                       const void *indices,
                                       GLsizei instances,
                                       GLint baseVertex,
                                       GLuint baseInstance,
                                       bool promoteDynamic,
                                       const gl::AttributesMask &activeDynamicAttribs);

    angle::Result updateElementArrayStorage(const gl::Context *context,
                                            GLsizei indexCount,
                                            gl::DrawElementsType indexType,
                                            const void *indices,
                                            bool restartEnabled);

    std::vector<VertexStorageType> mAttributeStorageTypes;
    std::vector<TranslatedAttribute> mTranslatedAttribs;

    // The mask of attributes marked as dynamic.
    gl::AttributesMask mDynamicAttribsMask;

    // A set of attributes we know are dirty, and need to be re-translated.
    gl::AttributesMask mAttribsToTranslate;

    UniqueSerial mCurrentStateSerial;

    // The numViews value used to adjust the divisor.
    int mAppliedNumViewsToDivisor;

    // If the index buffer needs re-streaming.
    Optional<gl::DrawElementsType> mLastDrawElementsType;
    Optional<const void *> mLastDrawElementsIndices;
    Optional<bool> mLastPrimitiveRestartEnabled;
    IndexStorageType mCurrentElementArrayStorage;
    Optional<TranslatedIndexData> mCachedIndexInfo;
    gl::DrawElementsType mCachedDestinationIndexType;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_VERTEXARRAY11_H_
