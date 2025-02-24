//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.cpp: Implements the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#include "libANGLE/ProgramPipeline.h"

#include <algorithm>

#include "libANGLE/Context.h"
#include "libANGLE/Program.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace gl
{

ProgramPipelineState::ProgramPipelineState(rx::GLImplFactory *factory)
    : mLabel(),
      mActiveShaderProgram(nullptr),
      mValid(false),
      mExecutable(makeNewExecutable(factory, {})),
      mIsLinked(false)
{
    for (const ShaderType shaderType : AllShaderTypes())
    {
        mPrograms[shaderType] = nullptr;
    }
}

ProgramPipelineState::~ProgramPipelineState() {}

const std::string &ProgramPipelineState::getLabel() const
{
    return mLabel;
}

void ProgramPipelineState::activeShaderProgram(Program *shaderProgram)
{
    mActiveShaderProgram = shaderProgram;
}

SharedProgramExecutable ProgramPipelineState::makeNewExecutable(
    rx::GLImplFactory *factory,
    ShaderMap<SharedProgramExecutable> &&ppoProgramExecutables)
{
    SharedProgramExecutable newExecutable = std::make_shared<ProgramExecutable>(factory, &mInfoLog);
    newExecutable->mIsPPO                 = true;
    newExecutable->mPPOProgramExecutables = std::move(ppoProgramExecutables);
    return newExecutable;
}

void ProgramPipelineState::useProgramStage(const Context *context,
                                           const ShaderType shaderType,
                                           Program *shaderProgram,
                                           angle::ObserverBinding *programObserverBinding,
                                           angle::ObserverBinding *programExecutableObserverBinding)
{
    Program *oldProgram = mPrograms[shaderType];
    if (oldProgram)
    {
        oldProgram->release(context);
    }

    // If program refers to a program object with a valid shader attached for the indicated shader
    // stage, glUseProgramStages installs the executable code for that stage in the indicated
    // program pipeline object pipeline.
    if (shaderProgram && (shaderProgram->id().value != 0) &&
        shaderProgram->getExecutable().hasLinkedShaderStage(shaderType))
    {
        mPrograms[shaderType] = shaderProgram;
        // Install the program executable, if not already
        if (shaderProgram->getSharedExecutable().get() !=
            mExecutable->mPPOProgramExecutables[shaderType].get())
        {
            InstallExecutable(context, shaderProgram->getSharedExecutable(),
                              &mExecutable->mPPOProgramExecutables[shaderType]);
        }
        shaderProgram->addRef();
    }
    else
    {
        // If program is zero, or refers to a program object with no valid shader executable for the
        // given stage, it is as if the pipeline object has no programmable stage configured for the
        // indicated shader stage.
        mPrograms[shaderType] = nullptr;
        UninstallExecutable(context, &mExecutable->mPPOProgramExecutables[shaderType]);
    }

    programObserverBinding->bind(mPrograms[shaderType]);
    programExecutableObserverBinding->bind(mExecutable->mPPOProgramExecutables[shaderType].get());
}

void ProgramPipelineState::useProgramStages(
    const Context *context,
    const ShaderBitSet &shaderTypes,
    Program *shaderProgram,
    std::vector<angle::ObserverBinding> *programObserverBindings,
    std::vector<angle::ObserverBinding> *programExecutableObserverBindings)
{
    for (ShaderType shaderType : shaderTypes)
    {
        useProgramStage(context, shaderType, shaderProgram,
                        &programObserverBindings->at(static_cast<size_t>(shaderType)),
                        &programExecutableObserverBindings->at(static_cast<size_t>(shaderType)));
    }
}

bool ProgramPipelineState::usesShaderProgram(ShaderProgramID programId) const
{
    for (const Program *program : mPrograms)
    {
        if (program && (program->id() == programId))
        {
            return true;
        }
    }

    return false;
}

void ProgramPipelineState::updateExecutableTextures()
{
    for (const ShaderType shaderType : mExecutable->getLinkedShaderStages())
    {
        const SharedProgramExecutable &programExecutable = getShaderProgramExecutable(shaderType);
        ASSERT(programExecutable);
        mExecutable->setActiveTextureMask(mExecutable->getActiveSamplersMask() |
                                          programExecutable->getActiveSamplersMask());
        mExecutable->setActiveImagesMask(mExecutable->getActiveImagesMask() |
                                         programExecutable->getActiveImagesMask());
        // Updates mActiveSamplerRefCounts, mActiveSamplerTypes, and mActiveSamplerFormats
        mExecutable->updateActiveSamplers(*programExecutable);
    }
}

void ProgramPipelineState::updateExecutableSpecConstUsageBits()
{
    rx::SpecConstUsageBits specConstUsageBits;
    for (const ShaderType shaderType : mExecutable->getLinkedShaderStages())
    {
        const SharedProgramExecutable &programExecutable = getShaderProgramExecutable(shaderType);
        ASSERT(programExecutable);
        specConstUsageBits |= programExecutable->getSpecConstUsageBits();
    }
    mExecutable->mPod.specConstUsageBits = specConstUsageBits;
}

ProgramPipeline::ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle)
    : RefCountObject(factory->generateSerial(), handle),
      mProgramPipelineImpl(factory->createProgramPipeline(mState)),
      mState(factory)
{
    ASSERT(mProgramPipelineImpl);

    for (const ShaderType shaderType : AllShaderTypes())
    {
        mProgramObserverBindings.emplace_back(this, static_cast<angle::SubjectIndex>(shaderType));
        mProgramExecutableObserverBindings.emplace_back(
            this, static_cast<angle::SubjectIndex>(shaderType));
    }
}

ProgramPipeline::~ProgramPipeline()
{
    mProgramPipelineImpl.reset(nullptr);
}

void ProgramPipeline::onDestroy(const Context *context)
{
    for (Program *program : mState.mPrograms)
    {
        if (program)
        {
            ASSERT(program->getRefCount());
            program->release(context);
        }
    }

    getImplementation()->destroy(context);
    UninstallExecutable(context, &mState.mExecutable);

    mState.destroyDiscardedExecutables(context);
}

void ProgramPipelineState::destroyDiscardedExecutables(const Context *context)
{
    for (SharedProgramExecutable &executable : mProgramExecutablesToDiscard)
    {
        UninstallExecutable(context, &executable);
    }
    mProgramExecutablesToDiscard.clear();
}

angle::Result ProgramPipeline::setLabel(const Context *context, const std::string &label)
{
    mState.mLabel = label;

    if (mProgramPipelineImpl)
    {
        return mProgramPipelineImpl->onLabelUpdate(context);
    }
    return angle::Result::Continue;
}

const std::string &ProgramPipeline::getLabel() const
{
    return mState.mLabel;
}

rx::ProgramPipelineImpl *ProgramPipeline::getImplementation() const
{
    return mProgramPipelineImpl.get();
}

void ProgramPipeline::activeShaderProgram(Program *shaderProgram)
{
    mState.activeShaderProgram(shaderProgram);
}

angle::Result ProgramPipeline::useProgramStages(const Context *context,
                                                GLbitfield stages,
                                                Program *shaderProgram)
{
    ShaderBitSet shaderTypes;
    if (stages != GL_ALL_SHADER_BITS)
    {
        ASSERT(stages < 256u);
        for (size_t singleShaderBit : angle::BitSet<8>(stages))
        {
            // Cast back to a bit after the iterator returns an index.
            ShaderType shaderType = GetShaderTypeFromBitfield(angle::Bit<size_t>(singleShaderBit));
            ASSERT(shaderType != ShaderType::InvalidEnum);
            shaderTypes.set(shaderType);
        }
    }
    else
    {
        shaderTypes.set();
    }
    ASSERT(shaderTypes.any());

    bool needToUpdatePipelineState = false;
    for (ShaderType shaderType : shaderTypes)
    {
        if (mState.getShaderProgram(shaderType) != shaderProgram ||
            (shaderProgram &&
             getShaderProgramExecutable(shaderType) != shaderProgram->getSharedExecutable()))
        {
            needToUpdatePipelineState = true;
            break;
        }
    }

    if (!needToUpdatePipelineState)
    {
        return angle::Result::Continue;
    }

    mState.useProgramStages(context, shaderTypes, shaderProgram, &mProgramObserverBindings,
                            &mProgramExecutableObserverBindings);

    mState.mIsLinked = false;
    onStateChange(angle::SubjectMessage::ProgramUnlinked);

    return angle::Result::Continue;
}

void ProgramPipeline::updateLinkedShaderStages()
{
    mState.mExecutable->resetLinkedShaderStages();

    for (const ShaderType shaderType : AllShaderTypes())
    {
        if (getShaderProgramExecutable(shaderType))
        {
            mState.mExecutable->setLinkedShaderStages(shaderType);
        }
    }

    mState.mExecutable->updateCanDrawWith();
}

void ProgramPipeline::updateExecutableAttributes()
{
    const SharedProgramExecutable &vertexExecutable =
        getShaderProgramExecutable(ShaderType::Vertex);

    if (!vertexExecutable)
    {
        return;
    }

    mState.mExecutable->mPod.activeAttribLocationsMask =
        vertexExecutable->mPod.activeAttribLocationsMask;
    mState.mExecutable->mPod.maxActiveAttribLocation =
        vertexExecutable->mPod.maxActiveAttribLocation;
    mState.mExecutable->mPod.attributesTypeMask = vertexExecutable->mPod.attributesTypeMask;
    mState.mExecutable->mPod.attributesMask     = vertexExecutable->mPod.attributesMask;
    mState.mExecutable->mProgramInputs          = vertexExecutable->mProgramInputs;

    mState.mExecutable->mPod.numViews             = vertexExecutable->mPod.numViews;
    mState.mExecutable->mPod.drawIDLocation       = vertexExecutable->mPod.drawIDLocation;
    mState.mExecutable->mPod.baseVertexLocation   = vertexExecutable->mPod.baseVertexLocation;
    mState.mExecutable->mPod.baseInstanceLocation = vertexExecutable->mPod.baseInstanceLocation;
}

void ProgramPipeline::updateTransformFeedbackMembers()
{
    ShaderType lastVertexProcessingStage =
        GetLastPreFragmentStage(getExecutable().getLinkedShaderStages());
    if (lastVertexProcessingStage == ShaderType::InvalidEnum)
    {
        return;
    }

    const SharedProgramExecutable &lastPreFragmentExecutable =
        getShaderProgramExecutable(lastVertexProcessingStage);
    ASSERT(lastPreFragmentExecutable);

    mState.mExecutable->mTransformFeedbackStrides =
        lastPreFragmentExecutable->mTransformFeedbackStrides;
    mState.mExecutable->mLinkedTransformFeedbackVaryings =
        lastPreFragmentExecutable->mLinkedTransformFeedbackVaryings;
}

void ProgramPipeline::updateShaderStorageBlocks()
{
    mState.mExecutable->mShaderStorageBlocks.clear();

    // Only copy the storage blocks from each Program in the PPO once, since each Program could
    // contain multiple shader stages.
    ShaderBitSet handledStages;

    for (const ShaderType shaderType : AllShaderTypes())
    {
        const SharedProgramExecutable &programExecutable = getShaderProgramExecutable(shaderType);
        if (programExecutable && !handledStages.test(shaderType))
        {
            // Only add each Program's blocks once.
            handledStages |= programExecutable->getLinkedShaderStages();

            for (const InterfaceBlock &block : programExecutable->getShaderStorageBlocks())
            {
                mState.mExecutable->mShaderStorageBlocks.emplace_back(block);
            }
        }
    }
}

void ProgramPipeline::updateImageBindings()
{
    mState.mExecutable->mImageBindings.clear();
    mState.mExecutable->mActiveImageShaderBits.fill({});

    // Only copy the storage blocks from each Program in the PPO once, since each Program could
    // contain multiple shader stages.
    ShaderBitSet handledStages;

    for (const ShaderType shaderType : AllShaderTypes())
    {
        const SharedProgramExecutable &programExecutable = getShaderProgramExecutable(shaderType);
        if (programExecutable && !handledStages.test(shaderType))
        {
            // Only add each Program's blocks once.
            handledStages |= programExecutable->getLinkedShaderStages();

            for (const ImageBinding &imageBinding : *programExecutable->getImageBindings())
            {
                mState.mExecutable->mImageBindings.emplace_back(imageBinding);
            }

            mState.mExecutable->updateActiveImages(*programExecutable);
        }
    }
}

void ProgramPipeline::updateExecutableGeometryProperties()
{
    const SharedProgramExecutable &geometryExecutable =
        getShaderProgramExecutable(ShaderType::Geometry);

    if (!geometryExecutable)
    {
        return;
    }

    mState.mExecutable->mPod.geometryShaderInputPrimitiveType =
        geometryExecutable->mPod.geometryShaderInputPrimitiveType;
    mState.mExecutable->mPod.geometryShaderOutputPrimitiveType =
        geometryExecutable->mPod.geometryShaderOutputPrimitiveType;
    mState.mExecutable->mPod.geometryShaderInvocations =
        geometryExecutable->mPod.geometryShaderInvocations;
    mState.mExecutable->mPod.geometryShaderMaxVertices =
        geometryExecutable->mPod.geometryShaderMaxVertices;
}

void ProgramPipeline::updateExecutableTessellationProperties()
{
    const SharedProgramExecutable &tessControlExecutable =
        getShaderProgramExecutable(ShaderType::TessControl);
    const SharedProgramExecutable &tessEvalExecutable =
        getShaderProgramExecutable(ShaderType::TessEvaluation);

    if (tessControlExecutable)
    {
        mState.mExecutable->mPod.tessControlShaderVertices =
            tessControlExecutable->mPod.tessControlShaderVertices;
    }

    if (tessEvalExecutable)
    {
        mState.mExecutable->mPod.tessGenMode        = tessEvalExecutable->mPod.tessGenMode;
        mState.mExecutable->mPod.tessGenSpacing     = tessEvalExecutable->mPod.tessGenSpacing;
        mState.mExecutable->mPod.tessGenVertexOrder = tessEvalExecutable->mPod.tessGenVertexOrder;
        mState.mExecutable->mPod.tessGenPointMode   = tessEvalExecutable->mPod.tessGenPointMode;
    }
}

void ProgramPipeline::updateFragmentInoutRangeAndEnablesPerSampleShading()
{
    const SharedProgramExecutable &fragmentExecutable =
        getShaderProgramExecutable(ShaderType::Fragment);

    if (!fragmentExecutable)
    {
        return;
    }

    mState.mExecutable->mPod.fragmentInoutIndices = fragmentExecutable->mPod.fragmentInoutIndices;
    mState.mExecutable->mPod.hasDiscard           = fragmentExecutable->mPod.hasDiscard;
    mState.mExecutable->mPod.enablesPerSampleShading =
        fragmentExecutable->mPod.enablesPerSampleShading;
    mState.mExecutable->mPod.hasDepthInputAttachment =
        fragmentExecutable->mPod.hasDepthInputAttachment;
    mState.mExecutable->mPod.hasStencilInputAttachment =
        fragmentExecutable->mPod.hasStencilInputAttachment;
}

void ProgramPipeline::updateLinkedVaryings()
{
    // Need to check all of the shader stages, not just linked, so we handle Compute correctly.
    for (const ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        const SharedProgramExecutable &programExecutable = getShaderProgramExecutable(shaderType);
        if (programExecutable)
        {
            mState.mExecutable->mLinkedOutputVaryings[shaderType] =
                programExecutable->getLinkedOutputVaryings(shaderType);
            mState.mExecutable->mLinkedInputVaryings[shaderType] =
                programExecutable->getLinkedInputVaryings(shaderType);
        }
    }

    const SharedProgramExecutable &computeExecutable =
        getShaderProgramExecutable(ShaderType::Compute);
    if (computeExecutable)
    {
        mState.mExecutable->mLinkedOutputVaryings[ShaderType::Compute] =
            computeExecutable->getLinkedOutputVaryings(ShaderType::Compute);
        mState.mExecutable->mLinkedInputVaryings[ShaderType::Compute] =
            computeExecutable->getLinkedInputVaryings(ShaderType::Compute);
    }
}

void ProgramPipeline::updateExecutable()
{
    // Vertex Shader ProgramExecutable properties
    updateExecutableAttributes();
    updateTransformFeedbackMembers();
    updateShaderStorageBlocks();
    updateImageBindings();

    // Geometry Shader ProgramExecutable properties
    updateExecutableGeometryProperties();

    // Tessellation Shaders ProgramExecutable properties
    updateExecutableTessellationProperties();

    // Fragment Shader ProgramExecutable properties
    updateFragmentInoutRangeAndEnablesPerSampleShading();

    // All Shader ProgramExecutable properties
    mState.updateExecutableTextures();
    mState.updateExecutableSpecConstUsageBits();
    updateLinkedVaryings();
}

void ProgramPipeline::resolveAttachedPrograms(const Context *context)
{
    // Wait for attached programs to finish linking
    for (Program *program : mState.mPrograms)
    {
        if (program != nullptr)
        {
            program->resolveLink(context);
        }
    }
}

// The attached shaders are checked for linking errors by matching up their variables.
// Uniform, input and output variables get collected.
// The code gets compiled into binaries.
angle::Result ProgramPipeline::link(const Context *context)
{
    mState.destroyDiscardedExecutables(context);

    // Make a new executable to hold the result of the link.
    SharedProgramExecutable newExecutable = mState.makeNewExecutable(
        context->getImplementation(), std::move(mState.mExecutable->mPPOProgramExecutables));

    InstallExecutable(context, newExecutable, &mState.mExecutable);
    onStateChange(angle::SubjectMessage::ProgramUnlinked);

    updateLinkedShaderStages();

    ASSERT(!mState.mIsLinked);

    ProgramMergedVaryings mergedVaryings;
    ProgramVaryingPacking varyingPacking;
    LinkingVariables linkingVariables;

    mState.mInfoLog.reset();

    linkingVariables.initForProgramPipeline(mState);

    const Caps &caps               = context->getCaps();
    const Limitations &limitations = context->getLimitations();
    const Version &clientVersion   = context->getClientVersion();
    const bool isWebGL             = context->isWebGL();

    if (mState.mExecutable->hasLinkedShaderStage(gl::ShaderType::TessControl) !=
        mState.mExecutable->hasLinkedShaderStage(gl::ShaderType::TessEvaluation))
    {
        return angle::Result::Stop;
    }

    if (mState.mExecutable->hasLinkedShaderStage(ShaderType::Vertex))
    {
        if (!linkVaryings())
        {
            return angle::Result::Stop;
        }

        if (!LinkValidateProgramGlobalNames(mState.mInfoLog, getExecutable(), linkingVariables))
        {
            return angle::Result::Stop;
        }

        const SharedProgramExecutable &fragmentExecutable =
            getShaderProgramExecutable(ShaderType::Fragment);
        if (fragmentExecutable)
        {
            // We should also be validating SSBO and image uniform counts.
            const GLuint combinedImageUniforms       = 0;
            const GLuint combinedShaderStorageBlocks = 0;
            ASSERT(mState.mExecutable->mOutputVariables.empty());
            mState.mExecutable->mOutputVariables = fragmentExecutable->getOutputVariables();
            if (!mState.mExecutable->linkValidateOutputVariables(
                    caps, clientVersion, combinedImageUniforms, combinedShaderStorageBlocks,
                    fragmentExecutable->getLinkedShaderVersion(ShaderType::Fragment),
                    ProgramAliasedBindings(), ProgramAliasedBindings()))
            {
                return angle::Result::Continue;
            }
        }
        mergedVaryings = GetMergedVaryingsFromLinkingVariables(linkingVariables);
        // If separable program objects are in use, the set of attributes captured is taken
        // from the program object active on the last vertex processing stage.
        ShaderType lastVertexProcessingStage =
            GetLastPreFragmentStage(getExecutable().getLinkedShaderStages());
        if (lastVertexProcessingStage == ShaderType::InvalidEnum)
        {
            //  If there is no active program for the vertex or fragment shader stages, the results
            //  of vertex and fragment shader execution will respectively be undefined. However,
            //  this is not an error.
            return angle::Result::Continue;
        }

        const SharedProgramExecutable *tfExecutable =
            &getShaderProgramExecutable(lastVertexProcessingStage);
        ASSERT(*tfExecutable);

        if (!*tfExecutable)
        {
            tfExecutable = &getShaderProgramExecutable(ShaderType::Vertex);
        }

        mState.mExecutable->mTransformFeedbackVaryingNames =
            (*tfExecutable)->mTransformFeedbackVaryingNames;
        mState.mExecutable->mPod.transformFeedbackBufferMode =
            (*tfExecutable)->mPod.transformFeedbackBufferMode;

        if (!mState.mExecutable->linkMergedVaryings(caps, limitations, clientVersion, isWebGL,
                                                    mergedVaryings, linkingVariables,
                                                    &varyingPacking))
        {
            return angle::Result::Stop;
        }
    }

    // Merge uniforms.
    mState.mExecutable->copyUniformsFromProgramMap(mState.mExecutable->mPPOProgramExecutables);

    if (mState.mExecutable->hasLinkedShaderStage(ShaderType::Vertex))
    {
        const SharedProgramExecutable &executable = getShaderProgramExecutable(ShaderType::Vertex);
        mState.mExecutable->copyInputsFromProgram(*executable);
    }

    // Merge shader buffers (UBOs, SSBOs, and atomic counter buffers) into the executable.
    // Also copy over image and sampler bindings.
    for (ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
    {
        const SharedProgramExecutable &executable = getShaderProgramExecutable(shaderType);
        mState.mExecutable->copyUniformBuffersFromProgram(*executable, shaderType,
                                                          &mState.mUniformBlockMap[shaderType]);
        mState.mExecutable->copyStorageBuffersFromProgram(*executable, shaderType);
        mState.mExecutable->copySamplerBindingsFromProgram(*executable);
        mState.mExecutable->copyImageBindingsFromProgram(*executable);
    }

    if (mState.mExecutable->hasLinkedShaderStage(ShaderType::Fragment))
    {
        const SharedProgramExecutable &executable =
            getShaderProgramExecutable(ShaderType::Fragment);
        mState.mExecutable->copyOutputsFromProgram(*executable);
    }

    mState.mExecutable->mActiveSamplerRefCounts.fill(0);
    updateExecutable();

    if (mState.mExecutable->hasLinkedShaderStage(ShaderType::Vertex) ||
        mState.mExecutable->hasLinkedShaderStage(ShaderType::Compute))
    {
        ANGLE_TRY(getImplementation()->link(context, mergedVaryings, varyingPacking));
    }

    mState.mIsLinked = true;
    onStateChange(angle::SubjectMessage::ProgramRelinked);

    return angle::Result::Continue;
}

int ProgramPipeline::getInfoLogLength() const
{
    return static_cast<int>(mState.mInfoLog.getLength());
}

void ProgramPipeline::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    return mState.mInfoLog.getLog(bufSize, length, infoLog);
}

bool ProgramPipeline::linkVaryings()
{
    ShaderType previousShaderType = ShaderType::InvalidEnum;
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        const SharedProgramExecutable &programExecutable = getShaderProgramExecutable(shaderType);
        if (!programExecutable)
        {
            continue;
        }

        if (previousShaderType != ShaderType::InvalidEnum)
        {
            const SharedProgramExecutable &previousExecutable =
                getShaderProgramExecutable(previousShaderType);
            ASSERT(previousExecutable);

            if (!LinkValidateShaderInterfaceMatching(
                    previousExecutable->getLinkedOutputVaryings(previousShaderType),
                    programExecutable->getLinkedInputVaryings(shaderType), previousShaderType,
                    shaderType, previousExecutable->getLinkedShaderVersion(previousShaderType),
                    programExecutable->getLinkedShaderVersion(shaderType), true, mState.mInfoLog))
            {
                return false;
            }
        }
        previousShaderType = shaderType;
    }

    // TODO: http://anglebug.com/42262233 and http://anglebug.com/42262234
    // Need to move logic of validating builtin varyings inside the for-loop above.
    // This is because the built-in symbols `gl_ClipDistance` and `gl_CullDistance`
    // can be redeclared in Geometry or Tessellation shaders as well.
    const SharedProgramExecutable &vertexExecutable =
        getShaderProgramExecutable(ShaderType::Vertex);
    const SharedProgramExecutable &fragmentExecutable =
        getShaderProgramExecutable(ShaderType::Fragment);
    if (!vertexExecutable || !fragmentExecutable)
    {
        return true;
    }
    return LinkValidateBuiltInVaryings(
        vertexExecutable->getLinkedOutputVaryings(ShaderType::Vertex),
        fragmentExecutable->getLinkedInputVaryings(ShaderType::Fragment), ShaderType::Vertex,
        ShaderType::Fragment, vertexExecutable->getLinkedShaderVersion(ShaderType::Vertex),
        fragmentExecutable->getLinkedShaderVersion(ShaderType::Fragment), mState.mInfoLog);
}

void ProgramPipeline::validate(const Context *context)
{
    updateLinkedShaderStages();

    const Caps &caps = context->getCaps();
    mState.mValid    = true;
    mState.mInfoLog.reset();

    if (mState.mExecutable->hasLinkedShaderStage(gl::ShaderType::TessControl) !=
        mState.mExecutable->hasLinkedShaderStage(gl::ShaderType::TessEvaluation))
    {
        mState.mValid = false;
        mState.mInfoLog << "Program pipeline must have both a Tessellation Control and Evaluation "
                           "shader or neither\n";
        return;
    }

    for (const ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
    {
        Program *shaderProgram = mState.mPrograms[shaderType];
        if (shaderProgram)
        {
            shaderProgram->resolveLink(context);
            shaderProgram->validate(caps);
            std::string shaderInfoString = shaderProgram->getExecutable().getInfoLogString();
            if (shaderInfoString.length())
            {
                mState.mValid = false;
                mState.mInfoLog << shaderInfoString << "\n";
                return;
            }
            if (!shaderProgram->isSeparable())
            {
                mState.mValid = false;
                mState.mInfoLog << GetShaderTypeString(shaderType) << " is not marked separable."
                                << "\n";
                return;
            }
        }
    }

    intptr_t programPipelineError = context->getStateCache().getProgramPipelineError(context);
    if (programPipelineError)
    {
        mState.mValid            = false;
        const char *errorMessage = reinterpret_cast<const char *>(programPipelineError);
        mState.mInfoLog << errorMessage << "\n";
        return;
    }

    if (!linkVaryings())
    {
        mState.mValid = false;

        for (const ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
        {
            Program *shaderProgram = mState.mPrograms[shaderType];
            ASSERT(shaderProgram);
            shaderProgram->validate(caps);
            std::string shaderInfoString = shaderProgram->getExecutable().getInfoLogString();
            if (shaderInfoString.length())
            {
                mState.mInfoLog << shaderInfoString << "\n";
            }
        }
    }
}

void ProgramPipeline::onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message)
{
    switch (message)
    {
        case angle::SubjectMessage::ProgramTextureOrImageBindingChanged:
            mState.mExecutable->mActiveSamplerRefCounts.fill(0);
            mState.updateExecutableTextures();
            break;

        case angle::SubjectMessage::ProgramUnlinked:
            // A used program is being relinked.  Ensure next usage of the PPO resolve the program
            // link and relinks the PPO.
            mState.mIsLinked = false;
            onStateChange(angle::SubjectMessage::ProgramUnlinked);
            break;

        case angle::SubjectMessage::ProgramRelinked:
        {
            ShaderType shaderType = static_cast<ShaderType>(index);
            ASSERT(mState.mPrograms[shaderType] != nullptr);
            ASSERT(mState.mExecutable->mPPOProgramExecutables[shaderType]);

            mState.mIsLinked = false;
            mState.mProgramExecutablesToDiscard.emplace_back(
                std::move(mState.mExecutable->mPPOProgramExecutables[shaderType]));
            mState.mExecutable->mPPOProgramExecutables[shaderType] =
                mState.mPrograms[shaderType]->getSharedExecutable();

            break;
        }
        case angle::SubjectMessage::SamplerUniformsUpdated:
            mState.mExecutable->clearSamplerBindings();
            for (ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
            {
                const SharedProgramExecutable &executable = getShaderProgramExecutable(shaderType);
                mState.mExecutable->copySamplerBindingsFromProgram(*executable);
            }
            mState.mExecutable->mActiveSamplerRefCounts.fill(0);
            mState.updateExecutableTextures();
            break;
        default:
            if (angle::IsProgramUniformBlockBindingUpdatedMessage(message))
            {
                if (mState.mIsLinked)
                {
                    ShaderType shaderType = static_cast<ShaderType>(index);
                    const SharedProgramExecutable &executable =
                        getShaderProgramExecutable(shaderType);
                    const GLuint blockIndex =
                        angle::ProgramUniformBlockBindingUpdatedMessageToIndex(message);
                    if (executable->getUniformBlocks()[blockIndex].isActive(shaderType))
                    {
                        const uint32_t blockIndexInPPO =
                            mState.mUniformBlockMap[shaderType][blockIndex];
                        ASSERT(blockIndexInPPO < mState.mExecutable->mUniformBlocks.size());

                        // Set the block buffer binding in the PPO to the same binding as the
                        // program's.
                        mState.mExecutable->remapUniformBlockBinding(
                            {blockIndexInPPO}, executable->getUniformBlockBinding(blockIndex));

                        // Notify the contexts that the block binding has changed.
                        onStateChange(angle::ProgramUniformBlockBindingUpdatedMessageFromIndex(
                            blockIndexInPPO));
                    }
                }
                break;
            }
            UNREACHABLE();
            break;
    }
}
}  // namespace gl
