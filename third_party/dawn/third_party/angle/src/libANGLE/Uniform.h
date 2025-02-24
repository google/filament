//
// Copyright 2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_UNIFORM_H_
#define LIBANGLE_UNIFORM_H_

#include <string>
#include <vector>

#include "angle_gl.h"
#include "common/MemoryBuffer.h"
#include "common/debug.h"
#include "common/uniform_type_info_autogen.h"
#include "common/utilities.h"
#include "compiler/translator/blocklayout.h"
#include "libANGLE/angletypes.h"

namespace gl
{
class BinaryInputStream;
class BinaryOutputStream;
struct UniformTypeInfo;
struct UsedUniform;
struct LinkedUniform;

#define ACTIVE_VARIABLE_COMMON_INTERFACES                         \
    void setActive(ShaderType shaderType, bool used, uint32_t id) \
    {                                                             \
        ASSERT(shaderType != ShaderType::InvalidEnum);            \
        pod.activeUseBits.set(shaderType, used);                  \
        pod.ids[shaderType] = id;                                 \
    }                                                             \
    ShaderType getFirstActiveShaderType() const                   \
    {                                                             \
        return pod.activeUseBits.first();                         \
    }                                                             \
    bool isActive(ShaderType shaderType) const                    \
    {                                                             \
        return pod.activeUseBits[shaderType];                     \
    }                                                             \
    const ShaderMap<uint32_t> &getIds() const                     \
    {                                                             \
        return pod.ids;                                           \
    }                                                             \
    uint32_t getId(ShaderType shaderType) const                   \
    {                                                             \
        return pod.ids[shaderType];                               \
    }                                                             \
    ShaderBitSet activeShaders() const                            \
    {                                                             \
        return pod.activeUseBits;                                 \
    }                                                             \
    uint32_t activeShaderCount() const                            \
    {                                                             \
        return static_cast<uint32_t>(pod.activeUseBits.count());  \
    }

struct ActiveVariable
{
    ActiveVariable() { memset(&pod, 0, sizeof(pod)); }

    ACTIVE_VARIABLE_COMMON_INTERFACES

    struct PODStruct
    {
        ShaderBitSet activeUseBits;
        // The id of a linked variable in each shader stage.  This id originates from
        // sh::ShaderVariable::id or sh::InterfaceBlock::id
        ShaderMap<uint32_t> ids;
    } pod;
};

inline const UniformTypeInfo &GetUniformTypeInfoFromIndex(UniformTypeIndex index)
{
    ASSERT(index.value >= 0 && index.value < kUniformInfoTable.size());
    return kUniformInfoTable[index.value];
}

// Important: This struct must have basic data types only, so that we can initialize with memcpy. Do
// not put any std::vector or objects with virtual functions in it.
// Helper struct representing a single shader uniform. Most of this structure's data member and
// access functions mirrors ShaderVariable; See ShaderVars.h for more info.
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
struct LinkedUniform
{
    LinkedUniform() = default;
    LinkedUniform(GLenum typeIn,
                  GLenum precisionIn,
                  const std::vector<unsigned int> &arraySizesIn,
                  const int bindingIn,
                  const int offsetIn,
                  const int locationIn,
                  const int bufferIndexIn,
                  const sh::BlockMemberInfo &blockInfoIn);
    LinkedUniform(const UsedUniform &usedUniform);

    const UniformTypeInfo &getUniformTypeInfo() const
    {
        return GetUniformTypeInfoFromIndex(pod.typeIndex);
    }

    bool isSampler() const { return getUniformTypeInfo().isSampler; }
    bool isImage() const { return getUniformTypeInfo().isImageType; }
    bool isAtomicCounter() const { return IsAtomicCounterType(getType()); }
    bool isInDefaultBlock() const { return pod.bufferIndex == -1; }
    size_t getElementSize() const { return getUniformTypeInfo().externalSize; }
    GLint getElementComponents() const { return GetUniformElementComponents(pod.typeIndex); }

    bool isTexelFetchStaticUse() const { return pod.flagBits.texelFetchStaticUse; }
    bool isFragmentInOut() const { return pod.flagBits.isFragmentInOut; }

    bool isArray() const { return pod.flagBits.isArray; }
    uint16_t getBasicTypeElementCount() const
    {
        ASSERT(pod.flagBits.isArray || pod.arraySize == 1u);
        return pod.arraySize;
    }

    GLenum getType() const { return getUniformTypeInfo().type; }
    uint16_t getOuterArrayOffset() const { return pod.outerArrayOffset; }
    uint16_t getOuterArraySizeProduct() const { return pod.outerArraySizeProduct; }
    int16_t getBinding() const { return pod.binding; }
    int16_t getOffset() const { return pod.offset; }
    int getBufferIndex() const { return pod.bufferIndex; }
    int getLocation() const { return pod.location; }
    GLenum getImageUnitFormat() const { return pod.imageUnitFormat; }

    ACTIVE_VARIABLE_COMMON_INTERFACES

    struct PODStruct
    {
        UniformTypeIndex typeIndex;
        uint16_t precision;

        int32_t location;

        // These are from sh::struct BlockMemberInfo struct. See locklayout.h for detail.
        uint16_t blockOffset;
        uint16_t blockArrayStride;

        uint16_t blockMatrixStride;
        uint16_t imageUnitFormat;

        // maxUniformVectorsCount is 4K due to we clamp maxUniformBlockSize to 64KB. All of these
        // variable should be enough to pack into 16 bits to reduce the size of mUniforms.
        int16_t binding;
        int16_t bufferIndex;

        int16_t offset;
        uint16_t arraySize;

        uint16_t outerArraySizeProduct;
        uint16_t outerArrayOffset;

        uint16_t parentArrayIndex;
        union
        {
            struct
            {
                uint8_t isFragmentInOut : 1;
                uint8_t texelFetchStaticUse : 1;
                uint8_t isArray : 1;
                uint8_t blockIsRowMajorMatrix : 1;
                uint8_t isBlock : 1;
                uint8_t padding : 3;
            } flagBits;
            uint8_t flagBitsAsUByte;
        };
        ShaderBitSet activeUseBits;

        uint32_t id;
        // The id of a linked variable in each shader stage.  This id originates from
        // sh::ShaderVariable::id or sh::InterfaceBlock::id
        ShaderMap<uint32_t> ids;
    } pod;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

struct BufferVariable
{
    BufferVariable();
    BufferVariable(GLenum type,
                   GLenum precision,
                   const std::string &name,
                   const std::vector<unsigned int> &arraySizes,
                   const int bufferIndex,
                   int topLevelArraySize,
                   const sh::BlockMemberInfo &blockInfo);
    ~BufferVariable() {}

    bool isArray() const { return pod.isArray; }
    uint32_t getBasicTypeElementCount() const { return pod.basicTypeElementCount; }

    ACTIVE_VARIABLE_COMMON_INTERFACES

    std::string name;
    std::string mappedName;

    ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
    struct PODStruct
    {
        uint16_t type;
        uint16_t precision;

        // 1 byte each
        ShaderBitSet activeUseBits;
        bool isArray;

        int16_t bufferIndex;

        // The id of a linked variable in each shader stage.  This id originates from
        // sh::ShaderVariable::id or sh::InterfaceBlock::id
        ShaderMap<uint32_t> ids;

        sh::BlockMemberInfo blockInfo;

        int32_t topLevelArraySize;
        uint32_t basicTypeElementCount;
    } pod;
    ANGLE_DISABLE_STRUCT_PADDING_WARNINGS
};

// Represents a single atomic counter buffer
struct AtomicCounterBuffer
{
    AtomicCounterBuffer();
    ~AtomicCounterBuffer() {}

    ACTIVE_VARIABLE_COMMON_INTERFACES
    int numActiveVariables() const { return static_cast<int>(memberIndexes.size()); }
    void unionReferencesWith(const LinkedUniform &otherUniform);

    std::vector<unsigned int> memberIndexes;

    ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
    struct PODStruct
    {
        // The id of a linked variable in each shader stage.  This id originates from
        // sh::ShaderVariable::id or sh::InterfaceBlock::id
        ShaderMap<uint32_t> ids;
        // The binding as specified by the shader.
        int inShaderBinding;
        unsigned int dataSize;
        ShaderBitSet activeUseBits;
        uint8_t pads[3];
    } pod;
    ANGLE_DISABLE_STRUCT_PADDING_WARNINGS
};

// Helper struct representing a single shader interface block
struct InterfaceBlock
{
    InterfaceBlock();
    InterfaceBlock(const std::string &nameIn,
                   const std::string &mappedNameIn,
                   bool isArrayIn,
                   bool isReadOnlyIn,
                   unsigned int arrayElementIn,
                   unsigned int firstFieldArraySizeIn,
                   int bindingIn);

    std::string nameWithArrayIndex() const;
    std::string mappedNameWithArrayIndex() const;

    ACTIVE_VARIABLE_COMMON_INTERFACES

    int numActiveVariables() const { return static_cast<int>(memberIndexes.size()); }

    std::string name;
    std::string mappedName;
    std::vector<unsigned int> memberIndexes;

    ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
    struct PODStruct
    {
        uint32_t arrayElement;
        uint32_t firstFieldArraySize;

        ShaderBitSet activeUseBits;
        uint8_t isArray : 1;
        // Only valid for SSBOs, specifies whether it has the readonly qualifier.
        uint8_t isReadOnly : 1;
        uint8_t padings : 6;
        // The binding as specified by the shader (0 if unspecified, per spec)
        int16_t inShaderBinding;

        unsigned int dataSize;
        // The id of a linked variable in each shader stage.  This id originates from
        // sh::ShaderVariable::id or sh::InterfaceBlock::id
        ShaderMap<uint32_t> ids;
    } pod;
    ANGLE_DISABLE_STRUCT_PADDING_WARNINGS
};

#undef ACTIVE_VARIABLE_COMMON_INTERFACES
}  // namespace gl

#endif  // LIBANGLE_UNIFORM_H_
