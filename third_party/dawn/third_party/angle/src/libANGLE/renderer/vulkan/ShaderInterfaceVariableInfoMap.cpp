//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderInterfaceVariableInfoMap: Maps shader interface variable SPIR-V ids to their Vulkan
// mapping.
//

#include "libANGLE/renderer/vulkan/ShaderInterfaceVariableInfoMap.h"

namespace rx
{
namespace
{
uint32_t HashSPIRVId(uint32_t id)
{
    ASSERT(id >= sh::vk::spirv::kIdShaderVariablesBegin);
    return id - sh::vk::spirv::kIdShaderVariablesBegin;
}

void LoadShaderInterfaceVariableXfbInfo(gl::BinaryInputStream *stream,
                                        ShaderInterfaceVariableXfbInfo *xfb)
{
    stream->readStruct(&xfb->pod);
    xfb->arrayElements.resize(stream->readInt<size_t>());
    for (ShaderInterfaceVariableXfbInfo &arrayElement : xfb->arrayElements)
    {
        LoadShaderInterfaceVariableXfbInfo(stream, &arrayElement);
    }
}

void SaveShaderInterfaceVariableXfbInfo(const ShaderInterfaceVariableXfbInfo &xfb,
                                        gl::BinaryOutputStream *stream)
{
    stream->writeStruct(xfb.pod);
    stream->writeInt(xfb.arrayElements.size());
    for (const ShaderInterfaceVariableXfbInfo &arrayElement : xfb.arrayElements)
    {
        SaveShaderInterfaceVariableXfbInfo(arrayElement, stream);
    }
}
}  // anonymous namespace

// ShaderInterfaceVariableInfoMap implementation.
ShaderInterfaceVariableInfoMap::ShaderInterfaceVariableInfoMap() = default;

ShaderInterfaceVariableInfoMap::~ShaderInterfaceVariableInfoMap() = default;

void ShaderInterfaceVariableInfoMap::clear()
{
    mData.clear();
    mXFBData.clear();
    memset(&mPod, 0, sizeof(mPod));
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mIdToIndexMap[shaderType].clear();
    }
}

void ShaderInterfaceVariableInfoMap::save(gl::BinaryOutputStream *stream)
{
    ASSERT(mXFBData.size() <= mData.size());
    stream->writeStruct(mPod);

    for (const IdToIndexMap &idToIndexMap : mIdToIndexMap)
    {
        stream->writeInt(idToIndexMap.size());
        if (idToIndexMap.size() > 0)
        {
            stream->writeBytes(reinterpret_cast<const uint8_t *>(idToIndexMap.data()),
                               idToIndexMap.size() * sizeof(*idToIndexMap.data()));
        }
    }

    stream->writeVector(mData);
    if (mPod.xfbInfoCount > 0)
    {
        uint32_t xfbInfoCount = 0;
        for (size_t xfbIndex = 0; xfbIndex < mXFBData.size(); xfbIndex++)
        {
            if (!mXFBData[xfbIndex])
            {
                continue;
            }
            stream->writeInt(xfbIndex);
            xfbInfoCount++;
            XFBInterfaceVariableInfo &info = *mXFBData[xfbIndex];
            SaveShaderInterfaceVariableXfbInfo(info.xfb, stream);
            stream->writeInt(info.fieldXfb.size());
            for (const ShaderInterfaceVariableXfbInfo &xfb : info.fieldXfb)
            {
                SaveShaderInterfaceVariableXfbInfo(xfb, stream);
            }
        }
        ASSERT(xfbInfoCount == mPod.xfbInfoCount);
    }
}
void ShaderInterfaceVariableInfoMap::load(gl::BinaryInputStream *stream)
{
    stream->readStruct(&mPod);

    for (IdToIndexMap &idToIndexMap : mIdToIndexMap)
    {
        // ASSERT(idToIndexMap.empty());
        size_t count = stream->readInt<size_t>();
        if (count > 0)
        {
            idToIndexMap.resetWithRawData(count,
                                          stream->getBytes(count * sizeof(*idToIndexMap.data())));
        }
    }

    stream->readVector(&mData);
    ASSERT(mXFBData.empty());
    ASSERT(mPod.xfbInfoCount <= mData.size());
    if (mPod.xfbInfoCount > 0)
    {
        mXFBData.resize(mData.size());
        for (uint32_t i = 0; i < mPod.xfbInfoCount; ++i)
        {
            size_t xfbIndex    = stream->readInt<size_t>();
            mXFBData[xfbIndex] = std::make_unique<XFBInterfaceVariableInfo>();

            XFBInterfaceVariableInfo &info = *mXFBData[xfbIndex];
            LoadShaderInterfaceVariableXfbInfo(stream, &info.xfb);
            info.fieldXfb.resize(stream->readInt<size_t>());
            for (ShaderInterfaceVariableXfbInfo &xfb : info.fieldXfb)
            {
                LoadShaderInterfaceVariableXfbInfo(stream, &xfb);
            }
        }
    }
}

void ShaderInterfaceVariableInfoMap::setInputPerVertexActiveMembers(
    gl::ShaderType shaderType,
    gl::PerVertexMemberBitSet activeMembers)
{
    // Input gl_PerVertex is only meaningful for tessellation and geometry stages
    ASSERT(shaderType == gl::ShaderType::TessControl ||
           shaderType == gl::ShaderType::TessEvaluation || shaderType == gl::ShaderType::Geometry ||
           activeMembers.none());
    mPod.inputPerVertexActiveMembers[shaderType] = activeMembers;
}

void ShaderInterfaceVariableInfoMap::setOutputPerVertexActiveMembers(
    gl::ShaderType shaderType,
    gl::PerVertexMemberBitSet activeMembers)
{
    // Output gl_PerVertex is only meaningful for vertex, tessellation and geometry stages
    ASSERT(shaderType == gl::ShaderType::Vertex || shaderType == gl::ShaderType::TessControl ||
           shaderType == gl::ShaderType::TessEvaluation || shaderType == gl::ShaderType::Geometry ||
           activeMembers.none());
    mPod.outputPerVertexActiveMembers[shaderType] = activeMembers;
}

void ShaderInterfaceVariableInfoMap::setVariableIndex(gl::ShaderType shaderType,
                                                      uint32_t id,
                                                      VariableIndex index)
{
    mIdToIndexMap[shaderType][HashSPIRVId(id)] = index;
}

const VariableIndex &ShaderInterfaceVariableInfoMap::getVariableIndex(gl::ShaderType shaderType,
                                                                      uint32_t id) const
{
    return mIdToIndexMap[shaderType].at(HashSPIRVId(id));
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::getMutable(gl::ShaderType shaderType,
                                                                        uint32_t id)
{
    ASSERT(hasVariable(shaderType, id));
    uint32_t index = getVariableIndex(shaderType, id).index;
    return mData[index];
}

XFBInterfaceVariableInfo *ShaderInterfaceVariableInfoMap::getXFBMutable(gl::ShaderType shaderType,
                                                                        uint32_t id)
{
    ASSERT(hasVariable(shaderType, id));
    uint32_t index = getVariableIndex(shaderType, id).index;
    if (index >= mXFBData.size())
    {
        mXFBData.resize(index + 1);
    }
    if (!mXFBData[index])
    {
        mXFBData[index]                   = std::make_unique<XFBInterfaceVariableInfo>();
        mData[index].hasTransformFeedback = true;
        mPod.xfbInfoCount++;
    }
    return mXFBData[index].get();
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::add(gl::ShaderType shaderType,
                                                                 uint32_t id)
{
    ASSERT(!hasVariable(shaderType, id));
    uint32_t index = static_cast<uint32_t>(mData.size());
    setVariableIndex(shaderType, id, {index});
    mData.resize(index + 1);
    return mData[index];
}

void ShaderInterfaceVariableInfoMap::addResource(gl::ShaderBitSet shaderTypes,
                                                 const gl::ShaderMap<uint32_t> &idInShaderTypes,
                                                 uint32_t descriptorSet,
                                                 uint32_t binding)
{
    uint32_t index = static_cast<uint32_t>(mData.size());
    mData.resize(index + 1);
    ShaderInterfaceVariableInfo *info = &mData[index];

    info->descriptorSet = descriptorSet;
    info->binding       = binding;
    info->activeStages  = shaderTypes;

    for (const gl::ShaderType shaderType : shaderTypes)
    {
        const uint32_t id = idInShaderTypes[shaderType];
        ASSERT(!hasVariable(shaderType, id));
        setVariableIndex(shaderType, id, {index});
    }
}

ShaderInterfaceVariableInfo &ShaderInterfaceVariableInfoMap::addOrGet(gl::ShaderType shaderType,
                                                                      uint32_t id)
{
    if (!hasVariable(shaderType, id))
    {
        return add(shaderType, id);
    }
    else
    {
        uint32_t index = getVariableIndex(shaderType, id).index;
        return mData[index];
    }
}

bool ShaderInterfaceVariableInfoMap::hasVariable(gl::ShaderType shaderType, uint32_t id) const
{
    const uint32_t hashedId = HashSPIRVId(id);
    return hashedId < mIdToIndexMap[shaderType].size() &&
           mIdToIndexMap[shaderType].at(hashedId).index != VariableIndex::kInvalid;
}

bool ShaderInterfaceVariableInfoMap::hasTransformFeedbackInfo(gl::ShaderType shaderType,
                                                              uint32_t bufferIndex) const
{
    return hasVariable(shaderType, SpvGetXfbBufferBlockId(bufferIndex));
}
}  // namespace rx
