//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderInterfaceVariableInfoMap: Maps shader interface variable SPIR-V ids to their Vulkan
// mapping.

#ifndef LIBANGLE_RENDERER_VULKAN_SHADERINTERFACEVARIABLEINFOMAP_H_
#define LIBANGLE_RENDERER_VULKAN_SHADERINTERFACEVARIABLEINFOMAP_H_

#include "common/FastVector.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/spv_utils.h"

#include <functional>

#include <stdio.h>

namespace rx
{

struct VariableIndex
{
    static constexpr uint32_t kInvalid = 0xFFFF'FFFF;
    uint32_t index                     = kInvalid;
};

class ShaderInterfaceVariableInfoMap final : angle::NonCopyable
{
  public:
    // For each interface variable, a ShaderInterfaceVariableInfo is created.  These are stored in a
    // flat array.
    using VariableInfoArray    = std::vector<ShaderInterfaceVariableInfo>;
    using XFBVariableInfoArray = std::vector<XFBVariableInfoPtr>;

    // Each interface variable has an associted SPIR-V id (which is different per shader type).
    // The following map is from a SPIR-V id to its associated info in VariableInfoArray.
    //
    // Note that the SPIR-V ids are mostly contiguous and start at
    // sh::vk::spirv::kIdShaderVariablesBegin.  As such, the map key is actually
    // |id - sh::vk::spirv::kIdShaderVariablesBegin|.
    static constexpr size_t kIdFastMapMax = 32;
    using IdToIndexMap                    = angle::FastMap<VariableIndex, kIdFastMapMax>;

    ShaderInterfaceVariableInfoMap();
    ~ShaderInterfaceVariableInfoMap();

    void clear();
    void load(gl::BinaryInputStream *stream);
    void save(gl::BinaryOutputStream *stream);

    ShaderInterfaceVariableInfo &add(gl::ShaderType shaderType, uint32_t id);
    void addResource(gl::ShaderBitSet shaderTypes,
                     const gl::ShaderMap<uint32_t> &idInShaderTypes,
                     uint32_t descriptorSet,
                     uint32_t binding);
    ShaderInterfaceVariableInfo &addOrGet(gl::ShaderType shaderType, uint32_t id);

    void setInputPerVertexActiveMembers(gl::ShaderType shaderType,
                                        gl::PerVertexMemberBitSet activeMembers);
    void setOutputPerVertexActiveMembers(gl::ShaderType shaderType,
                                         gl::PerVertexMemberBitSet activeMembers);
    ShaderInterfaceVariableInfo &getMutable(gl::ShaderType shaderType, uint32_t id);
    XFBInterfaceVariableInfo *getXFBMutable(gl::ShaderType shaderType, uint32_t id);

    const ShaderInterfaceVariableInfo &getDefaultUniformInfo(gl::ShaderType shaderType) const;
    const ShaderInterfaceVariableInfo &getAtomicCounterInfo(gl::ShaderType shaderType) const;
    bool hasTransformFeedbackInfo(gl::ShaderType shaderType, uint32_t bufferIndex) const;
    const ShaderInterfaceVariableInfo &getEmulatedXfbBufferInfo(uint32_t bufferIndex) const;

    uint32_t getDefaultUniformBinding(gl::ShaderType shaderType) const;
    uint32_t getEmulatedXfbBufferBinding(uint32_t xfbBufferIndex) const;
    uint32_t getAtomicCounterBufferBinding(gl::ShaderType shaderType,
                                           uint32_t atomicCounterBufferIndex) const;

    bool hasVariable(gl::ShaderType shaderType, uint32_t id) const;
    const ShaderInterfaceVariableInfo &getVariableById(gl::ShaderType shaderType,
                                                       uint32_t id) const;
    const VariableInfoArray &getData() const { return mData; }
    const gl::ShaderMap<IdToIndexMap> &getIdToIndexMap() const { return mIdToIndexMap; }
    const XFBInterfaceVariableInfo &getXFBDataForVariableInfo(
        const ShaderInterfaceVariableInfo *info) const
    {
        size_t index = info - mData.data();
        ASSERT(index < mXFBData.size());
        ASSERT(mXFBData[index]);
        return *mXFBData[index];
    }
    const gl::ShaderMap<gl::PerVertexMemberBitSet> &getInputPerVertexActiveMembers() const
    {
        return mPod.inputPerVertexActiveMembers;
    }
    const gl::ShaderMap<gl::PerVertexMemberBitSet> &getOutputPerVertexActiveMembers() const
    {
        return mPod.outputPerVertexActiveMembers;
    }

    void setHasAliasingAttributes() { mPod.hasAliasingAttributes = true; }
    bool hasAliasingAttributes() const { return mPod.hasAliasingAttributes; }

  private:
    void setVariableIndex(gl::ShaderType shaderType, uint32_t id, VariableIndex index);
    const VariableIndex &getVariableIndex(gl::ShaderType shaderType, uint32_t id) const;

    VariableInfoArray mData;
    // Transform feedback array will be empty if no XFB is used.
    XFBVariableInfoArray mXFBData;
    gl::ShaderMap<IdToIndexMap> mIdToIndexMap;

    ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
    struct PodStruct
    {
        PodStruct() : xfbInfoCount(0), hasAliasingAttributes(false) {}
        // Active members of `in gl_PerVertex` and `out gl_PerVertex`. 6 bytes each
        gl::ShaderMap<gl::PerVertexMemberBitSet> inputPerVertexActiveMembers;
        gl::ShaderMap<gl::PerVertexMemberBitSet> outputPerVertexActiveMembers;

        uint32_t xfbInfoCount : 31;
        // Whether the vertex shader has aliasing attributes.  Used by the SPIR-V transformer to
        // tell if emulation is needed.
        uint32_t hasAliasingAttributes : 1;
    } mPod;
    ANGLE_DISABLE_STRUCT_PADDING_WARNINGS
};

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getDefaultUniformInfo(gl::ShaderType shaderType) const
{
    return getVariableById(shaderType, sh::vk::spirv::kIdDefaultUniformsBlock);
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getAtomicCounterInfo(gl::ShaderType shaderType) const
{
    return getVariableById(shaderType, sh::vk::spirv::kIdAtomicCounterBlock);
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &
ShaderInterfaceVariableInfoMap::getEmulatedXfbBufferInfo(uint32_t bufferIndex) const
{
    ASSERT(bufferIndex < 4);
    static_assert(sh::vk::spirv::kIdXfbEmulationBufferBlockOne ==
                  sh::vk::spirv::kIdXfbEmulationBufferBlockZero + 1);
    static_assert(sh::vk::spirv::kIdXfbEmulationBufferBlockTwo ==
                  sh::vk::spirv::kIdXfbEmulationBufferBlockZero + 2);
    static_assert(sh::vk::spirv::kIdXfbEmulationBufferBlockThree ==
                  sh::vk::spirv::kIdXfbEmulationBufferBlockZero + 3);

    // Transform feedback emulation only supports vertex shaders.
    return getVariableById(gl::ShaderType::Vertex,
                           sh::vk::spirv::kIdXfbEmulationBufferBlockZero + bufferIndex);
}

ANGLE_INLINE uint32_t
ShaderInterfaceVariableInfoMap::getDefaultUniformBinding(gl::ShaderType shaderType) const
{
    return getDefaultUniformInfo(shaderType).binding;
}

ANGLE_INLINE uint32_t
ShaderInterfaceVariableInfoMap::getEmulatedXfbBufferBinding(uint32_t bufferIndex) const
{
    return getEmulatedXfbBufferInfo(bufferIndex).binding;
}

ANGLE_INLINE uint32_t ShaderInterfaceVariableInfoMap::getAtomicCounterBufferBinding(
    gl::ShaderType shaderType,
    uint32_t atomicCounterBufferIndex) const
{
    return getAtomicCounterInfo(shaderType).binding + atomicCounterBufferIndex;
}

ANGLE_INLINE const ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::getVariableById(
    gl::ShaderType shaderType,
    uint32_t id) const
{
    return mData[getVariableIndex(shaderType, id).index];
}
}  // namespace rx
#endif  // LIBANGLE_RENDERER_VULKAN_SHADERINTERFACEVARIABLEINFOMAP_H_
