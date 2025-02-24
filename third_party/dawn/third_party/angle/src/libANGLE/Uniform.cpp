//
// Copyright 2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/Uniform.h"
#include "common/BinaryStream.h"
#include "libANGLE/ProgramLinkedResources.h"

#include <cstring>

namespace gl
{

LinkedUniform::LinkedUniform(GLenum typeIn,
                             GLenum precisionIn,
                             const std::vector<unsigned int> &arraySizesIn,
                             const int bindingIn,
                             const int offsetIn,
                             const int locationIn,
                             const int bufferIndexIn,
                             const sh::BlockMemberInfo &blockInfoIn)
{
    // arrays are always flattened, which means at most 1D array
    ASSERT(arraySizesIn.size() <= 1);

    memset(this, 0, sizeof(*this));
    pod.typeIndex = GetUniformTypeIndex(typeIn);
    SetBitField(pod.precision, precisionIn);
    pod.location = locationIn;
    SetBitField(pod.binding, bindingIn);
    SetBitField(pod.offset, offsetIn);
    SetBitField(pod.bufferIndex, bufferIndexIn);
    pod.outerArraySizeProduct = 1;
    SetBitField(pod.arraySize, arraySizesIn.empty() ? 1u : arraySizesIn[0]);
    SetBitField(pod.flagBits.isArray, !arraySizesIn.empty());
    if (!(blockInfoIn == sh::kDefaultBlockMemberInfo))
    {
        pod.flagBits.isBlock               = 1;
        pod.flagBits.blockIsRowMajorMatrix = blockInfoIn.isRowMajorMatrix;
        SetBitField(pod.blockOffset, blockInfoIn.offset);
        SetBitField(pod.blockArrayStride, blockInfoIn.arrayStride);
        SetBitField(pod.blockMatrixStride, blockInfoIn.matrixStride);
    }
}

LinkedUniform::LinkedUniform(const UsedUniform &usedUniform)
{
    ASSERT(!usedUniform.isArrayOfArrays());
    ASSERT(!usedUniform.isStruct());
    ASSERT(usedUniform.active);
    ASSERT(usedUniform.blockInfo == sh::kDefaultBlockMemberInfo);

    // Note: Ensure every data member is initialized.
    pod.flagBitsAsUByte = 0;
    pod.typeIndex       = GetUniformTypeIndex(usedUniform.type);
    SetBitField(pod.precision, usedUniform.precision);
    SetBitField(pod.imageUnitFormat, usedUniform.imageUnitFormat);
    pod.location          = usedUniform.location;
    pod.blockOffset       = 0;
    pod.blockArrayStride  = 0;
    pod.blockMatrixStride = 0;
    SetBitField(pod.binding, usedUniform.binding);
    SetBitField(pod.offset, usedUniform.offset);

    SetBitField(pod.bufferIndex, usedUniform.bufferIndex);
    SetBitField(pod.parentArrayIndex, usedUniform.parentArrayIndex());
    SetBitField(pod.outerArraySizeProduct, ArraySizeProduct(usedUniform.outerArraySizes));
    SetBitField(pod.outerArrayOffset, usedUniform.outerArrayOffset);
    SetBitField(pod.arraySize, usedUniform.isArray() ? usedUniform.getArraySizeProduct() : 1u);
    SetBitField(pod.flagBits.isArray, usedUniform.isArray());

    pod.id            = usedUniform.id;
    pod.activeUseBits = usedUniform.activeVariable.activeShaders();
    pod.ids           = usedUniform.activeVariable.getIds();

    SetBitField(pod.flagBits.isFragmentInOut, usedUniform.isFragmentInOut);
    SetBitField(pod.flagBits.texelFetchStaticUse, usedUniform.texelFetchStaticUse);
    ASSERT(!usedUniform.isArray() || pod.arraySize == usedUniform.getArraySizeProduct());
}

BufferVariable::BufferVariable()
{
    memset(&pod, 0, sizeof(pod));
    pod.bufferIndex       = -1;
    pod.blockInfo         = sh::kDefaultBlockMemberInfo;
    pod.topLevelArraySize = -1;
}

BufferVariable::BufferVariable(GLenum type,
                               GLenum precision,
                               const std::string &name,
                               const std::vector<unsigned int> &arraySizes,
                               const int bufferIndex,
                               int topLevelArraySize,
                               const sh::BlockMemberInfo &blockInfo)
    : name(name)
{
    memset(&pod, 0, sizeof(pod));
    SetBitField(pod.type, type);
    SetBitField(pod.precision, precision);
    SetBitField(pod.bufferIndex, bufferIndex);
    pod.blockInfo = blockInfo;
    SetBitField(pod.topLevelArraySize, topLevelArraySize);
    pod.isArray = !arraySizes.empty();
    SetBitField(pod.basicTypeElementCount, arraySizes.empty() ? 1u : arraySizes.back());
}

AtomicCounterBuffer::AtomicCounterBuffer()
{
    memset(&pod, 0, sizeof(pod));
}

void AtomicCounterBuffer::unionReferencesWith(const LinkedUniform &other)
{
    pod.activeUseBits |= other.pod.activeUseBits;
    for (const ShaderType shaderType : AllShaderTypes())
    {
        ASSERT(pod.ids[shaderType] == 0 || other.getId(shaderType) == 0 ||
               pod.ids[shaderType] == other.getId(shaderType));
        if (pod.ids[shaderType] == 0)
        {
            pod.ids[shaderType] = other.getId(shaderType);
        }
    }
}

InterfaceBlock::InterfaceBlock()
{
    memset(&pod, 0, sizeof(pod));
}

InterfaceBlock::InterfaceBlock(const std::string &name,
                               const std::string &mappedName,
                               bool isArray,
                               bool isReadOnly,
                               unsigned int arrayElementIn,
                               unsigned int firstFieldArraySizeIn,
                               int binding)
    : name(name), mappedName(mappedName)
{
    memset(&pod, 0, sizeof(pod));

    SetBitField(pod.isArray, isArray);
    SetBitField(pod.isReadOnly, isReadOnly);
    SetBitField(pod.inShaderBinding, binding);
    pod.arrayElement        = arrayElementIn;
    pod.firstFieldArraySize = firstFieldArraySizeIn;
}

std::string InterfaceBlock::nameWithArrayIndex() const
{
    std::stringstream fullNameStr;
    fullNameStr << name;
    if (pod.isArray)
    {
        fullNameStr << "[" << pod.arrayElement << "]";
    }

    return fullNameStr.str();
}

std::string InterfaceBlock::mappedNameWithArrayIndex() const
{
    std::stringstream fullNameStr;
    fullNameStr << mappedName;
    if (pod.isArray)
    {
        fullNameStr << "[" << pod.arrayElement << "]";
    }

    return fullNameStr.str();
}
}  // namespace gl
