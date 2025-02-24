//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Utilities to map shader interface variables to Vulkan mappings, and transform the SPIR-V
// accordingly.
//

#include "libANGLE/renderer/vulkan/spv_utils.h"

#include <array>
#include <cctype>
#include <numeric>

#include "common/FixedVector.h"
#include "common/spirv/spirv_instruction_builder_autogen.h"
#include "common/spirv/spirv_instruction_parser_autogen.h"
#include "common/string_utils.h"
#include "common/utilities.h"
#include "libANGLE/Caps.h"
#include "libANGLE/ProgramLinkedResources.h"
#include "libANGLE/renderer/vulkan/ShaderInterfaceVariableInfoMap.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/trace.h"

namespace spirv = angle::spirv;

namespace rx
{
namespace
{

// Test if there are non-zero indices in the uniform name, returning false in that case.  This
// happens for multi-dimensional arrays, where a uniform is created for every possible index of the
// array (except for the innermost dimension).  When assigning decorations (set/binding/etc), only
// the indices corresponding to the first element of the array should be specified.  This function
// is used to skip the other indices.
bool UniformNameIsIndexZero(const std::string &name)
{
    size_t lastBracketClose = 0;

    while (true)
    {
        size_t openBracket = name.find('[', lastBracketClose);
        if (openBracket == std::string::npos)
        {
            break;
        }
        size_t closeBracket = name.find(']', openBracket);

        // If the index between the brackets is not zero, ignore this uniform.
        if (name.substr(openBracket + 1, closeBracket - openBracket - 1) != "0")
        {
            return false;
        }
        lastBracketClose = closeBracket;
    }

    return true;
}

uint32_t SpvIsXfbBufferBlockId(spirv::IdRef id)
{
    return id >= sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferVarZero &&
           id < sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferVarZero + 4;
}

template <typename OutputIter, typename ImplicitIter>
uint32_t CountExplicitOutputs(OutputIter outputsBegin,
                              OutputIter outputsEnd,
                              ImplicitIter implicitsBegin,
                              ImplicitIter implicitsEnd)
{
    auto reduce = [implicitsBegin, implicitsEnd](uint32_t count, const gl::ProgramOutput &var) {
        bool isExplicit = std::find(implicitsBegin, implicitsEnd, var.name) == implicitsEnd;
        return count + isExplicit;
    };

    return std::accumulate(outputsBegin, outputsEnd, 0, reduce);
}

ShaderInterfaceVariableInfo *AddResourceInfoToAllStages(ShaderInterfaceVariableInfoMap *infoMap,
                                                        gl::ShaderType shaderType,
                                                        uint32_t varId,
                                                        uint32_t descriptorSet,
                                                        uint32_t binding)
{
    gl::ShaderBitSet allStages;
    allStages.set();

    ShaderInterfaceVariableInfo &info = infoMap->add(shaderType, varId);
    info.descriptorSet                = descriptorSet;
    info.binding                      = binding;
    info.activeStages                 = allStages;
    return &info;
}

ShaderInterfaceVariableInfo *AddResourceInfo(ShaderInterfaceVariableInfoMap *infoMap,
                                             gl::ShaderBitSet stages,
                                             gl::ShaderType shaderType,
                                             uint32_t varId,
                                             uint32_t descriptorSet,
                                             uint32_t binding)
{
    ShaderInterfaceVariableInfo &info = infoMap->add(shaderType, varId);
    info.descriptorSet                = descriptorSet;
    info.binding                      = binding;
    info.activeStages                 = stages;
    return &info;
}

// Add location information for an in/out variable.
ShaderInterfaceVariableInfo *AddLocationInfo(ShaderInterfaceVariableInfoMap *infoMap,
                                             gl::ShaderType shaderType,
                                             uint32_t varId,
                                             uint32_t location,
                                             uint32_t component,
                                             uint8_t attributeComponentCount,
                                             uint8_t attributeLocationCount)
{
    // The info map for this id may or may not exist already.  This function merges the
    // location/component information.
    ShaderInterfaceVariableInfo &info = infoMap->addOrGet(shaderType, varId);

    ASSERT(info.descriptorSet == ShaderInterfaceVariableInfo::kInvalid);
    ASSERT(info.binding == ShaderInterfaceVariableInfo::kInvalid);
    if (info.location != ShaderInterfaceVariableInfo::kInvalid)
    {
        ASSERT(info.location == location);
        ASSERT(info.component == component);
    }
    ASSERT(info.component == ShaderInterfaceVariableInfo::kInvalid);

    info.location  = location;
    info.component = component;
    info.activeStages.set(shaderType);
    info.attributeComponentCount = attributeComponentCount;
    info.attributeLocationCount  = attributeLocationCount;

    return &info;
}

// Add location information for an in/out variable
void AddVaryingLocationInfo(ShaderInterfaceVariableInfoMap *infoMap,
                            const gl::VaryingInShaderRef &ref,
                            const uint32_t location,
                            const uint32_t component)
{
    // Skip statically-unused varyings, they are already pruned by the translator
    if (ref.varying->id != 0)
    {
        AddLocationInfo(infoMap, ref.stage, ref.varying->id, location, component, 0, 0);
    }
}

// Modify an existing out variable and add transform feedback information.
void SetXfbInfo(ShaderInterfaceVariableInfoMap *infoMap,
                gl::ShaderType shaderType,
                uint32_t varId,
                int fieldIndex,
                uint32_t xfbBuffer,
                uint32_t xfbOffset,
                uint32_t xfbStride,
                uint32_t arraySize,
                uint32_t columnCount,
                uint32_t rowCount,
                uint32_t arrayIndex,
                GLenum componentType)
{
    XFBInterfaceVariableInfo *info = infoMap->getXFBMutable(shaderType, varId);
    ASSERT(info != nullptr);

    ShaderInterfaceVariableXfbInfo *xfb = &info->xfb;

    if (fieldIndex >= 0)
    {
        if (info->fieldXfb.size() <= static_cast<size_t>(fieldIndex))
        {
            info->fieldXfb.resize(fieldIndex + 1);
        }
        xfb = &info->fieldXfb[fieldIndex];
    }

    ASSERT(xfb->pod.buffer == ShaderInterfaceVariableXfbInfo::kInvalid);
    ASSERT(xfb->pod.offset == ShaderInterfaceVariableXfbInfo::kInvalid);
    ASSERT(xfb->pod.stride == ShaderInterfaceVariableXfbInfo::kInvalid);

    if (arrayIndex != ShaderInterfaceVariableXfbInfo::kInvalid)
    {
        xfb->arrayElements.emplace_back();
        xfb = &xfb->arrayElements.back();
    }

    xfb->pod.buffer        = xfbBuffer;
    xfb->pod.offset        = xfbOffset;
    xfb->pod.stride        = xfbStride;
    xfb->pod.arraySize     = arraySize;
    xfb->pod.columnCount   = columnCount;
    xfb->pod.rowCount      = rowCount;
    xfb->pod.arrayIndex    = arrayIndex;
    xfb->pod.componentType = componentType;
}

void AssignTransformFeedbackEmulationBindings(gl::ShaderType shaderType,
                                              const gl::ProgramExecutable &programExecutable,
                                              bool isTransformFeedbackStage,
                                              SpvProgramInterfaceInfo *programInterfaceInfo,
                                              ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    size_t bufferCount = 0;
    if (isTransformFeedbackStage)
    {
        ASSERT(!programExecutable.getLinkedTransformFeedbackVaryings().empty());
        const bool isInterleaved =
            programExecutable.getTransformFeedbackBufferMode() == GL_INTERLEAVED_ATTRIBS;
        bufferCount =
            isInterleaved ? 1 : programExecutable.getLinkedTransformFeedbackVaryings().size();
    }

    // Add entries for the transform feedback buffers to the info map, so they can have correct
    // set/binding.
    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
    {
        AddResourceInfo(variableInfoMapOut, gl::ShaderBitSet().set(shaderType), shaderType,
                        SpvGetXfbBufferBlockId(bufferIndex),
                        ToUnderlying(DescriptorSetIndex::UniformsAndXfb),
                        programInterfaceInfo->currentUniformBindingIndex);
        ++programInterfaceInfo->currentUniformBindingIndex;
    }

    // Remove inactive transform feedback buffers.
    for (uint32_t bufferIndex = static_cast<uint32_t>(bufferCount);
         bufferIndex < gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS; ++bufferIndex)
    {
        variableInfoMapOut->add(shaderType, SpvGetXfbBufferBlockId(bufferIndex));
    }
}

bool IsFirstRegisterOfVarying(const gl::PackedVaryingRegister &varyingReg,
                              bool allowFields,
                              uint32_t expectArrayIndex)
{
    const gl::PackedVarying &varying = *varyingReg.packedVarying;

    // In Vulkan GLSL, struct fields are not allowed to have location assignments.  The varying of a
    // struct type is thus given a location equal to the one assigned to its first field.  With I/O
    // blocks, transform feedback can capture an arbitrary field.  In that case, we need to look at
    // every field, not just the first one.
    if (!allowFields && varying.isStructField() &&
        (varying.fieldIndex > 0 || varying.secondaryFieldIndex > 0))
    {
        return false;
    }

    // Similarly, assign array varying locations to the assigned location of the first element.
    // Transform feedback may capture array elements, so if a specific non-zero element is
    // requested, accept that only.
    if (varyingReg.varyingArrayIndex != expectArrayIndex ||
        (varying.arrayIndex != GL_INVALID_INDEX && varying.arrayIndex != expectArrayIndex))
    {
        return false;
    }

    // Similarly, assign matrix varying locations to the assigned location of the first row.
    if (varyingReg.varyingRowIndex != 0)
    {
        return false;
    }

    return true;
}

void AssignAttributeLocations(const gl::ProgramExecutable &programExecutable,
                              gl::ShaderType shaderType,
                              ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    const std::array<std::string, 2> implicitInputs = {"gl_VertexID", "gl_InstanceID"};
    gl::AttributesMask isLocationAssigned;
    bool hasAliasingAttributes = false;

    // Assign attribute locations for the vertex shader.
    for (const gl::ProgramInput &attribute : programExecutable.getProgramInputs())
    {
        ASSERT(attribute.isActive());

        if (std::find(implicitInputs.begin(), implicitInputs.end(), attribute.name) !=
            implicitInputs.end())
        {
            continue;
        }

        const uint8_t colCount = static_cast<uint8_t>(gl::VariableColumnCount(attribute.getType()));
        const uint8_t rowCount = static_cast<uint8_t>(gl::VariableRowCount(attribute.getType()));
        const bool isMatrix    = colCount > 1 && rowCount > 1;

        const uint8_t componentCount = isMatrix ? rowCount : colCount;
        const uint8_t locationCount  = isMatrix ? colCount : rowCount;

        AddLocationInfo(variableInfoMapOut, shaderType, attribute.getId(), attribute.getLocation(),
                        ShaderInterfaceVariableInfo::kInvalid, componentCount, locationCount);

        // Detect if there are aliasing attributes.
        if (!hasAliasingAttributes &&
            programExecutable.getLinkedShaderVersion(gl::ShaderType::Vertex) == 100)
        {
            for (uint8_t offset = 0; offset < locationCount; ++offset)
            {
                uint32_t location = attribute.getLocation() + offset;

                // If there's aliasing, no need for futher processing.
                if (isLocationAssigned.test(location))
                {
                    hasAliasingAttributes = true;
                    break;
                }

                isLocationAssigned.set(location);
            }
        }
    }

    if (hasAliasingAttributes)
    {
        variableInfoMapOut->setHasAliasingAttributes();
    }
}

void AssignSecondaryOutputLocations(const gl::ProgramExecutable &programExecutable,
                                    ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    const auto &secondaryOutputLocations = programExecutable.getSecondaryOutputLocations();
    const auto &outputVariables          = programExecutable.getOutputVariables();

    // Handle EXT_blend_func_extended secondary outputs (ones with index=1)
    for (const gl::VariableLocation &outputLocation : secondaryOutputLocations)
    {
        if (outputLocation.arrayIndex == 0 && outputLocation.used() && !outputLocation.ignored)
        {
            const gl::ProgramOutput &outputVar = outputVariables[outputLocation.index];

            uint32_t location = 0;
            if (outputVar.pod.location != -1)
            {
                location = outputVar.pod.location;
            }

            ShaderInterfaceVariableInfo *info =
                AddLocationInfo(variableInfoMapOut, gl::ShaderType::Fragment, outputVar.pod.id,
                                location, ShaderInterfaceVariableInfo::kInvalid, 0, 0);

            // Index 1 is used to specify that the color be used as the second color input to
            // the blend equation
            info->index = 1;

            ASSERT(!outputVar.isArray() || outputVar.getOutermostArraySize() == 1);
            info->isArray = outputVar.isArray();
        }
    }
    // Handle secondary outputs for ESSL version less than 3.00
    if (programExecutable.hasLinkedShaderStage(gl::ShaderType::Fragment) &&
        programExecutable.getLinkedShaderVersion(gl::ShaderType::Fragment) == 100)
    {
        const std::array<std::string, 2> secondaryFrag = {"gl_SecondaryFragColorEXT",
                                                          "gl_SecondaryFragDataEXT"};

        for (const gl::ProgramOutput &outputVar : outputVariables)
        {
            if (std::find(secondaryFrag.begin(), secondaryFrag.end(), outputVar.name) !=
                secondaryFrag.end())
            {
                ShaderInterfaceVariableInfo *info =
                    AddLocationInfo(variableInfoMapOut, gl::ShaderType::Fragment, outputVar.pod.id,
                                    0, ShaderInterfaceVariableInfo::kInvalid, 0, 0);

                info->index = 1;

                ASSERT(!outputVar.isArray() || outputVar.getOutermostArraySize() == 1);
                info->isArray = outputVar.isArray();

                // SecondaryFragColor and SecondaryFragData cannot be present simultaneously.
                break;
            }
        }
    }
}

void AssignOutputLocations(const gl::ProgramExecutable &programExecutable,
                           const gl::ShaderType shaderType,
                           ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    // Assign output locations for the fragment shader.
    ASSERT(shaderType == gl::ShaderType::Fragment);

    const auto &outputLocations                      = programExecutable.getOutputLocations();
    const auto &outputVariables                      = programExecutable.getOutputVariables();
    const std::array<std::string, 3> implicitOutputs = {"gl_FragDepth", "gl_SampleMask",
                                                        "gl_FragStencilRefARB"};

    for (const gl::VariableLocation &outputLocation : outputLocations)
    {
        if (outputLocation.arrayIndex == 0 && outputLocation.used() && !outputLocation.ignored)
        {
            const gl::ProgramOutput &outputVar = outputVariables[outputLocation.index];

            uint32_t location = 0;
            if (outputVar.pod.location != -1)
            {
                location = outputVar.pod.location;
            }
            else if (std::find(implicitOutputs.begin(), implicitOutputs.end(), outputVar.name) ==
                     implicitOutputs.end())
            {
                // If there is only one output, it is allowed not to have a location qualifier, in
                // which case it defaults to 0.  GLSL ES 3.00 spec, section 4.3.8.2.
                ASSERT(CountExplicitOutputs(outputVariables.begin(), outputVariables.end(),
                                            implicitOutputs.begin(), implicitOutputs.end()) == 1);
            }

            AddLocationInfo(variableInfoMapOut, shaderType, outputVar.pod.id, location,
                            ShaderInterfaceVariableInfo::kInvalid, 0, 0);
        }
    }
    // Handle outputs for ESSL version less than 3.00
    if (programExecutable.hasLinkedShaderStage(gl::ShaderType::Fragment) &&
        programExecutable.getLinkedShaderVersion(gl::ShaderType::Fragment) == 100)
    {
        for (const gl::ProgramOutput &outputVar : outputVariables)
        {
            if (outputVar.name == "gl_FragColor" || outputVar.name == "gl_FragData")
            {
                AddLocationInfo(variableInfoMapOut, gl::ShaderType::Fragment, outputVar.pod.id, 0,
                                ShaderInterfaceVariableInfo::kInvalid, 0, 0);
            }
        }
    }

    AssignSecondaryOutputLocations(programExecutable, variableInfoMapOut);
}

void AssignVaryingLocations(const SpvSourceOptions &options,
                            const gl::VaryingPacking &varyingPacking,
                            const gl::ShaderType shaderType,
                            const gl::ShaderType frontShaderType,
                            SpvProgramInterfaceInfo *programInterfaceInfo,
                            ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    uint32_t locationsUsedForEmulation = programInterfaceInfo->locationsUsedForXfbExtension;

    // Assign varying locations.
    for (const gl::PackedVaryingRegister &varyingReg : varyingPacking.getRegisterList())
    {
        if (!IsFirstRegisterOfVarying(varyingReg, false, 0))
        {
            continue;
        }

        const gl::PackedVarying &varying = *varyingReg.packedVarying;

        uint32_t location  = varyingReg.registerRow + locationsUsedForEmulation;
        uint32_t component = ShaderInterfaceVariableInfo::kInvalid;
        if (varyingReg.registerColumn > 0)
        {
            ASSERT(!varying.varying().isStruct());
            ASSERT(!gl::IsMatrixType(varying.varying().type));
            component = varyingReg.registerColumn;
        }

        if (varying.frontVarying.varying && (varying.frontVarying.stage == shaderType))
        {
            AddVaryingLocationInfo(variableInfoMapOut, varying.frontVarying, location, component);
        }

        if (varying.backVarying.varying && (varying.backVarying.stage == shaderType))
        {
            AddVaryingLocationInfo(variableInfoMapOut, varying.backVarying, location, component);
        }
    }

    // Add an entry for inactive varyings.
    const gl::ShaderMap<std::vector<uint32_t>> &inactiveVaryingIds =
        varyingPacking.getInactiveVaryingIds();
    for (const uint32_t varyingId : inactiveVaryingIds[shaderType])
    {
        // If id is already in the map, it will automatically have marked all other stages inactive.
        if (variableInfoMapOut->hasVariable(shaderType, varyingId))
        {
            continue;
        }

        // Otherwise, add an entry for it with all locations inactive.
        ShaderInterfaceVariableInfo &info = variableInfoMapOut->addOrGet(shaderType, varyingId);
        ASSERT(info.location == ShaderInterfaceVariableInfo::kInvalid);
    }

    // Add an entry for gl_PerVertex, for use with transform feedback capture of built-ins.
    ShaderInterfaceVariableInfo &info =
        variableInfoMapOut->addOrGet(shaderType, sh::vk::spirv::kIdOutputPerVertexBlock);
    info.activeStages.set(shaderType);
}

// Calculates XFB layout qualifier arguments for each transform feedback varying. Stores calculated
// values for the SPIR-V transformation.
void AssignTransformFeedbackQualifiers(const gl::ProgramExecutable &programExecutable,
                                       const gl::VaryingPacking &varyingPacking,
                                       const gl::ShaderType shaderType,
                                       bool usesExtension,
                                       ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    const std::vector<gl::TransformFeedbackVarying> &tfVaryings =
        programExecutable.getLinkedTransformFeedbackVaryings();
    const std::vector<GLsizei> &varyingStrides = programExecutable.getTransformFeedbackStrides();
    const bool isInterleaved =
        programExecutable.getTransformFeedbackBufferMode() == GL_INTERLEAVED_ATTRIBS;

    uint32_t currentOffset = 0;
    uint32_t currentStride = 0;
    uint32_t bufferIndex   = 0;

    for (uint32_t varyingIndex = 0; varyingIndex < tfVaryings.size(); ++varyingIndex)
    {
        if (isInterleaved)
        {
            bufferIndex = 0;
            if (varyingIndex > 0)
            {
                const gl::TransformFeedbackVarying &prev = tfVaryings[varyingIndex - 1];
                currentOffset += prev.size() * gl::VariableExternalSize(prev.type);
            }
            currentStride = varyingStrides[0];
        }
        else
        {
            bufferIndex   = varyingIndex;
            currentOffset = 0;
            currentStride = varyingStrides[varyingIndex];
        }

        const gl::TransformFeedbackVarying &tfVarying = tfVaryings[varyingIndex];
        const gl::UniformTypeInfo &uniformInfo        = gl::GetUniformTypeInfo(tfVarying.type);
        const uint32_t varyingSize =
            tfVarying.isArray() ? tfVarying.size() : ShaderInterfaceVariableXfbInfo::kInvalid;

        if (tfVarying.isBuiltIn())
        {
            if (usesExtension && tfVarying.name == "gl_Position")
            {
                // With the extension, gl_Position is captured via a special varying.
                SetXfbInfo(variableInfoMapOut, shaderType, sh::vk::spirv::kIdXfbExtensionPosition,
                           -1, bufferIndex, currentOffset, currentStride, varyingSize,
                           uniformInfo.columnCount, uniformInfo.rowCount,
                           ShaderInterfaceVariableXfbInfo::kInvalid, uniformInfo.componentType);
            }
            else
            {
                // gl_PerVertex is always defined as:
                //
                //    Field 0: gl_Position
                //    Field 1: gl_PointSize
                //    Field 2: gl_ClipDistance
                //    Field 3: gl_CullDistance
                //
                // With the extension, all fields except gl_Position can be captured directly by
                // decorating gl_PerVertex fields.
                int fieldIndex                                                              = -1;
                constexpr int kPerVertexMemberCount                                         = 4;
                constexpr std::array<const char *, kPerVertexMemberCount> kPerVertexMembers = {
                    "gl_Position",
                    "gl_PointSize",
                    "gl_ClipDistance",
                    "gl_CullDistance",
                };
                for (int index = 0; index < kPerVertexMemberCount; ++index)
                {
                    if (tfVarying.name == kPerVertexMembers[index])
                    {
                        fieldIndex = index;
                        break;
                    }
                }
                ASSERT(fieldIndex != -1);
                ASSERT(!usesExtension || fieldIndex > 0);

                SetXfbInfo(variableInfoMapOut, shaderType, sh::vk::spirv::kIdOutputPerVertexBlock,
                           fieldIndex, bufferIndex, currentOffset, currentStride, varyingSize,
                           uniformInfo.columnCount, uniformInfo.rowCount,
                           ShaderInterfaceVariableXfbInfo::kInvalid, uniformInfo.componentType);
            }

            continue;
        }
        // Note: capturing individual array elements using the Vulkan transform feedback extension
        // is currently not supported due to limitations in the extension.
        // ANGLE supports capturing the whole array.
        // http://anglebug.com/42262773
        if (usesExtension && tfVarying.isArray() && tfVarying.arrayIndex != GL_INVALID_INDEX)
        {
            continue;
        }

        // Find the varying with this name.  If a struct is captured, we would be iterating over its
        // fields.  This is done when the first field of the struct is visited.  For I/O blocks on
        // the other hand, we need to decorate the exact member that is captured (as whole-block
        // capture is not supported).
        const gl::PackedVarying *originalVarying = nullptr;
        for (const gl::PackedVaryingRegister &varyingReg : varyingPacking.getRegisterList())
        {
            const uint32_t arrayIndex =
                tfVarying.arrayIndex == GL_INVALID_INDEX ? 0 : tfVarying.arrayIndex;
            if (!IsFirstRegisterOfVarying(varyingReg, tfVarying.isShaderIOBlock, arrayIndex))
            {
                continue;
            }

            const gl::PackedVarying *varying = varyingReg.packedVarying;

            if (tfVarying.isShaderIOBlock)
            {
                if (varying->frontVarying.parentStructName == tfVarying.structOrBlockName)
                {
                    size_t pos = tfVarying.name.find_first_of(".");
                    std::string fieldName =
                        pos == std::string::npos ? tfVarying.name : tfVarying.name.substr(pos + 1);

                    if (fieldName == varying->frontVarying.varying->name.c_str())
                    {
                        originalVarying = varying;
                        break;
                    }
                }
            }
            else if (varying->frontVarying.varying->name == tfVarying.name)
            {
                originalVarying = varying;
                break;
            }
        }

        if (originalVarying)
        {
            const int fieldIndex = tfVarying.isShaderIOBlock ? originalVarying->fieldIndex : -1;
            const uint32_t arrayIndex = tfVarying.arrayIndex == GL_INVALID_INDEX
                                            ? ShaderInterfaceVariableXfbInfo::kInvalid
                                            : tfVarying.arrayIndex;

            // Set xfb info for this varying.  AssignVaryingLocations should have already added
            // location information for these varyings.
            SetXfbInfo(variableInfoMapOut, shaderType, originalVarying->frontVarying.varying->id,
                       fieldIndex, bufferIndex, currentOffset, currentStride, varyingSize,
                       uniformInfo.columnCount, uniformInfo.rowCount, arrayIndex,
                       uniformInfo.componentType);
        }
    }
}

void AssignUniformBindings(const SpvSourceOptions &options,
                           const gl::ProgramExecutable &programExecutable,
                           SpvProgramInterfaceInfo *programInterfaceInfo,
                           ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    for (const gl::ShaderType shaderType : programExecutable.getLinkedShaderStages())
    {
        AddResourceInfo(variableInfoMapOut, gl::ShaderBitSet().set(shaderType), shaderType,
                        sh::vk::spirv::kIdDefaultUniformsBlock,
                        ToUnderlying(DescriptorSetIndex::UniformsAndXfb),
                        programInterfaceInfo->currentUniformBindingIndex);
        ++programInterfaceInfo->currentUniformBindingIndex;

        // Assign binding to the driver uniforms block
        AddResourceInfoToAllStages(variableInfoMapOut, shaderType,
                                   sh::vk::spirv::kIdDriverUniformsBlock,
                                   ToUnderlying(DescriptorSetIndex::Internal), 0);
    }
}

void AssignInputAttachmentBindings(const SpvSourceOptions &options,
                                   const gl::ProgramExecutable &programExecutable,
                                   SpvProgramInterfaceInfo *programInterfaceInfo,
                                   ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    if (!programExecutable.hasLinkedShaderStage(gl::ShaderType::Fragment))
    {
        return;
    }

    if (!programExecutable.usesColorFramebufferFetch() &&
        !programExecutable.usesDepthFramebufferFetch() &&
        !programExecutable.usesStencilFramebufferFetch())
    {
        return;
    }

    uint32_t baseInputAttachmentBindingIndex =
        programInterfaceInfo->currentShaderResourceBindingIndex;
    const gl::ShaderBitSet activeShaders{gl::ShaderType::Fragment};

    // If depth/stencil framebuffer fetch is enabled, place their bindings before the color
    // attachments.  When binding descriptors, this results in a smaller gap that would need to be
    // filled with bogus bindings.
    if (options.supportsDepthStencilInputAttachments)
    {
        AddResourceInfo(variableInfoMapOut, activeShaders, gl::ShaderType::Fragment,
                        sh::vk::spirv::kIdDepthInputAttachment,
                        ToUnderlying(DescriptorSetIndex::ShaderResource),
                        baseInputAttachmentBindingIndex++);
        AddResourceInfo(variableInfoMapOut, activeShaders, gl::ShaderType::Fragment,
                        sh::vk::spirv::kIdStencilInputAttachment,
                        ToUnderlying(DescriptorSetIndex::ShaderResource),
                        baseInputAttachmentBindingIndex++);

        programInterfaceInfo->currentShaderResourceBindingIndex += 2;
    }

    if (programExecutable.usesColorFramebufferFetch())
    {
        // sh::vk::spirv::ReservedIds supports max 8 draw buffers
        ASSERT(options.maxColorInputAttachmentCount <= 8);
        ASSERT(options.maxColorInputAttachmentCount >= 1);

        for (size_t index : programExecutable.getFragmentInoutIndices())
        {
            const uint32_t inputAttachmentBindingIndex =
                baseInputAttachmentBindingIndex + static_cast<uint32_t>(index);

            AddResourceInfo(variableInfoMapOut, activeShaders, gl::ShaderType::Fragment,
                            sh::vk::spirv::kIdInputAttachment0 + static_cast<uint32_t>(index),
                            ToUnderlying(DescriptorSetIndex::ShaderResource),
                            inputAttachmentBindingIndex);
        }
    }

    // For input attachment uniform, the descriptor set binding indices are allocated as much as
    // the maximum draw buffers.
    programInterfaceInfo->currentShaderResourceBindingIndex += options.maxColorInputAttachmentCount;
}

void AssignInterfaceBlockBindings(const SpvSourceOptions &options,
                                  const gl::ProgramExecutable &programExecutable,
                                  const std::vector<gl::InterfaceBlock> &blocks,

                                  SpvProgramInterfaceInfo *programInterfaceInfo,
                                  ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    for (uint32_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex)
    {
        const gl::InterfaceBlock &block = blocks[blockIndex];

        // TODO: http://anglebug.com/42263134: All blocks should be active
        const gl::ShaderBitSet activeShaders =
            programExecutable.getLinkedShaderStages() & block.activeShaders();
        if (activeShaders.none())
        {
            continue;
        }

        const bool isIndexZero = !block.pod.isArray || block.pod.arrayElement == 0;
        if (!isIndexZero)
        {
            continue;
        }

        variableInfoMapOut->addResource(activeShaders, block.getIds(),
                                        ToUnderlying(DescriptorSetIndex::ShaderResource),
                                        programInterfaceInfo->currentShaderResourceBindingIndex++);
    }
}

void AssignAtomicCounterBufferBindings(const SpvSourceOptions &options,
                                       const gl::ProgramExecutable &programExecutable,
                                       SpvProgramInterfaceInfo *programInterfaceInfo,
                                       ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    const std::vector<gl::AtomicCounterBuffer> &buffers =
        programExecutable.getAtomicCounterBuffers();

    if (buffers.size() == 0)
    {
        return;
    }

    const gl::ShaderBitSet activeShaders = programExecutable.getLinkedShaderStages();
    ASSERT(activeShaders.any());

    gl::ShaderMap<uint32_t> ids = {};
    for (const gl::ShaderType shaderType : activeShaders)
    {
        ids[shaderType] = sh::vk::spirv::kIdAtomicCounterBlock;
    }

    variableInfoMapOut->addResource(activeShaders, ids,
                                    ToUnderlying(DescriptorSetIndex::ShaderResource),
                                    programInterfaceInfo->currentShaderResourceBindingIndex++);
}

void AssignImageBindings(const SpvSourceOptions &options,
                         const gl::ProgramExecutable &programExecutable,
                         SpvProgramInterfaceInfo *programInterfaceInfo,
                         ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    const std::vector<gl::LinkedUniform> &uniforms = programExecutable.getUniforms();
    const gl::RangeUI &imageUniformRange           = programExecutable.getImageUniformRange();
    for (unsigned int uniformIndex : imageUniformRange)
    {
        const gl::LinkedUniform &imageUniform = uniforms[uniformIndex];

        // TODO: http://anglebug.com/42263134: All uniforms should be active
        const gl::ShaderBitSet activeShaders =
            programExecutable.getLinkedShaderStages() & imageUniform.activeShaders();
        if (activeShaders.none())
        {
            continue;
        }

        const bool isIndexZero =
            UniformNameIsIndexZero(programExecutable.getUniformNameByIndex(uniformIndex));
        if (!isIndexZero)
        {
            continue;
        }

        variableInfoMapOut->addResource(activeShaders, imageUniform.getIds(),
                                        ToUnderlying(DescriptorSetIndex::ShaderResource),
                                        programInterfaceInfo->currentShaderResourceBindingIndex++);
    }
}

void AssignNonTextureBindings(const SpvSourceOptions &options,
                              const gl::ProgramExecutable &programExecutable,
                              SpvProgramInterfaceInfo *programInterfaceInfo,
                              ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    AssignInputAttachmentBindings(options, programExecutable, programInterfaceInfo,
                                  variableInfoMapOut);

    const std::vector<gl::InterfaceBlock> &uniformBlocks = programExecutable.getUniformBlocks();
    AssignInterfaceBlockBindings(options, programExecutable, uniformBlocks, programInterfaceInfo,
                                 variableInfoMapOut);

    const std::vector<gl::InterfaceBlock> &storageBlocks =
        programExecutable.getShaderStorageBlocks();
    AssignInterfaceBlockBindings(options, programExecutable, storageBlocks, programInterfaceInfo,
                                 variableInfoMapOut);

    AssignAtomicCounterBufferBindings(options, programExecutable, programInterfaceInfo,
                                      variableInfoMapOut);

    AssignImageBindings(options, programExecutable, programInterfaceInfo, variableInfoMapOut);
}

void AssignTextureBindings(const SpvSourceOptions &options,
                           const gl::ProgramExecutable &programExecutable,
                           SpvProgramInterfaceInfo *programInterfaceInfo,
                           ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    // Assign textures to a descriptor set and binding.
    const std::vector<gl::LinkedUniform> &uniforms = programExecutable.getUniforms();
    const gl::RangeUI &samplerUniformRange         = programExecutable.getSamplerUniformRange();

    for (unsigned int uniformIndex : samplerUniformRange)
    {
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];

        // TODO: http://anglebug.com/42263134: All uniforms should be active
        const gl::ShaderBitSet activeShaders =
            programExecutable.getLinkedShaderStages() & samplerUniform.activeShaders();
        if (activeShaders.none())
        {
            continue;
        }

        const bool isIndexZero =
            UniformNameIsIndexZero(programExecutable.getUniformNameByIndex(uniformIndex));
        if (!isIndexZero)
        {
            continue;
        }

        variableInfoMapOut->addResource(activeShaders, samplerUniform.getIds(),
                                        ToUnderlying(DescriptorSetIndex::Texture),
                                        programInterfaceInfo->currentTextureBindingIndex++);
    }
}

bool IsNonSemanticInstruction(const uint32_t *instruction)
{
    // To avoid parsing the numerous GLSL OpExtInst instructions, take a quick peek at the set and
    // skip instructions that aren't non-semantic.
    return instruction[3] == sh::vk::spirv::kIdNonSemanticInstructionSet;
}

enum class EntryPointList
{
    // Prior to SPIR-V 1.4, only the Input and Output variables are listed in OpEntryPoint.
    InterfaceVariables,
    // Since SPIR-V 1.4, all global variables must be listed in OpEntryPoint.
    GlobalVariables,
};

// Base class for SPIR-V transformations.
class SpirvTransformerBase : angle::NonCopyable
{
  public:
    SpirvTransformerBase(const spirv::Blob &spirvBlobIn,
                         const ShaderInterfaceVariableInfoMap &variableInfoMap,
                         spirv::Blob *spirvBlobOut)
        : mSpirvBlobIn(spirvBlobIn), mVariableInfoMap(variableInfoMap), mSpirvBlobOut(spirvBlobOut)
    {
        gl::ShaderBitSet allStages;
        allStages.set();
        mBuiltinVariableInfo.activeStages = allStages;
    }

    std::vector<const ShaderInterfaceVariableInfo *> &getVariableInfoByIdMap()
    {
        return mVariableInfoById;
    }

    static spirv::IdRef GetNewId(spirv::Blob *blob);
    spirv::IdRef getNewId();

    EntryPointList entryPointList() const { return mEntryPointList; }
    spv::StorageClass storageBufferStorageClass() const { return mStorageBufferStorageClass; }

  protected:
    // Common utilities
    void onTransformBegin();
    const uint32_t *getCurrentInstruction(spv::Op *opCodeOut, uint32_t *wordCountOut) const;
    void copyInstruction(const uint32_t *instruction, size_t wordCount);

    // SPIR-V to transform:
    const spirv::Blob &mSpirvBlobIn;

    // Input shader variable info map:
    const ShaderInterfaceVariableInfoMap &mVariableInfoMap;

    // Transformed SPIR-V:
    spirv::Blob *mSpirvBlobOut;

    // Traversal state:
    size_t mCurrentWord       = 0;
    bool mIsInFunctionSection = false;

    // Transformation state:

    // Required behavior based on SPIR-V version.
    EntryPointList mEntryPointList               = EntryPointList::InterfaceVariables;
    spv::StorageClass mStorageBufferStorageClass = spv::StorageClassUniform;

    // Shader variable info per id, if id is a shader variable.
    std::vector<const ShaderInterfaceVariableInfo *> mVariableInfoById;
    ShaderInterfaceVariableInfo mBuiltinVariableInfo;
};

void SpirvTransformerBase::onTransformBegin()
{
    // The translator succeeded in outputting SPIR-V, so we assume it's valid.
    ASSERT(mSpirvBlobIn.size() >= spirv::kHeaderIndexInstructions);
    // Since SPIR-V comes from a local call to the translator, it necessarily has the same
    // endianness as the running architecture, so no byte-swapping is necessary.
    ASSERT(mSpirvBlobIn[spirv::kHeaderIndexMagic] == spv::MagicNumber);

    // Make sure the transformer is not reused to avoid having to reinitialize it here.
    ASSERT(mCurrentWord == 0);
    ASSERT(mIsInFunctionSection == false);

    // Make sure the spirv::Blob is not reused.
    ASSERT(mSpirvBlobOut->empty());

    // Copy the header to SPIR-V blob, we need that to be defined for SpirvTransformerBase::getNewId
    // to work.
    mSpirvBlobOut->assign(mSpirvBlobIn.begin(),
                          mSpirvBlobIn.begin() + spirv::kHeaderIndexInstructions);

    mCurrentWord = spirv::kHeaderIndexInstructions;

    if (mSpirvBlobIn[spirv::kHeaderIndexVersion] >= spirv::kVersion_1_4)
    {
        mEntryPointList            = EntryPointList::GlobalVariables;
        mStorageBufferStorageClass = spv::StorageClassStorageBuffer;
    }
}

const uint32_t *SpirvTransformerBase::getCurrentInstruction(spv::Op *opCodeOut,
                                                            uint32_t *wordCountOut) const
{
    ASSERT(mCurrentWord < mSpirvBlobIn.size());
    const uint32_t *instruction = &mSpirvBlobIn[mCurrentWord];

    spirv::GetInstructionOpAndLength(instruction, opCodeOut, wordCountOut);

    // The translator succeeded in outputting SPIR-V, so we assume it's valid.
    ASSERT(mCurrentWord + *wordCountOut <= mSpirvBlobIn.size());

    return instruction;
}

void SpirvTransformerBase::copyInstruction(const uint32_t *instruction, size_t wordCount)
{
    mSpirvBlobOut->insert(mSpirvBlobOut->end(), instruction, instruction + wordCount);
}

spirv::IdRef SpirvTransformerBase::GetNewId(spirv::Blob *blob)
{
    return spirv::IdRef((*blob)[spirv::kHeaderIndexIndexBound]++);
}

spirv::IdRef SpirvTransformerBase::getNewId()
{
    return GetNewId(mSpirvBlobOut);
}

enum class TransformationState
{
    Transformed,
    Unchanged,
};

class SpirvNonSemanticInstructions final : angle::NonCopyable
{
  public:
    SpirvNonSemanticInstructions(bool isLastPass) : mIsLastPass(isLastPass) {}

    // Returns whether this is a non-semantic instruction (as opposed to GLSL extended
    // instructions).  If it is non-semantic, returns the instruction code.
    bool visitExtInst(const uint32_t *instruction, sh::vk::spirv::NonSemanticInstruction *instOut);

    // Cleans up non-semantic instructions in the last SPIR-V pass.
    TransformationState transformExtInst(const uint32_t *instruction);

    bool hasSampleRateShading() const
    {
        return (mOverviewFlags & sh::vk::spirv::kOverviewHasSampleRateShadingMask) != 0;
    }
    bool hasSampleID() const
    {
        return (mOverviewFlags & sh::vk::spirv::kOverviewHasSampleIDMask) != 0;
    }
    bool hasOutputPerVertex() const
    {
        return (mOverviewFlags & sh::vk::spirv::kOverviewHasOutputPerVertexMask) != 0;
    }

  private:
    // Whether this is the last SPIR-V pass. The non-semantics instructions are removed from the
    // SPIR-V in the last pass.
    const bool mIsLastPass;

    uint32_t mOverviewFlags;
};

bool SpirvNonSemanticInstructions::visitExtInst(const uint32_t *instruction,
                                                sh::vk::spirv::NonSemanticInstruction *instOut)
{
    if (!IsNonSemanticInstruction(instruction))
    {
        return false;
    }

    spirv::IdResultType typeId;
    spirv::IdResult id;
    spirv::IdRef set;
    spirv::LiteralExtInstInteger extInst;
    spirv::ParseExtInst(instruction, &typeId, &id, &set, &extInst, nullptr);

    ASSERT(set == sh::vk::spirv::kIdNonSemanticInstructionSet);
    const uint32_t inst = extInst & sh::vk::spirv::kNonSemanticInstructionMask;

    // Recover the additional overview flags placed in the instruction id.
    if (inst == sh::vk::spirv::kNonSemanticOverview)
    {
        mOverviewFlags = extInst & ~sh::vk::spirv::kNonSemanticInstructionMask;
    }

    *instOut = static_cast<sh::vk::spirv::NonSemanticInstruction>(inst);

    return true;
}

TransformationState SpirvNonSemanticInstructions::transformExtInst(const uint32_t *instruction)
{
    return IsNonSemanticInstruction(instruction) && mIsLastPass ? TransformationState::Transformed
                                                                : TransformationState::Unchanged;
}

namespace ID
{
namespace
{
[[maybe_unused]] constexpr spirv::IdRef EntryPoint(sh::vk::spirv::kIdEntryPoint);
[[maybe_unused]] constexpr spirv::IdRef Void(sh::vk::spirv::kIdVoid);
[[maybe_unused]] constexpr spirv::IdRef Float(sh::vk::spirv::kIdFloat);
[[maybe_unused]] constexpr spirv::IdRef Vec2(sh::vk::spirv::kIdVec2);
[[maybe_unused]] constexpr spirv::IdRef Vec3(sh::vk::spirv::kIdVec3);
[[maybe_unused]] constexpr spirv::IdRef Vec4(sh::vk::spirv::kIdVec4);
[[maybe_unused]] constexpr spirv::IdRef Mat2(sh::vk::spirv::kIdMat2);
[[maybe_unused]] constexpr spirv::IdRef Mat3(sh::vk::spirv::kIdMat3);
[[maybe_unused]] constexpr spirv::IdRef Mat4(sh::vk::spirv::kIdMat4);
[[maybe_unused]] constexpr spirv::IdRef Int(sh::vk::spirv::kIdInt);
[[maybe_unused]] constexpr spirv::IdRef IVec4(sh::vk::spirv::kIdIVec4);
[[maybe_unused]] constexpr spirv::IdRef Uint(sh::vk::spirv::kIdUint);
[[maybe_unused]] constexpr spirv::IdRef IntZero(sh::vk::spirv::kIdIntZero);
[[maybe_unused]] constexpr spirv::IdRef IntOne(sh::vk::spirv::kIdIntOne);
[[maybe_unused]] constexpr spirv::IdRef IntTwo(sh::vk::spirv::kIdIntTwo);
[[maybe_unused]] constexpr spirv::IdRef IntThree(sh::vk::spirv::kIdIntThree);
[[maybe_unused]] constexpr spirv::IdRef IntInputTypePointer(sh::vk::spirv::kIdIntInputTypePointer);
[[maybe_unused]] constexpr spirv::IdRef Vec4OutputTypePointer(
    sh::vk::spirv::kIdVec4OutputTypePointer);
[[maybe_unused]] constexpr spirv::IdRef IVec4FunctionTypePointer(
    sh::vk::spirv::kIdIVec4FunctionTypePointer);
[[maybe_unused]] constexpr spirv::IdRef OutputPerVertexTypePointer(
    sh::vk::spirv::kIdOutputPerVertexTypePointer);
[[maybe_unused]] constexpr spirv::IdRef TransformPositionFunction(
    sh::vk::spirv::kIdTransformPositionFunction);
[[maybe_unused]] constexpr spirv::IdRef XfbEmulationGetOffsetsFunction(
    sh::vk::spirv::kIdXfbEmulationGetOffsetsFunction);
[[maybe_unused]] constexpr spirv::IdRef SampleID(sh::vk::spirv::kIdSampleID);

[[maybe_unused]] constexpr spirv::IdRef InputPerVertexBlock(sh::vk::spirv::kIdInputPerVertexBlock);
[[maybe_unused]] constexpr spirv::IdRef OutputPerVertexBlock(
    sh::vk::spirv::kIdOutputPerVertexBlock);
[[maybe_unused]] constexpr spirv::IdRef OutputPerVertexVar(sh::vk::spirv::kIdOutputPerVertexVar);
[[maybe_unused]] constexpr spirv::IdRef XfbExtensionPosition(
    sh::vk::spirv::kIdXfbExtensionPosition);
[[maybe_unused]] constexpr spirv::IdRef XfbEmulationBufferBlockZero(
    sh::vk::spirv::kIdXfbEmulationBufferBlockZero);
[[maybe_unused]] constexpr spirv::IdRef XfbEmulationBufferBlockOne(
    sh::vk::spirv::kIdXfbEmulationBufferBlockOne);
[[maybe_unused]] constexpr spirv::IdRef XfbEmulationBufferBlockTwo(
    sh::vk::spirv::kIdXfbEmulationBufferBlockTwo);
[[maybe_unused]] constexpr spirv::IdRef XfbEmulationBufferBlockThree(
    sh::vk::spirv::kIdXfbEmulationBufferBlockThree);
}  // anonymous namespace
}  // namespace ID

// Helper class that trims input and output gl_PerVertex declarations to remove inactive builtins.
//
// gl_PerVertex is unique in that it's the only builtin of struct type.  This struct is pruned
// by removing trailing inactive members.  Note that intermediate stages, i.e. geometry and
// tessellation have two gl_PerVertex declarations, one for input and one for output.
class SpirvPerVertexTrimmer final : angle::NonCopyable
{
  public:
    SpirvPerVertexTrimmer(const SpvTransformOptions &options,
                          const ShaderInterfaceVariableInfoMap &variableInfoMap)
        : mInputPerVertexMaxActiveMember{gl::PerVertexMember::Position},
          mOutputPerVertexMaxActiveMember{gl::PerVertexMember::Position},
          mInputPerVertexMaxActiveMemberIndex(0),
          mOutputPerVertexMaxActiveMemberIndex(0)
    {
        const gl::PerVertexMemberBitSet inputPerVertexActiveMembers =
            variableInfoMap.getInputPerVertexActiveMembers()[options.shaderType];
        const gl::PerVertexMemberBitSet outputPerVertexActiveMembers =
            variableInfoMap.getOutputPerVertexActiveMembers()[options.shaderType];

        // Currently, this transformation does not trim inactive members in between two active
        // members.
        if (inputPerVertexActiveMembers.any())
        {
            mInputPerVertexMaxActiveMember = inputPerVertexActiveMembers.last();
        }
        if (outputPerVertexActiveMembers.any())
        {
            mOutputPerVertexMaxActiveMember = outputPerVertexActiveMembers.last();
        }
    }

    void visitMemberDecorate(spirv::IdRef id,
                             spirv::LiteralInteger member,
                             spv::Decoration decoration,
                             const spirv::LiteralIntegerList &valueList);

    TransformationState transformMemberDecorate(spirv::IdRef typeId,
                                                spirv::LiteralInteger member,
                                                spv::Decoration decoration);
    TransformationState transformMemberName(spirv::IdRef id,
                                            spirv::LiteralInteger member,
                                            const spirv::LiteralString &name);
    TransformationState transformTypeStruct(spirv::IdResult id,
                                            spirv::IdRefList *memberList,
                                            spirv::Blob *blobOut);

  private:
    bool isPerVertex(spirv::IdRef typeId) const
    {
        return typeId == ID::OutputPerVertexBlock || typeId == ID::InputPerVertexBlock;
    }
    uint32_t getPerVertexMaxActiveMember(spirv::IdRef typeId) const
    {
        ASSERT(isPerVertex(typeId));
        return typeId == ID::OutputPerVertexBlock ? mOutputPerVertexMaxActiveMemberIndex
                                                  : mInputPerVertexMaxActiveMemberIndex;
    }

    gl::PerVertexMember mInputPerVertexMaxActiveMember;
    gl::PerVertexMember mOutputPerVertexMaxActiveMember;

    // If gl_ClipDistance and gl_CullDistance are not used, they are missing from gl_PerVertex.  So
    // the index of gl_CullDistance may not be the same as the value of
    // gl::PerVertexMember::CullDistance.
    //
    // By looking at OpMemberDecorate %kIdInput/OutputPerVertexBlock <Index> BuiltIn <Member>, the
    // <Index> corresponding to mInput/OutputPerVertexMaxActiveMember is discovered and kept in
    // mInput/OutputPerVertexMaxActiveMemberIndex
    uint32_t mInputPerVertexMaxActiveMemberIndex;
    uint32_t mOutputPerVertexMaxActiveMemberIndex;
};

void SpirvPerVertexTrimmer::visitMemberDecorate(spirv::IdRef id,
                                                spirv::LiteralInteger member,
                                                spv::Decoration decoration,
                                                const spirv::LiteralIntegerList &valueList)
{
    if (decoration != spv::DecorationBuiltIn || !isPerVertex(id))
    {
        return;
    }

    // Map spv::BuiltIn to gl::PerVertexMember.
    ASSERT(!valueList.empty());
    const uint32_t builtIn              = valueList[0];
    gl::PerVertexMember perVertexMember = gl::PerVertexMember::Position;
    switch (builtIn)
    {
        case spv::BuiltInPosition:
            perVertexMember = gl::PerVertexMember::Position;
            break;
        case spv::BuiltInPointSize:
            perVertexMember = gl::PerVertexMember::PointSize;
            break;
        case spv::BuiltInClipDistance:
            perVertexMember = gl::PerVertexMember::ClipDistance;
            break;
        case spv::BuiltInCullDistance:
            perVertexMember = gl::PerVertexMember::CullDistance;
            break;
        default:
            UNREACHABLE();
    }

    if (id == ID::OutputPerVertexBlock && perVertexMember == mOutputPerVertexMaxActiveMember)
    {
        mOutputPerVertexMaxActiveMemberIndex = member;
    }
    else if (id == ID::InputPerVertexBlock && perVertexMember == mInputPerVertexMaxActiveMember)
    {
        mInputPerVertexMaxActiveMemberIndex = member;
    }
}

TransformationState SpirvPerVertexTrimmer::transformMemberDecorate(spirv::IdRef typeId,
                                                                   spirv::LiteralInteger member,
                                                                   spv::Decoration decoration)
{
    // Transform the following:
    //
    // - OpMemberDecorate %gl_PerVertex N BuiltIn B
    // - OpMemberDecorate %gl_PerVertex N Invariant
    // - OpMemberDecorate %gl_PerVertex N RelaxedPrecision
    if (!isPerVertex(typeId) ||
        (decoration != spv::DecorationBuiltIn && decoration != spv::DecorationInvariant &&
         decoration != spv::DecorationRelaxedPrecision))
    {
        return TransformationState::Unchanged;
    }

    // Drop stripped fields.
    return member > getPerVertexMaxActiveMember(typeId) ? TransformationState::Transformed
                                                        : TransformationState::Unchanged;
}

TransformationState SpirvPerVertexTrimmer::transformMemberName(spirv::IdRef id,
                                                               spirv::LiteralInteger member,
                                                               const spirv::LiteralString &name)
{
    // Remove the instruction if it's a stripped member of gl_PerVertex.
    return isPerVertex(id) && member > getPerVertexMaxActiveMember(id)
               ? TransformationState::Transformed
               : TransformationState::Unchanged;
}

TransformationState SpirvPerVertexTrimmer::transformTypeStruct(spirv::IdResult id,
                                                               spirv::IdRefList *memberList,
                                                               spirv::Blob *blobOut)
{
    if (!isPerVertex(id))
    {
        return TransformationState::Unchanged;
    }

    const uint32_t maxMembers = getPerVertexMaxActiveMember(id);

    // Change the definition of the gl_PerVertex struct by stripping unused fields at the end.
    const uint32_t memberCount = maxMembers + 1;
    memberList->resize_down(memberCount);

    spirv::WriteTypeStruct(blobOut, id, *memberList);

    return TransformationState::Transformed;
}

// Helper class that removes inactive varyings and replaces them with Private variables.
class SpirvInactiveVaryingRemover final : angle::NonCopyable
{
  public:
    SpirvInactiveVaryingRemover() {}

    void init(size_t indexCount);

    TransformationState transformAccessChain(spirv::IdResultType typeId,
                                             spirv::IdResult id,
                                             spirv::IdRef baseId,
                                             const spirv::IdRefList &indexList,
                                             spirv::Blob *blobOut);
    TransformationState transformDecorate(const ShaderInterfaceVariableInfo &info,
                                          gl::ShaderType shaderType,
                                          spirv::IdRef id,
                                          spv::Decoration decoration,
                                          const spirv::LiteralIntegerList &decorationValues,
                                          spirv::Blob *blobOut);
    TransformationState transformTypePointer(spirv::IdResult id,
                                             spv::StorageClass storageClass,
                                             spirv::IdRef typeId,
                                             spirv::Blob *blobOut);
    TransformationState transformVariable(spirv::IdResultType typeId,
                                          spirv::IdResult id,
                                          spv::StorageClass storageClass,
                                          spirv::Blob *blobOut);

    void modifyEntryPointInterfaceList(
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        gl::ShaderType shaderType,
        EntryPointList entryPointList,
        spirv::IdRefList *interfaceList);

    bool isInactive(spirv::IdRef id) const { return mIsInactiveById[id]; }

    spirv::IdRef getTransformedPrivateType(spirv::IdRef id) const
    {
        ASSERT(id < mTypePointerTransformedId.size());
        return mTypePointerTransformedId[id];
    }

  private:
    // Each OpTypePointer instruction that defines a type with the Output storage class is
    // duplicated with a similar instruction but which defines a type with the Private storage
    // class.  If inactive varyings are encountered, its type is changed to the Private one.  The
    // following vector maps the Output type id to the corresponding Private one.
    std::vector<spirv::IdRef> mTypePointerTransformedId;

    // Whether a variable has been marked inactive.
    std::vector<bool> mIsInactiveById;
};

void SpirvInactiveVaryingRemover::init(size_t indexBound)
{
    // Allocate storage for Output type pointer map.  At index i, this vector holds the identical
    // type as %i except for its storage class turned to Private.
    mTypePointerTransformedId.resize(indexBound);
    mIsInactiveById.resize(indexBound, false);
}

TransformationState SpirvInactiveVaryingRemover::transformAccessChain(
    spirv::IdResultType typeId,
    spirv::IdResult id,
    spirv::IdRef baseId,
    const spirv::IdRefList &indexList,
    spirv::Blob *blobOut)
{
    // Modifiy the instruction to use the private type.
    ASSERT(typeId < mTypePointerTransformedId.size());
    ASSERT(mTypePointerTransformedId[typeId].valid());

    spirv::WriteAccessChain(blobOut, mTypePointerTransformedId[typeId], id, baseId, indexList);

    return TransformationState::Transformed;
}

TransformationState SpirvInactiveVaryingRemover::transformDecorate(
    const ShaderInterfaceVariableInfo &info,
    gl::ShaderType shaderType,
    spirv::IdRef id,
    spv::Decoration decoration,
    const spirv::LiteralIntegerList &decorationValues,
    spirv::Blob *blobOut)
{
    // If it's an inactive varying, remove the decoration altogether.
    return info.activeStages[shaderType] ? TransformationState::Unchanged
                                         : TransformationState::Transformed;
}

void SpirvInactiveVaryingRemover::modifyEntryPointInterfaceList(
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    gl::ShaderType shaderType,
    EntryPointList entryPointList,
    spirv::IdRefList *interfaceList)
{
    // Nothing to do if SPIR-V 1.4, each inactive variable is replaced with a Private varaible, but
    // its ID is retained and stays in the variable list.
    if (entryPointList == EntryPointList::GlobalVariables)
    {
        return;
    }

    // Filter out inactive varyings from entry point interface declaration.
    size_t writeIndex = 0;
    for (size_t index = 0; index < interfaceList->size(); ++index)
    {
        spirv::IdRef id((*interfaceList)[index]);
        const ShaderInterfaceVariableInfo *info = variableInfoById[id];

        ASSERT(info);

        if (!info->activeStages[shaderType])
        {
            continue;
        }

        (*interfaceList)[writeIndex] = id;
        ++writeIndex;
    }

    // Update the number of interface variables.
    interfaceList->resize_down(writeIndex);
}

TransformationState SpirvInactiveVaryingRemover::transformTypePointer(
    spirv::IdResult id,
    spv::StorageClass storageClass,
    spirv::IdRef typeId,
    spirv::Blob *blobOut)
{
    // If the storage class is output, this may be used to create a variable corresponding to an
    // inactive varying, or if that varying is a struct, an Op*AccessChain retrieving a field of
    // that inactive varying.
    //
    // SPIR-V specifies the storage class both on the type and the variable declaration.  Otherwise
    // it would have been sufficient to modify the OpVariable instruction. For simplicity, duplicate
    // every "OpTypePointer Output" and "OpTypePointer Input" instruction except with the Private
    // storage class, in case it may be necessary later.

    // Cannot create a Private type declaration from builtins such as gl_PerVertex.
    if (typeId == sh::vk::spirv::kIdInputPerVertexBlock ||
        typeId == sh::vk::spirv::kIdOutputPerVertexBlock ||
        typeId == sh::vk::spirv::kIdInputPerVertexBlockArray ||
        typeId == sh::vk::spirv::kIdOutputPerVertexBlockArray)
    {
        return TransformationState::Unchanged;
    }

    if (storageClass != spv::StorageClassOutput && storageClass != spv::StorageClassInput)
    {
        return TransformationState::Unchanged;
    }

    const spirv::IdRef newPrivateTypeId(SpirvTransformerBase::GetNewId(blobOut));

    // Write OpTypePointer for the new PrivateType.
    spirv::WriteTypePointer(blobOut, newPrivateTypeId, spv::StorageClassPrivate, typeId);

    // Remember the id of the replacement.
    ASSERT(id < mTypePointerTransformedId.size());
    mTypePointerTransformedId[id] = newPrivateTypeId;

    // The original instruction should still be present as well.  At this point, we don't know
    // whether we will need the original or Private type.
    return TransformationState::Unchanged;
}

TransformationState SpirvInactiveVaryingRemover::transformVariable(spirv::IdResultType typeId,
                                                                   spirv::IdResult id,
                                                                   spv::StorageClass storageClass,
                                                                   spirv::Blob *blobOut)
{
    ASSERT(storageClass == spv::StorageClassOutput || storageClass == spv::StorageClassInput);

    ASSERT(typeId < mTypePointerTransformedId.size());
    ASSERT(mTypePointerTransformedId[typeId].valid());
    spirv::WriteVariable(blobOut, mTypePointerTransformedId[typeId], id, spv::StorageClassPrivate,
                         nullptr);

    mIsInactiveById[id] = true;

    return TransformationState::Transformed;
}

// Helper class that fixes varying precisions so they match between shader stages.
class SpirvVaryingPrecisionFixer final : angle::NonCopyable
{
  public:
    SpirvVaryingPrecisionFixer() {}

    void init(size_t indexBound);

    void visitTypePointer(spirv::IdResult id, spv::StorageClass storageClass, spirv::IdRef typeId);
    void visitVariable(const ShaderInterfaceVariableInfo &info,
                       gl::ShaderType shaderType,
                       spirv::IdResultType typeId,
                       spirv::IdResult id,
                       spv::StorageClass storageClass,
                       spirv::Blob *blobOut);

    TransformationState transformVariable(const ShaderInterfaceVariableInfo &info,
                                          spirv::IdResultType typeId,
                                          spirv::IdResult id,
                                          spv::StorageClass storageClass,
                                          spirv::Blob *blobOut);

    void modifyEntryPointInterfaceList(EntryPointList entryPointList,
                                       spirv::IdRefList *interfaceList);
    void addDecorate(spirv::IdRef replacedId, spirv::Blob *blobOut);
    void writeInputPreamble(
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        gl::ShaderType shaderType,
        spirv::Blob *blobOut);
    void writeOutputPrologue(
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        gl::ShaderType shaderType,
        spirv::Blob *blobOut);

    bool isReplaced(spirv::IdRef id) const { return mFixedVaryingId[id].valid(); }
    spirv::IdRef getReplacementId(spirv::IdRef id) const
    {
        return mFixedVaryingId[id].valid() ? mFixedVaryingId[id] : id;
    }

  private:
    std::vector<spirv::IdRef> mTypePointerTypeId;
    std::vector<spirv::IdRef> mFixedVaryingId;
    std::vector<spirv::IdRef> mFixedVaryingTypeId;
};

void SpirvVaryingPrecisionFixer::init(size_t indexBound)
{
    // Allocate storage for precision mismatch fix up.
    mTypePointerTypeId.resize(indexBound);
    mFixedVaryingId.resize(indexBound);
    mFixedVaryingTypeId.resize(indexBound);
}

void SpirvVaryingPrecisionFixer::visitTypePointer(spirv::IdResult id,
                                                  spv::StorageClass storageClass,
                                                  spirv::IdRef typeId)
{
    mTypePointerTypeId[id] = typeId;
}

void SpirvVaryingPrecisionFixer::visitVariable(const ShaderInterfaceVariableInfo &info,
                                               gl::ShaderType shaderType,
                                               spirv::IdResultType typeId,
                                               spirv::IdResult id,
                                               spv::StorageClass storageClass,
                                               spirv::Blob *blobOut)
{
    if (info.useRelaxedPrecision && info.activeStages[shaderType] && !mFixedVaryingId[id].valid())
    {
        mFixedVaryingId[id]     = SpirvTransformerBase::GetNewId(blobOut);
        mFixedVaryingTypeId[id] = typeId;
    }
}

TransformationState SpirvVaryingPrecisionFixer::transformVariable(
    const ShaderInterfaceVariableInfo &info,
    spirv::IdResultType typeId,
    spirv::IdResult id,
    spv::StorageClass storageClass,
    spirv::Blob *blobOut)
{
    if (info.useRelaxedPrecision &&
        (storageClass == spv::StorageClassOutput || storageClass == spv::StorageClassInput))
    {
        // Change existing OpVariable to use fixedVaryingId
        ASSERT(mFixedVaryingId[id].valid());
        spirv::WriteVariable(blobOut, typeId, mFixedVaryingId[id], storageClass, nullptr);

        return TransformationState::Transformed;
    }
    return TransformationState::Unchanged;
}

void SpirvVaryingPrecisionFixer::writeInputPreamble(
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    gl::ShaderType shaderType,
    spirv::Blob *blobOut)
{
    if (shaderType == gl::ShaderType::Vertex || shaderType == gl::ShaderType::Compute)
    {
        return;
    }

    // Copy from corrected varyings to temp global variables with original precision.
    for (uint32_t idIndex = spirv::kMinValidId; idIndex < variableInfoById.size(); idIndex++)
    {
        const spirv::IdRef id(idIndex);
        const ShaderInterfaceVariableInfo *info = variableInfoById[id];
        if (info && info->useRelaxedPrecision && info->activeStages[shaderType] &&
            info->varyingIsInput)
        {
            // This is an input varying, need to cast the mediump value that came from
            // the previous stage into a highp value that the code wants to work with.
            ASSERT(mFixedVaryingTypeId[id].valid());

            // Build OpLoad instruction to load the mediump value into a temporary
            const spirv::IdRef tempVar(SpirvTransformerBase::GetNewId(blobOut));
            const spirv::IdRef tempVarType(mTypePointerTypeId[mFixedVaryingTypeId[id]]);
            ASSERT(tempVarType.valid());

            spirv::WriteLoad(blobOut, tempVarType, tempVar, mFixedVaryingId[id], nullptr);

            // Build OpStore instruction to cast the mediump value to highp for use in
            // the function
            spirv::WriteStore(blobOut, id, tempVar, nullptr);
        }
    }
}

void SpirvVaryingPrecisionFixer::modifyEntryPointInterfaceList(EntryPointList entryPointList,
                                                               spirv::IdRefList *interfaceList)
{
    // With SPIR-V 1.3, modify interface list if any ID was replaced due to varying precision
    // mismatch.
    //
    // With SPIR-V 1.4, the original variables are changed to Private and should remain in the list.
    // The new variables should be added to the variable list.
    //
    // If any ID is beyond the original bound, it was added by another transformation, and should be
    // left intact.
    const size_t variableCount = interfaceList->size();
    for (size_t index = 0; index < variableCount; ++index)
    {
        const spirv::IdRef id            = (*interfaceList)[index];
        const spirv::IdRef replacementId = id < mFixedVaryingId.size() ? getReplacementId(id) : id;
        if (replacementId != id)
        {
            if (entryPointList == EntryPointList::InterfaceVariables)
            {
                (*interfaceList)[index] = replacementId;
            }
            else
            {
                interfaceList->push_back(replacementId);
            }
        }
    }
}

void SpirvVaryingPrecisionFixer::addDecorate(spirv::IdRef replacedId, spirv::Blob *blobOut)
{
    spirv::WriteDecorate(blobOut, replacedId, spv::DecorationRelaxedPrecision, {});
}

void SpirvVaryingPrecisionFixer::writeOutputPrologue(
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    gl::ShaderType shaderType,
    spirv::Blob *blobOut)
{
    if (shaderType == gl::ShaderType::Fragment || shaderType == gl::ShaderType::Compute)
    {
        return;
    }

    // Copy from temp global variables with original precision to corrected varyings.
    for (uint32_t idIndex = spirv::kMinValidId; idIndex < variableInfoById.size(); idIndex++)
    {
        const spirv::IdRef id(idIndex);
        const ShaderInterfaceVariableInfo *info = variableInfoById[id];
        if (info && info->useRelaxedPrecision && info->activeStages[shaderType] &&
            info->varyingIsOutput)
        {
            ASSERT(mFixedVaryingTypeId[id].valid());

            // Build OpLoad instruction to load the highp value into a temporary
            const spirv::IdRef tempVar(SpirvTransformerBase::GetNewId(blobOut));
            const spirv::IdRef tempVarType(mTypePointerTypeId[mFixedVaryingTypeId[id]]);
            ASSERT(tempVarType.valid());

            spirv::WriteLoad(blobOut, tempVarType, tempVar, id, nullptr);

            // Build OpStore instruction to cast the highp value to mediump for output
            spirv::WriteStore(blobOut, mFixedVaryingId[id], tempVar, nullptr);
        }
    }
}

// Helper class that generates code for transform feedback
class SpirvTransformFeedbackCodeGenerator final : angle::NonCopyable
{
  public:
    SpirvTransformFeedbackCodeGenerator(const SpvTransformOptions &options)
        : mIsEmulated(options.isTransformFeedbackEmulated),
          mHasTransformFeedbackOutput(false),
          mIsPositionCapturedByTransformFeedbackExtension(false)
    {}

    void visitVariable(const ShaderInterfaceVariableInfo &info,
                       const XFBInterfaceVariableInfo &xfbInfo,
                       gl::ShaderType shaderType,
                       spirv::IdResultType typeId,
                       spirv::IdResult id,
                       spv::StorageClass storageClass);

    TransformationState transformCapability(spv::Capability capability, spirv::Blob *blobOut);
    TransformationState transformDecorate(const ShaderInterfaceVariableInfo *info,
                                          gl::ShaderType shaderType,
                                          spirv::IdRef id,
                                          spv::Decoration decoration,
                                          const spirv::LiteralIntegerList &decorationValues,
                                          spirv::Blob *blobOut);
    TransformationState transformMemberDecorate(const ShaderInterfaceVariableInfo *info,
                                                gl::ShaderType shaderType,
                                                spirv::IdRef id,
                                                spirv::LiteralInteger member,
                                                spv::Decoration decoration,
                                                spirv::Blob *blobOut);
    TransformationState transformName(spirv::IdRef id, spirv::LiteralString name);
    TransformationState transformMemberName(spirv::IdRef id,
                                            spirv::LiteralInteger member,
                                            spirv::LiteralString name);
    TransformationState transformTypeStruct(const ShaderInterfaceVariableInfo *info,
                                            gl::ShaderType shaderType,
                                            spirv::IdResult id,
                                            const spirv::IdRefList &memberList,
                                            spirv::Blob *blobOut);
    TransformationState transformTypePointer(const ShaderInterfaceVariableInfo *info,
                                             gl::ShaderType shaderType,
                                             spirv::IdResult id,
                                             spv::StorageClass storageClass,
                                             spirv::IdRef typeId,
                                             spirv::Blob *blobOut);
    TransformationState transformVariable(const ShaderInterfaceVariableInfo &info,
                                          const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                          gl::ShaderType shaderType,
                                          spv::StorageClass storageBufferStorageClass,
                                          spirv::IdResultType typeId,
                                          spirv::IdResult id,
                                          spv::StorageClass storageClass);

    void modifyEntryPointInterfaceList(
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        gl::ShaderType shaderType,
        EntryPointList entryPointList,
        spirv::IdRefList *interfaceList);

    void writePendingDeclarations(
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        spv::StorageClass storageBufferStorageClass,
        spirv::Blob *blobOut);
    void writeTransformFeedbackExtensionOutput(spirv::IdRef positionId, spirv::Blob *blobOut);
    void writeTransformFeedbackEmulationOutput(
        const SpirvInactiveVaryingRemover &inactiveVaryingRemover,
        const SpirvVaryingPrecisionFixer &varyingPrecisionFixer,
        const bool usePrecisionFixer,
        spirv::Blob *blobOut);
    void addExecutionMode(spirv::IdRef entryPointId, spirv::Blob *blobOut);
    void addMemberDecorate(const XFBInterfaceVariableInfo &info,
                           spirv::IdRef id,
                           spirv::Blob *blobOut);
    void addDecorate(const XFBInterfaceVariableInfo &xfbInfo,
                     spirv::IdRef id,
                     spirv::Blob *blobOut);

  private:
    void gatherXfbVaryings(const XFBInterfaceVariableInfo &info, spirv::IdRef id);
    void visitXfbVarying(const ShaderInterfaceVariableXfbInfo &xfb,
                         spirv::IdRef baseId,
                         uint32_t fieldIndex);
    TransformationState transformTypeHelper(const ShaderInterfaceVariableInfo *info,
                                            gl::ShaderType shaderType,
                                            spirv::IdResult id);
    void writeIntConstant(uint32_t value, spirv::IdRef intId, spirv::Blob *blobOut);
    void getVaryingTypeIds(GLenum componentType,
                           bool isPrivate,
                           spirv::IdRef *typeIdOut,
                           spirv::IdRef *typePtrOut);
    void writeGetOffsetsCall(spirv::IdRef xfbOffsets, spirv::Blob *blobOut);
    void writeComponentCapture(uint32_t bufferIndex,
                               spirv::IdRef xfbOffset,
                               spirv::IdRef varyingTypeId,
                               spirv::IdRef varyingTypePtr,
                               spirv::IdRef varyingBaseId,
                               const spirv::IdRefList &accessChainIndices,
                               GLenum componentType,
                               spirv::Blob *blobOut);

    static constexpr size_t kXfbDecorationCount                           = 3;
    static constexpr spv::Decoration kXfbDecorations[kXfbDecorationCount] = {
        spv::DecorationXfbBuffer,
        spv::DecorationXfbStride,
        spv::DecorationOffset,
    };

    bool mIsEmulated;
    bool mHasTransformFeedbackOutput;

    // Ids needed to generate transform feedback support code.
    bool mIsPositionCapturedByTransformFeedbackExtension;
    gl::TransformFeedbackBuffersArray<spirv::IdRef> mBufferStrides;
    spirv::IdRef mBufferStridesCompositeId;

    // Type and constant ids:
    //
    // - mFloatOutputPointerId: id of OpTypePointer Output %kIdFloat
    // - mIntOutputPointerId: id of OpTypePointer Output %kIdInt
    // - mUintOutputPointerId: id of OpTypePointer Output %kIdUint
    // - mFloatPrivatePointerId, mIntPrivatePointerId, mUintPrivatePointerId: identical to the
    //   above, but with the Private storage class.  Used to load from varyings that have been
    //   replaced as part of precision mismatch fixup.
    // - mFloatUniformPointerId: id of OpTypePointer Uniform %kIdFloat
    //
    // - mIntNIds[n]: id of OpConstant %kIdInt n
    spirv::IdRef mFloatOutputPointerId;
    spirv::IdRef mIntOutputPointerId;
    spirv::IdRef mUintOutputPointerId;
    spirv::IdRef mFloatPrivatePointerId;
    spirv::IdRef mIntPrivatePointerId;
    spirv::IdRef mUintPrivatePointerId;
    spirv::IdRef mFloatUniformPointerId;

    // Id of constants such as row, column and array index.  Integers 0, 1, 2 and 3 are always
    // defined by the compiler.
    angle::FastVector<spirv::IdRef, 4> mIntNIds;

    // For transform feedback emulation, the captured elements are gathered in a list and sorted.
    // This allows the output generation code to always use offset += 1, thus relying on only one
    // constant (1).
    struct XfbVarying
    {
        // The varyings are sorted by info.offset.
        const ShaderInterfaceVariableXfbInfo *info;
        // Id of the base variable.
        spirv::IdRef baseId;
        // The field index, if a member of an I/O blocks
        uint32_t fieldIndex;
    };
    gl::TransformFeedbackBuffersArray<std::vector<XfbVarying>> mXfbVaryings;
};

constexpr size_t SpirvTransformFeedbackCodeGenerator::kXfbDecorationCount;
constexpr spv::Decoration SpirvTransformFeedbackCodeGenerator::kXfbDecorations[kXfbDecorationCount];

void SpirvTransformFeedbackCodeGenerator::visitVariable(const ShaderInterfaceVariableInfo &info,
                                                        const XFBInterfaceVariableInfo &xfbInfo,
                                                        gl::ShaderType shaderType,
                                                        spirv::IdResultType typeId,
                                                        spirv::IdResult id,
                                                        spv::StorageClass storageClass)
{
    if (mIsEmulated)
    {
        gatherXfbVaryings(xfbInfo, id);
        return;
    }

    // Note if the variable is captured by transform feedback.  In that case, the TransformFeedback
    // capability needs to be added.
    if ((xfbInfo.xfb.pod.buffer != ShaderInterfaceVariableInfo::kInvalid ||
         !xfbInfo.fieldXfb.empty()) &&
        info.activeStages[shaderType])
    {
        mHasTransformFeedbackOutput = true;

        // If this is the special ANGLEXfbPosition variable, remember its id to be used for the
        // ANGLEXfbPosition = gl_Position; assignment code generation.
        if (id == ID::XfbExtensionPosition)
        {
            mIsPositionCapturedByTransformFeedbackExtension = true;
        }
    }
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformCapability(
    spv::Capability capability,
    spirv::Blob *blobOut)
{
    if (!mHasTransformFeedbackOutput || mIsEmulated)
    {
        return TransformationState::Unchanged;
    }

    // Transform feedback capability shouldn't have already been specified.
    ASSERT(capability != spv::CapabilityTransformFeedback);

    // Vulkan shaders have either Shader, Geometry or Tessellation capability.  We find this
    // capability, and add the TransformFeedback capability right before it.
    if (capability != spv::CapabilityShader && capability != spv::CapabilityGeometry &&
        capability != spv::CapabilityTessellation)
    {
        return TransformationState::Unchanged;
    }

    // Write the TransformFeedback capability declaration.
    spirv::WriteCapability(blobOut, spv::CapabilityTransformFeedback);

    // The original capability is retained.
    return TransformationState::Unchanged;
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformName(spirv::IdRef id,
                                                                       spirv::LiteralString name)
{
    // In the case of ANGLEXfbN, unconditionally remove the variable names.  If transform
    // feedback is not active, the corresponding variables will be removed.
    switch (id)
    {
        case sh::vk::spirv::kIdXfbEmulationBufferBlockZero:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockOne:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockTwo:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockThree:
        case sh::vk::spirv::kIdXfbEmulationBufferVarZero:
        case sh::vk::spirv::kIdXfbEmulationBufferVarOne:
        case sh::vk::spirv::kIdXfbEmulationBufferVarTwo:
        case sh::vk::spirv::kIdXfbEmulationBufferVarThree:
            return TransformationState::Transformed;
        default:
            return TransformationState::Unchanged;
    }
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformMemberName(
    spirv::IdRef id,
    spirv::LiteralInteger member,
    spirv::LiteralString name)
{
    switch (id)
    {
        case sh::vk::spirv::kIdXfbEmulationBufferBlockZero:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockOne:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockTwo:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockThree:
            return TransformationState::Transformed;
        default:
            return TransformationState::Unchanged;
    }
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformTypeHelper(
    const ShaderInterfaceVariableInfo *info,
    gl::ShaderType shaderType,
    spirv::IdResult id)
{
    switch (id)
    {
        case sh::vk::spirv::kIdXfbEmulationBufferBlockZero:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockOne:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockTwo:
        case sh::vk::spirv::kIdXfbEmulationBufferBlockThree:
            ASSERT(info);
            return info->activeStages[shaderType] ? TransformationState::Unchanged
                                                  : TransformationState::Transformed;
        default:
            return TransformationState::Unchanged;
    }
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformDecorate(
    const ShaderInterfaceVariableInfo *info,
    gl::ShaderType shaderType,
    spirv::IdRef id,
    spv::Decoration decoration,
    const spirv::LiteralIntegerList &decorationValues,
    spirv::Blob *blobOut)
{
    return transformTypeHelper(info, shaderType, id);
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformMemberDecorate(
    const ShaderInterfaceVariableInfo *info,
    gl::ShaderType shaderType,
    spirv::IdRef id,
    spirv::LiteralInteger member,
    spv::Decoration decoration,
    spirv::Blob *blobOut)
{
    return transformTypeHelper(info, shaderType, id);
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformTypeStruct(
    const ShaderInterfaceVariableInfo *info,
    gl::ShaderType shaderType,
    spirv::IdResult id,
    const spirv::IdRefList &memberList,
    spirv::Blob *blobOut)
{
    return transformTypeHelper(info, shaderType, id);
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformTypePointer(
    const ShaderInterfaceVariableInfo *info,
    gl::ShaderType shaderType,
    spirv::IdResult id,
    spv::StorageClass storageClass,
    spirv::IdRef typeId,
    spirv::Blob *blobOut)
{
    return transformTypeHelper(info, shaderType, typeId);
}

TransformationState SpirvTransformFeedbackCodeGenerator::transformVariable(
    const ShaderInterfaceVariableInfo &info,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    gl::ShaderType shaderType,
    spv::StorageClass storageBufferStorageClass,
    spirv::IdResultType typeId,
    spirv::IdResult id,
    spv::StorageClass storageClass)
{
    // This function is currently called for inactive variables.
    ASSERT(!info.activeStages[shaderType]);

    if (shaderType == gl::ShaderType::Vertex && storageClass == storageBufferStorageClass)
    {
        // The ANGLEXfbN variables are unconditionally generated and may be inactive.  Remove these
        // variables in that case.
        ASSERT(&info == &variableInfoMap.getVariableById(shaderType, SpvGetXfbBufferBlockId(0)) ||
               &info == &variableInfoMap.getVariableById(shaderType, SpvGetXfbBufferBlockId(1)) ||
               &info == &variableInfoMap.getVariableById(shaderType, SpvGetXfbBufferBlockId(2)) ||
               &info == &variableInfoMap.getVariableById(shaderType, SpvGetXfbBufferBlockId(3)));

        // Drop the declaration.
        return TransformationState::Transformed;
    }

    return TransformationState::Unchanged;
}

void SpirvTransformFeedbackCodeGenerator::gatherXfbVaryings(const XFBInterfaceVariableInfo &info,
                                                            spirv::IdRef id)
{
    visitXfbVarying(info.xfb, id, ShaderInterfaceVariableXfbInfo::kInvalid);

    for (size_t fieldIndex = 0; fieldIndex < info.fieldXfb.size(); ++fieldIndex)
    {
        visitXfbVarying(info.fieldXfb[fieldIndex], id, static_cast<uint32_t>(fieldIndex));
    }
}

void SpirvTransformFeedbackCodeGenerator::visitXfbVarying(const ShaderInterfaceVariableXfbInfo &xfb,
                                                          spirv::IdRef baseId,
                                                          uint32_t fieldIndex)
{
    for (const ShaderInterfaceVariableXfbInfo &arrayElement : xfb.arrayElements)
    {
        visitXfbVarying(arrayElement, baseId, fieldIndex);
    }

    if (xfb.pod.buffer == ShaderInterfaceVariableXfbInfo::kInvalid)
    {
        return;
    }

    // Varyings captured to the same buffer have the same stride.
    ASSERT(mXfbVaryings[xfb.pod.buffer].empty() ||
           mXfbVaryings[xfb.pod.buffer][0].info->pod.stride == xfb.pod.stride);

    mXfbVaryings[xfb.pod.buffer].push_back({&xfb, baseId, fieldIndex});
}

void SpirvTransformFeedbackCodeGenerator::writeIntConstant(uint32_t value,
                                                           spirv::IdRef intId,
                                                           spirv::Blob *blobOut)
{
    if (value == ShaderInterfaceVariableXfbInfo::kInvalid)
    {
        return;
    }

    if (mIntNIds.size() <= value)
    {
        // This member never resized down, so new elements can't have previous values.
        mIntNIds.resize_maybe_value_reuse(value + 1);
    }
    else if (mIntNIds[value].valid())
    {
        return;
    }

    mIntNIds[value] = SpirvTransformerBase::GetNewId(blobOut);
    spirv::WriteConstant(blobOut, ID::Int, mIntNIds[value],
                         spirv::LiteralContextDependentNumber(value));
}

void SpirvTransformFeedbackCodeGenerator::modifyEntryPointInterfaceList(
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    gl::ShaderType shaderType,
    EntryPointList entryPointList,
    spirv::IdRefList *interfaceList)
{
    if (entryPointList == EntryPointList::GlobalVariables)
    {
        // Filter out unused xfb blocks from entry point interface declaration.
        size_t writeIndex = 0;
        for (size_t index = 0; index < interfaceList->size(); ++index)
        {
            spirv::IdRef id((*interfaceList)[index]);
            if (SpvIsXfbBufferBlockId(id))
            {
                const ShaderInterfaceVariableInfo *info = variableInfoById[id];
                ASSERT(info);

                if (!info->activeStages[shaderType])
                {
                    continue;
                }
            }

            (*interfaceList)[writeIndex] = id;
            ++writeIndex;
        }

        // Update the number of interface variables.
        interfaceList->resize_down(writeIndex);
    }
}

void SpirvTransformFeedbackCodeGenerator::writePendingDeclarations(
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    spv::StorageClass storageBufferStorageClass,
    spirv::Blob *blobOut)
{
    if (!mIsEmulated)
    {
        return;
    }

    mFloatOutputPointerId  = SpirvTransformerBase::GetNewId(blobOut);
    mFloatPrivatePointerId = SpirvTransformerBase::GetNewId(blobOut);
    spirv::WriteTypePointer(blobOut, mFloatOutputPointerId, spv::StorageClassOutput, ID::Float);
    spirv::WriteTypePointer(blobOut, mFloatPrivatePointerId, spv::StorageClassPrivate, ID::Float);

    mIntOutputPointerId  = SpirvTransformerBase::GetNewId(blobOut);
    mIntPrivatePointerId = SpirvTransformerBase::GetNewId(blobOut);
    spirv::WriteTypePointer(blobOut, mIntOutputPointerId, spv::StorageClassOutput, ID::Int);
    spirv::WriteTypePointer(blobOut, mIntPrivatePointerId, spv::StorageClassPrivate, ID::Int);

    mUintOutputPointerId  = SpirvTransformerBase::GetNewId(blobOut);
    mUintPrivatePointerId = SpirvTransformerBase::GetNewId(blobOut);
    spirv::WriteTypePointer(blobOut, mUintOutputPointerId, spv::StorageClassOutput, ID::Uint);
    spirv::WriteTypePointer(blobOut, mUintPrivatePointerId, spv::StorageClassPrivate, ID::Uint);

    mFloatUniformPointerId = SpirvTransformerBase::GetNewId(blobOut);
    spirv::WriteTypePointer(blobOut, mFloatUniformPointerId, storageBufferStorageClass, ID::Float);

    ASSERT(mIntNIds.empty());
    // All new elements initialized later after the resize. Additionally mIntNIds was always empty
    // before this resize, so previous value reuse is not possible.
    mIntNIds.resize_maybe_value_reuse(4);
    mIntNIds[0] = ID::IntZero;
    mIntNIds[1] = ID::IntOne;
    mIntNIds[2] = ID::IntTwo;
    mIntNIds[3] = ID::IntThree;

    spirv::IdRefList compositeIds;
    for (const std::vector<XfbVarying> &varyings : mXfbVaryings)
    {
        if (varyings.empty())
        {
            compositeIds.push_back(ID::IntZero);
            continue;
        }

        const ShaderInterfaceVariableXfbInfo *info0 = varyings[0].info;

        // Define the buffer stride constant
        ASSERT(info0->pod.stride % sizeof(float) == 0);
        uint32_t stride = info0->pod.stride / sizeof(float);

        mBufferStrides[info0->pod.buffer] = SpirvTransformerBase::GetNewId(blobOut);
        spirv::WriteConstant(blobOut, ID::Int, mBufferStrides[info0->pod.buffer],
                             spirv::LiteralContextDependentNumber(stride));

        compositeIds.push_back(mBufferStrides[info0->pod.buffer]);

        // Define all the constants that would be necessary to load the components of the varying.
        for (const XfbVarying &varying : varyings)
        {
            writeIntConstant(varying.fieldIndex, ID::Int, blobOut);
            const ShaderInterfaceVariableXfbInfo *info = varying.info;
            if (info->pod.arraySize == ShaderInterfaceVariableXfbInfo::kInvalid)
            {
                continue;
            }

            uint32_t arrayIndexStart =
                varying.info->pod.arrayIndex != ShaderInterfaceVariableXfbInfo::kInvalid
                    ? varying.info->pod.arrayIndex
                    : 0;
            uint32_t arrayIndexEnd = arrayIndexStart + info->pod.arraySize;

            for (uint32_t arrayIndex = arrayIndexStart; arrayIndex < arrayIndexEnd; ++arrayIndex)
            {
                writeIntConstant(arrayIndex, ID::Int, blobOut);
            }
        }
    }

    mBufferStridesCompositeId = SpirvTransformerBase::GetNewId(blobOut);
    spirv::WriteConstantComposite(blobOut, ID::IVec4, mBufferStridesCompositeId, compositeIds);
}

void SpirvTransformFeedbackCodeGenerator::writeTransformFeedbackExtensionOutput(
    spirv::IdRef positionId,
    spirv::Blob *blobOut)
{
    if (mIsEmulated)
    {
        return;
    }

    if (mIsPositionCapturedByTransformFeedbackExtension)
    {
        spirv::WriteStore(blobOut, ID::XfbExtensionPosition, positionId, nullptr);
    }
}

class AccessChainIndexListAppend final : angle::NonCopyable
{
  public:
    AccessChainIndexListAppend(bool condition,
                               angle::FastVector<spirv::IdRef, 4> intNIds,
                               uint32_t index,
                               spirv::IdRefList *indexList)
        : mCondition(condition), mIndexList(indexList)
    {
        if (mCondition)
        {
            mIndexList->push_back(intNIds[index]);
        }
    }
    ~AccessChainIndexListAppend()
    {
        if (mCondition)
        {
            mIndexList->pop_back();
        }
    }

  private:
    bool mCondition;
    spirv::IdRefList *mIndexList;
};

void SpirvTransformFeedbackCodeGenerator::writeTransformFeedbackEmulationOutput(
    const SpirvInactiveVaryingRemover &inactiveVaryingRemover,
    const SpirvVaryingPrecisionFixer &varyingPrecisionFixer,
    const bool usePrecisionFixer,
    spirv::Blob *blobOut)
{
    if (!mIsEmulated)
    {
        return;
    }

    // First, sort the varyings by offset, to simplify calculation of the output offset.
    for (std::vector<XfbVarying> &varyings : mXfbVaryings)
    {
        std::sort(varyings.begin(), varyings.end(),
                  [](const XfbVarying &first, const XfbVarying &second) {
                      return first.info->pod.offset < second.info->pod.offset;
                  });
    }

    // The following code is generated for transform feedback emulation:
    //
    //     ivec4 xfbOffsets = ANGLEGetXfbOffsets(ivec4(stride0, stride1, stride2, stride3));
    //     // For buffer N:
    //     int xfbOffset = xfbOffsets[N]
    //     ANGLEXfbN.xfbOut[xfbOffset] = tfVarying0.field[index][row][col]
    //     xfbOffset += 1;
    //     ANGLEXfbN.xfbOut[xfbOffset] = tfVarying0.field[index][row][col + 1]
    //     xfbOffset += 1;
    //     ...
    //
    // The following pieces of SPIR-V code are generated according to the above:
    //
    // - For the initial offsets calculation:
    //
    //    %xfbOffsetsResult = OpFunctionCall %ivec4 %ANGLEGetXfbOffsets %stridesComposite
    //       %xfbOffsetsVar = OpVariable %kIdIVec4FunctionTypePointer Function
    //                        OpStore %xfbOffsetsVar %xfbOffsetsResult
    //          %xfbOffsets = OpLoad %ivec4 %xfbOffsetsVar
    //
    // - Initial code for each buffer N:
    //
    //           %xfbOffset = OpCompositeExtract %int %xfbOffsets N
    //
    // - For each varying being captured:
    //
    //                        // Load the component
    //        %componentPtr = OpAccessChain %floatOutputPtr %baseId %field %arrayIndex %row %col
    //           %component = OpLoad %float %componentPtr
    //                        // Store in xfb output
    //           %xfbOutPtr = OpAccessChain %floatUniformPtr %xfbBufferN %int0 %xfbOffset
    //                        OpStore %xfbOutPtr %component
    //                        // Increment offset
    //           %xfbOffset = OpIAdd %int %xfbOffset %int1
    //
    //   Note that if the varying being captured is integer, the first two instructions above would
    //   use the intger equivalent types, and the following instruction would bitcast it to float
    //   for storage:
    //
    //             %asFloat = OpBitcast %float %component
    //

    spirv::IdRef xfbOffsets;

    if (!mXfbVaryings.empty())
    {
        xfbOffsets = SpirvTransformerBase::GetNewId(blobOut);

        // ivec4 xfbOffsets = ANGLEGetXfbOffsets(ivec4(stride0, stride1, stride2, stride3));
        writeGetOffsetsCall(xfbOffsets, blobOut);
    }

    // Go over the buffers one by one and capture the varyings.
    for (uint32_t bufferIndex = 0; bufferIndex < mXfbVaryings.size(); ++bufferIndex)
    {
        spirv::IdRef xfbOffset(SpirvTransformerBase::GetNewId(blobOut));

        // Get the offset corresponding to this buffer:
        //
        //     int xfbOffset = xfbOffsets[N]
        spirv::WriteCompositeExtract(blobOut, ID::Int, xfbOffset, xfbOffsets,
                                     {spirv::LiteralInteger(bufferIndex)});

        // Track offsets for verification.
        uint32_t offsetForVerification = 0;

        // Go over the varyings of this buffer in order.
        const std::vector<XfbVarying> &varyings = mXfbVaryings[bufferIndex];
        for (size_t varyingIndex = 0; varyingIndex < varyings.size(); ++varyingIndex)
        {
            const XfbVarying &varying                  = varyings[varyingIndex];
            const ShaderInterfaceVariableXfbInfo *info = varying.info;
            ASSERT(info->pod.buffer == bufferIndex);

            // Each component of the varying being captured is loaded one by one.  This uses the
            // OpAccessChain instruction that takes a chain of "indices" to end up with the
            // component starting from the base variable.  For example:
            //
            //     var.member[3][2][0]
            //
            // where member is field number 4 in var and is a mat4, the access chain would be:
            //
            //     4 3 2 0
            //     ^ ^ ^ ^
            //     | | | |
            //     | | | row 0
            //     | | column 2
            //     | array element 3
            //     field 4
            //
            // The following tracks the access chain as the field, array elements, columns and rows
            // are looped over.
            spirv::IdRefList indexList;
            AccessChainIndexListAppend appendField(
                varying.fieldIndex != ShaderInterfaceVariableXfbInfo::kInvalid, mIntNIds,
                varying.fieldIndex, &indexList);

            // The varying being captured is either:
            //
            // - Not an array: In this case, no entry is added in the access chain
            // - An element of the array
            // - The whole array
            //
            uint32_t arrayIndexStart = 0;
            uint32_t arrayIndexEnd   = info->pod.arraySize;
            const bool isArray = info->pod.arraySize != ShaderInterfaceVariableXfbInfo::kInvalid;
            if (varying.info->pod.arrayIndex != ShaderInterfaceVariableXfbInfo::kInvalid)
            {
                // Capturing a single element.
                arrayIndexStart = varying.info->pod.arrayIndex;
                arrayIndexEnd   = arrayIndexStart + 1;
            }
            else if (!isArray)
            {
                // Not an array.
                arrayIndexEnd = 1;
            }

            // Sorting the varyings should have ensured that offsets are in order and that writing
            // to the output buffer sequentially ends up using the correct offsets.
            ASSERT(info->pod.offset == offsetForVerification);
            offsetForVerification += (arrayIndexEnd - arrayIndexStart) * info->pod.rowCount *
                                     info->pod.columnCount * sizeof(float);

            // Determine the type of the component being captured.  OpBitcast is used (the
            // implementation of intBitsToFloat() and uintBitsToFloat() for non-float types).
            spirv::IdRef varyingTypeId;
            spirv::IdRef varyingTypePtr;
            const bool isPrivate =
                inactiveVaryingRemover.isInactive(varying.baseId) ||
                (usePrecisionFixer && varyingPrecisionFixer.isReplaced(varying.baseId));
            getVaryingTypeIds(info->pod.componentType, isPrivate, &varyingTypeId, &varyingTypePtr);

            for (uint32_t arrayIndex = arrayIndexStart; arrayIndex < arrayIndexEnd; ++arrayIndex)
            {
                AccessChainIndexListAppend appendArrayIndex(isArray, mIntNIds, arrayIndex,
                                                            &indexList);
                for (uint32_t col = 0; col < info->pod.columnCount; ++col)
                {
                    AccessChainIndexListAppend appendColumn(info->pod.columnCount > 1, mIntNIds,
                                                            col, &indexList);
                    for (uint32_t row = 0; row < info->pod.rowCount; ++row)
                    {
                        AccessChainIndexListAppend appendRow(info->pod.rowCount > 1, mIntNIds, row,
                                                             &indexList);

                        // Generate the code to capture a single component of the varying:
                        //
                        //     ANGLEXfbN.xfbOut[xfbOffset] = tfVarying0.field[index][row][col]
                        writeComponentCapture(bufferIndex, xfbOffset, varyingTypeId, varyingTypePtr,
                                              varying.baseId, indexList, info->pod.componentType,
                                              blobOut);

                        // Increment the offset:
                        //
                        //     xfbOffset += 1;
                        //
                        // which translates to:
                        //
                        //     %newOffsetId = OpIAdd %int %currentOffsetId %int1
                        spirv::IdRef nextOffset(SpirvTransformerBase::GetNewId(blobOut));
                        spirv::WriteIAdd(blobOut, ID::Int, nextOffset, xfbOffset, ID::IntOne);
                        xfbOffset = nextOffset;
                    }
                }
            }
        }
    }
}

void SpirvTransformFeedbackCodeGenerator::getVaryingTypeIds(GLenum componentType,
                                                            bool isPrivate,
                                                            spirv::IdRef *typeIdOut,
                                                            spirv::IdRef *typePtrOut)
{
    switch (componentType)
    {
        case GL_INT:
            *typeIdOut  = ID::Int;
            *typePtrOut = isPrivate ? mIntPrivatePointerId : mIntOutputPointerId;
            break;
        case GL_UNSIGNED_INT:
            *typeIdOut  = ID::Uint;
            *typePtrOut = isPrivate ? mUintPrivatePointerId : mUintOutputPointerId;
            break;
        case GL_FLOAT:
            *typeIdOut  = ID::Float;
            *typePtrOut = isPrivate ? mFloatPrivatePointerId : mFloatOutputPointerId;
            break;
        default:
            UNREACHABLE();
    }

    ASSERT(typeIdOut->valid());
    ASSERT(typePtrOut->valid());
}

void SpirvTransformFeedbackCodeGenerator::writeGetOffsetsCall(spirv::IdRef xfbOffsets,
                                                              spirv::Blob *blobOut)
{
    const spirv::IdRef xfbOffsetsResult(SpirvTransformerBase::GetNewId(blobOut));
    const spirv::IdRef xfbOffsetsVar(SpirvTransformerBase::GetNewId(blobOut));

    // Generate code for the following:
    //
    //     ivec4 xfbOffsets = ANGLEGetXfbOffsets(ivec4(stride0, stride1, stride2, stride3));

    // Create a variable to hold the result.
    spirv::WriteVariable(blobOut, ID::IVec4FunctionTypePointer, xfbOffsetsVar,
                         spv::StorageClassFunction, nullptr);
    // Call a helper function generated by the translator to calculate the offsets for the current
    // vertex.
    spirv::WriteFunctionCall(blobOut, ID::IVec4, xfbOffsetsResult,
                             ID::XfbEmulationGetOffsetsFunction, {mBufferStridesCompositeId});
    // Store the results.
    spirv::WriteStore(blobOut, xfbOffsetsVar, xfbOffsetsResult, nullptr);
    // Load from the variable for use in expressions.
    spirv::WriteLoad(blobOut, ID::IVec4, xfbOffsets, xfbOffsetsVar, nullptr);
}

void SpirvTransformFeedbackCodeGenerator::writeComponentCapture(
    uint32_t bufferIndex,
    spirv::IdRef xfbOffset,
    spirv::IdRef varyingTypeId,
    spirv::IdRef varyingTypePtr,
    spirv::IdRef varyingBaseId,
    const spirv::IdRefList &accessChainIndices,
    GLenum componentType,
    spirv::Blob *blobOut)
{
    spirv::IdRef component(SpirvTransformerBase::GetNewId(blobOut));
    spirv::IdRef xfbOutPtr(SpirvTransformerBase::GetNewId(blobOut));

    // Generate code for the following:
    //
    //     ANGLEXfbN.xfbOut[xfbOffset] = tfVarying0.field[index][row][col]

    // Load from the component traversing the base variable with the given indices.  If there are no
    // indices, the variable can be loaded directly.
    spirv::IdRef loadPtr = varyingBaseId;
    if (!accessChainIndices.empty())
    {
        loadPtr = SpirvTransformerBase::GetNewId(blobOut);
        spirv::WriteAccessChain(blobOut, varyingTypePtr, loadPtr, varyingBaseId,
                                accessChainIndices);
    }
    spirv::WriteLoad(blobOut, varyingTypeId, component, loadPtr, nullptr);

    // If the varying is int or uint, bitcast it to float to store in the float[] array used to
    // capture transform feedback output.
    spirv::IdRef asFloat = component;
    if (componentType != GL_FLOAT)
    {
        asFloat = SpirvTransformerBase::GetNewId(blobOut);
        spirv::WriteBitcast(blobOut, ID::Float, asFloat, component);
    }

    // Store into the transform feedback capture buffer at the current offset.  Note that this
    // buffer has only one field (xfbOut), hence ANGLEXfbN.xfbOut[xfbOffset] translates to ANGLEXfbN
    // with access chain {0, xfbOffset}.
    static_assert(gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS == 4);
    static_assert(sh::vk::spirv::kIdXfbEmulationBufferVarOne ==
                  sh::vk::spirv::kIdXfbEmulationBufferVarZero + 1);
    static_assert(sh::vk::spirv::kIdXfbEmulationBufferVarTwo ==
                  sh::vk::spirv::kIdXfbEmulationBufferVarZero + 2);
    static_assert(sh::vk::spirv::kIdXfbEmulationBufferVarThree ==
                  sh::vk::spirv::kIdXfbEmulationBufferVarZero + 3);
    spirv::WriteAccessChain(blobOut, mFloatUniformPointerId, xfbOutPtr,
                            spirv::IdRef(sh::vk::spirv::kIdXfbEmulationBufferVarZero + bufferIndex),
                            {ID::IntZero, xfbOffset});
    spirv::WriteStore(blobOut, xfbOutPtr, asFloat, nullptr);
}

void SpirvTransformFeedbackCodeGenerator::addExecutionMode(spirv::IdRef entryPointId,
                                                           spirv::Blob *blobOut)
{
    if (mIsEmulated)
    {
        return;
    }

    if (mHasTransformFeedbackOutput)
    {
        spirv::WriteExecutionMode(blobOut, entryPointId, spv::ExecutionModeXfb, {});
    }
}

void SpirvTransformFeedbackCodeGenerator::addMemberDecorate(const XFBInterfaceVariableInfo &info,
                                                            spirv::IdRef id,
                                                            spirv::Blob *blobOut)
{
    if (mIsEmulated || info.fieldXfb.empty())
    {
        return;
    }

    for (uint32_t fieldIndex = 0; fieldIndex < info.fieldXfb.size(); ++fieldIndex)
    {
        const ShaderInterfaceVariableXfbInfo &xfb = info.fieldXfb[fieldIndex];

        if (xfb.pod.buffer == ShaderInterfaceVariableXfbInfo::kInvalid)
        {
            continue;
        }

        ASSERT(xfb.pod.stride != ShaderInterfaceVariableXfbInfo::kInvalid);
        ASSERT(xfb.pod.offset != ShaderInterfaceVariableXfbInfo::kInvalid);

        const uint32_t xfbDecorationValues[kXfbDecorationCount] = {
            xfb.pod.buffer,
            xfb.pod.stride,
            xfb.pod.offset,
        };

        // Generate the following three instructions:
        //
        //     OpMemberDecorate %id fieldIndex XfbBuffer xfb.buffer
        //     OpMemberDecorate %id fieldIndex XfbStride xfb.stride
        //     OpMemberDecorate %id fieldIndex Offset xfb.offset
        for (size_t i = 0; i < kXfbDecorationCount; ++i)
        {
            spirv::WriteMemberDecorate(blobOut, id, spirv::LiteralInteger(fieldIndex),
                                       kXfbDecorations[i],
                                       {spirv::LiteralInteger(xfbDecorationValues[i])});
        }
    }
}

void SpirvTransformFeedbackCodeGenerator::addDecorate(const XFBInterfaceVariableInfo &info,
                                                      spirv::IdRef id,
                                                      spirv::Blob *blobOut)
{
    if (mIsEmulated || info.xfb.pod.buffer == ShaderInterfaceVariableXfbInfo::kInvalid)
    {
        return;
    }

    ASSERT(info.xfb.pod.stride != ShaderInterfaceVariableXfbInfo::kInvalid);
    ASSERT(info.xfb.pod.offset != ShaderInterfaceVariableXfbInfo::kInvalid);

    const uint32_t xfbDecorationValues[kXfbDecorationCount] = {
        info.xfb.pod.buffer,
        info.xfb.pod.stride,
        info.xfb.pod.offset,
    };

    // Generate the following three instructions:
    //
    //     OpDecorate %id XfbBuffer xfb.buffer
    //     OpDecorate %id XfbStride xfb.stride
    //     OpDecorate %id Offset xfb.offset
    for (size_t i = 0; i < kXfbDecorationCount; ++i)
    {
        spirv::WriteDecorate(blobOut, id, kXfbDecorations[i],
                             {spirv::LiteralInteger(xfbDecorationValues[i])});
    }
}

// Helper class that generates code for gl_Position transformations
class SpirvPositionTransformer final : angle::NonCopyable
{
  public:
    SpirvPositionTransformer(const SpvTransformOptions &options) : mOptions(options) {}

    void writePositionTransformation(spirv::IdRef positionPointerId,
                                     spirv::IdRef positionId,
                                     spirv::Blob *blobOut);

  private:
    SpvTransformOptions mOptions;
};

void SpirvPositionTransformer::writePositionTransformation(spirv::IdRef positionPointerId,
                                                           spirv::IdRef positionId,
                                                           spirv::Blob *blobOut)
{
    // Generate the following SPIR-V for prerotation and depth transformation:
    //
    //     // Transform position based on uniforms by making a call to the ANGLETransformPosition
    //     // function that the translator has already provided.
    //     %transformed = OpFunctionCall %kIdVec4 %kIdTransformPositionFunction %position
    //
    //     // Store the results back in gl_Position
    //     OpStore %PositionPointer %transformedPosition
    //
    const spirv::IdRef transformedPositionId(SpirvTransformerBase::GetNewId(blobOut));

    spirv::WriteFunctionCall(blobOut, ID::Vec4, transformedPositionId,
                             ID::TransformPositionFunction, {positionId});
    spirv::WriteStore(blobOut, positionPointerId, transformedPositionId, nullptr);
}

// A transformation to handle both the isMultisampledFramebufferFetch and enableSampleShading
// options.  The common transformation between these two options is the addition of the
// SampleRateShading capability.
class SpirvMultisampleTransformer final : angle::NonCopyable
{
  public:
    SpirvMultisampleTransformer(const SpvTransformOptions &options)
        : mOptions(options), mSampleIDDecorationsAdded(false), mAnyImageTypesModified(false)
    {}
    ~SpirvMultisampleTransformer()
    {
        ASSERT(!mOptions.isMultisampledFramebufferFetch || mAnyImageTypesModified);
    }

    void init(size_t indexBound);

    void visitDecorate(spirv::IdRef id,
                       spv::Decoration decoration,
                       const spirv::LiteralIntegerList &valueList);

    void visitMemberDecorate(spirv::IdRef id,
                             spirv::LiteralInteger member,
                             spv::Decoration decoration);

    void visitTypeStruct(spirv::IdResult id, const spirv::IdRefList &memberList);

    void visitTypePointer(gl::ShaderType shaderType,
                          spirv::IdResult id,
                          spv::StorageClass storageClass,
                          spirv::IdRef typeId);

    void visitVariable(gl::ShaderType shaderType,
                       spirv::IdResultType typeId,
                       spirv::IdResult id,
                       spv::StorageClass storageClass);

    TransformationState transformCapability(const SpirvNonSemanticInstructions &nonSemantic,
                                            spv::Capability capability,
                                            spirv::Blob *blobOut);

    TransformationState transformTypeImage(const uint32_t *instruction, spirv::Blob *blobOut);

    void modifyEntryPointInterfaceList(const SpirvNonSemanticInstructions &nonSemantic,
                                       EntryPointList entryPointList,
                                       spirv::IdRefList *interfaceList,
                                       spirv::Blob *blobOut);

    void writePendingDeclarations(
        const SpirvNonSemanticInstructions &nonSemantic,
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        spirv::Blob *blobOut);

    TransformationState transformDecorate(const SpirvNonSemanticInstructions &nonSemantic,
                                          const ShaderInterfaceVariableInfo &info,
                                          gl::ShaderType shaderType,
                                          spirv::IdRef id,
                                          spirv::IdRef replacementId,
                                          spv::Decoration &decoration,
                                          spirv::Blob *blobOut);

    TransformationState transformImageRead(const uint32_t *instruction, spirv::Blob *blobOut);

  private:
    void visitVarying(gl::ShaderType shaderType, spirv::IdRef id, spv::StorageClass storageClass);
    bool skipSampleDecoration(spv::Decoration decoration);

    SpvTransformOptions mOptions;
    // Used to assert that the transformation is not unnecessarily run.
    bool mSampleIDDecorationsAdded;
    bool mAnyImageTypesModified;

    struct VaryingInfo
    {
        // Whether any variable is a varying
        bool isVarying = false;
        // Whether any variable or its members are already sample-, centroid- or flat-qualified.
        bool skipSampleDecoration = false;
        std::vector<bool> skipMemberSampleDecoration;
    };
    std::vector<VaryingInfo> mVaryingInfoById;
};

void SpirvMultisampleTransformer::init(size_t indexBound)
{
    mVaryingInfoById.resize(indexBound);
}

TransformationState SpirvMultisampleTransformer::transformImageRead(const uint32_t *instruction,
                                                                    spirv::Blob *blobOut)
{
    // Transform the following:
    // %21 = OpImageRead %v4float %13 %20
    // to
    // %21 = OpImageRead %v4float %13 %20 Sample %17
    // where
    // %17 = OpLoad %int %gl_SampleID

    if (!mOptions.isMultisampledFramebufferFetch)
    {
        return TransformationState::Unchanged;
    }

    spirv::IdResultType idResultType;
    spirv::IdResult idResult;
    spirv::IdRef image;
    spirv::IdRef coordinate;
    spv::ImageOperandsMask imageOperands;
    spirv::IdRefList imageOperandIdsList;

    spirv::ParseImageRead(instruction, &idResultType, &idResult, &image, &coordinate,
                          &imageOperands, &imageOperandIdsList);

    ASSERT(ID::Int.valid());

    spirv::IdRef builtInSampleIDOpLoad = SpirvTransformerBase::GetNewId(blobOut);

    spirv::WriteLoad(blobOut, ID::Int, builtInSampleIDOpLoad, ID::SampleID, nullptr);

    imageOperands = spv::ImageOperandsMask::ImageOperandsSampleMask;
    imageOperandIdsList.push_back(builtInSampleIDOpLoad);
    spirv::WriteImageRead(blobOut, idResultType, idResult, image, coordinate, &imageOperands,
                          imageOperandIdsList);
    return TransformationState::Transformed;
}

void SpirvMultisampleTransformer::writePendingDeclarations(
    const SpirvNonSemanticInstructions &nonSemantic,
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    spirv::Blob *blobOut)
{
    // Add following declarations if they are not available yet

    // %int = OpTypeInt 32 1
    // %_ptr_Input_int = OpTypePointer Input %int
    // %gl_SampleID = OpVariable %_ptr_Input_int Input

    if (!mOptions.isMultisampledFramebufferFetch)
    {
        return;
    }

    if (nonSemantic.hasSampleID())
    {
        return;
    }

    spirv::WriteVariable(blobOut, ID::IntInputTypePointer, ID::SampleID, spv::StorageClassInput,
                         nullptr);
}

TransformationState SpirvMultisampleTransformer::transformTypeImage(const uint32_t *instruction,
                                                                    spirv::Blob *blobOut)
{
    // Transform the following
    // %10 = OpTypeImage %float SubpassData 0 0 0 2
    // To
    // %10 = OpTypeImage %float SubpassData 0 0 1 2

    if (!mOptions.isMultisampledFramebufferFetch)
    {
        return TransformationState::Unchanged;
    }

    spirv::IdResult idResult;
    spirv::IdRef sampledType;
    spv::Dim dim;
    spirv::LiteralInteger depth;
    spirv::LiteralInteger arrayed;
    spirv::LiteralInteger ms;
    spirv::LiteralInteger sampled;
    spv::ImageFormat imageFormat;
    spv::AccessQualifier accessQualifier;
    spirv::ParseTypeImage(instruction, &idResult, &sampledType, &dim, &depth, &arrayed, &ms,
                          &sampled, &imageFormat, &accessQualifier);

    // Only transform input attachment image types.
    if (dim != spv::DimSubpassData)
    {
        return TransformationState::Unchanged;
    }

    ms = spirv::LiteralInteger(1);
    spirv::WriteTypeImage(blobOut, idResult, sampledType, dim, depth, arrayed, ms, sampled,
                          imageFormat, nullptr);

    mAnyImageTypesModified = true;

    return TransformationState::Transformed;
}

namespace
{
bool verifyEntryPointsContainsID(const spirv::IdRefList &interfaceList)
{
    for (spirv::IdRef interfaceId : interfaceList)
    {
        if (interfaceId == ID::SampleID)
        {
            return true;
        }
    }
    return false;
}
}  // namespace

void SpirvMultisampleTransformer::modifyEntryPointInterfaceList(
    const SpirvNonSemanticInstructions &nonSemantic,
    EntryPointList entryPointList,
    spirv::IdRefList *interfaceList,
    spirv::Blob *blobOut)
{
    // Append %gl_sampleID to OpEntryPoint
    // Transform the following
    //
    //     OpEntryPoint Fragment %main "main" %_uo_color
    //
    // To
    //
    //     OpEntryPoint Fragment %main "main" %_uo_color %gl_SampleID

    if (!mOptions.isMultisampledFramebufferFetch)
    {
        return;
    }

    // Nothing to do if the shader had already declared SampleID
    if (nonSemantic.hasSampleID())
    {
        ASSERT(verifyEntryPointsContainsID(*interfaceList));
        return;
    }

    // Add the SampleID id to the interfaceList.  The variable will later be decalred in
    // writePendingDeclarations.
    interfaceList->push_back(ID::SampleID);
    return;
}

TransformationState SpirvMultisampleTransformer::transformCapability(
    const SpirvNonSemanticInstructions &nonSemantic,
    const spv::Capability capability,
    spirv::Blob *blobOut)
{
    // Add a new OpCapability line:
    //
    //     OpCapability SampleRateShading
    //
    // right before the following instruction
    //
    //     OpCapability InputAttachment

    if (!mOptions.isMultisampledFramebufferFetch && !mOptions.enableSampleShading)
    {
        return TransformationState::Unchanged;
    }

    // Do not add the capability if the SPIR-V already has it
    if (nonSemantic.hasSampleRateShading())
    {
        return TransformationState::Unchanged;
    }

    // Make sure no duplicates
    ASSERT(capability != spv::CapabilitySampleRateShading);

    // Make sure we only add the new line on top of "OpCapability Shader"
    if (capability != spv::CapabilityShader)
    {
        return TransformationState::Unchanged;
    }

    spirv::WriteCapability(blobOut, spv::CapabilitySampleRateShading);

    // Leave the original OpCapability untouched
    return TransformationState::Unchanged;
}

TransformationState SpirvMultisampleTransformer::transformDecorate(
    const SpirvNonSemanticInstructions &nonSemantic,
    const ShaderInterfaceVariableInfo &info,
    gl::ShaderType shaderType,
    spirv::IdRef id,
    spirv::IdRef replacementId,
    spv::Decoration &decoration,
    spirv::Blob *blobOut)
{
    if (mOptions.isMultisampledFramebufferFetch && !nonSemantic.hasSampleID() &&
        !mSampleIDDecorationsAdded)
    {
        // Add the following instructions if they are not available yet:
        //
        //     OpDecorate %gl_SampleID RelaxedPrecision
        //     OpDecorate %gl_SampleID Flat
        //     OpDecorate %gl_SampleID BuiltIn SampleId

        spirv::WriteDecorate(blobOut, ID::SampleID, spv::DecorationRelaxedPrecision, {});
        spirv::WriteDecorate(blobOut, ID::SampleID, spv::DecorationFlat, {});
        spirv::WriteDecorate(blobOut, ID::SampleID, spv::DecorationBuiltIn,
                             {spirv::LiteralInteger(spv::BuiltIn::BuiltInSampleId)});

        mSampleIDDecorationsAdded = true;
    }
    if (mOptions.enableSampleShading && mVaryingInfoById[id].isVarying &&
        !mVaryingInfoById[id].skipSampleDecoration)
    {
        if (decoration == spv::DecorationLocation && info.activeStages[shaderType])
        {
            // Add the following instructions when the Location decoration is met, if the varying is
            // not already decorated with Sample:
            //
            //     OpDecorate %id Sample
            spirv::WriteDecorate(blobOut, replacementId, spv::DecorationSample, {});
        }
        else if (decoration == spv::DecorationBlock)
        {
            // Add the following instructions when the Block decoration is met, for any member that
            // is not already decorated with Sample:
            //
            //     OpMemberDecorate %id member Sample
            for (uint32_t member = 0;
                 member < mVaryingInfoById[id].skipMemberSampleDecoration.size(); ++member)
            {
                if (!mVaryingInfoById[id].skipMemberSampleDecoration[member])
                {
                    spirv::WriteMemberDecorate(blobOut, replacementId,
                                               spirv::LiteralInteger(member), spv::DecorationSample,
                                               {});
                }
            }
        }
    }

    return TransformationState::Unchanged;
}

bool SpirvMultisampleTransformer::skipSampleDecoration(spv::Decoration decoration)
{
    // If a variable is already decorated with Sample, Patch or Centroid, it shouldn't be decorated
    // with Sample.  BuiltIns are also excluded.
    return decoration == spv::DecorationPatch || decoration == spv::DecorationCentroid ||
           decoration == spv::DecorationSample || decoration == spv::DecorationBuiltIn;
}

void SpirvMultisampleTransformer::visitDecorate(spirv::IdRef id,
                                                spv::Decoration decoration,
                                                const spirv::LiteralIntegerList &valueList)
{
    if (mOptions.enableSampleShading)
    {
        // Determine whether the id is already decorated with Sample.
        if (skipSampleDecoration(decoration))
        {
            mVaryingInfoById[id].skipSampleDecoration = true;
        }
    }
    return;
}

void SpirvMultisampleTransformer::visitMemberDecorate(spirv::IdRef id,
                                                      spirv::LiteralInteger member,
                                                      spv::Decoration decoration)
{
    if (!mOptions.enableSampleShading)
    {
        return;
    }

    if (mVaryingInfoById[id].skipMemberSampleDecoration.size() <= member)
    {
        mVaryingInfoById[id].skipMemberSampleDecoration.resize(member + 1, false);
    }
    // Determine whether the member is already decorated with Sample.
    if (skipSampleDecoration(decoration))
    {
        mVaryingInfoById[id].skipMemberSampleDecoration[member] = true;
    }
}

void SpirvMultisampleTransformer::visitVarying(gl::ShaderType shaderType,
                                               spirv::IdRef id,
                                               spv::StorageClass storageClass)
{
    if (!mOptions.enableSampleShading)
    {
        return;
    }

    // Vertex input and fragment output variables are not varyings and don't need to be decorated
    // with Sample.
    if ((shaderType == gl::ShaderType::Fragment && storageClass == spv::StorageClassOutput) ||
        (shaderType == gl::ShaderType::Vertex && storageClass == spv::StorageClassInput))
    {
        return;
    }

    const bool isVarying =
        storageClass == spv::StorageClassInput || storageClass == spv::StorageClassOutput;
    mVaryingInfoById[id].isVarying = isVarying;
}

void SpirvMultisampleTransformer::visitTypeStruct(spirv::IdResult id,
                                                  const spirv::IdRefList &memberList)
{
    if (mOptions.enableSampleShading)
    {
        mVaryingInfoById[id].skipMemberSampleDecoration.resize(memberList.size(), false);
    }
}

void SpirvMultisampleTransformer::visitTypePointer(gl::ShaderType shaderType,
                                                   spirv::IdResult id,
                                                   spv::StorageClass storageClass,
                                                   spirv::IdRef typeId)
{
    // For I/O blocks, the Sample decoration should be specified on the members of the struct type.
    // For that purpose, we consider the struct type as the varying instead.
    visitVarying(shaderType, typeId, storageClass);
}

void SpirvMultisampleTransformer::visitVariable(gl::ShaderType shaderType,
                                                spirv::IdResultType typeId,
                                                spirv::IdResult id,
                                                spv::StorageClass storageClass)
{
    visitVarying(shaderType, id, storageClass);
}

// Helper class that flattens secondary fragment output arrays.
class SpirvSecondaryOutputTransformer final : angle::NonCopyable
{
  public:
    SpirvSecondaryOutputTransformer() {}

    void init(size_t indexBound);

    void visitTypeArray(spirv::IdResult id, spirv::IdRef typeId);

    void visitTypePointer(spirv::IdResult id, spirv::IdRef typeId);

    TransformationState transformAccessChain(spirv::IdResultType typeId,
                                             spirv::IdResult id,
                                             spirv::IdRef baseId,
                                             const spirv::IdRefList &indexList,
                                             spirv::Blob *blobOut);

    TransformationState transformDecorate(spirv::IdRef id,
                                          spv::Decoration decoration,
                                          const spirv::LiteralIntegerList &decorationValues,
                                          spirv::Blob *blobOut);

    TransformationState transformVariable(spirv::IdResultType typeId,
                                          spirv::IdResultType privateTypeId,
                                          spirv::IdResult id,
                                          spirv::Blob *blobOut);

    void modifyEntryPointInterfaceList(
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        EntryPointList entryPointList,
        spirv::IdRefList *interfaceList,
        spirv::Blob *blobOut);

    void writeOutputPrologue(
        const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
        spirv::Blob *blobOut);

    static_assert(gl::IMPLEMENTATION_MAX_DUAL_SOURCE_DRAW_BUFFERS == 1,
                  "This transformer is incompatible with two or more dual-source draw buffers");

  private:
    void visitTypeHelper(spirv::IdResult id, spirv::IdRef typeId) { mTypeCache[id] = typeId; }

    // This list is filled during visitTypePointer and visitTypeArray steps,
    // to resolve the element type ID of the original output array variable.
    std::vector<spirv::IdRef> mTypeCache;
    spirv::IdRef mElementTypeId;
    spirv::IdRef mArrayVariableId;
    spirv::IdRef mReplacementVariableId;
    spirv::IdRef mElementPointerTypeId;
};

void SpirvSecondaryOutputTransformer::init(size_t indexBound)
{
    mTypeCache.resize(indexBound);
}

void SpirvSecondaryOutputTransformer::visitTypeArray(spirv::IdResult id, spirv::IdRef typeId)
{
    visitTypeHelper(id, typeId);
}

void SpirvSecondaryOutputTransformer::visitTypePointer(spirv::IdResult id, spirv::IdRef typeId)
{
    visitTypeHelper(id, typeId);
}

void SpirvSecondaryOutputTransformer::modifyEntryPointInterfaceList(
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    EntryPointList entryPointList,
    spirv::IdRefList *interfaceList,
    spirv::Blob *blobOut)
{
    // Flatten a secondary output array (if any).
    for (size_t index = 0; index < interfaceList->size(); ++index)
    {
        const spirv::IdRef id((*interfaceList)[index]);
        const ShaderInterfaceVariableInfo *info =
            id < variableInfoById.size() ? variableInfoById[id] : nullptr;

        if (info == nullptr || info->index != 1 || !info->isArray)
        {
            continue;
        }

        mArrayVariableId       = id;
        mReplacementVariableId = SpirvTransformerBase::GetNewId(blobOut);

        // With SPIR-V 1.3, modify interface list with the replacement ID.
        //
        // With SPIR-V 1.4, the original variable is changed to Private and should remain in the
        // list.  The new variable should be added to the variable list.
        if (entryPointList == EntryPointList::InterfaceVariables)
        {
            (*interfaceList)[index] = mReplacementVariableId;
        }
        else
        {
            interfaceList->push_back(mReplacementVariableId);
        }
        break;
    }
}

TransformationState SpirvSecondaryOutputTransformer::transformAccessChain(
    spirv::IdResultType typeId,
    spirv::IdResult id,
    spirv::IdRef baseId,
    const spirv::IdRefList &indexList,
    spirv::Blob *blobOut)
{
    if (baseId != mArrayVariableId)
    {
        return TransformationState::Unchanged;
    }
    ASSERT(typeId.valid());
    spirv::WriteAccessChain(blobOut, typeId, id, baseId, indexList);
    return TransformationState::Transformed;
}

TransformationState SpirvSecondaryOutputTransformer::transformDecorate(
    spirv::IdRef id,
    spv::Decoration decoration,
    const spirv::LiteralIntegerList &decorationValues,
    spirv::Blob *blobOut)
{
    if (id != mArrayVariableId)
    {
        return TransformationState::Unchanged;
    }
    ASSERT(mReplacementVariableId.valid());
    if (decoration == spv::DecorationLocation)
    {
        // Drop the Location decoration from the original variable and add
        // it together with an Index decoration to the replacement variable.
        spirv::WriteDecorate(blobOut, mReplacementVariableId, spv::DecorationLocation,
                             {spirv::LiteralInteger(0)});
        spirv::WriteDecorate(blobOut, mReplacementVariableId, spv::DecorationIndex,
                             {spirv::LiteralInteger(1)});
    }
    else
    {
        // Apply other decorations, such as RelaxedPrecision, to both variables.
        spirv::WriteDecorate(blobOut, id, decoration, decorationValues);
        spirv::WriteDecorate(blobOut, mReplacementVariableId, decoration, decorationValues);
    }
    return TransformationState::Transformed;
}

TransformationState SpirvSecondaryOutputTransformer::transformVariable(
    spirv::IdResultType typeId,
    spirv::IdResultType privateTypeId,
    spirv::IdResult id,
    spirv::Blob *blobOut)
{
    if (id != mArrayVariableId)
    {
        return TransformationState::Unchanged;
    }

    // Change the original variable to use private storage.
    ASSERT(privateTypeId.valid());
    spirv::WriteVariable(blobOut, privateTypeId, id, spv::StorageClassPrivate, nullptr);

    ASSERT(!mElementTypeId.valid());
    mElementTypeId = mTypeCache[mTypeCache[typeId]];
    ASSERT(mElementTypeId.valid());

    // Pointer type for accessing the array element value.
    mElementPointerTypeId = SpirvTransformerBase::GetNewId(blobOut);
    spirv::WriteTypePointer(blobOut, mElementPointerTypeId, spv::StorageClassPrivate,
                            mElementTypeId);

    // Pointer type for the replacement output variable.
    const spirv::IdRef outputPointerTypeId(SpirvTransformerBase::GetNewId(blobOut));
    spirv::WriteTypePointer(blobOut, outputPointerTypeId, spv::StorageClassOutput, mElementTypeId);

    ASSERT(mReplacementVariableId.valid());
    spirv::WriteVariable(blobOut, outputPointerTypeId, mReplacementVariableId,
                         spv::StorageClassOutput, nullptr);

    return TransformationState::Transformed;
}

void SpirvSecondaryOutputTransformer::writeOutputPrologue(
    const std::vector<const ShaderInterfaceVariableInfo *> &variableInfoById,
    spirv::Blob *blobOut)
{
    if (mArrayVariableId.valid())
    {
        const spirv::IdRef accessChainId(SpirvTransformerBase::GetNewId(blobOut));
        spirv::WriteAccessChain(blobOut, mElementPointerTypeId, accessChainId, mArrayVariableId,
                                {ID::IntZero});

        ASSERT(mElementTypeId.valid());
        const spirv::IdRef loadId(SpirvTransformerBase::GetNewId(blobOut));
        spirv::WriteLoad(blobOut, mElementTypeId, loadId, accessChainId, nullptr);

        ASSERT(mReplacementVariableId.valid());
        spirv::WriteStore(blobOut, mReplacementVariableId, loadId, nullptr);
    }
}

// Helper class that removes mentions of depth/stencil input attachments and replaces subpassLoads
// of these attachments with 0.
class SpirvDepthStencilInputRemover final : angle::NonCopyable
{
  public:
    SpirvDepthStencilInputRemover() {}

    TransformationState transformName(spirv::IdRef id, spirv::LiteralString name);
    TransformationState transformDecorate(spirv::IdRef id,
                                          spv::Decoration decoration,
                                          const spirv::LiteralIntegerList &decorationValues,
                                          spirv::Blob *blobOut);
    TransformationState transformVariable(spirv::IdResultType typeId,
                                          spirv::IdResult id,
                                          spirv::Blob *blobOut);
    TransformationState transformLoad(spirv::IdResultType typeId,
                                      spirv::IdResult id,
                                      spirv::IdRef pointerId,
                                      spirv::Blob *blobOut);

    TransformationState transformImageRead(const uint32_t *instruction, spirv::Blob *blobOut);

    void modifyEntryPointInterfaceList(spirv::IdRefList *interfaceList, spirv::Blob *blobOut);

    void writePendingDeclarations(spirv::Blob *blobOut);

  private:
    bool isDepthStencilInput(spirv::IdRef id)
    {
        return id == sh::vk::spirv::kIdDepthInputAttachment ||
               id == sh::vk::spirv::kIdStencilInputAttachment;
    }

    spirv::IdRef mVec4ZeroId;
    spirv::IdRef mIVec4ZeroId;

    std::vector<spirv::IdRef> mImageReadParamIdsToRemove;
};

TransformationState SpirvDepthStencilInputRemover::transformName(spirv::IdRef id,
                                                                 spirv::LiteralString name)
{
    // Both depth and stencil input variables are removed, so remove their debug info too
    return isDepthStencilInput(id) ? TransformationState::Transformed
                                   : TransformationState::Unchanged;
}

TransformationState SpirvDepthStencilInputRemover::transformDecorate(
    spirv::IdRef id,
    spv::Decoration decoration,
    const spirv::LiteralIntegerList &decorationValues,
    spirv::Blob *blobOut)
{
    // Both depth and stencil input variables are removed, so remove their decorations too
    return isDepthStencilInput(id) ? TransformationState::Transformed
                                   : TransformationState::Unchanged;
}

TransformationState SpirvDepthStencilInputRemover::transformVariable(spirv::IdResultType typeId,
                                                                     spirv::IdResult id,
                                                                     spirv::Blob *blobOut)
{
    // Both depth and stencil input variables are removed
    return isDepthStencilInput(id) ? TransformationState::Transformed
                                   : TransformationState::Unchanged;
}

TransformationState SpirvDepthStencilInputRemover::transformLoad(spirv::IdResultType typeId,
                                                                 spirv::IdResult id,
                                                                 spirv::IdRef pointerId,
                                                                 spirv::Blob *blobOut)
{
    if (isDepthStencilInput(pointerId))
    {
        // Both depth and stencil input variables are removed, so OpLoad from them needs to be
        // removed.  Later, OpImageRead from the result of this instruction should also be removed,
        // so keep that in |mImageReadParamIdsToRemove|.
        mImageReadParamIdsToRemove.push_back(id);

        // The result of OpLoad for the stencil attachment is decorated with RelaxedPrecision.  At
        // this point, we have passed the OpDecorate section; instead of adding another pass to
        // either discover this ID earlier or remove the OpDecorate later, this instruction is
        // simply replaced with OpCopyObject given the ivec4(0) constant.  The driver would
        // eliminate it as dead code.
        if (pointerId == sh::vk::spirv::kIdStencilInputAttachment)
        {
            spirv::WriteCopyObject(blobOut, ID::IVec4, id, mIVec4ZeroId);
        }

        return TransformationState::Transformed;
    }

    return TransformationState::Unchanged;
}

TransformationState SpirvDepthStencilInputRemover::transformImageRead(const uint32_t *instruction,
                                                                      spirv::Blob *blobOut)
{
    spirv::IdResultType idResultType;
    spirv::IdResult idResult;
    spirv::IdRef image;
    spirv::IdRef coordinate;
    spv::ImageOperandsMask imageOperands;
    spirv::IdRefList imageOperandIdsList;

    spirv::ParseImageRead(instruction, &idResultType, &idResult, &image, &coordinate,
                          &imageOperands, &imageOperandIdsList);

    if (std::find(mImageReadParamIdsToRemove.begin(), mImageReadParamIdsToRemove.end(), image) !=
        mImageReadParamIdsToRemove.end())
    {
        // Replace the OpImageRead from removed images with OpCopyObject from [i]vec4(0).
        ASSERT(idResultType == ID::Vec4 || idResultType == ID::IVec4);
        spirv::WriteCopyObject(blobOut, idResultType, idResult,
                               idResultType == ID::Vec4 ? mVec4ZeroId : mIVec4ZeroId);
        return TransformationState::Transformed;
    }

    return TransformationState::Unchanged;
}

void SpirvDepthStencilInputRemover::modifyEntryPointInterfaceList(spirv::IdRefList *interfaceList,
                                                                  spirv::Blob *blobOut)
{
    // Remove the depth and stencil input variables from the interface list.
    size_t writeIndex = 0;
    for (size_t index = 0; index < interfaceList->size(); ++index)
    {
        spirv::IdRef id((*interfaceList)[index]);
        if (!isDepthStencilInput(id))
        {
            (*interfaceList)[writeIndex] = id;
            ++writeIndex;
        }
    }

    interfaceList->resize_down(writeIndex);
}

void SpirvDepthStencilInputRemover::writePendingDeclarations(spirv::Blob *blobOut)
{
    // Add vec4(0) and uvec4(0) declarations for future use.
    mVec4ZeroId  = SpirvTransformerBase::GetNewId(blobOut);
    mIVec4ZeroId = SpirvTransformerBase::GetNewId(blobOut);

    spirv::WriteConstantNull(blobOut, ID::Vec4, mVec4ZeroId);
    spirv::WriteConstantNull(blobOut, ID::IVec4, mIVec4ZeroId);
}

// A SPIR-V transformer.  It walks the instructions and modifies them as necessary, for example to
// assign bindings or locations.
class SpirvTransformer final : public SpirvTransformerBase
{
  public:
    SpirvTransformer(const spirv::Blob &spirvBlobIn,
                     const SpvTransformOptions &options,
                     bool isLastPass,
                     const ShaderInterfaceVariableInfoMap &variableInfoMap,
                     spirv::Blob *spirvBlobOut)
        : SpirvTransformerBase(spirvBlobIn, variableInfoMap, spirvBlobOut),
          mOptions(options),
          mOverviewFlags(0),
          mNonSemanticInstructions(isLastPass),
          mPerVertexTrimmer(options, variableInfoMap),
          mXfbCodeGenerator(options),
          mPositionTransformer(options),
          mMultisampleTransformer(options)
    {}

    void transform();

  private:
    // A prepass to resolve interesting ids:
    void resolveVariableIds();

    // Transform instructions:
    void transformInstruction();

    // Instructions that are purely informational:
    void visitDecorate(const uint32_t *instruction);
    void visitMemberDecorate(const uint32_t *instruction);
    void visitTypeArray(const uint32_t *instruction);
    void visitTypePointer(const uint32_t *instruction);
    void visitTypeStruct(const uint32_t *instruction);
    void visitVariable(const uint32_t *instruction);
    void visitCapability(const uint32_t *instruction);
    bool visitExtInst(const uint32_t *instruction);

    // Instructions that potentially need transformation.  They return true if the instruction is
    // transformed.  If false is returned, the instruction should be copied as-is.
    TransformationState transformAccessChain(const uint32_t *instruction);
    TransformationState transformCapability(const uint32_t *instruction);
    TransformationState transformEntryPoint(const uint32_t *instruction);
    TransformationState transformExtension(const uint32_t *instruction);
    TransformationState transformExtInstImport(const uint32_t *instruction);
    TransformationState transformExtInst(const uint32_t *instruction);
    TransformationState transformLoad(const uint32_t *instruction);
    TransformationState transformDecorate(const uint32_t *instruction);
    TransformationState transformMemberDecorate(const uint32_t *instruction);
    TransformationState transformName(const uint32_t *instruction);
    TransformationState transformMemberName(const uint32_t *instruction);
    TransformationState transformTypePointer(const uint32_t *instruction);
    TransformationState transformTypeStruct(const uint32_t *instruction);
    TransformationState transformVariable(const uint32_t *instruction);
    TransformationState transformTypeImage(const uint32_t *instruction);
    TransformationState transformImageRead(const uint32_t *instruction);

    // Helpers:
    void visitTypeHelper(spirv::IdResult id, spirv::IdRef typeId);
    void writePendingDeclarations();
    void writeInputPreamble();
    void writeOutputPrologue();

    // Special flags:
    SpvTransformOptions mOptions;

    // Traversal state:
    spirv::IdRef mCurrentFunctionId;

    // Transformation state:

    uint32_t mOverviewFlags;
    SpirvNonSemanticInstructions mNonSemanticInstructions;
    SpirvPerVertexTrimmer mPerVertexTrimmer;
    SpirvInactiveVaryingRemover mInactiveVaryingRemover;
    SpirvVaryingPrecisionFixer mVaryingPrecisionFixer;
    SpirvTransformFeedbackCodeGenerator mXfbCodeGenerator;
    SpirvPositionTransformer mPositionTransformer;
    SpirvMultisampleTransformer mMultisampleTransformer;
    SpirvSecondaryOutputTransformer mSecondaryOutputTransformer;
    SpirvDepthStencilInputRemover mDepthStencilInputRemover;
};

void SpirvTransformer::transform()
{
    onTransformBegin();

    // First, find all necessary ids and associate them with the information required to transform
    // their decorations.  This is mostly derived from |mVariableInfoMap|, but may have additional
    // mappings; for example |mVariableInfoMap| maps an interface block's type ID to info, but the
    // transformer needs to discover the variable associated with that block and map it to the same
    // info.
    resolveVariableIds();

    while (mCurrentWord < mSpirvBlobIn.size())
    {
        transformInstruction();
    }
}

void SpirvTransformer::resolveVariableIds()
{
    const size_t indexBound = mSpirvBlobIn[spirv::kHeaderIndexIndexBound];

    mInactiveVaryingRemover.init(indexBound);
    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        mVaryingPrecisionFixer.init(indexBound);
    }
    if (mOptions.isMultisampledFramebufferFetch || mOptions.enableSampleShading)
    {
        mMultisampleTransformer.init(indexBound);
    }
    if (mOptions.shaderType == gl::ShaderType::Fragment)
    {
        mSecondaryOutputTransformer.init(indexBound);
    }

    // Allocate storage for id-to-info map.  If %i is an id in mVariableInfoMap, index i in this
    // vector will hold a pointer to the ShaderInterfaceVariableInfo object associated with that
    // name in mVariableInfoMap.
    mVariableInfoById.resize(indexBound, nullptr);

    // Pre-populate from mVariableInfoMap.
    {
        const ShaderInterfaceVariableInfoMap::VariableInfoArray &data = mVariableInfoMap.getData();
        const ShaderInterfaceVariableInfoMap::IdToIndexMap &idToIndexMap =
            mVariableInfoMap.getIdToIndexMap()[mOptions.shaderType];

        for (uint32_t hashedId = 0; hashedId < idToIndexMap.size(); ++hashedId)
        {
            const uint32_t id                  = hashedId + sh::vk::spirv::kIdShaderVariablesBegin;
            const VariableIndex &variableIndex = idToIndexMap.at(hashedId);
            if (variableIndex.index == VariableIndex::kInvalid)
            {
                continue;
            }

            const ShaderInterfaceVariableInfo &info = data[variableIndex.index];

            ASSERT(id < mVariableInfoById.size());
            mVariableInfoById[id] = &info;
        }
    }

    size_t currentWord = spirv::kHeaderIndexInstructions;

    while (currentWord < mSpirvBlobIn.size())
    {
        const uint32_t *instruction = &mSpirvBlobIn[currentWord];

        uint32_t wordCount;
        spv::Op opCode;
        spirv::GetInstructionOpAndLength(instruction, &opCode, &wordCount);

        switch (opCode)
        {
            case spv::OpDecorate:
                visitDecorate(instruction);
                break;
            case spv::OpMemberDecorate:
                visitMemberDecorate(instruction);
                break;
            case spv::OpTypeArray:
                visitTypeArray(instruction);
                break;
            case spv::OpTypePointer:
                visitTypePointer(instruction);
                break;
            case spv::OpTypeStruct:
                visitTypeStruct(instruction);
                break;
            case spv::OpVariable:
                visitVariable(instruction);
                break;
            case spv::OpExtInst:
                if (visitExtInst(instruction))
                {
                    return;
                }
                break;
            default:
                break;
        }

        currentWord += wordCount;
    }
    UNREACHABLE();
}

void SpirvTransformer::transformInstruction()
{
    uint32_t wordCount;
    spv::Op opCode;
    const uint32_t *instruction = getCurrentInstruction(&opCode, &wordCount);

    if (opCode == spv::OpFunction)
    {
        spirv::IdResultType id;
        spv::FunctionControlMask functionControl;
        spirv::IdRef functionType;
        spirv::ParseFunction(instruction, &id, &mCurrentFunctionId, &functionControl,
                             &functionType);

        // SPIR-V is structured in sections.  Function declarations come last.  Only a few
        // instructions such as Op*Access* or OpEmitVertex opcodes inside functions need to be
        // inspected.
        mIsInFunctionSection = true;
    }

    // Only look at interesting instructions.
    TransformationState transformationState = TransformationState::Unchanged;

    if (mIsInFunctionSection)
    {
        // Look at in-function opcodes.
        switch (opCode)
        {
            case spv::OpExtInst:
                transformationState = transformExtInst(instruction);
                break;
            case spv::OpAccessChain:
            case spv::OpInBoundsAccessChain:
            case spv::OpPtrAccessChain:
            case spv::OpInBoundsPtrAccessChain:
                transformationState = transformAccessChain(instruction);
                break;
            case spv::OpLoad:
                transformationState = transformLoad(instruction);
                break;
            case spv::OpImageRead:
                transformationState = transformImageRead(instruction);
                break;
            default:
                break;
        }
    }
    else
    {
        // Look at global declaration opcodes.
        switch (opCode)
        {
            case spv::OpExtension:
                transformationState = transformExtension(instruction);
                break;
            case spv::OpExtInstImport:
                transformationState = transformExtInstImport(instruction);
                break;
            case spv::OpExtInst:
                transformationState = transformExtInst(instruction);
                break;
            case spv::OpName:
                transformationState = transformName(instruction);
                break;
            case spv::OpMemberName:
                transformationState = transformMemberName(instruction);
                break;
            case spv::OpCapability:
                transformationState = transformCapability(instruction);
                break;
            case spv::OpEntryPoint:
                transformationState = transformEntryPoint(instruction);
                break;
            case spv::OpDecorate:
                transformationState = transformDecorate(instruction);
                break;
            case spv::OpMemberDecorate:
                transformationState = transformMemberDecorate(instruction);
                break;
            case spv::OpTypeImage:
                transformationState = transformTypeImage(instruction);
                break;
            case spv::OpTypePointer:
                transformationState = transformTypePointer(instruction);
                break;
            case spv::OpTypeStruct:
                transformationState = transformTypeStruct(instruction);
                break;
            case spv::OpVariable:
                transformationState = transformVariable(instruction);
                break;
            default:
                break;
        }
    }

    // If the instruction was not transformed, copy it to output as is.
    if (transformationState == TransformationState::Unchanged)
    {
        copyInstruction(instruction, wordCount);
    }

    // Advance to next instruction.
    mCurrentWord += wordCount;
}

// Called at the end of the declarations section.  Any declarations that are necessary but weren't
// present in the original shader need to be done here.
void SpirvTransformer::writePendingDeclarations()
{
    if (mOptions.removeDepthStencilInput)
    {
        mDepthStencilInputRemover.writePendingDeclarations(mSpirvBlobOut);
    }

    mMultisampleTransformer.writePendingDeclarations(mNonSemanticInstructions, mVariableInfoById,
                                                     mSpirvBlobOut);

    // Pre-rotation and transformation of depth to Vulkan clip space require declarations that may
    // not necessarily be in the shader.  Transform feedback emulation additionally requires a few
    // overlapping ids.
    if (!mOptions.isLastPreFragmentStage)
    {
        return;
    }

    if (mOptions.isTransformFeedbackStage)
    {
        mXfbCodeGenerator.writePendingDeclarations(mVariableInfoById, storageBufferStorageClass(),
                                                   mSpirvBlobOut);
    }
}

// Called by transformInstruction to insert necessary instructions for casting varyings.
void SpirvTransformer::writeInputPreamble()
{
    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        mVaryingPrecisionFixer.writeInputPreamble(mVariableInfoById, mOptions.shaderType,
                                                  mSpirvBlobOut);
    }
}

// Called by transformInstruction to insert necessary instructions for casting varyings and
// modifying gl_Position.
void SpirvTransformer::writeOutputPrologue()
{
    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        mVaryingPrecisionFixer.writeOutputPrologue(mVariableInfoById, mOptions.shaderType,
                                                   mSpirvBlobOut);
    }
    if (mOptions.shaderType == gl::ShaderType::Fragment)
    {
        mSecondaryOutputTransformer.writeOutputPrologue(mVariableInfoById, mSpirvBlobOut);
    }
    if (!mNonSemanticInstructions.hasOutputPerVertex())
    {
        return;
    }

    // Whether gl_Position should be transformed to account for pre-rotation and Vulkan clip space.
    const bool transformPosition = mOptions.isLastPreFragmentStage;
    const bool isXfbExtensionStage =
        mOptions.isTransformFeedbackStage && !mOptions.isTransformFeedbackEmulated;
    if (!transformPosition && !isXfbExtensionStage)
    {
        return;
    }

    // Load gl_Position with the following SPIR-V:
    //
    //     // Create an access chain to gl_PerVertex.gl_Position, which is always at index 0.
    //     %PositionPointer = OpAccessChain %kIdVec4OutputTypePointer %kIdOutputPerVertexVar
    //                                      %kIdIntZero
    //     // Load gl_Position
    //     %Position = OpLoad %kIdVec4 %PositionPointer
    //
    const spirv::IdRef positionPointerId(getNewId());
    const spirv::IdRef positionId(getNewId());

    spirv::WriteAccessChain(mSpirvBlobOut, ID::Vec4OutputTypePointer, positionPointerId,
                            ID::OutputPerVertexVar, {ID::IntZero});
    spirv::WriteLoad(mSpirvBlobOut, ID::Vec4, positionId, positionPointerId, nullptr);

    // Write transform feedback output before modifying gl_Position.
    if (isXfbExtensionStage)
    {
        mXfbCodeGenerator.writeTransformFeedbackExtensionOutput(positionId, mSpirvBlobOut);
    }

    if (transformPosition)
    {
        mPositionTransformer.writePositionTransformation(positionPointerId, positionId,
                                                         mSpirvBlobOut);
    }
}

void SpirvTransformer::visitDecorate(const uint32_t *instruction)
{
    spirv::IdRef id;
    spv::Decoration decoration;
    spirv::LiteralIntegerList valueList;
    spirv::ParseDecorate(instruction, &id, &decoration, &valueList);

    mMultisampleTransformer.visitDecorate(id, decoration, valueList);
}

void SpirvTransformer::visitMemberDecorate(const uint32_t *instruction)
{
    spirv::IdRef typeId;
    spirv::LiteralInteger member;
    spv::Decoration decoration;
    spirv::LiteralIntegerList valueList;
    spirv::ParseMemberDecorate(instruction, &typeId, &member, &decoration, &valueList);

    mPerVertexTrimmer.visitMemberDecorate(typeId, member, decoration, valueList);
    mMultisampleTransformer.visitMemberDecorate(typeId, member, decoration);
}

void SpirvTransformer::visitTypeHelper(spirv::IdResult id, spirv::IdRef typeId)
{
    // Carry forward the mapping of typeId->info to id->info.  For interface block, it's the block
    // id that is mapped to the info, so this is necessary to eventually be able to map the variable
    // itself to the info.
    mVariableInfoById[id] = mVariableInfoById[typeId];
}

void SpirvTransformer::visitTypeArray(const uint32_t *instruction)
{
    spirv::IdResult id;
    spirv::IdRef elementType;
    spirv::IdRef length;
    spirv::ParseTypeArray(instruction, &id, &elementType, &length);

    visitTypeHelper(id, elementType);
    if (mOptions.shaderType == gl::ShaderType::Fragment)
    {
        mSecondaryOutputTransformer.visitTypeArray(id, elementType);
    }
}

void SpirvTransformer::visitTypePointer(const uint32_t *instruction)
{
    spirv::IdResult id;
    spv::StorageClass storageClass;
    spirv::IdRef typeId;
    spirv::ParseTypePointer(instruction, &id, &storageClass, &typeId);

    visitTypeHelper(id, typeId);
    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        mVaryingPrecisionFixer.visitTypePointer(id, storageClass, typeId);
    }
    mMultisampleTransformer.visitTypePointer(mOptions.shaderType, id, storageClass, typeId);
    if (mOptions.shaderType == gl::ShaderType::Fragment)
    {
        mSecondaryOutputTransformer.visitTypePointer(id, typeId);
    }
}

void SpirvTransformer::visitTypeStruct(const uint32_t *instruction)
{
    spirv::IdResult id;
    spirv::IdRefList memberList;
    ParseTypeStruct(instruction, &id, &memberList);

    mMultisampleTransformer.visitTypeStruct(id, memberList);
}

void SpirvTransformer::visitVariable(const uint32_t *instruction)
{
    spirv::IdResultType typeId;
    spirv::IdResult id;
    spv::StorageClass storageClass;
    spirv::ParseVariable(instruction, &typeId, &id, &storageClass, nullptr);

    // If storage class indicates that this is not a shader interface variable, ignore it.
    const bool isInterfaceBlockVariable =
        storageClass == spv::StorageClassUniform || storageClass == spv::StorageClassStorageBuffer;
    const bool isOpaqueUniform = storageClass == spv::StorageClassUniformConstant;
    const bool isInOut =
        storageClass == spv::StorageClassInput || storageClass == spv::StorageClassOutput;

    if (!isInterfaceBlockVariable && !isOpaqueUniform && !isInOut)
    {
        return;
    }

    // If no info is already associated with this id, carry that forward from the type.  This
    // happens for interface blocks, where the id->info association is done on the type id.
    ASSERT(mVariableInfoById[id] == nullptr || mVariableInfoById[typeId] == nullptr);
    if (mVariableInfoById[id] == nullptr)
    {
        mVariableInfoById[id] = mVariableInfoById[typeId];
    }

    const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];

    // If this is an interface variable but no info is associated with it, it must be a built-in.
    if (info == nullptr)
    {
        // Make all builtins point to this no-op info.  Adding this entry allows us to ASSERT that
        // every shader interface variable is processed during the SPIR-V transformation.  This is
        // done when iterating the ids provided by OpEntryPoint.
        mVariableInfoById[id] = &mBuiltinVariableInfo;
        return;
    }

    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        mVaryingPrecisionFixer.visitVariable(*info, mOptions.shaderType, typeId, id, storageClass,
                                             mSpirvBlobOut);
    }
    if (mOptions.isTransformFeedbackStage && mVariableInfoById[id]->hasTransformFeedback)
    {
        const XFBInterfaceVariableInfo &xfbInfo =
            mVariableInfoMap.getXFBDataForVariableInfo(mVariableInfoById[id]);
        mXfbCodeGenerator.visitVariable(*info, xfbInfo, mOptions.shaderType, typeId, id,
                                        storageClass);
    }

    mMultisampleTransformer.visitVariable(mOptions.shaderType, typeId, id, storageClass);
}

bool SpirvTransformer::visitExtInst(const uint32_t *instruction)
{
    sh::vk::spirv::NonSemanticInstruction inst;
    if (!mNonSemanticInstructions.visitExtInst(instruction, &inst))
    {
        return false;
    }

    switch (inst)
    {
        case sh::vk::spirv::kNonSemanticOverview:
            // SPIR-V is structured in sections (SPIR-V 1.0 Section 2.4 Logical Layout of a Module).
            // Names appear before decorations, which are followed by type+variables and finally
            // functions.  We are only interested in name and variable declarations (as well as type
            // declarations for the sake of nameless interface blocks).  Early out when the function
            // declaration section is met.
            //
            // This non-semantic instruction marks the beginning of the functions section.
            return true;
        default:
            UNREACHABLE();
    }

    return false;
}

TransformationState SpirvTransformer::transformDecorate(const uint32_t *instruction)
{
    spirv::IdRef id;
    spv::Decoration decoration;
    spirv::LiteralIntegerList decorationValues;
    spirv::ParseDecorate(instruction, &id, &decoration, &decorationValues);

    ASSERT(id < mVariableInfoById.size());
    const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];

    // If variable is not a shader interface variable that needs modification, there's nothing to
    // do.
    if (info == nullptr)
    {
        return TransformationState::Unchanged;
    }

    if (mOptions.shaderType == gl::ShaderType::Fragment)
    {
        // Handle decorations for the secondary fragment output array.
        if (mSecondaryOutputTransformer.transformDecorate(id, decoration, decorationValues,
                                                          mSpirvBlobOut) ==
            TransformationState::Transformed)
        {
            return TransformationState::Transformed;
        }
    }

    if (mOptions.removeDepthStencilInput)
    {
        if (mDepthStencilInputRemover.transformDecorate(id, decoration, decorationValues,
                                                        mSpirvBlobOut) ==
            TransformationState::Transformed)
        {
            return TransformationState::Transformed;
        }
    }

    if (mInactiveVaryingRemover.transformDecorate(*info, mOptions.shaderType, id, decoration,
                                                  decorationValues, mSpirvBlobOut) ==
        TransformationState::Transformed)
    {
        return TransformationState::Transformed;
    }

    if (mXfbCodeGenerator.transformDecorate(info, mOptions.shaderType, id, decoration,
                                            decorationValues,
                                            mSpirvBlobOut) == TransformationState::Transformed)
    {
        return TransformationState::Transformed;
    }

    // If using relaxed precision, generate instructions for the replacement id instead.
    spirv::IdRef replacementId = id;
    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        replacementId = mVaryingPrecisionFixer.getReplacementId(id);
    }

    mMultisampleTransformer.transformDecorate(mNonSemanticInstructions, *info, mOptions.shaderType,
                                              id, replacementId, decoration, mSpirvBlobOut);

    uint32_t newDecorationValue = ShaderInterfaceVariableInfo::kInvalid;

    switch (decoration)
    {
        case spv::DecorationLocation:
            newDecorationValue = info->location;
            break;
        case spv::DecorationBinding:
            newDecorationValue = info->binding;
            break;
        case spv::DecorationDescriptorSet:
            newDecorationValue = info->descriptorSet;
            break;
        case spv::DecorationFlat:
        case spv::DecorationNoPerspective:
        case spv::DecorationCentroid:
        case spv::DecorationSample:
            if (mOptions.useSpirvVaryingPrecisionFixer && info->useRelaxedPrecision)
            {
                // Change the id to replacement variable
                spirv::WriteDecorate(mSpirvBlobOut, replacementId, decoration, decorationValues);
                return TransformationState::Transformed;
            }
            break;
        case spv::DecorationBlock:
            // If this is the Block decoration of a shader I/O block, add the transform feedback
            // decorations to its members right away.
            if (mOptions.isTransformFeedbackStage && info->hasTransformFeedback)
            {
                const XFBInterfaceVariableInfo &xfbInfo =
                    mVariableInfoMap.getXFBDataForVariableInfo(info);
                mXfbCodeGenerator.addMemberDecorate(xfbInfo, id, mSpirvBlobOut);
            }
            break;
        case spv::DecorationInvariant:
            spirv::WriteDecorate(mSpirvBlobOut, replacementId, spv::DecorationInvariant, {});
            return TransformationState::Transformed;
        default:
            break;
    }

    // If the decoration is not something we care about modifying, there's nothing to do.
    if (newDecorationValue == ShaderInterfaceVariableInfo::kInvalid)
    {
        return TransformationState::Unchanged;
    }

    // Modify the decoration value.
    ASSERT(decorationValues.size() == 1);
    spirv::WriteDecorate(mSpirvBlobOut, replacementId, decoration,
                         {spirv::LiteralInteger(newDecorationValue)});

    // If there are decorations to be added, add them right after the Location decoration is
    // encountered.
    if (decoration != spv::DecorationLocation)
    {
        return TransformationState::Transformed;
    }

    // If any, the replacement variable is always reduced precision so add that decoration to
    // fixedVaryingId.
    if (mOptions.useSpirvVaryingPrecisionFixer && info->useRelaxedPrecision)
    {
        mVaryingPrecisionFixer.addDecorate(replacementId, mSpirvBlobOut);
    }

    // Add component decoration, if any.
    if (info->component != ShaderInterfaceVariableInfo::kInvalid)
    {
        spirv::WriteDecorate(mSpirvBlobOut, replacementId, spv::DecorationComponent,
                             {spirv::LiteralInteger(info->component)});
    }

    // Add index decoration, if any.
    if (info->index != ShaderInterfaceVariableInfo::kInvalid)
    {
        spirv::WriteDecorate(mSpirvBlobOut, replacementId, spv::DecorationIndex,
                             {spirv::LiteralInteger(info->index)});
    }

    // Add Xfb decorations, if any.
    if (mOptions.isTransformFeedbackStage && info->hasTransformFeedback)
    {
        const XFBInterfaceVariableInfo &xfbInfo = mVariableInfoMap.getXFBDataForVariableInfo(info);
        mXfbCodeGenerator.addDecorate(xfbInfo, replacementId, mSpirvBlobOut);
    }

    return TransformationState::Transformed;
}

TransformationState SpirvTransformer::transformMemberDecorate(const uint32_t *instruction)
{
    spirv::IdRef typeId;
    spirv::LiteralInteger member;
    spv::Decoration decoration;
    spirv::ParseMemberDecorate(instruction, &typeId, &member, &decoration, nullptr);

    if (mPerVertexTrimmer.transformMemberDecorate(typeId, member, decoration) ==
        TransformationState::Transformed)
    {
        return TransformationState::Transformed;
    }

    ASSERT(typeId < mVariableInfoById.size());
    const ShaderInterfaceVariableInfo *info = mVariableInfoById[typeId];

    return mXfbCodeGenerator.transformMemberDecorate(info, mOptions.shaderType, typeId, member,
                                                     decoration, mSpirvBlobOut);
}

TransformationState SpirvTransformer::transformCapability(const uint32_t *instruction)
{
    spv::Capability capability;
    spirv::ParseCapability(instruction, &capability);

    TransformationState xfbTransformState =
        mXfbCodeGenerator.transformCapability(capability, mSpirvBlobOut);
    ASSERT(xfbTransformState == TransformationState::Unchanged);

    TransformationState multiSampleTransformState = mMultisampleTransformer.transformCapability(
        mNonSemanticInstructions, capability, mSpirvBlobOut);
    ASSERT(multiSampleTransformState == TransformationState::Unchanged);

    return TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformName(const uint32_t *instruction)
{
    spirv::IdRef id;
    spirv::LiteralString name;
    spirv::ParseName(instruction, &id, &name);

    if (mOptions.removeDepthStencilInput)
    {
        if (mDepthStencilInputRemover.transformName(id, name) == TransformationState::Transformed)
        {
            return TransformationState::Transformed;
        }
    }

    return mXfbCodeGenerator.transformName(id, name);
}

TransformationState SpirvTransformer::transformMemberName(const uint32_t *instruction)
{
    spirv::IdRef id;
    spirv::LiteralInteger member;
    spirv::LiteralString name;
    spirv::ParseMemberName(instruction, &id, &member, &name);

    if (mXfbCodeGenerator.transformMemberName(id, member, name) == TransformationState::Transformed)
    {
        return TransformationState::Transformed;
    }

    return mPerVertexTrimmer.transformMemberName(id, member, name);
}

TransformationState SpirvTransformer::transformEntryPoint(const uint32_t *instruction)
{
    spv::ExecutionModel executionModel;
    spirv::IdRef entryPointId;
    spirv::LiteralString name;
    spirv::IdRefList interfaceList;
    spirv::ParseEntryPoint(instruction, &executionModel, &entryPointId, &name, &interfaceList);

    // Should only have one EntryPoint
    ASSERT(entryPointId == ID::EntryPoint);

    mInactiveVaryingRemover.modifyEntryPointInterfaceList(mVariableInfoById, mOptions.shaderType,
                                                          entryPointList(), &interfaceList);

    if (mOptions.shaderType == gl::ShaderType::Fragment)
    {
        mSecondaryOutputTransformer.modifyEntryPointInterfaceList(
            mVariableInfoById, entryPointList(), &interfaceList, mSpirvBlobOut);
    }

    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        mVaryingPrecisionFixer.modifyEntryPointInterfaceList(entryPointList(), &interfaceList);
    }
    if (mOptions.removeDepthStencilInput)
    {
        mDepthStencilInputRemover.modifyEntryPointInterfaceList(&interfaceList, mSpirvBlobOut);
    }

    mMultisampleTransformer.modifyEntryPointInterfaceList(
        mNonSemanticInstructions, entryPointList(), &interfaceList, mSpirvBlobOut);
    mXfbCodeGenerator.modifyEntryPointInterfaceList(mVariableInfoById, mOptions.shaderType,
                                                    entryPointList(), &interfaceList);

    // Write the entry point with the inactive interface variables removed.
    spirv::WriteEntryPoint(mSpirvBlobOut, executionModel, ID::EntryPoint, name, interfaceList);

    // Add an OpExecutionMode Xfb instruction if necessary.
    mXfbCodeGenerator.addExecutionMode(ID::EntryPoint, mSpirvBlobOut);

    return TransformationState::Transformed;
}

TransformationState SpirvTransformer::transformTypePointer(const uint32_t *instruction)
{
    spirv::IdResult id;
    spv::StorageClass storageClass;
    spirv::IdRef typeId;
    spirv::ParseTypePointer(instruction, &id, &storageClass, &typeId);

    if (mInactiveVaryingRemover.transformTypePointer(id, storageClass, typeId, mSpirvBlobOut) ==
        TransformationState::Transformed)
    {
        return TransformationState::Transformed;
    }

    ASSERT(id < mVariableInfoById.size());
    const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];

    return mXfbCodeGenerator.transformTypePointer(info, mOptions.shaderType, id, storageClass,
                                                  typeId, mSpirvBlobOut);
}

TransformationState SpirvTransformer::transformExtension(const uint32_t *instruction)
{
    // Drop the OpExtension "SPV_KHR_non_semantic_info" extension instruction.
    // SPV_KHR_non_semantic_info is used purely as a means of communication between the compiler and
    // the SPIR-V transformer, and is stripped away before the SPIR-V is handed off to the driver.
    spirv::LiteralString name;
    spirv::ParseExtension(instruction, &name);

    return strcmp(name, "SPV_KHR_non_semantic_info") == 0 ? TransformationState::Transformed
                                                          : TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformExtInstImport(const uint32_t *instruction)
{
    // Drop the OpExtInstImport "NonSemantic.ANGLE" instruction.
    spirv::IdResult id;
    spirv::LiteralString name;
    ParseExtInstImport(instruction, &id, &name);

    return id == sh::vk::spirv::kIdNonSemanticInstructionSet ? TransformationState::Transformed
                                                             : TransformationState::Unchanged;
}

TransformationState SpirvTransformer::transformExtInst(const uint32_t *instruction)
{
    sh::vk::spirv::NonSemanticInstruction inst;
    if (!mNonSemanticInstructions.visitExtInst(instruction, &inst))
    {
        return TransformationState::Unchanged;
    }

    switch (inst)
    {
        case sh::vk::spirv::kNonSemanticOverview:
            // Declare anything that we need but didn't find there already.
            writePendingDeclarations();
            break;
        case sh::vk::spirv::kNonSemanticEnter:
            // If there are any precision mismatches that need to be handled, temporary global
            // variables are created with the original precision.  Initialize those variables from
            // the varyings at the beginning of the shader.
            writeInputPreamble();
            break;
        case sh::vk::spirv::kNonSemanticOutput:
            // Generate gl_Position transformations and transform feedback capture (through
            // extension) before return or EmitVertex().  Additionally, if there are any precision
            // mismatches that need to be ahendled, write the temporary variables that hold varyings
            // data. Copy a secondary fragment output value if it was declared as an array.
            writeOutputPrologue();
            break;
        case sh::vk::spirv::kNonSemanticTransformFeedbackEmulation:
            // Transform feedback emulation is written to a designated function.  Allow its code to
            // be generated if this is the right function.
            if (mOptions.isTransformFeedbackStage)
            {
                mXfbCodeGenerator.writeTransformFeedbackEmulationOutput(
                    mInactiveVaryingRemover, mVaryingPrecisionFixer,
                    mOptions.useSpirvVaryingPrecisionFixer, mSpirvBlobOut);
            }
            break;
        default:
            UNREACHABLE();
            break;
    }

    // Drop the instruction if this is the last pass
    return mNonSemanticInstructions.transformExtInst(instruction);
}

TransformationState SpirvTransformer::transformLoad(const uint32_t *instruction)
{
    if (!mOptions.removeDepthStencilInput)
    {
        return TransformationState::Unchanged;
    }

    spirv::IdResultType typeId;
    spirv::IdResult id;
    spirv::IdRef pointerId;
    ParseLoad(instruction, &typeId, &id, &pointerId, nullptr);

    return mDepthStencilInputRemover.transformLoad(typeId, id, pointerId, mSpirvBlobOut);
}

TransformationState SpirvTransformer::transformTypeStruct(const uint32_t *instruction)
{
    spirv::IdResult id;
    spirv::IdRefList memberList;
    ParseTypeStruct(instruction, &id, &memberList);

    if (mPerVertexTrimmer.transformTypeStruct(id, &memberList, mSpirvBlobOut) ==
        TransformationState::Transformed)
    {
        return TransformationState::Transformed;
    }

    ASSERT(id < mVariableInfoById.size());
    const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];

    return mXfbCodeGenerator.transformTypeStruct(info, mOptions.shaderType, id, memberList,
                                                 mSpirvBlobOut);
}

TransformationState SpirvTransformer::transformVariable(const uint32_t *instruction)
{
    spirv::IdResultType typeId;
    spirv::IdResult id;
    spv::StorageClass storageClass;
    spirv::ParseVariable(instruction, &typeId, &id, &storageClass, nullptr);

    const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];

    // If variable is not a shader interface variable that needs modification, there's nothing to
    // do.
    if (info == nullptr)
    {
        return TransformationState::Unchanged;
    }

    if (mOptions.shaderType == gl::ShaderType::Fragment && storageClass == spv::StorageClassOutput)
    {
        // If present, make the secondary fragment output array
        // private and declare a non-array output instead.
        if (mSecondaryOutputTransformer.transformVariable(
                typeId, mInactiveVaryingRemover.getTransformedPrivateType(typeId), id,
                mSpirvBlobOut) == TransformationState::Transformed)
        {
            return TransformationState::Transformed;
        }
    }

    if (mOptions.removeDepthStencilInput)
    {
        if (mDepthStencilInputRemover.transformVariable(typeId, id, mSpirvBlobOut) ==
            TransformationState::Transformed)
        {
            return TransformationState::Transformed;
        }
    }

    // Furthermore, if it's not an inactive varying output, there's nothing to do.  Note that
    // inactive varying inputs are already pruned by the translator.
    // However, input or output storage class for interface block will not be pruned when a shader
    // is compiled separately.
    if (info->activeStages[mOptions.shaderType])
    {
        if (mOptions.useSpirvVaryingPrecisionFixer &&
            mVaryingPrecisionFixer.transformVariable(
                *info, typeId, id, storageClass, mSpirvBlobOut) == TransformationState::Transformed)
        {
            // Make original variable a private global
            return mInactiveVaryingRemover.transformVariable(typeId, id, storageClass,
                                                             mSpirvBlobOut);
        }
        return TransformationState::Unchanged;
    }

    if (mXfbCodeGenerator.transformVariable(*info, mVariableInfoMap, mOptions.shaderType,
                                            storageBufferStorageClass(), typeId, id,
                                            storageClass) == TransformationState::Transformed)
    {
        return TransformationState::Transformed;
    }

    // The variable is inactive.  Output a modified variable declaration, where the type is the
    // corresponding type with the Private storage class.
    return mInactiveVaryingRemover.transformVariable(typeId, id, storageClass, mSpirvBlobOut);
}

TransformationState SpirvTransformer::transformTypeImage(const uint32_t *instruction)
{
    return mMultisampleTransformer.transformTypeImage(instruction, mSpirvBlobOut);
}

TransformationState SpirvTransformer::transformImageRead(const uint32_t *instruction)
{
    if (mOptions.removeDepthStencilInput)
    {
        if (mDepthStencilInputRemover.transformImageRead(instruction, mSpirvBlobOut) ==
            TransformationState::Transformed)
        {
            return TransformationState::Transformed;
        }
    }
    return mMultisampleTransformer.transformImageRead(instruction, mSpirvBlobOut);
}

TransformationState SpirvTransformer::transformAccessChain(const uint32_t *instruction)
{
    spirv::IdResultType typeId;
    spirv::IdResult id;
    spirv::IdRef baseId;
    spirv::IdRefList indexList;
    spirv::ParseAccessChain(instruction, &typeId, &id, &baseId, &indexList);

    // If not accessing an inactive output varying, nothing to do.
    const ShaderInterfaceVariableInfo *info = mVariableInfoById[baseId];
    if (info == nullptr)
    {
        return TransformationState::Unchanged;
    }

    if (mOptions.shaderType == gl::ShaderType::Fragment)
    {
        // Update the type used for accessing the secondary fragment output array.
        if (mSecondaryOutputTransformer.transformAccessChain(
                mInactiveVaryingRemover.getTransformedPrivateType(typeId), id, baseId, indexList,
                mSpirvBlobOut) == TransformationState::Transformed)
        {
            return TransformationState::Transformed;
        }
    }

    if (mOptions.useSpirvVaryingPrecisionFixer)
    {
        if (info->activeStages[mOptions.shaderType] && !info->useRelaxedPrecision)
        {
            return TransformationState::Unchanged;
        }
    }
    else
    {
        if (info->activeStages[mOptions.shaderType])
        {
            return TransformationState::Unchanged;
        }
    }

    return mInactiveVaryingRemover.transformAccessChain(typeId, id, baseId, indexList,
                                                        mSpirvBlobOut);
}

struct AliasingAttributeMap
{
    // The SPIR-V id of the aliasing attribute with the most components.  This attribute will be
    // used to read from this location instead of every aliasing one.
    spirv::IdRef attribute;

    // SPIR-V ids of aliasing attributes.
    std::vector<spirv::IdRef> aliasingAttributes;
};

void ValidateShaderInterfaceVariableIsAttribute(const ShaderInterfaceVariableInfo *info)
{
    ASSERT(info);
    ASSERT(info->activeStages[gl::ShaderType::Vertex]);
    ASSERT(info->attributeComponentCount > 0);
    ASSERT(info->attributeLocationCount > 0);
    ASSERT(info->location != ShaderInterfaceVariableInfo::kInvalid);
}

void ValidateIsAliasingAttribute(const AliasingAttributeMap *aliasingMap, uint32_t id)
{
    ASSERT(id != aliasingMap->attribute);
    ASSERT(std::find(aliasingMap->aliasingAttributes.begin(), aliasingMap->aliasingAttributes.end(),
                     id) != aliasingMap->aliasingAttributes.end());
}

// A transformation that resolves vertex attribute aliases.  Note that vertex attribute aliasing is
// only allowed in GLSL ES 100, where the attribute types can only be one of float, vec2, vec3,
// vec4, mat2, mat3, and mat4.  Matrix attributes are handled by expanding them to multiple vector
// attributes, each occupying one location.
class SpirvVertexAttributeAliasingTransformer final : public SpirvTransformerBase
{
  public:
    SpirvVertexAttributeAliasingTransformer(
        const spirv::Blob &spirvBlobIn,
        const ShaderInterfaceVariableInfoMap &variableInfoMap,
        std::vector<const ShaderInterfaceVariableInfo *> &&variableInfoById,
        spirv::Blob *spirvBlobOut)
        : SpirvTransformerBase(spirvBlobIn, variableInfoMap, spirvBlobOut),
          mNonSemanticInstructions(true)
    {
        mVariableInfoById = std::move(variableInfoById);
    }

    void transform();

  private:
    // Preprocess aliasing attributes in preparation for their removal.
    void preprocessAliasingAttributes();

    // Transform instructions:
    void transformInstruction();

    // Helpers:
    spirv::IdRef getAliasingAttributeReplacementId(spirv::IdRef aliasingId, uint32_t offset) const;
    bool isMatrixAttribute(spirv::IdRef id) const;

    // Instructions that are purely informational:
    void visitTypePointer(const uint32_t *instruction);

    // Instructions that potentially need transformation.  They return true if the instruction is
    // transformed.  If false is returned, the instruction should be copied as-is.
    TransformationState transformEntryPoint(const uint32_t *instruction);
    TransformationState transformExtInst(const uint32_t *instruction);
    TransformationState transformName(const uint32_t *instruction);
    TransformationState transformDecorate(const uint32_t *instruction);
    TransformationState transformVariable(const uint32_t *instruction);
    TransformationState transformAccessChain(const uint32_t *instruction);
    void transformLoadHelper(spirv::IdRef pointerId,
                             spirv::IdRef typeId,
                             spirv::IdRef replacementId,
                             spirv::IdRef resultId);
    TransformationState transformLoad(const uint32_t *instruction);

    void declareExpandedMatrixVectors();
    void writeExpandedMatrixInitialization();

    // Transformation state:

    // Map of aliasing attributes per location.
    gl::AttribArray<AliasingAttributeMap> mAliasingAttributeMap;

    // For each id, this map indicates whether it refers to an aliasing attribute that needs to be
    // removed.
    std::vector<bool> mIsAliasingAttributeById;

    // Matrix attributes are split into vectors, each occupying one location.  The SPIR-V
    // declaration would need to change from:
    //
    //     %type = OpTypeMatrix %vectorType N
    //     %matrixType = OpTypePointer Input %type
    //     %matrix = OpVariable %matrixType Input
    //
    // to:
    //
    //     %matrixType = OpTypePointer Private %type
    //     %matrix = OpVariable %matrixType Private
    //
    //     %vecType = OpTypePointer Input %vectorType
    //
    //     %vec0 = OpVariable %vecType Input
    //     ...
    //     %vecN-1 = OpVariable %vecType Input
    //
    // For each id %matrix (which corresponds to a matrix attribute), this map contains %vec0.  The
    // ids of the split vectors are consecutive, so %veci == %vec0 + i.  %veciType is taken from
    // mInputTypePointers.
    std::vector<spirv::IdRef> mExpandedMatrixFirstVectorIdById;

    // Id of attribute types; float and veci.
    spirv::IdRef floatType(uint32_t componentCount)
    {
        static_assert(sh::vk::spirv::kIdVec2 == sh::vk::spirv::kIdFloat + 1);
        static_assert(sh::vk::spirv::kIdVec3 == sh::vk::spirv::kIdFloat + 2);
        static_assert(sh::vk::spirv::kIdVec4 == sh::vk::spirv::kIdFloat + 3);
        ASSERT(componentCount <= 4);
        return spirv::IdRef(sh::vk::spirv::kIdFloat + (componentCount - 1));
    }

    // Id of matrix attribute types.  Note that only square matrices are possible as attributes in
    // GLSL ES 1.00.
    spirv::IdRef matrixType(uint32_t dimension)
    {
        static_assert(sh::vk::spirv::kIdMat3 == sh::vk::spirv::kIdMat2 + 1);
        static_assert(sh::vk::spirv::kIdMat4 == sh::vk::spirv::kIdMat2 + 2);
        ASSERT(dimension >= 2 && dimension <= 4);
        return spirv::IdRef(sh::vk::spirv::kIdMat2 + (dimension - 2));
    }

    // Corresponding to floatType(), [i]: id of OpTypePointer Input %floatType(i).  [0] is unused.
    std::array<spirv::IdRef, 5> mInputTypePointers;

    // Corresponding to floatType(), [i]: id of OpTypePointer Private %floatType(i).  [0] is
    // unused.
    std::array<spirv::IdRef, 5> mPrivateFloatTypePointers;

    // Corresponding to matrixType(), [i]: id of OpTypePointer Private %matrixType(i).  [0] and
    // [1] are unused.
    std::array<spirv::IdRef, 5> mPrivateMatrixTypePointers;

    SpirvNonSemanticInstructions mNonSemanticInstructions;
};

void SpirvVertexAttributeAliasingTransformer::transform()
{
    onTransformBegin();

    preprocessAliasingAttributes();

    while (mCurrentWord < mSpirvBlobIn.size())
    {
        transformInstruction();
    }
}

void SpirvVertexAttributeAliasingTransformer::preprocessAliasingAttributes()
{
    const uint32_t indexBound = mSpirvBlobIn[spirv::kHeaderIndexIndexBound];

    mVariableInfoById.resize(indexBound, nullptr);
    mIsAliasingAttributeById.resize(indexBound, false);
    mExpandedMatrixFirstVectorIdById.resize(indexBound);

    // Go through attributes and find out which alias which.
    for (uint32_t idIndex = spirv::kMinValidId; idIndex < indexBound; ++idIndex)
    {
        const spirv::IdRef id(idIndex);

        const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];

        // Ignore non attribute ids.
        if (info == nullptr || info->attributeComponentCount == 0)
        {
            continue;
        }

        ASSERT(info->activeStages[gl::ShaderType::Vertex]);
        ASSERT(info->location != ShaderInterfaceVariableInfo::kInvalid);

        const bool isMatrixAttribute = info->attributeLocationCount > 1;

        for (uint32_t offset = 0; offset < info->attributeLocationCount; ++offset)
        {
            uint32_t location = info->location + offset;
            ASSERT(location < mAliasingAttributeMap.size());

            spirv::IdRef attributeId(id);

            // If this is a matrix attribute, expand it to vectors.
            if (isMatrixAttribute)
            {
                const spirv::IdRef matrixId(id);

                // Get a new id for this location and associate it with the matrix.
                attributeId = getNewId();
                if (offset == 0)
                {
                    mExpandedMatrixFirstVectorIdById[matrixId] = attributeId;
                }
                // The ids are consecutive.
                ASSERT(attributeId == mExpandedMatrixFirstVectorIdById[matrixId] + offset);

                mIsAliasingAttributeById.resize(attributeId + 1, false);
                mVariableInfoById.resize(attributeId + 1, nullptr);
                mVariableInfoById[attributeId] = info;
            }

            AliasingAttributeMap *aliasingMap = &mAliasingAttributeMap[location];

            // If this is the first attribute in this location, remember it.
            if (!aliasingMap->attribute.valid())
            {
                aliasingMap->attribute = attributeId;
                continue;
            }

            // Otherwise, either add it to the list of aliasing attributes, or replace the main
            // attribute (and add that to the list of aliasing attributes).  The one with the
            // largest number of components is used as the main attribute.
            const ShaderInterfaceVariableInfo *curMainAttribute =
                mVariableInfoById[aliasingMap->attribute];
            ASSERT(curMainAttribute != nullptr && curMainAttribute->attributeComponentCount > 0);

            spirv::IdRef aliasingId;
            if (info->attributeComponentCount > curMainAttribute->attributeComponentCount)
            {
                aliasingId             = aliasingMap->attribute;
                aliasingMap->attribute = attributeId;
            }
            else
            {
                aliasingId = attributeId;
            }

            aliasingMap->aliasingAttributes.push_back(aliasingId);
            ASSERT(!mIsAliasingAttributeById[aliasingId]);
            mIsAliasingAttributeById[aliasingId] = true;
        }
    }
}

void SpirvVertexAttributeAliasingTransformer::transformInstruction()
{
    uint32_t wordCount;
    spv::Op opCode;
    const uint32_t *instruction = getCurrentInstruction(&opCode, &wordCount);

    if (opCode == spv::OpFunction)
    {
        // SPIR-V is structured in sections.  Function declarations come last.
        mIsInFunctionSection = true;
    }

    // Only look at interesting instructions.
    TransformationState transformationState = TransformationState::Unchanged;

    if (mIsInFunctionSection)
    {
        // Look at in-function opcodes.
        switch (opCode)
        {
            case spv::OpExtInst:
                transformationState = transformExtInst(instruction);
                break;
            case spv::OpAccessChain:
            case spv::OpInBoundsAccessChain:
                transformationState = transformAccessChain(instruction);
                break;
            case spv::OpLoad:
                transformationState = transformLoad(instruction);
                break;
            default:
                break;
        }
    }
    else
    {
        // Look at global declaration opcodes.
        switch (opCode)
        {
            // Informational instructions:
            case spv::OpTypePointer:
                visitTypePointer(instruction);
                break;
            // Instructions that may need transformation:
            case spv::OpEntryPoint:
                transformationState = transformEntryPoint(instruction);
                break;
            case spv::OpExtInst:
                transformationState = transformExtInst(instruction);
                break;
            case spv::OpName:
                transformationState = transformName(instruction);
                break;
            case spv::OpDecorate:
                transformationState = transformDecorate(instruction);
                break;
            case spv::OpVariable:
                transformationState = transformVariable(instruction);
                break;
            default:
                break;
        }
    }

    // If the instruction was not transformed, copy it to output as is.
    if (transformationState == TransformationState::Unchanged)
    {
        copyInstruction(instruction, wordCount);
    }

    // Advance to next instruction.
    mCurrentWord += wordCount;
}

spirv::IdRef SpirvVertexAttributeAliasingTransformer::getAliasingAttributeReplacementId(
    spirv::IdRef aliasingId,
    uint32_t offset) const
{
    // Get variable info corresponding to the aliasing attribute.
    const ShaderInterfaceVariableInfo *aliasingInfo = mVariableInfoById[aliasingId];
    ValidateShaderInterfaceVariableIsAttribute(aliasingInfo);

    // Find the replacement attribute.
    const AliasingAttributeMap *aliasingMap =
        &mAliasingAttributeMap[aliasingInfo->location + offset];
    ValidateIsAliasingAttribute(aliasingMap, aliasingId);

    const spirv::IdRef replacementId(aliasingMap->attribute);
    ASSERT(replacementId.valid() && replacementId < mIsAliasingAttributeById.size());
    ASSERT(!mIsAliasingAttributeById[replacementId]);

    return replacementId;
}

bool SpirvVertexAttributeAliasingTransformer::isMatrixAttribute(spirv::IdRef id) const
{
    return mExpandedMatrixFirstVectorIdById[id].valid();
}

void SpirvVertexAttributeAliasingTransformer::visitTypePointer(const uint32_t *instruction)
{
    spirv::IdResult id;
    spv::StorageClass storageClass;
    spirv::IdRef typeId;
    spirv::ParseTypePointer(instruction, &id, &storageClass, &typeId);

    // Only interested in OpTypePointer Input %vecN, where %vecN is the id of OpTypeVector %f32 N,
    // as well as OpTypePointer Private %matN, where %matN is the id of OpTypeMatrix %vecN N.
    // This is only for matN types (as allowed by GLSL ES 1.00), so N >= 2.
    if (storageClass == spv::StorageClassInput)
    {
        for (uint32_t n = 2; n <= 4; ++n)
        {
            if (typeId == floatType(n))
            {
                ASSERT(!mInputTypePointers[n].valid());
                mInputTypePointers[n] = id;
                break;
            }
        }
    }
    else if (storageClass == spv::StorageClassPrivate)
    {
        for (uint32_t n = 2; n <= 4; ++n)
        {
            // Note that Private types may not be unique, as the previous transformation can
            // generate duplicates.
            if (typeId == floatType(n))
            {
                mPrivateFloatTypePointers[n] = id;
                break;
            }
            if (typeId == matrixType(n))
            {
                mPrivateMatrixTypePointers[n] = id;
                break;
            }
        }
    }
}

TransformationState SpirvVertexAttributeAliasingTransformer::transformEntryPoint(
    const uint32_t *instruction)
{
    // Remove aliasing attributes from the shader interface declaration.
    spv::ExecutionModel executionModel;
    spirv::IdRef entryPointId;
    spirv::LiteralString name;
    spirv::IdRefList interfaceList;
    spirv::ParseEntryPoint(instruction, &executionModel, &entryPointId, &name, &interfaceList);

    // Should only have one EntryPoint
    ASSERT(entryPointId == ID::EntryPoint);

    // As a first pass, filter out matrix attributes and append their replacement vectors.
    size_t originalInterfaceListSize = interfaceList.size();
    for (size_t index = 0; index < originalInterfaceListSize; ++index)
    {
        const spirv::IdRef matrixId(interfaceList[index]);

        if (!mExpandedMatrixFirstVectorIdById[matrixId].valid())
        {
            continue;
        }

        const ShaderInterfaceVariableInfo *info = mVariableInfoById[matrixId];
        ValidateShaderInterfaceVariableIsAttribute(info);

        // Replace the matrix id with its first vector id.
        const spirv::IdRef vec0Id(mExpandedMatrixFirstVectorIdById[matrixId]);
        interfaceList[index] = vec0Id;

        // Append the rest of the vectors to the entry point.
        for (uint32_t offset = 1; offset < info->attributeLocationCount; ++offset)
        {
            const spirv::IdRef vecId(vec0Id + offset);
            interfaceList.push_back(vecId);
        }

        // With SPIR-V 1.4, keep the Private variable in the interface list.
        if (entryPointList() == EntryPointList::GlobalVariables)
        {
            interfaceList.push_back(matrixId);
        }
    }

    // Filter out aliasing attributes from entry point interface declaration.
    size_t writeIndex = 0;
    for (size_t index = 0; index < interfaceList.size(); ++index)
    {
        const spirv::IdRef id(interfaceList[index]);

        // If this is an attribute that's aliasing another one in the same location, remove it.
        if (mIsAliasingAttributeById[id])
        {
            const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];
            ValidateShaderInterfaceVariableIsAttribute(info);

            // The following assertion is only valid for non-matrix attributes.
            if (info->attributeLocationCount == 1)
            {
                const AliasingAttributeMap *aliasingMap = &mAliasingAttributeMap[info->location];
                ValidateIsAliasingAttribute(aliasingMap, id);
            }

            continue;
        }

        interfaceList[writeIndex] = id;
        ++writeIndex;
    }

    // Update the number of interface variables.
    interfaceList.resize_down(writeIndex);

    // Write the entry point with the aliasing attributes removed.
    spirv::WriteEntryPoint(mSpirvBlobOut, executionModel, ID::EntryPoint, name, interfaceList);

    return TransformationState::Transformed;
}

TransformationState SpirvVertexAttributeAliasingTransformer::transformExtInst(
    const uint32_t *instruction)
{
    sh::vk::spirv::NonSemanticInstruction inst;
    if (!mNonSemanticInstructions.visitExtInst(instruction, &inst))
    {
        return TransformationState::Unchanged;
    }

    switch (inst)
    {
        case sh::vk::spirv::kNonSemanticOverview:
            // Declare the expanded matrix variables
            declareExpandedMatrixVectors();
            break;
        case sh::vk::spirv::kNonSemanticEnter:
            // The matrix attribute declarations have been changed to have Private storage class,
            // and they are initialized from the expanded (and potentially aliased) Input vectors.
            // This is done at the beginning of the entry point.
            writeExpandedMatrixInitialization();
            break;
        case sh::vk::spirv::kNonSemanticOutput:
        case sh::vk::spirv::kNonSemanticTransformFeedbackEmulation:
            // Unused by this transformation
            break;
        default:
            UNREACHABLE();
            break;
    }

    // Drop the instruction if this is the last pass
    return mNonSemanticInstructions.transformExtInst(instruction);
}

TransformationState SpirvVertexAttributeAliasingTransformer::transformName(
    const uint32_t *instruction)
{
    spirv::IdRef id;
    spirv::LiteralString name;
    spirv::ParseName(instruction, &id, &name);

    // If id is not that of an aliasing attribute, there's nothing to do.
    ASSERT(id < mIsAliasingAttributeById.size());
    if (!mIsAliasingAttributeById[id])
    {
        return TransformationState::Unchanged;
    }

    // Drop debug annotations for this id.
    return TransformationState::Transformed;
}

TransformationState SpirvVertexAttributeAliasingTransformer::transformDecorate(
    const uint32_t *instruction)
{
    spirv::IdRef id;
    spv::Decoration decoration;
    spirv::ParseDecorate(instruction, &id, &decoration, nullptr);

    if (isMatrixAttribute(id))
    {
        // If it's a matrix attribute, it's expanded to multiple vectors.  Insert the Location
        // decorations for these vectors here.

        // Keep all decorations except for Location.
        if (decoration != spv::DecorationLocation)
        {
            return TransformationState::Unchanged;
        }

        const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];
        ValidateShaderInterfaceVariableIsAttribute(info);

        const spirv::IdRef vec0Id(mExpandedMatrixFirstVectorIdById[id]);
        ASSERT(vec0Id.valid());

        for (uint32_t offset = 0; offset < info->attributeLocationCount; ++offset)
        {
            const spirv::IdRef vecId(vec0Id + offset);
            if (mIsAliasingAttributeById[vecId])
            {
                continue;
            }

            spirv::WriteDecorate(mSpirvBlobOut, vecId, decoration,
                                 {spirv::LiteralInteger(info->location + offset)});
        }
    }
    else
    {
        // If id is not that of an active attribute, there's nothing to do.
        const ShaderInterfaceVariableInfo *info = mVariableInfoById[id];
        if (info == nullptr || info->attributeComponentCount == 0 ||
            !info->activeStages[gl::ShaderType::Vertex])
        {
            return TransformationState::Unchanged;
        }

        // Always drop RelaxedPrecision from input attributes.  The temporary variable the attribute
        // is loaded into has RelaxedPrecision and will implicitly convert.
        if (decoration == spv::DecorationRelaxedPrecision)
        {
            return TransformationState::Transformed;
        }

        // If id is not that of an aliasing attribute, there's nothing else to do.
        ASSERT(id < mIsAliasingAttributeById.size());
        if (!mIsAliasingAttributeById[id])
        {
            return TransformationState::Unchanged;
        }
    }

    // Drop every decoration for this id.
    return TransformationState::Transformed;
}

TransformationState SpirvVertexAttributeAliasingTransformer::transformVariable(
    const uint32_t *instruction)
{
    spirv::IdResultType typeId;
    spirv::IdResult id;
    spv::StorageClass storageClass;
    spirv::ParseVariable(instruction, &typeId, &id, &storageClass, nullptr);

    if (!isMatrixAttribute(id))
    {
        // If id is not that of an aliasing attribute, there's nothing to do.  Note that matrix
        // declarations are always replaced.
        ASSERT(id < mIsAliasingAttributeById.size());
        if (!mIsAliasingAttributeById[id])
        {
            return TransformationState::Unchanged;
        }
    }

    ASSERT(storageClass == spv::StorageClassInput);

    // Drop the declaration.
    return TransformationState::Transformed;
}

TransformationState SpirvVertexAttributeAliasingTransformer::transformAccessChain(
    const uint32_t *instruction)
{
    spirv::IdResultType typeId;
    spirv::IdResult id;
    spirv::IdRef baseId;
    spirv::IdRefList indexList;
    spirv::ParseAccessChain(instruction, &typeId, &id, &baseId, &indexList);

    if (isMatrixAttribute(baseId))
    {
        // Write a modified OpAccessChain instruction.  Only modification is that the %type is
        // replaced with the Private version of it.  If there is one %index, that would be a vector
        // type, and if there are two %index'es, it's a float type.
        spirv::IdRef replacementTypeId;

        if (indexList.size() == 1)
        {
            // If indexed once, it uses a vector type.
            const ShaderInterfaceVariableInfo *info = mVariableInfoById[baseId];
            ValidateShaderInterfaceVariableIsAttribute(info);

            const uint32_t componentCount = info->attributeComponentCount;

            // %type must have been the Input vector type with the matrice's component size.
            ASSERT(typeId == mInputTypePointers[componentCount]);

            // Replace the type with the corresponding Private one.
            replacementTypeId = mPrivateFloatTypePointers[componentCount];
        }
        else
        {
            // If indexed twice, it uses the float type.
            ASSERT(indexList.size() == 2);

            // Replace the type with the Private pointer to float32.
            replacementTypeId = mPrivateFloatTypePointers[1];
        }

        spirv::WriteAccessChain(mSpirvBlobOut, replacementTypeId, id, baseId, indexList);
    }
    else
    {
        // If base id is not that of an aliasing attribute, there's nothing to do.
        ASSERT(baseId < mIsAliasingAttributeById.size());
        if (!mIsAliasingAttributeById[baseId])
        {
            return TransformationState::Unchanged;
        }

        // Find the replacement attribute for the aliasing one.
        const spirv::IdRef replacementId(getAliasingAttributeReplacementId(baseId, 0));

        // Get variable info corresponding to the replacement attribute.
        const ShaderInterfaceVariableInfo *replacementInfo = mVariableInfoById[replacementId];
        ValidateShaderInterfaceVariableIsAttribute(replacementInfo);

        // Write a modified OpAccessChain instruction.  Currently, the instruction is:
        //
        //     %id = OpAccessChain %type %base %index
        //
        // This is modified to:
        //
        //     %id = OpAccessChain %type %replacement %index
        //
        // Note that the replacement has at least as many components as the aliasing attribute,
        // and both attributes start at component 0 (GLSL ES restriction).  So, indexing the
        // replacement attribute with the same index yields the same result and type.
        spirv::WriteAccessChain(mSpirvBlobOut, typeId, id, replacementId, indexList);
    }

    return TransformationState::Transformed;
}

void SpirvVertexAttributeAliasingTransformer::transformLoadHelper(spirv::IdRef pointerId,
                                                                  spirv::IdRef typeId,
                                                                  spirv::IdRef replacementId,
                                                                  spirv::IdRef resultId)
{
    // Get variable info corresponding to the replacement attribute.
    const ShaderInterfaceVariableInfo *replacementInfo = mVariableInfoById[replacementId];
    ValidateShaderInterfaceVariableIsAttribute(replacementInfo);

    // Currently, the instruction is:
    //
    //     %id = OpLoad %type %pointer
    //
    // This is modified to:
    //
    //     %newId = OpLoad %replacementType %replacement
    //
    const spirv::IdRef loadResultId(getNewId());
    const spirv::IdRef replacementTypeId(floatType(replacementInfo->attributeComponentCount));
    ASSERT(replacementTypeId.valid());

    spirv::WriteLoad(mSpirvBlobOut, replacementTypeId, loadResultId, replacementId, nullptr);

    // If swizzle is not necessary, assign %newId to %resultId.
    const ShaderInterfaceVariableInfo *aliasingInfo = mVariableInfoById[pointerId];
    if (aliasingInfo->attributeComponentCount == replacementInfo->attributeComponentCount)
    {
        spirv::WriteCopyObject(mSpirvBlobOut, typeId, resultId, loadResultId);
        return;
    }

    // Take as many components from the replacement as the aliasing attribute wanted.  This is done
    // by either of the following instructions:
    //
    // - If aliasing attribute has only one component:
    //
    //     %resultId = OpCompositeExtract %floatType %newId 0
    //
    // - If aliasing attribute has more than one component:
    //
    //     %resultId = OpVectorShuffle %vecType %newId %newId 0 1 ...
    //
    ASSERT(aliasingInfo->attributeComponentCount < replacementInfo->attributeComponentCount);
    ASSERT(floatType(aliasingInfo->attributeComponentCount) == typeId);

    if (aliasingInfo->attributeComponentCount == 1)
    {
        spirv::WriteCompositeExtract(mSpirvBlobOut, typeId, resultId, loadResultId,
                                     {spirv::LiteralInteger(0)});
    }
    else
    {
        spirv::LiteralIntegerList swizzle = {spirv::LiteralInteger(0), spirv::LiteralInteger(1),
                                             spirv::LiteralInteger(2), spirv::LiteralInteger(3)};
        swizzle.resize_down(aliasingInfo->attributeComponentCount);

        spirv::WriteVectorShuffle(mSpirvBlobOut, typeId, resultId, loadResultId, loadResultId,
                                  swizzle);
    }
}

TransformationState SpirvVertexAttributeAliasingTransformer::transformLoad(
    const uint32_t *instruction)
{
    spirv::IdResultType typeId;
    spirv::IdResult id;
    spirv::IdRef pointerId;
    ParseLoad(instruction, &typeId, &id, &pointerId, nullptr);

    // Currently, the instruction is:
    //
    //     %id = OpLoad %type %pointer
    //
    // If non-matrix, this is modifed to load from the aliasing vector instead if aliasing.
    //
    // If matrix, this is modified such that %type points to the Private version of it.
    //
    if (isMatrixAttribute(pointerId))
    {
        const ShaderInterfaceVariableInfo *info = mVariableInfoById[pointerId];
        ValidateShaderInterfaceVariableIsAttribute(info);

        const spirv::IdRef replacementTypeId(matrixType(info->attributeLocationCount));

        spirv::WriteLoad(mSpirvBlobOut, replacementTypeId, id, pointerId, nullptr);
    }
    else
    {
        // If pointer id is not that of an aliasing attribute, there's nothing to do.
        ASSERT(pointerId < mIsAliasingAttributeById.size());
        if (!mIsAliasingAttributeById[pointerId])
        {
            return TransformationState::Unchanged;
        }

        // Find the replacement attribute for the aliasing one.
        const spirv::IdRef replacementId(getAliasingAttributeReplacementId(pointerId, 0));

        // Replace the load instruction by a load from the replacement id.
        transformLoadHelper(pointerId, typeId, replacementId, id);
    }

    return TransformationState::Transformed;
}

void SpirvVertexAttributeAliasingTransformer::declareExpandedMatrixVectors()
{
    // Go through matrix attributes and expand them.
    for (uint32_t matrixIdIndex = spirv::kMinValidId;
         matrixIdIndex < mExpandedMatrixFirstVectorIdById.size(); ++matrixIdIndex)
    {
        const spirv::IdRef matrixId(matrixIdIndex);

        if (!mExpandedMatrixFirstVectorIdById[matrixId].valid())
        {
            continue;
        }

        const spirv::IdRef vec0Id(mExpandedMatrixFirstVectorIdById[matrixId]);

        const ShaderInterfaceVariableInfo *info = mVariableInfoById[matrixId];
        ValidateShaderInterfaceVariableIsAttribute(info);

        // Need to generate the following:
        //
        //     %privateType = OpTypePointer Private %matrixType
        //     %id = OpVariable %privateType Private
        //     %vecType = OpTypePointer %vecType Input
        //     %vec0 = OpVariable %vecType Input
        //     ...
        //     %vecN-1 = OpVariable %vecType Input
        const uint32_t componentCount = info->attributeComponentCount;
        const uint32_t locationCount  = info->attributeLocationCount;
        ASSERT(componentCount == locationCount);

        // OpTypePointer Private %matrixType
        spirv::IdRef privateType(mPrivateMatrixTypePointers[locationCount]);
        if (!privateType.valid())
        {
            privateType                               = getNewId();
            mPrivateMatrixTypePointers[locationCount] = privateType;
            spirv::WriteTypePointer(mSpirvBlobOut, privateType, spv::StorageClassPrivate,
                                    matrixType(locationCount));
        }

        // OpVariable %privateType Private
        spirv::WriteVariable(mSpirvBlobOut, privateType, matrixId, spv::StorageClassPrivate,
                             nullptr);

        // If the OpTypePointer is not declared for the vector type corresponding to each location,
        // declare it now.
        //
        //     %vecType = OpTypePointer %vecType Input
        spirv::IdRef inputType(mInputTypePointers[componentCount]);
        if (!inputType.valid())
        {
            inputType                          = getNewId();
            mInputTypePointers[componentCount] = inputType;
            spirv::WriteTypePointer(mSpirvBlobOut, inputType, spv::StorageClassInput,
                                    floatType(componentCount));
        }

        // Declare a vector for each column of the matrix.
        for (uint32_t offset = 0; offset < info->attributeLocationCount; ++offset)
        {
            const spirv::IdRef vecId(vec0Id + offset);
            if (!mIsAliasingAttributeById[vecId])
            {
                spirv::WriteVariable(mSpirvBlobOut, inputType, vecId, spv::StorageClassInput,
                                     nullptr);
            }
        }
    }

    // Additionally, declare OpTypePointer Private %floatType(i) in case needed (used in
    // Op*AccessChain instructions, if any).
    for (uint32_t n = 1; n <= 4; ++n)
    {
        if (!mPrivateFloatTypePointers[n].valid())
        {
            const spirv::IdRef privateType(getNewId());
            mPrivateFloatTypePointers[n] = privateType;
            spirv::WriteTypePointer(mSpirvBlobOut, privateType, spv::StorageClassPrivate,
                                    floatType(n));
        }
    }
}

void SpirvVertexAttributeAliasingTransformer::writeExpandedMatrixInitialization()
{
    // Go through matrix attributes and initialize them.  Note that their declaration is replaced
    // with a Private storage class, but otherwise has the same id.
    for (uint32_t matrixIdIndex = spirv::kMinValidId;
         matrixIdIndex < mExpandedMatrixFirstVectorIdById.size(); ++matrixIdIndex)
    {
        const spirv::IdRef matrixId(matrixIdIndex);

        if (!mExpandedMatrixFirstVectorIdById[matrixId].valid())
        {
            continue;
        }

        const spirv::IdRef vec0Id(mExpandedMatrixFirstVectorIdById[matrixId]);

        // For every matrix, need to generate the following:
        //
        //     %vec0Id = OpLoad %vecType %vec0Pointer
        //     ...
        //     %vecN-1Id = OpLoad %vecType %vecN-1Pointer
        //     %mat = OpCompositeConstruct %matrixType %vec0 ... %vecN-1
        //     OpStore %matrixId %mat

        const ShaderInterfaceVariableInfo *info = mVariableInfoById[matrixId];
        ValidateShaderInterfaceVariableIsAttribute(info);

        spirv::IdRefList vecLoadIds;
        const uint32_t locationCount = info->attributeLocationCount;
        for (uint32_t offset = 0; offset < locationCount; ++offset)
        {
            const spirv::IdRef vecId(vec0Id + offset);

            // Load into temporary, potentially through an aliasing vector.
            spirv::IdRef replacementId(vecId);
            ASSERT(vecId < mIsAliasingAttributeById.size());
            if (mIsAliasingAttributeById[vecId])
            {
                replacementId = getAliasingAttributeReplacementId(vecId, offset);
            }

            // Write a load instruction from the replacement id.
            vecLoadIds.push_back(getNewId());
            transformLoadHelper(matrixId, floatType(info->attributeComponentCount), replacementId,
                                vecLoadIds.back());
        }

        // Aggregate the vector loads into a matrix.
        const spirv::IdRef compositeId(getNewId());
        spirv::WriteCompositeConstruct(mSpirvBlobOut, matrixType(locationCount), compositeId,
                                       vecLoadIds);

        // Store it in the private variable.
        spirv::WriteStore(mSpirvBlobOut, matrixId, compositeId, nullptr);
    }
}
}  // anonymous namespace

SpvSourceOptions SpvCreateSourceOptions(const angle::FeaturesVk &features,
                                        uint32_t maxColorInputAttachmentCount)
{
    SpvSourceOptions options;

    options.maxColorInputAttachmentCount = maxColorInputAttachmentCount;
    options.supportsTransformFeedbackExtension =
        features.supportsTransformFeedbackExtension.enabled;
    options.supportsTransformFeedbackEmulation = features.emulateTransformFeedback.enabled;
    options.enableTransformFeedbackEmulation   = options.supportsTransformFeedbackEmulation;
    options.supportsDepthStencilInputAttachments =
        features.supportsShaderFramebufferFetchDepthStencil.enabled;

    return options;
}

uint32_t SpvGetXfbBufferBlockId(const uint32_t bufferIndex)
{
    ASSERT(bufferIndex < 4);
    static_assert(sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferBlockOne ==
                  sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferBlockZero + 1);
    static_assert(sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferBlockTwo ==
                  sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferBlockZero + 2);
    static_assert(sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferBlockThree ==
                  sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferBlockZero + 3);

    return sh::vk::spirv::ReservedIds::kIdXfbEmulationBufferBlockZero + bufferIndex;
}

void SpvAssignLocations(const SpvSourceOptions &options,
                        const gl::ProgramExecutable &programExecutable,
                        const gl::ProgramVaryingPacking &varyingPacking,
                        const gl::ShaderType transformFeedbackStage,
                        SpvProgramInterfaceInfo *programInterfaceInfo,
                        ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    const gl::ShaderBitSet shaderStages = programExecutable.getLinkedShaderStages();

    // Assign outputs to the fragment shader, if any.
    if (shaderStages[gl::ShaderType::Fragment] &&
        programExecutable.hasLinkedShaderStage(gl::ShaderType::Fragment))
    {
        AssignOutputLocations(programExecutable, gl::ShaderType::Fragment, variableInfoMapOut);
    }

    // Assign attributes to the vertex shader, if any.
    if (shaderStages[gl::ShaderType::Vertex] &&
        programExecutable.hasLinkedShaderStage(gl::ShaderType::Vertex))
    {
        AssignAttributeLocations(programExecutable, gl::ShaderType::Vertex, variableInfoMapOut);

        if (options.supportsTransformFeedbackEmulation)
        {
            // If transform feedback emulation is not enabled, mark all transform feedback output
            // buffers as inactive.
            const bool isTransformFeedbackStage =
                transformFeedbackStage == gl::ShaderType::Vertex &&
                options.enableTransformFeedbackEmulation &&
                !programExecutable.getLinkedTransformFeedbackVaryings().empty();

            AssignTransformFeedbackEmulationBindings(gl::ShaderType::Vertex, programExecutable,
                                                     isTransformFeedbackStage, programInterfaceInfo,
                                                     variableInfoMapOut);
        }
    }

    gl::ShaderType frontShaderType = gl::ShaderType::InvalidEnum;
    for (const gl::ShaderType shaderType : shaderStages)
    {
        if (programExecutable.hasLinkedGraphicsShader())
        {
            const gl::VaryingPacking &inputPacking  = varyingPacking.getInputPacking(shaderType);
            const gl::VaryingPacking &outputPacking = varyingPacking.getOutputPacking(shaderType);

            // Assign varying locations.
            if (shaderType != gl::ShaderType::Vertex)
            {
                AssignVaryingLocations(options, inputPacking, shaderType, frontShaderType,
                                       programInterfaceInfo, variableInfoMapOut);

                // Record active members of in gl_PerVertex.
                if (shaderType != gl::ShaderType::Fragment &&
                    frontShaderType != gl::ShaderType::InvalidEnum)
                {
                    // If an output builtin is active in the previous stage, assume it's active in
                    // the input of the current stage as well.
                    const gl::ShaderMap<gl::PerVertexMemberBitSet> &outputPerVertexActiveMembers =
                        inputPacking.getOutputPerVertexActiveMembers();
                    variableInfoMapOut->setInputPerVertexActiveMembers(
                        shaderType, outputPerVertexActiveMembers[frontShaderType]);
                }
            }
            if (shaderType != gl::ShaderType::Fragment)
            {
                AssignVaryingLocations(options, outputPacking, shaderType, frontShaderType,
                                       programInterfaceInfo, variableInfoMapOut);

                // Record active members of out gl_PerVertex.
                const gl::ShaderMap<gl::PerVertexMemberBitSet> &outputPerVertexActiveMembers =
                    outputPacking.getOutputPerVertexActiveMembers();
                variableInfoMapOut->setOutputPerVertexActiveMembers(
                    shaderType, outputPerVertexActiveMembers[shaderType]);
            }

            // Assign qualifiers to all varyings captured by transform feedback
            if (!programExecutable.getLinkedTransformFeedbackVaryings().empty() &&
                shaderType == programExecutable.getLinkedTransformFeedbackStage())
            {
                AssignTransformFeedbackQualifiers(programExecutable, outputPacking, shaderType,
                                                  options.supportsTransformFeedbackExtension,
                                                  variableInfoMapOut);
            }
        }

        frontShaderType = shaderType;
    }

    AssignUniformBindings(options, programExecutable, programInterfaceInfo, variableInfoMapOut);
    AssignTextureBindings(options, programExecutable, programInterfaceInfo, variableInfoMapOut);
    AssignNonTextureBindings(options, programExecutable, programInterfaceInfo, variableInfoMapOut);
}

void SpvAssignTransformFeedbackLocations(gl::ShaderType shaderType,
                                         const gl::ProgramExecutable &programExecutable,
                                         bool isTransformFeedbackStage,
                                         SpvProgramInterfaceInfo *programInterfaceInfo,
                                         ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    // The only varying that requires additional resources is gl_Position, as it's indirectly
    // captured through ANGLEXfbPosition.

    const std::vector<gl::TransformFeedbackVarying> &tfVaryings =
        programExecutable.getLinkedTransformFeedbackVaryings();

    bool capturesPosition = false;

    if (isTransformFeedbackStage)
    {
        for (uint32_t varyingIndex = 0; varyingIndex < tfVaryings.size(); ++varyingIndex)
        {
            const gl::TransformFeedbackVarying &tfVarying = tfVaryings[varyingIndex];
            const std::string &tfVaryingName              = tfVarying.name;

            if (tfVaryingName == "gl_Position")
            {
                ASSERT(tfVarying.isBuiltIn());
                capturesPosition = true;
                break;
            }
        }
    }

    if (capturesPosition)
    {
        AddLocationInfo(variableInfoMapOut, shaderType, sh::vk::spirv::kIdXfbExtensionPosition,
                        programInterfaceInfo->locationsUsedForXfbExtension, 0, 0, 0);
        ++programInterfaceInfo->locationsUsedForXfbExtension;
    }
    else
    {
        // Make sure this varying is removed from the other stages, or if position is not captured
        // at all.
        variableInfoMapOut->add(shaderType, sh::vk::spirv::kIdXfbExtensionPosition);
    }
}

void SpvGetShaderSpirvCode(const gl::ProgramState &programState,
                           gl::ShaderMap<const spirv::Blob *> *spirvBlobsOut)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::SharedCompiledShaderState &glShader = programState.getAttachedShader(shaderType);
        (*spirvBlobsOut)[shaderType] = glShader ? &glShader->compiledBinary : nullptr;
    }
}

void SpvAssignAllLocations(const SpvSourceOptions &options,
                           const gl::ProgramState &programState,
                           const gl::ProgramLinkedResources &resources,
                           ShaderInterfaceVariableInfoMap *variableInfoMapOut)
{
    SpvProgramInterfaceInfo spvProgramInterfaceInfo = {};
    const gl::ProgramExecutable &programExecutable  = programState.getExecutable();
    gl::ShaderType xfbStage = programState.getAttachedTransformFeedbackStage();

    // This should be done before assigning varying location. Otherwise, We can encounter shader
    // interface mismatching problem in case the transformFeedback stage is not Vertex stage.
    for (const gl::ShaderType shaderType : programExecutable.getLinkedShaderStages())
    {
        // Assign location to varyings generated for transform feedback capture
        const bool isXfbStage = shaderType == xfbStage &&
                                !programExecutable.getLinkedTransformFeedbackVaryings().empty();
        if (options.supportsTransformFeedbackExtension &&
            gl::ShaderTypeSupportsTransformFeedback(shaderType))
        {
            SpvAssignTransformFeedbackLocations(shaderType, programExecutable, isXfbStage,
                                                &spvProgramInterfaceInfo, variableInfoMapOut);
        }
    }

    SpvAssignLocations(options, programExecutable, resources.varyingPacking, xfbStage,
                       &spvProgramInterfaceInfo, variableInfoMapOut);
}

angle::Result SpvTransformSpirvCode(const SpvTransformOptions &options,
                                    const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                    const spirv::Blob &initialSpirvBlob,
                                    spirv::Blob *spirvBlobOut)
{
    if (initialSpirvBlob.empty())
    {
        return angle::Result::Continue;
    }

    const bool hasAliasingAttributes =
        options.shaderType == gl::ShaderType::Vertex && variableInfoMap.hasAliasingAttributes();

    // Transform the SPIR-V code by assigning location/set/binding values.
    SpirvTransformer transformer(initialSpirvBlob, options, !hasAliasingAttributes, variableInfoMap,
                                 spirvBlobOut);
    transformer.transform();

    // If there are aliasing vertex attributes, transform the SPIR-V again to remove them.
    if (hasAliasingAttributes)
    {
        spirv::Blob preTransformBlob = std::move(*spirvBlobOut);
        SpirvVertexAttributeAliasingTransformer aliasingTransformer(
            preTransformBlob, variableInfoMap, std::move(transformer.getVariableInfoByIdMap()),
            spirvBlobOut);
        aliasingTransformer.transform();
    }

    spirvBlobOut->shrink_to_fit();

    if (options.validate)
    {
        ASSERT(spirv::Validate(*spirvBlobOut));
    }

    return angle::Result::Continue;
}
}  // namespace rx
