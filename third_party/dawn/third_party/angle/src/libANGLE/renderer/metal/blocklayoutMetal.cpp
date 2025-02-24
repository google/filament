//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayoutMetal.cpp:
//   Implementation for metal block layout classes and methods.
//

#include "libANGLE/renderer/metal/blocklayoutMetal.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "compiler/translator/blocklayout.h"
namespace rx
{
namespace mtl
{
// Sizes and types are available at
// https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
// Section 2
size_t GetMetalSizeForGLType(GLenum type)
{
    switch (type)
    {
        case GL_BOOL:
            return 1;
        case GL_BOOL_VEC2:
            return 2;
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
            return 4;
        case GL_FLOAT:
            return 4;
        case GL_FLOAT_VEC2:
            return 8;
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
            return 16;
        case GL_FLOAT_MAT2:  // 2x2
            return 16;
        case GL_FLOAT_MAT3:  // 3x4
            return 48;
        case GL_FLOAT_MAT4:  // 4x4
            return 64;
        case GL_FLOAT_MAT2x3:  // 2x4
            return 32;
        case GL_FLOAT_MAT3x2:  // 3x2
            return 24;
        case GL_FLOAT_MAT2x4:  // 2x4
            return 32;
        case GL_FLOAT_MAT4x2:  // 4x2
            return 32;
        case GL_FLOAT_MAT3x4:  // 3x4
            return 48;
        case GL_FLOAT_MAT4x3:  // 4x4
            return 64;
        case GL_INT:
            return 4;
        case GL_INT_VEC2:
            return 8;
        case GL_INT_VEC3:
        case GL_INT_VEC4:
            return 16;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_UNSIGNED_INT_VEC2:
            return 8;
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
            return 16;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_RECT_ANGLE:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_EXTERNAL_OES:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_IMAGE_2D:
        case GL_INT_IMAGE_2D:
        case GL_UNSIGNED_INT_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_INT_IMAGE_3D:
        case GL_UNSIGNED_INT_IMAGE_3D:
        case GL_IMAGE_2D_ARRAY:
        case GL_INT_IMAGE_2D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        case GL_IMAGE_CUBE:
        case GL_INT_IMAGE_CUBE:
        case GL_UNSIGNED_INT_IMAGE_CUBE:
        case GL_IMAGE_CUBE_MAP_ARRAY:
        case GL_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_IMAGE_BUFFER:
        case GL_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        case GL_SAMPLER_VIDEO_IMAGE_WEBGL:
        case GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT:
            UNREACHABLE();
            break;
        default:
            UNREACHABLE();
            break;
    }
    return 0;
}

size_t GetMetalAlignmentForGLType(GLenum type)
{
    switch (type)
    {
        case GL_BOOL:
            return 1;
        case GL_BOOL_VEC2:
            return 2;
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
            return 4;
        case GL_FLOAT:
            return 4;
        case GL_FLOAT_VEC2:
            return 8;
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
            return 16;
        case GL_FLOAT_MAT2:
            return 8;
        case GL_FLOAT_MAT3:
            return 16;
        case GL_FLOAT_MAT4:
            return 16;
        case GL_FLOAT_MAT2x3:
            return 16;
        case GL_FLOAT_MAT3x2:
            return 8;
        case GL_FLOAT_MAT2x4:
            return 16;
        case GL_FLOAT_MAT4x2:
            return 8;
        case GL_FLOAT_MAT3x4:
            return 16;
        case GL_FLOAT_MAT4x3:
            return 16;
        case GL_INT:
            return 4;
        case GL_INT_VEC2:
            return 8;
        case GL_INT_VEC3:
            return 16;
        case GL_INT_VEC4:
            return 16;
        case GL_UNSIGNED_INT:
            return 4;
        case GL_UNSIGNED_INT_VEC2:
            return 8;
        case GL_UNSIGNED_INT_VEC3:
        case GL_UNSIGNED_INT_VEC4:
            return 16;
        case GL_SAMPLER_2D:
        case GL_SAMPLER_2D_RECT_ANGLE:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_CUBE_MAP_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_EXTERNAL_OES:
        case GL_SAMPLER_2D_MULTISAMPLE:
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_INT_SAMPLER_BUFFER:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        case GL_SAMPLER_2D_SHADOW:
        case GL_SAMPLER_BUFFER:
        case GL_SAMPLER_CUBE_SHADOW:
        case GL_SAMPLER_2D_ARRAY_SHADOW:
        case GL_IMAGE_2D:
        case GL_INT_IMAGE_2D:
        case GL_UNSIGNED_INT_IMAGE_2D:
        case GL_IMAGE_3D:
        case GL_INT_IMAGE_3D:
        case GL_UNSIGNED_INT_IMAGE_3D:
        case GL_IMAGE_2D_ARRAY:
        case GL_INT_IMAGE_2D_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
        case GL_IMAGE_CUBE:
        case GL_INT_IMAGE_CUBE:
        case GL_UNSIGNED_INT_IMAGE_CUBE:
        case GL_IMAGE_CUBE_MAP_ARRAY:
        case GL_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
        case GL_IMAGE_BUFFER:
        case GL_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_SAMPLER_BUFFER:
        case GL_UNSIGNED_INT_IMAGE_BUFFER:
        case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        case GL_SAMPLER_VIDEO_IMAGE_WEBGL:
        case GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT:
            UNREACHABLE();
            break;
        default:
            UNREACHABLE();
            break;
    }
    return 0;
}

size_t GetMTLBaseAlignment(GLenum variableType, bool isRowMajor)
{
    return mtl::GetMetalAlignmentForGLType(variableType);
}

void MetalAlignmentVisitor::visitVariable(const sh::ShaderVariable &variable, bool isRowMajor)
{
    size_t baseAlignment = GetMTLBaseAlignment(variable.type, isRowMajor);
    mCurrentAlignment    = std::max(mCurrentAlignment, baseAlignment);
}

BlockLayoutEncoderMTL::BlockLayoutEncoderMTL() : BlockLayoutEncoder() {}

void BlockLayoutEncoderMTL::getBlockLayoutInfo(GLenum type,
                                               const std::vector<unsigned int> &arraySizes,
                                               bool isRowMajorMatrix,
                                               int *arrayStrideOut,
                                               int *matrixStrideOut)
{
    size_t baseAlignment = 0;
    int matrixStride     = 0;
    int arrayStride      = 0;

    if (gl::IsMatrixType(type))
    {
        baseAlignment = static_cast<int>(mtl::GetMetalAlignmentForGLType(type));
        matrixStride  = static_cast<int>(mtl::GetMetalAlignmentForGLType(type));
        if (!arraySizes.empty())
        {
            arrayStride = static_cast<int>(mtl::GetMetalSizeForGLType(type));
        }
    }
    else if (!arraySizes.empty())
    {
        baseAlignment = static_cast<int>(mtl::GetMetalAlignmentForGLType(type));
        arrayStride   = static_cast<int>(mtl::GetMetalSizeForGLType(type));
    }
    else
    {
        baseAlignment = mtl::GetMetalAlignmentForGLType(type);
    }
    align(baseAlignment);
    *matrixStrideOut = matrixStride;
    *arrayStrideOut  = arrayStride;
}
sh::BlockMemberInfo BlockLayoutEncoderMTL::encodeType(GLenum type,
                                                      const std::vector<unsigned int> &arraySizes,
                                                      bool isRowMajorMatrix)
{
    int arrayStride;
    int matrixStride;

    getBlockLayoutInfo(type, arraySizes, isRowMajorMatrix, &arrayStride, &matrixStride);
    const sh::BlockMemberInfo memberInfo(
        type, static_cast<int>(mCurrentOffset), static_cast<int>(arrayStride),
        static_cast<int>(matrixStride), gl::ArraySizeProduct(arraySizes), isRowMajorMatrix);
    assert(memberInfo.offset >= 0);

    advanceOffset(type, arraySizes, isRowMajorMatrix, arrayStride, matrixStride);

    return memberInfo;
}

sh::BlockMemberInfo BlockLayoutEncoderMTL::encodeArrayOfPreEncodedStructs(
    size_t size,
    const std::vector<unsigned int> &arraySizes)
{
    const unsigned int innerArraySizeProduct = gl::InnerArraySizeProduct(arraySizes);
    const unsigned int outermostArraySize    = gl::OutermostArraySize(arraySizes);

    // The size of struct is expected to be already aligned appropriately.
    const size_t arrayStride = size * innerArraySizeProduct;
    // Aggregate blockMemberInfo types not needed: only used by the Metal bakcend.
    const sh::BlockMemberInfo memberInfo(GL_INVALID_ENUM, static_cast<int>(mCurrentOffset),
                                         static_cast<int>(arrayStride), -1,
                                         gl::ArraySizeProduct(arraySizes), false);

    angle::base::CheckedNumeric<size_t> checkedOffset(arrayStride);
    checkedOffset *= outermostArraySize;
    checkedOffset += mCurrentOffset;
    mCurrentOffset = checkedOffset.ValueOrDie();

    return memberInfo;
}

size_t BlockLayoutEncoderMTL::getCurrentOffset() const
{
    angle::base::CheckedNumeric<size_t> checkedOffset(mCurrentOffset);
    return checkedOffset.ValueOrDie();
}

void BlockLayoutEncoderMTL::enterAggregateType(const sh::ShaderVariable &structVar)
{
    align(getBaseAlignment(structVar));
}

void BlockLayoutEncoderMTL::exitAggregateType(const sh::ShaderVariable &structVar)
{
    align(getBaseAlignment(structVar));
}

size_t BlockLayoutEncoderMTL::getShaderVariableSize(const sh::ShaderVariable &structVar,
                                                    bool isRowMajor)
{
    size_t currentOffset = mCurrentOffset;
    mCurrentOffset       = 0;
    sh::BlockEncoderVisitor visitor("", "", this);
    enterAggregateType(structVar);
    TraverseShaderVariables(structVar.fields, isRowMajor, &visitor);
    exitAggregateType(structVar);
    size_t structVarSize = getCurrentOffset();
    mCurrentOffset       = currentOffset;
    return structVarSize;
}

void BlockLayoutEncoderMTL::advanceOffset(GLenum type,
                                          const std::vector<unsigned int> &arraySizes,
                                          bool isRowMajorMatrix,
                                          int arrayStride,
                                          int matrixStride)
{
    if (!arraySizes.empty())
    {
        angle::base::CheckedNumeric<size_t> checkedOffset(arrayStride);
        checkedOffset *= gl::ArraySizeProduct(arraySizes);
        checkedOffset += mCurrentOffset;
        mCurrentOffset = checkedOffset.ValueOrDie();
    }
    else if (gl::IsMatrixType(type))
    {
        angle::base::CheckedNumeric<size_t> checkedOffset;
        checkedOffset = mtl::GetMetalSizeForGLType(type);
        checkedOffset += mCurrentOffset;
        mCurrentOffset = checkedOffset.ValueOrDie();
    }
    else
    {
        angle::base::CheckedNumeric<size_t> checkedOffset(mCurrentOffset);
        checkedOffset += mtl::GetMetalSizeForGLType(type);
        mCurrentOffset = checkedOffset.ValueOrDie();
    }
}

size_t BlockLayoutEncoderMTL::getBaseAlignment(const sh::ShaderVariable &shaderVar) const
{
    if (shaderVar.isStruct())
    {
        MetalAlignmentVisitor visitor;
        TraverseShaderVariables(shaderVar.fields, false, &visitor);
        return visitor.getBaseAlignment();
    }

    return GetMTLBaseAlignment(shaderVar.type, shaderVar.isRowMajorLayout);
}

size_t BlockLayoutEncoderMTL::getTypeBaseAlignment(GLenum type, bool isRowMajorMatrix) const
{
    return GetMTLBaseAlignment(type, isRowMajorMatrix);
}

}  // namespace mtl
}  // namespace rx
