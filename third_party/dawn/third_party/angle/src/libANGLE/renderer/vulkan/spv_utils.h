//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Utilities to map shader interface variables to Vulkan mappings, and transform the SPIR-V
// accordingly.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SPV_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_SPV_UTILS_H_

#include <functional>

#include "common/spirv/spirv_types.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "platform/autogen/FeaturesVk_autogen.h"

namespace rx
{
class ShaderInterfaceVariableInfoMap;

struct SpvProgramInterfaceInfo
{
    uint32_t currentUniformBindingIndex        = 0;
    uint32_t currentTextureBindingIndex        = 0;
    uint32_t currentShaderResourceBindingIndex = 0;

    uint32_t locationsUsedForXfbExtension = 0;
};

struct SpvSourceOptions
{
    uint32_t maxColorInputAttachmentCount     = 0;
    bool supportsTransformFeedbackExtension = false;
    bool supportsTransformFeedbackEmulation = false;
    bool enableTransformFeedbackEmulation   = false;
    bool supportsDepthStencilInputAttachments = false;
};

SpvSourceOptions SpvCreateSourceOptions(const angle::FeaturesVk &features,
                                        uint32_t maxColorInputAttachmentCount);

struct SpvTransformOptions
{
    gl::ShaderType shaderType           = gl::ShaderType::InvalidEnum;
    bool isLastPreFragmentStage         = false;
    bool isTransformFeedbackStage       = false;
    bool isTransformFeedbackEmulated    = false;
    bool isMultisampledFramebufferFetch = false;
    bool enableSampleShading            = false;
    bool validate                       = true;
    bool useSpirvVaryingPrecisionFixer  = false;
    bool removeDepthStencilInput        = false;
};

struct ShaderInterfaceVariableXfbInfo
{
    static constexpr uint32_t kInvalid = std::numeric_limits<uint32_t>::max();

    ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
    struct PODStruct
    {
        // Used by both extension and emulation
        uint32_t buffer = kInvalid;
        uint32_t offset = kInvalid;
        uint32_t stride = kInvalid;

        // Used only by emulation (array index support is missing from VK_EXT_transform_feedback)
        uint32_t arraySize   = kInvalid;
        uint32_t columnCount = kInvalid;
        uint32_t rowCount    = kInvalid;
        uint32_t arrayIndex  = kInvalid;
        GLenum componentType = GL_FLOAT;
    } pod;
    ANGLE_DISABLE_STRUCT_PADDING_WARNINGS
    // If empty, the whole array is captured.  Otherwise only the specified members are captured.
    std::vector<ShaderInterfaceVariableXfbInfo> arrayElements;
};

// Information for each shader interface variable.  Not all fields are relevant to each shader
// interface variable.  For example opaque uniforms require a set and binding index, while vertex
// attributes require a location.
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
struct ShaderInterfaceVariableInfo
{
    ShaderInterfaceVariableInfo()
        : descriptorSet(kInvalid),
          binding(kInvalid),
          location(kInvalid),
          component(kInvalid),
          index(kInvalid),
          useRelaxedPrecision(false),
          varyingIsInput(false),
          varyingIsOutput(false),
          hasTransformFeedback(false),
          isArray(false),
          padding(0),
          attributeComponentCount(0),
          attributeLocationCount(0)
    {}

    static constexpr uint32_t kInvalid = std::numeric_limits<uint32_t>::max();

    // Used for interface blocks and opaque uniforms.
    uint32_t descriptorSet;
    uint32_t binding;
    // Used for vertex attributes, fragment shader outputs and varyings.  There could be different
    // variables that share the same name, such as a vertex attribute and a fragment output.  They
    // will share this object since they have the same name, but will find possibly different
    // locations in their respective slots.
    uint32_t location;
    uint32_t component;
    uint32_t index;

    // The stages this shader interface variable is active.
    gl::ShaderBitSet activeStages;

    // Indicates that the precision needs to be modified in the generated SPIR-V
    // to support only transferring medium precision data when there's a precision
    // mismatch between the shaders. For example, either the VS casts highp->mediump
    // or the FS casts mediump->highp.
    uint8_t useRelaxedPrecision : 1;
    // Indicate if varying is input or output, or both (in case of for example gl_Position in a
    // geometry shader)
    uint8_t varyingIsInput : 1;
    uint8_t varyingIsOutput : 1;
    uint8_t hasTransformFeedback : 1;
    uint8_t isArray : 1;
    uint8_t padding : 3;

    // For vertex attributes, this is the number of components / locations.  These are used by the
    // vertex attribute aliasing transformation only.
    uint8_t attributeComponentCount;
    uint8_t attributeLocationCount;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

struct XFBInterfaceVariableInfo
{
    // Used for transform feedback extension to decorate vertex shader output.
    ShaderInterfaceVariableXfbInfo xfb;
    std::vector<ShaderInterfaceVariableXfbInfo> fieldXfb;
};
using XFBVariableInfoPtr = std::unique_ptr<XFBInterfaceVariableInfo>;

uint32_t SpvGetXfbBufferBlockId(const uint32_t bufferIndex);

void SpvAssignLocations(const SpvSourceOptions &options,
                        const gl::ProgramExecutable &programExecutable,
                        const gl::ProgramVaryingPacking &varyingPacking,
                        const gl::ShaderType transformFeedbackStage,
                        SpvProgramInterfaceInfo *programInterfaceInfo,
                        ShaderInterfaceVariableInfoMap *variableInfoMapOut);

void SpvAssignTransformFeedbackLocations(gl::ShaderType shaderType,
                                         const gl::ProgramExecutable &programExecutable,
                                         bool isTransformFeedbackStage,
                                         SpvProgramInterfaceInfo *programInterfaceInfo,
                                         ShaderInterfaceVariableInfoMap *variableInfoMapOut);

// Retrieves the compiled SPIR-V code for each shader stage.
void SpvGetShaderSpirvCode(const gl::ProgramState &programState,
                           gl::ShaderMap<const angle::spirv::Blob *> *spirvBlobsOut);

// Calls |SpvAssign*Locations| as necessary.
void SpvAssignAllLocations(const SpvSourceOptions &options,
                           const gl::ProgramState &programState,
                           const gl::ProgramLinkedResources &resources,
                           ShaderInterfaceVariableInfoMap *variableInfoMapOut);

angle::Result SpvTransformSpirvCode(const SpvTransformOptions &options,
                                    const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                    const angle::spirv::Blob &initialSpirvBlob,
                                    angle::spirv::Blob *spirvBlobOut);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SPV_UTILS_H_
