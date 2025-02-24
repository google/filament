//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayoutmetal.h:
//   Methods and classes related to uniform layout and packing in Metal
//

#ifndef COMMON_BLOCKLAYOUT_METAL__H_
#define COMMON_BLOCKLAYOUT_METAL__H_

#include <cstddef>
#include <map>
#include <vector>

#include <GLSLANG/ShaderLang.h>
#include "angle_gl.h"
#include "compiler/translator/blocklayout.h"

namespace rx
{

namespace mtl
{

size_t GetMetalSizeForGLType(GLenum type);
size_t GetMetalAlignmentForGLType(GLenum type);

size_t GetMTLBaseAlignment(GLenum variableType, bool isRowMajor);

class MetalAlignmentVisitor : public sh::ShaderVariableVisitor
{
  public:
    MetalAlignmentVisitor() = default;
    void visitVariable(const sh::ShaderVariable &variable, bool isRowMajor) override;
    // This is in bytes rather than components.
    size_t getBaseAlignment() const { return mCurrentAlignment; }

  private:
    size_t mCurrentAlignment = 0;
};

class BlockLayoutEncoderMTL : public sh::BlockLayoutEncoder
{
  public:
    BlockLayoutEncoderMTL();
    ~BlockLayoutEncoderMTL() override {}

    sh::BlockMemberInfo encodeType(GLenum type,
                                   const std::vector<unsigned int> &arraySizes,
                                   bool isRowMajorMatrix) override;
    // Advance the offset based on struct size and array dimensions.  Size can be calculated with
    // getShaderVariableSize() or equivalent.  |enterAggregateType|/|exitAggregateType| is necessary
    // around this call.
    sh::BlockMemberInfo encodeArrayOfPreEncodedStructs(
        size_t size,
        const std::vector<unsigned int> &arraySizes) override;

    size_t getCurrentOffset() const override;
    size_t getShaderVariableSize(const sh::ShaderVariable &structVar, bool isRowMajor) override;

    // Called when entering/exiting a structure variable.
    void enterAggregateType(const sh::ShaderVariable &structVar) override;
    void exitAggregateType(const sh::ShaderVariable &structVar) override;

  private:
    void getBlockLayoutInfo(GLenum type,
                            const std::vector<unsigned int> &arraySizes,
                            bool isRowMajorMatrix,
                            int *arrayStrideOut,
                            int *matrixStrideOut) override;
    void advanceOffset(GLenum type,
                       const std::vector<unsigned int> &arraySizes,
                       bool isRowMajorMatrix,
                       int arrayStride,
                       int matrixStride) override;

    size_t getBaseAlignment(const sh::ShaderVariable &variable) const;
    size_t getTypeBaseAlignment(GLenum type, bool isRowMajorMatrix) const;
};

}  // namespace mtl

}  // namespace rx

#endif  // COMMON_BLOCKLAYOUT_METAL_H_
