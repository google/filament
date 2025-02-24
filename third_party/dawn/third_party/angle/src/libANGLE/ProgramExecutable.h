//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutable.h: Collects the information and interfaces common to both Programs and
// ProgramPipelines in order to execute/draw with either.

#ifndef LIBANGLE_PROGRAMEXECUTABLE_H_
#define LIBANGLE_PROGRAMEXECUTABLE_H_

#include "common/BinaryStream.h"
#include "libANGLE/Caps.h"
#include "libANGLE/InfoLog.h"
#include "libANGLE/ProgramLinkedResources.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/VaryingPacking.h"
#include "libANGLE/angletypes.h"

namespace rx
{
class GLImplFactory;
class LinkSubTask;
class ProgramExecutableImpl;
}  // namespace rx

namespace gl
{

// This small structure encapsulates binding sampler uniforms to active GL textures.
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
struct SamplerBinding
{
    SamplerBinding() = default;
    SamplerBinding(TextureType textureTypeIn,
                   GLenum samplerTypeIn,
                   SamplerFormat formatIn,
                   uint16_t startIndex,
                   uint16_t elementCount)
        : textureType(textureTypeIn),
          format(formatIn),
          textureUnitsStartIndex(startIndex),
          textureUnitsCount(elementCount)
    {
        SetBitField(samplerType, samplerTypeIn);
    }

    GLuint getTextureUnit(const std::vector<GLuint> &boundTextureUnits,
                          unsigned int arrayIndex) const
    {
        return boundTextureUnits[textureUnitsStartIndex + arrayIndex];
    }

    // Necessary for retrieving active textures from the GL state.
    TextureType textureType;
    SamplerFormat format;
    uint16_t samplerType;
    // [textureUnitsStartIndex, textureUnitsStartIndex+textureUnitsCount) Points to the subset in
    // mSamplerBoundTextureUnits that stores the texture unit bound to this sampler. Cropped by the
    // amount of unused elements reported by the driver.
    uint16_t textureUnitsStartIndex;
    uint16_t textureUnitsCount;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

struct ImageBinding
{
    ImageBinding() = default;
    ImageBinding(size_t count, TextureType textureTypeIn)
        : textureType(textureTypeIn), boundImageUnits(count, 0)
    {}
    ImageBinding(GLuint imageUnit, size_t count, TextureType textureTypeIn);

    // Necessary for distinguishing between textures with images and texture buffers.
    TextureType textureType;

    // List of all textures bound.
    // Cropped by the amount of unused elements reported by the driver.
    std::vector<GLuint> boundImageUnits;
};

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
struct ProgramInput
{
    ProgramInput() = default;
    ProgramInput(const sh::ShaderVariable &var);

    GLenum getType() const { return pod.type; }
    bool isBuiltIn() const { return pod.flagBits.isBuiltIn; }
    bool isArray() const { return pod.flagBits.isArray; }
    bool isActive() const { return pod.flagBits.active; }
    bool isPatch() const { return pod.flagBits.isPatch; }
    int getLocation() const { return pod.location; }
    unsigned int getBasicTypeElementCount() const { return pod.basicTypeElementCount; }
    unsigned int getArraySizeProduct() const { return pod.arraySizeProduct; }
    uint32_t getId() const { return pod.id; }
    sh::InterpolationType getInterpolation() const
    {
        return static_cast<sh::InterpolationType>(pod.interpolation);
    }

    void setLocation(int location) { pod.location = location; }
    void resetEffectiveLocation()
    {
        if (pod.flagBits.hasImplicitLocation)
        {
            pod.location = -1;
        }
    }

    std::string name;
    std::string mappedName;

    // The struct bellow must only contain data of basic type so that entire struct can memcpy-able.
    struct PODStruct
    {
        uint16_t type;  // GLenum
        uint16_t arraySizeProduct;

        int location;

        uint8_t interpolation;  // sh::InterpolationType
        union
        {
            struct
            {
                uint8_t active : 1;
                uint8_t isPatch : 1;
                uint8_t hasImplicitLocation : 1;
                uint8_t isArray : 1;
                uint8_t isBuiltIn : 1;
                uint8_t padding : 3;
            } flagBits;
            uint8_t flagBitsAsUByte;
        };
        int16_t basicTypeElementCount;

        uint32_t id;
    } pod;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
struct ProgramOutput
{
    ProgramOutput() = default;
    ProgramOutput(const sh::ShaderVariable &var);
    bool isBuiltIn() const { return pod.isBuiltIn; }
    bool isArray() const { return pod.isArray; }
    int getLocation() const { return pod.location; }
    unsigned int getOutermostArraySize() const { return pod.outermostArraySize; }
    void resetEffectiveLocation()
    {
        if (pod.hasImplicitLocation)
        {
            pod.location = -1;
        }
    }

    std::string name;
    std::string mappedName;

    struct PODStruct
    {
        GLenum type;
        int location;
        int index;
        uint32_t id;

        uint16_t outermostArraySize;
        uint16_t basicTypeElementCount;

        uint32_t isPatch : 1;
        uint32_t yuv : 1;
        uint32_t isBuiltIn : 1;
        uint32_t isArray : 1;
        uint32_t hasImplicitLocation : 1;
        uint32_t hasShaderAssignedLocation : 1;
        uint32_t hasApiAssignedLocation : 1;
        uint32_t pad : 25;
    } pod;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

// A varying with transform feedback enabled. If it's an array, either the whole array or one of its
// elements specified by 'arrayIndex' can set to be enabled.
struct TransformFeedbackVarying : public sh::ShaderVariable
{
    TransformFeedbackVarying() = default;

    TransformFeedbackVarying(const sh::ShaderVariable &varyingIn, GLuint arrayIndexIn)
        : sh::ShaderVariable(varyingIn), arrayIndex(arrayIndexIn)
    {
        ASSERT(!isArrayOfArrays());
    }

    TransformFeedbackVarying(const sh::ShaderVariable &field, const sh::ShaderVariable &parent)
        : arrayIndex(GL_INVALID_INDEX)
    {
        sh::ShaderVariable *thisVar = this;
        *thisVar                    = field;
        interpolation               = parent.interpolation;
        isInvariant                 = parent.isInvariant;
        ASSERT(parent.isShaderIOBlock || !parent.name.empty());
        if (!parent.name.empty())
        {
            name       = parent.name + "." + name;
            mappedName = parent.mappedName + "." + mappedName;
        }
        structOrBlockName       = parent.structOrBlockName;
        mappedStructOrBlockName = parent.mappedStructOrBlockName;
    }

    std::string nameWithArrayIndex() const
    {
        std::stringstream fullNameStr;
        fullNameStr << name;
        if (arrayIndex != GL_INVALID_INDEX)
        {
            fullNameStr << "[" << arrayIndex << "]";
        }
        return fullNameStr.str();
    }
    GLsizei size() const
    {
        return (isArray() && arrayIndex == GL_INVALID_INDEX ? getOutermostArraySize() : 1);
    }

    GLuint arrayIndex;
};

class ProgramState;
class ProgramPipelineState;

class ProgramExecutable;
using SharedProgramExecutable = std::shared_ptr<ProgramExecutable>;

class ProgramExecutable final : public angle::Subject
{
  public:
    ProgramExecutable(rx::GLImplFactory *factory, InfoLog *infoLog);
    ~ProgramExecutable() override;

    void destroy(const Context *context);

    ANGLE_INLINE rx::ProgramExecutableImpl *getImplementation() const { return mImplementation; }

    void save(gl::BinaryOutputStream *stream) const;
    void load(gl::BinaryInputStream *stream);

    InfoLog &getInfoLog() const { return *mInfoLog; }
    std::string getInfoLogString() const;
    void resetInfoLog() const { mInfoLog->reset(); }

    void resetLinkedShaderStages() { mPod.linkedShaderStages.reset(); }
    const ShaderBitSet getLinkedShaderStages() const { return mPod.linkedShaderStages; }
    void setLinkedShaderStages(ShaderType shaderType)
    {
        mPod.linkedShaderStages.set(shaderType);
        updateCanDrawWith();
    }
    bool hasLinkedShaderStage(ShaderType shaderType) const
    {
        ASSERT(shaderType != ShaderType::InvalidEnum);
        return mPod.linkedShaderStages[shaderType];
    }
    size_t getLinkedShaderStageCount() const { return mPod.linkedShaderStages.count(); }
    bool hasLinkedGraphicsShader() const
    {
        return mPod.linkedShaderStages.any() &&
               mPod.linkedShaderStages != gl::ShaderBitSet{gl::ShaderType::Compute};
    }
    bool hasLinkedTessellationShader() const
    {
        return mPod.linkedShaderStages[ShaderType::TessEvaluation];
    }
    ShaderType getFirstLinkedShaderStageType() const;
    ShaderType getLastLinkedShaderStageType() const;

    ShaderType getLinkedTransformFeedbackStage() const
    {
        return GetLastPreFragmentStage(mPod.linkedShaderStages);
    }

    const AttributesMask &getActiveAttribLocationsMask() const
    {
        return mPod.activeAttribLocationsMask;
    }
    bool isAttribLocationActive(size_t attribLocation) const
    {
        ASSERT(attribLocation < mPod.activeAttribLocationsMask.size());
        return mPod.activeAttribLocationsMask[attribLocation];
    }

    AttributesMask getNonBuiltinAttribLocationsMask() const { return mPod.attributesMask; }
    unsigned int getMaxActiveAttribLocation() const { return mPod.maxActiveAttribLocation; }
    ComponentTypeMask getAttributesTypeMask() const { return mPod.attributesTypeMask; }
    AttributesMask getAttributesMask() const { return mPod.attributesMask; }

    const ActiveTextureMask &getActiveSamplersMask() const { return mActiveSamplersMask; }
    void setActiveTextureMask(ActiveTextureMask mask) { mActiveSamplersMask = mask; }
    SamplerFormat getSamplerFormatForTextureUnitIndex(size_t textureUnitIndex) const
    {
        return mActiveSamplerFormats[textureUnitIndex];
    }
    const ShaderBitSet getSamplerShaderBitsForTextureUnitIndex(size_t textureUnitIndex) const
    {
        return mActiveSamplerShaderBits[textureUnitIndex];
    }
    const ActiveTextureMask &getActiveImagesMask() const { return mActiveImagesMask; }
    void setActiveImagesMask(ActiveTextureMask mask) { mActiveImagesMask = mask; }
    const ActiveTextureArray<ShaderBitSet> &getActiveImageShaderBits() const
    {
        return mActiveImageShaderBits;
    }

    const ActiveTextureMask &getActiveYUVSamplers() const { return mActiveSamplerYUV; }

    const ActiveTextureArray<TextureType> &getActiveSamplerTypes() const
    {
        return mActiveSamplerTypes;
    }

    void setActive(size_t textureUnit,
                   const SamplerBinding &samplerBinding,
                   const gl::LinkedUniform &samplerUniform);
    void setInactive(size_t textureUnit);
    void hasSamplerTypeConflict(size_t textureUnit);
    void hasSamplerFormatConflict(size_t textureUnit);

    void updateActiveSamplers(const ProgramExecutable &executable);

    bool hasDefaultUniforms() const { return !getDefaultUniformRange().empty(); }
    bool hasTextures() const { return !getSamplerBindings().empty(); }
    bool hasUniformBuffers() const { return !mUniformBlocks.empty(); }
    bool hasStorageBuffers() const { return !mShaderStorageBlocks.empty(); }
    bool hasAtomicCounterBuffers() const { return !mAtomicCounterBuffers.empty(); }
    bool hasImages() const { return !mImageBindings.empty(); }
    bool hasTransformFeedbackOutput() const
    {
        return !getLinkedTransformFeedbackVaryings().empty();
    }
    bool usesColorFramebufferFetch() const { return mPod.fragmentInoutIndices.any(); }
    bool usesDepthFramebufferFetch() const { return mPod.hasDepthInputAttachment; }
    bool usesStencilFramebufferFetch() const { return mPod.hasStencilInputAttachment; }

    // Count the number of uniform and storage buffer declarations, counting arrays as one.
    size_t getTransformFeedbackBufferCount() const { return mTransformFeedbackStrides.size(); }

    void updateCanDrawWith() { mPod.canDrawWith = hasLinkedShaderStage(ShaderType::Vertex); }
    bool hasVertexShader() const { return mPod.canDrawWith; }

    const std::vector<ProgramInput> &getProgramInputs() const { return mProgramInputs; }
    const std::vector<ProgramOutput> &getOutputVariables() const { return mOutputVariables; }
    const std::vector<VariableLocation> &getOutputLocations() const { return mOutputLocations; }
    const std::vector<VariableLocation> &getSecondaryOutputLocations() const
    {
        return mSecondaryOutputLocations;
    }
    const std::vector<LinkedUniform> &getUniforms() const { return mUniforms; }
    const std::vector<std::string> &getUniformNames() const { return mUniformNames; }
    const std::vector<std::string> &getUniformMappedNames() const { return mUniformMappedNames; }
    const std::vector<InterfaceBlock> &getUniformBlocks() const { return mUniformBlocks; }
    const std::vector<VariableLocation> &getUniformLocations() const { return mUniformLocations; }
    const std::vector<SamplerBinding> &getSamplerBindings() const { return mSamplerBindings; }
    const std::vector<GLuint> &getSamplerBoundTextureUnits() const
    {
        return mSamplerBoundTextureUnits;
    }
    const std::vector<ImageBinding> &getImageBindings() const { return mImageBindings; }
    const std::vector<ShPixelLocalStorageFormat> &getPixelLocalStorageFormats() const
    {
        return mPixelLocalStorageFormats;
    }
    std::vector<ImageBinding> *getImageBindings() { return &mImageBindings; }
    const RangeUI &getDefaultUniformRange() const { return mPod.defaultUniformRange; }
    const RangeUI &getSamplerUniformRange() const { return mPod.samplerUniformRange; }
    const RangeUI &getImageUniformRange() const { return mPod.imageUniformRange; }
    const RangeUI &getAtomicCounterUniformRange() const { return mPod.atomicCounterUniformRange; }
    DrawBufferMask getFragmentInoutIndices() const { return mPod.fragmentInoutIndices; }
    bool hasClipDistance() const { return mPod.hasClipDistance; }
    bool hasDiscard() const { return mPod.hasDiscard; }
    bool hasDepthInputAttachment() const { return mPod.hasDepthInputAttachment; }
    bool hasStencilInputAttachment() const { return mPod.hasStencilInputAttachment; }
    bool enablesPerSampleShading() const { return mPod.enablesPerSampleShading; }
    BlendEquationBitSet getAdvancedBlendEquations() const { return mPod.advancedBlendEquations; }
    const std::vector<TransformFeedbackVarying> &getLinkedTransformFeedbackVaryings() const
    {
        return mLinkedTransformFeedbackVaryings;
    }
    GLint getTransformFeedbackBufferMode() const { return mPod.transformFeedbackBufferMode; }
    const sh::WorkGroupSize &getComputeShaderLocalSize() const
    {
        return mPod.computeShaderLocalSize;
    }
    void remapUniformBlockBinding(UniformBlockIndex uniformBlockIndex, GLuint uniformBlockBinding);
    GLuint getUniformBlockBinding(size_t uniformBlockIndex) const
    {
        ASSERT(uniformBlockIndex < mUniformBlocks.size());

        // Unlike SSBOs and atomic counter buffers, GLES allows UBOs bindings to be remapped.  Note
        // that desktop GL allows SSBO bindings to also be remapped, but that's not allowed in GLES.
        //
        // It's therefore important to never directly reference block.pod.inShaderBinding unless the
        // specific shader-specified binding is required.
        return mUniformBlockIndexToBufferBinding[uniformBlockIndex];
    }
    GLuint getShaderStorageBlockBinding(size_t blockIndex) const
    {
        ASSERT(blockIndex < mShaderStorageBlocks.size());
        // The buffer binding for SSBOs is the one specified in the shader
        return mShaderStorageBlocks[blockIndex].pod.inShaderBinding;
    }
    GLuint getAtomicCounterBufferBinding(size_t blockIndex) const
    {
        ASSERT(blockIndex < mAtomicCounterBuffers.size());
        // The buffer binding for atomic counter buffers is the one specified in the shader
        return mAtomicCounterBuffers[blockIndex].pod.inShaderBinding;
    }
    const InterfaceBlock &getUniformBlockByIndex(size_t index) const
    {
        ASSERT(index < mUniformBlocks.size());
        return mUniformBlocks[index];
    }
    const InterfaceBlock &getShaderStorageBlockByIndex(size_t index) const
    {
        ASSERT(index < mShaderStorageBlocks.size());
        return mShaderStorageBlocks[index];
    }
    const BufferVariable &getBufferVariableByIndex(size_t index) const
    {
        ASSERT(index < mBufferVariables.size());
        return mBufferVariables[index];
    }
    const std::vector<GLsizei> &getTransformFeedbackStrides() const
    {
        return mTransformFeedbackStrides;
    }
    const std::vector<AtomicCounterBuffer> &getAtomicCounterBuffers() const
    {
        return mAtomicCounterBuffers;
    }
    const std::vector<InterfaceBlock> &getShaderStorageBlocks() const
    {
        return mShaderStorageBlocks;
    }
    const std::vector<BufferVariable> &getBufferVariables() const { return mBufferVariables; }
    const LinkedUniform &getUniformByIndex(size_t index) const
    {
        ASSERT(index < static_cast<size_t>(mUniforms.size()));
        return mUniforms[index];
    }
    const std::string &getUniformNameByIndex(size_t index) const
    {
        ASSERT(index < static_cast<size_t>(mUniforms.size()));
        return mUniformNames[index];
    }

    GLuint getUniformIndexFromImageIndex(size_t imageIndex) const
    {
        ASSERT(imageIndex < mPod.imageUniformRange.length());
        return static_cast<GLuint>(imageIndex) + mPod.imageUniformRange.low();
    }

    GLuint getUniformIndexFromSamplerIndex(size_t samplerIndex) const
    {
        ASSERT(samplerIndex < mPod.samplerUniformRange.length());
        return static_cast<GLuint>(samplerIndex) + mPod.samplerUniformRange.low();
    }

    void saveLinkedStateInfo(const ProgramState &state);
    const std::vector<sh::ShaderVariable> &getLinkedOutputVaryings(ShaderType shaderType) const
    {
        return mLinkedOutputVaryings[shaderType];
    }
    const std::vector<sh::ShaderVariable> &getLinkedInputVaryings(ShaderType shaderType) const
    {
        return mLinkedInputVaryings[shaderType];
    }

    const std::vector<sh::ShaderVariable> &getLinkedUniforms(ShaderType shaderType) const
    {
        return mLinkedUniforms[shaderType];
    }

    const std::vector<sh::InterfaceBlock> &getLinkedUniformBlocks(ShaderType shaderType) const
    {
        return mLinkedUniformBlocks[shaderType];
    }

    int getLinkedShaderVersion(ShaderType shaderType) const
    {
        return mPod.linkedShaderVersions[shaderType];
    }

    bool isYUVOutput() const { return mPod.hasYUVOutput; }

    PrimitiveMode getGeometryShaderInputPrimitiveType() const
    {
        return mPod.geometryShaderInputPrimitiveType;
    }

    PrimitiveMode getGeometryShaderOutputPrimitiveType() const
    {
        return mPod.geometryShaderOutputPrimitiveType;
    }

    int getGeometryShaderInvocations() const { return mPod.geometryShaderInvocations; }

    int getGeometryShaderMaxVertices() const { return mPod.geometryShaderMaxVertices; }

    GLint getTessControlShaderVertices() const { return mPod.tessControlShaderVertices; }
    GLenum getTessGenMode() const { return mPod.tessGenMode; }
    GLenum getTessGenPointMode() const { return mPod.tessGenPointMode; }
    GLenum getTessGenSpacing() const { return mPod.tessGenSpacing; }
    GLenum getTessGenVertexOrder() const { return mPod.tessGenVertexOrder; }

    int getNumViews() const { return mPod.numViews; }
    bool usesMultiview() const { return mPod.numViews != -1; }

    rx::SpecConstUsageBits getSpecConstUsageBits() const { return mPod.specConstUsageBits; }

    int getDrawIDLocation() const { return mPod.drawIDLocation; }
    int getBaseVertexLocation() const { return mPod.baseVertexLocation; }
    int getBaseInstanceLocation() const { return mPod.baseInstanceLocation; }

    bool hasDrawIDUniform() const { return getDrawIDLocation() >= 0; }
    bool hasBaseVertexUniform() const { return getBaseVertexLocation() >= 0; }
    bool hasBaseInstanceUniform() const { return getBaseInstanceLocation() >= 0; }

    void resetCachedValidateSamplersResult() { mCachedValidateSamplersResult.reset(); }
    bool validateSamplers(const Caps &caps) const
    {
        // Use the cache if:
        // - we aren't using an info log (which gives the full error).
        // - The sample mapping hasn't changed and we've already validated.
        if (mCachedValidateSamplersResult.valid())
        {
            return mCachedValidateSamplersResult.value();
        }

        return validateSamplersImpl(caps);
    }

    ComponentTypeMask getFragmentOutputsTypeMask() const { return mPod.drawBufferTypeMask; }
    DrawBufferMask getActiveOutputVariablesMask() const { return mPod.activeOutputVariablesMask; }
    DrawBufferMask getActiveSecondaryOutputVariablesMask() const
    {
        return mPod.activeSecondaryOutputVariablesMask;
    }

    GLuint getInputResourceIndex(const GLchar *name) const;
    GLuint getOutputResourceIndex(const GLchar *name) const;
    void getInputResourceName(GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) const;
    void getOutputResourceName(GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) const;
    void getUniformResourceName(GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) const;
    void getBufferVariableResourceName(GLuint index,
                                       GLsizei bufSize,
                                       GLsizei *length,
                                       GLchar *name) const;
    const ProgramInput &getInputResource(size_t index) const
    {
        ASSERT(index < mProgramInputs.size());
        return mProgramInputs[index];
    }
    GLuint getInputResourceMaxNameSize() const;
    GLuint getOutputResourceMaxNameSize() const;
    GLuint getInputResourceLocation(const GLchar *name) const;
    GLuint getOutputResourceLocation(const GLchar *name) const;
    const std::string getInputResourceName(GLuint index) const;
    const std::string getOutputResourceName(GLuint index) const;
    const gl::ProgramOutput &getOutputResource(size_t index) const
    {
        ASSERT(index < mOutputVariables.size());
        return mOutputVariables[index];
    }

    GLint getFragDataLocation(const std::string &name) const;

    // EXT_blend_func_extended
    GLint getFragDataIndex(const std::string &name) const;

    GLsizei getTransformFeedbackVaryingMaxLength() const;
    GLuint getTransformFeedbackVaryingResourceIndex(const GLchar *name) const;
    const TransformFeedbackVarying &getTransformFeedbackVaryingResource(GLuint index) const;
    void getTransformFeedbackVarying(GLuint index,
                                     GLsizei bufSize,
                                     GLsizei *length,
                                     GLsizei *size,
                                     GLenum *type,
                                     GLchar *name) const;

    void getActiveAttribute(GLuint index,
                            GLsizei bufsize,
                            GLsizei *length,
                            GLint *size,
                            GLenum *type,
                            GLchar *name) const;
    GLint getActiveAttributeMaxLength() const;
    GLuint getAttributeLocation(const std::string &name) const;

    void getActiveUniform(GLuint index,
                          GLsizei bufsize,
                          GLsizei *length,
                          GLint *size,
                          GLenum *type,
                          GLchar *name) const;
    GLint getActiveUniformMaxLength() const;
    bool isValidUniformLocation(UniformLocation location) const;
    const LinkedUniform &getUniformByLocation(UniformLocation location) const;
    const VariableLocation &getUniformLocation(UniformLocation location) const;
    UniformLocation getUniformLocation(const std::string &name) const;
    GLuint getUniformIndex(const std::string &name) const;

    void getActiveUniformBlockName(const Context *context,
                                   const UniformBlockIndex blockIndex,
                                   GLsizei bufSize,
                                   GLsizei *length,
                                   GLchar *blockName) const;
    void getActiveShaderStorageBlockName(const GLuint blockIndex,
                                         GLsizei bufSize,
                                         GLsizei *length,
                                         GLchar *blockName) const;

    GLint getActiveUniformBlockMaxNameLength() const;
    GLint getActiveShaderStorageBlockMaxNameLength() const;

    GLuint getUniformBlockIndex(const std::string &name) const;
    GLuint getShaderStorageBlockIndex(const std::string &name) const;

    GLuint getUniformIndexFromName(const std::string &name) const;
    GLuint getUniformIndexFromLocation(UniformLocation location) const;
    Optional<GLuint> getSamplerIndex(UniformLocation location) const;
    bool isSamplerUniformIndex(GLuint index) const;
    GLuint getSamplerIndexFromUniformIndex(GLuint uniformIndex) const;
    bool isImageUniformIndex(GLuint index) const;
    GLuint getImageIndexFromUniformIndex(GLuint uniformIndex) const;
    GLuint getBufferVariableIndexFromName(const std::string &name) const;

    bool linkUniforms(const Caps &caps,
                      const ShaderMap<std::vector<sh::ShaderVariable>> &shaderUniforms,
                      const ProgramAliasedBindings &uniformLocationBindings,
                      GLuint *combinedImageUniformsCount,
                      std::vector<UnusedUniform> *unusedUniforms);

    void copyInputsFromProgram(const ProgramExecutable &executable);
    void copyUniformBuffersFromProgram(const ProgramExecutable &executable,
                                       ShaderType shaderType,
                                       ProgramUniformBlockArray<GLuint> *ppoUniformBlockMap);
    void copyStorageBuffersFromProgram(const ProgramExecutable &executable, ShaderType shaderType);
    void clearSamplerBindings();
    void copySamplerBindingsFromProgram(const ProgramExecutable &executable);
    void copyImageBindingsFromProgram(const ProgramExecutable &executable);
    void copyOutputsFromProgram(const ProgramExecutable &executable);
    void copyUniformsFromProgramMap(const ShaderMap<SharedProgramExecutable> &executables);

    void setUniform1fv(UniformLocation location, GLsizei count, const GLfloat *v);
    void setUniform2fv(UniformLocation location, GLsizei count, const GLfloat *v);
    void setUniform3fv(UniformLocation location, GLsizei count, const GLfloat *v);
    void setUniform4fv(UniformLocation location, GLsizei count, const GLfloat *v);
    void setUniform1iv(Context *context, UniformLocation location, GLsizei count, const GLint *v);
    void setUniform2iv(UniformLocation location, GLsizei count, const GLint *v);
    void setUniform3iv(UniformLocation location, GLsizei count, const GLint *v);
    void setUniform4iv(UniformLocation location, GLsizei count, const GLint *v);
    void setUniform1uiv(UniformLocation location, GLsizei count, const GLuint *v);
    void setUniform2uiv(UniformLocation location, GLsizei count, const GLuint *v);
    void setUniform3uiv(UniformLocation location, GLsizei count, const GLuint *v);
    void setUniform4uiv(UniformLocation location, GLsizei count, const GLuint *v);
    void setUniformMatrix2fv(UniformLocation location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value);
    void setUniformMatrix3fv(UniformLocation location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value);
    void setUniformMatrix4fv(UniformLocation location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value);
    void setUniformMatrix2x3fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value);
    void setUniformMatrix3x2fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value);
    void setUniformMatrix2x4fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value);
    void setUniformMatrix4x2fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value);
    void setUniformMatrix3x4fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value);
    void setUniformMatrix4x3fv(UniformLocation location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value);

    void getUniformfv(const Context *context, UniformLocation location, GLfloat *params) const;
    void getUniformiv(const Context *context, UniformLocation location, GLint *params) const;
    void getUniformuiv(const Context *context, UniformLocation location, GLuint *params) const;

    void setDrawIDUniform(GLint drawid);
    void setBaseVertexUniform(GLint baseVertex);
    void setBaseInstanceUniform(GLuint baseInstance);

    ProgramUniformBlockMask getUniformBufferBlocksMappedToBinding(size_t uniformBufferIndex)
    {
        return mUniformBufferBindingToUniformBlocks[uniformBufferIndex];
    }

    const ProgramUniformBlockArray<GLuint> &getUniformBlockIndexToBufferBindingForCapture() const
    {
        return mUniformBlockIndexToBufferBinding;
    }

    const ShaderMap<SharedProgramExecutable> &getPPOProgramExecutables() const
    {
        return mPPOProgramExecutables;
    }

    bool IsPPO() const { return mIsPPO; }

    // Post-link task helpers
    const std::vector<std::shared_ptr<rx::LinkSubTask>> &getPostLinkSubTasks() const
    {
        return mPostLinkSubTasks;
    }

    const std::vector<std::shared_ptr<angle::WaitableEvent>> &getPostLinkSubTaskWaitableEvents()
        const
    {
        return mPostLinkSubTaskWaitableEvents;
    }

    void onPostLinkTasksComplete() const
    {
        mPostLinkSubTasks.clear();
        mPostLinkSubTaskWaitableEvents.clear();
    }

    void waitForPostLinkTasks(const Context *context);

  private:
    friend class Program;
    friend class ProgramPipeline;
    friend class ProgramState;
    friend class ProgramPipelineState;

    void reset();

    void updateActiveImages(const ProgramExecutable &executable);

    bool linkMergedVaryings(const Caps &caps,
                            const Limitations &limitations,
                            const Version &clientVersion,
                            bool webglCompatibility,
                            const ProgramMergedVaryings &mergedVaryings,
                            const LinkingVariables &linkingVariables,
                            ProgramVaryingPacking *varyingPacking);

    bool linkValidateTransformFeedback(const Caps &caps,
                                       const Version &clientVersion,
                                       const ProgramMergedVaryings &varyings,
                                       ShaderType stage);

    void gatherTransformFeedbackVaryings(const ProgramMergedVaryings &varyings, ShaderType stage);

    void updateTransformFeedbackStrides();

    bool validateSamplersImpl(const Caps &caps) const;

    bool linkValidateOutputVariables(const Caps &caps,
                                     const Version &version,
                                     GLuint combinedImageUniformsCount,
                                     GLuint combinedShaderStorageBlocksCount,
                                     int fragmentShaderVersion,
                                     const ProgramAliasedBindings &fragmentOutputLocations,
                                     const ProgramAliasedBindings &fragmentOutputIndices);

    bool gatherOutputTypes();

    void linkSamplerAndImageBindings(GLuint *combinedImageUniformsCount);
    bool linkAtomicCounterBuffers(const Caps &caps);

    void getResourceName(const std::string name,
                         GLsizei bufSize,
                         GLsizei *length,
                         GLchar *dest) const;
    bool shouldIgnoreUniform(UniformLocation location) const;
    GLuint getSamplerUniformBinding(const VariableLocation &uniformLocation) const;
    GLuint getImageUniformBinding(const VariableLocation &uniformLocation) const;

    void initInterfaceBlockBindings();
    void setUniformValuesFromBindingQualifiers();

    // Both these function update the cached uniform values and return a modified "count"
    // so that the uniform update doesn't overflow the uniform.
    template <typename T>
    GLsizei clampUniformCount(const VariableLocation &locationInfo,
                              GLsizei count,
                              int vectorSize,
                              const T *v);
    template <size_t cols, size_t rows, typename T>
    GLsizei clampMatrixUniformCount(UniformLocation location,
                                    GLsizei count,
                                    GLboolean transpose,
                                    const T *v);

    void updateSamplerUniform(Context *context,
                              const VariableLocation &locationInfo,
                              GLsizei clampedCount,
                              const GLint *v);

    // Scans the sampler bindings for type conflicts with sampler 'textureUnitIndex'.
    void setSamplerUniformTextureTypeAndFormat(size_t textureUnitIndex);

    template <typename DestT>
    void getUniformInternal(const Context *context,
                            DestT *dataOut,
                            UniformLocation location,
                            GLenum nativeType,
                            int components) const;

    template <typename UniformT,
              GLint UniformSize,
              void (rx::ProgramExecutableImpl::*SetUniformFunc)(GLint, GLsizei, const UniformT *)>
    void setUniformGeneric(UniformLocation location, GLsizei count, const UniformT *v);

    template <typename UniformT,
              GLint MatrixC,
              GLint MatrixR,
              void (rx::ProgramExecutableImpl::*
                        SetUniformMatrixFunc)(GLint, GLsizei, GLboolean, const UniformT *)>
    void setUniformMatrixGeneric(UniformLocation location,
                                 GLsizei count,
                                 GLboolean transpose,
                                 const UniformT *v);

    rx::ProgramExecutableImpl *mImplementation;

    // A reference to the owning object's (Program or ProgramPipeline) info log.  It's kept here for
    // convenience as numerous functions reference it.
    InfoLog *mInfoLog;

    ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
    struct PODStruct
    {
        // 8 bytes each
        angle::BitSet<MAX_VERTEX_ATTRIBS> activeAttribLocationsMask;
        ComponentTypeMask attributesTypeMask;
        // attributesMask is identical to mActiveAttribLocationsMask with built-in attributes
        // removed.
        AttributesMask attributesMask;
        ComponentTypeMask drawBufferTypeMask;

        // 4 bytes each
        uint32_t maxActiveAttribLocation;
        // KHR_blend_equation_advanced supported equation list
        BlendEquationBitSet advancedBlendEquations;

        // 1 byte each
        ShaderBitSet linkedShaderStages;
        DrawBufferMask activeOutputVariablesMask;
        DrawBufferMask activeSecondaryOutputVariablesMask;
        uint8_t hasClipDistance : 1;
        uint8_t hasDiscard : 1;
        uint8_t hasYUVOutput : 1;
        uint8_t hasDepthInputAttachment : 1;
        uint8_t hasStencilInputAttachment : 1;
        uint8_t enablesPerSampleShading : 1;
        uint8_t canDrawWith : 1;
        uint8_t isSeparable : 1;

        // 12 bytes
        sh::WorkGroupSize computeShaderLocalSize;

        // 8 bytes each
        RangeUI defaultUniformRange;
        RangeUI samplerUniformRange;
        RangeUI imageUniformRange;
        RangeUI atomicCounterUniformRange;

        // 1 byte.  Bitset of which input attachments have been declared
        DrawBufferMask fragmentInoutIndices;

        // GL_EXT_geometry_shader.
        uint8_t pad0;
        PrimitiveMode geometryShaderInputPrimitiveType;
        PrimitiveMode geometryShaderOutputPrimitiveType;
        int32_t geometryShaderInvocations;
        int32_t geometryShaderMaxVertices;
        GLenum transformFeedbackBufferMode;

        // 4 bytes each. GL_OVR_multiview / GL_OVR_multiview2
        int32_t numViews;
        // GL_ANGLE_multi_draw
        int32_t drawIDLocation;

        // GL_ANGLE_base_vertex_base_instance_shader_builtin
        int32_t baseVertexLocation;
        int32_t baseInstanceLocation;

        // GL_EXT_tessellation_shader
        int32_t tessControlShaderVertices;
        GLenum tessGenMode;
        GLenum tessGenSpacing;
        GLenum tessGenVertexOrder;
        GLenum tessGenPointMode;

        // 4 bytes
        rx::SpecConstUsageBits specConstUsageBits;

        // 24 bytes
        ShaderMap<int> linkedShaderVersions;
    } mPod;
    ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

    // Cached mask of active samplers and sampler types.
    ActiveTextureMask mActiveSamplersMask;
    ActiveTextureArray<uint32_t> mActiveSamplerRefCounts;
    ActiveTextureArray<TextureType> mActiveSamplerTypes;
    ActiveTextureMask mActiveSamplerYUV;
    ActiveTextureArray<SamplerFormat> mActiveSamplerFormats;
    ActiveTextureArray<ShaderBitSet> mActiveSamplerShaderBits;

    // Cached mask of active images.
    ActiveTextureMask mActiveImagesMask;
    ActiveTextureArray<ShaderBitSet> mActiveImageShaderBits;

    // Names and mapped names of output variables that are arrays include [0] in the end, similarly
    // to uniforms.
    std::vector<ProgramOutput> mOutputVariables;
    std::vector<VariableLocation> mOutputLocations;
    // EXT_blend_func_extended secondary outputs (ones with index 1)
    std::vector<VariableLocation> mSecondaryOutputLocations;
    // Vertex attributes, Fragment input varyings, etc.
    std::vector<ProgramInput> mProgramInputs;
    std::vector<TransformFeedbackVarying> mLinkedTransformFeedbackVaryings;
    // Duplicate of ProgramState::mTransformFeedbackVaryingNames.  This is cached here because the
    // xfb names may change, relink may fail, yet program pipeline link should be able to function
    // with the last installed executable.  In truth, program pipeline link should have been able to
    // hoist transform feedback varyings directly from the executable, among most other things, but
    // that is currently not done.
    //
    // This array is not serialized, it's already done by the program, and will be duplicated during
    // deserialization.
    std::vector<std::string> mTransformFeedbackVaryingNames;
    // The size of the data written to each transform feedback buffer per vertex.
    std::vector<GLsizei> mTransformFeedbackStrides;
    // Uniforms are sorted in order:
    //  1. Non-opaque uniforms
    //  2. Sampler uniforms
    //  3. Image uniforms
    //  4. Atomic counter uniforms
    //  5. Uniform block uniforms
    // This makes opaque uniform validation easier, since we don't need a separate list.
    // For generating the entries and naming them we follow the spec: GLES 3.1 November 2016 section
    // 7.3.1.1 Naming Active Resources. There's a separate entry for each struct member and each
    // inner array of an array of arrays. Names and mapped names of uniforms that are arrays include
    // [0] in the end. This makes implementation of queries simpler.
    std::vector<LinkedUniform> mUniforms;
    std::vector<std::string> mUniformNames;
    // Only used by GL and D3D backend
    std::vector<std::string> mUniformMappedNames;
    std::vector<InterfaceBlock> mUniformBlocks;
    std::vector<VariableLocation> mUniformLocations;

    std::vector<AtomicCounterBuffer> mAtomicCounterBuffers;
    std::vector<InterfaceBlock> mShaderStorageBlocks;
    std::vector<BufferVariable> mBufferVariables;

    // An array of the samplers that are used by the program
    std::vector<SamplerBinding> mSamplerBindings;
    // List of all textures bound to all samplers. Each SamplerBinding will point to a subset in
    // this vector.
    std::vector<GLuint> mSamplerBoundTextureUnits;

    // An array of the images that are used by the program
    std::vector<ImageBinding> mImageBindings;

    // ANGLE_shader_pixel_local_storage: A mapping from binding index to the PLS uniform format at
    // that index.
    std::vector<ShPixelLocalStorageFormat> mPixelLocalStorageFormats;

    ShaderMap<std::vector<sh::ShaderVariable>> mLinkedOutputVaryings;
    ShaderMap<std::vector<sh::ShaderVariable>> mLinkedInputVaryings;
    ShaderMap<std::vector<sh::ShaderVariable>> mLinkedUniforms;
    ShaderMap<std::vector<sh::InterfaceBlock>> mLinkedUniformBlocks;

    // Cached value of base vertex and base instance
    // need to reset them to zero if using non base vertex or base instance draw calls.
    GLint mCachedBaseVertex;
    GLuint mCachedBaseInstance;

    // GLES allows uniform block indices in the program to be remapped to arbitrary buffer bindings
    // through calls to glUniformBlockBinding.  (Desktop GL also includes
    // glShaderStorageBlockBinding, which does not exist in GLES).
    // This is not a part of the link results, and must be reset on glProgramBinary, so it's not
    // serialized.
    // A map from the program uniform block index to the buffer binding it is mapped to.
    ProgramUniformBlockArray<GLuint> mUniformBlockIndexToBufferBinding;
    // The reverse of the above map, i.e. from buffer bindings to the uniform blocks that are mapped
    // to it.  For example, if the program's uniform blocks 1, 3 and 4 are mapped to buffer binding
    // 2, then mUniformBufferBindingToUniformBlocks[2] will be {1, 3, 4}.
    //
    // This is used to efficiently mark uniform blocks dirty when a buffer bound to a binding has
    // been modified.
    UniformBufferBindingArray<ProgramUniformBlockMask> mUniformBufferBindingToUniformBlocks;

    // PPO only: installed executables from the programs.  Note that these may be different from the
    // programs' current executables, because they may have been unsuccessfully relinked.
    ShaderMap<SharedProgramExecutable> mPPOProgramExecutables;
    // Flag for an easy check for PPO without inspecting mPPOProgramExecutables
    bool mIsPPO;

    // Cache for sampler validation
    mutable Optional<bool> mCachedValidateSamplersResult;

    // Post-link subtask and wait events
    // These tasks are not waited on in |resolveLink|, but instead they are free to
    // run until first usage of the program (or relink).  This is used by the backends (currently
    // only Vulkan) to run post-link optimization tasks which don't affect the link results.
    mutable std::vector<std::shared_ptr<rx::LinkSubTask>> mPostLinkSubTasks;
    mutable std::vector<std::shared_ptr<angle::WaitableEvent>> mPostLinkSubTaskWaitableEvents;
};

void InstallExecutable(const Context *context,
                       const SharedProgramExecutable &toInstall,
                       SharedProgramExecutable *executable);
void UninstallExecutable(const Context *context, SharedProgramExecutable *executable);
}  // namespace gl

#endif  // LIBANGLE_PROGRAMEXECUTABLE_H_
