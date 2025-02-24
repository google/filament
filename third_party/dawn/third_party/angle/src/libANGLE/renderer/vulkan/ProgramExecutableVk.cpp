//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutableVk.cpp: Collects the information and interfaces common to both ProgramVks and
// ProgramPipelineVks in order to execute/draw with either.

#include "libANGLE/renderer/vulkan/ProgramExecutableVk.h"

#include "common/string_utils.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/ProgramPipelineVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
namespace
{

// Limit decompressed vulkan pipelines to 10MB per program.
static constexpr size_t kMaxLocalPipelineCacheSize = 10 * 1024 * 1024;

bool ValidateTransformedSpirV(vk::ErrorContext *context,
                              const gl::ShaderBitSet &linkedShaderStages,
                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                              const gl::ShaderMap<angle::spirv::Blob> &spirvBlobs)
{
    gl::ShaderType lastPreFragmentStage = gl::GetLastPreFragmentStage(linkedShaderStages);

    for (gl::ShaderType shaderType : linkedShaderStages)
    {
        SpvTransformOptions options;
        options.shaderType = shaderType;
        options.isLastPreFragmentStage =
            shaderType == lastPreFragmentStage && shaderType != gl::ShaderType::TessControl;
        options.isTransformFeedbackStage = options.isLastPreFragmentStage;
        options.useSpirvVaryingPrecisionFixer =
            context->getFeatures().varyingsRequireMatchingPrecisionInSpirv.enabled;

        angle::spirv::Blob transformed;
        if (SpvTransformSpirvCode(options, variableInfoMap, spirvBlobs[shaderType], &transformed) !=
            angle::Result::Continue)
        {
            return false;
        }
    }
    return true;
}

uint32_t GetInterfaceBlockArraySize(const std::vector<gl::InterfaceBlock> &blocks,
                                    uint32_t bufferIndex)
{
    const gl::InterfaceBlock &block = blocks[bufferIndex];

    if (!block.pod.isArray)
    {
        return 1;
    }

    ASSERT(block.pod.arrayElement == 0);

    // Search consecutively until all array indices of this block are visited.
    uint32_t arraySize;
    for (arraySize = 1; bufferIndex + arraySize < blocks.size(); ++arraySize)
    {
        const gl::InterfaceBlock &nextBlock = blocks[bufferIndex + arraySize];

        if (nextBlock.pod.arrayElement != arraySize)
        {
            break;
        }

        // It's unexpected for an array to start at a non-zero array size, so we can always rely on
        // the sequential `arrayElement`s to belong to the same block.
        ASSERT(nextBlock.name == block.name);
        ASSERT(nextBlock.pod.isArray);
    }

    return arraySize;
}

void SetupDefaultPipelineState(const vk::ErrorContext *context,
                               const gl::ProgramExecutable &glExecutable,
                               gl::PrimitiveMode mode,
                               vk::PipelineRobustness pipelineRobustness,
                               vk::PipelineProtectedAccess pipelineProtectedAccess,
                               vk::GraphicsPipelineSubset subset,
                               vk::GraphicsPipelineDesc *graphicsPipelineDescOut)
{
    graphicsPipelineDescOut->initDefaults(context, vk::GraphicsPipelineSubset::Complete,
                                          pipelineRobustness, pipelineProtectedAccess);

    // Set render pass state, affecting both complete and shaders-only pipelines.
    graphicsPipelineDescOut->setTopology(mode);
    graphicsPipelineDescOut->setRenderPassSampleCount(1);
    graphicsPipelineDescOut->setRenderPassFramebufferFetchMode(
        vk::GetProgramFramebufferFetchMode(&glExecutable));

    const std::vector<gl::ProgramOutput> &outputVariables    = glExecutable.getOutputVariables();
    const std::vector<gl::VariableLocation> &outputLocations = glExecutable.getOutputLocations();

    gl::DrawBufferMask drawBuffers;

    for (const gl::VariableLocation &outputLocation : outputLocations)
    {
        if (outputLocation.arrayIndex == 0 && outputLocation.used() && !outputLocation.ignored)
        {
            const gl::ProgramOutput &outputVar = outputVariables[outputLocation.index];

            if (angle::BeginsWith(outputVar.name, "gl_") && outputVar.name != "gl_FragColor")
            {
                continue;
            }

            uint32_t location = 0;
            if (outputVar.pod.location != -1)
            {
                location = outputVar.pod.location;
            }

            GLenum type            = gl::VariableComponentType(outputVar.pod.type);
            angle::FormatID format = angle::FormatID::R8G8B8A8_UNORM;
            if (type == GL_INT)
            {
                format = angle::FormatID::R8G8B8A8_SINT;
            }
            else if (type == GL_UNSIGNED_INT)
            {
                format = angle::FormatID::R8G8B8A8_UINT;
            }

            const size_t arraySize = outputVar.isArray() ? outputVar.getOutermostArraySize() : 1;
            for (size_t arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex)
            {
                graphicsPipelineDescOut->setRenderPassColorAttachmentFormat(location + arrayIndex,
                                                                            format);
                drawBuffers.set(location + arrayIndex);
            }
        }
    }

    for (const gl::ProgramOutput &outputVar : outputVariables)
    {
        if (outputVar.name == "gl_FragColor" || outputVar.name == "gl_FragData")
        {
            const size_t arraySize = outputVar.isArray() ? outputVar.getOutermostArraySize() : 1;
            for (size_t arrayIndex = 0; arrayIndex < arraySize; ++arrayIndex)
            {
                graphicsPipelineDescOut->setRenderPassColorAttachmentFormat(
                    arrayIndex, angle::FormatID::R8G8B8A8_UNORM);
                drawBuffers.set(arrayIndex);
            }
        }
    }

    if (subset == vk::GraphicsPipelineSubset::Complete)
    {
        // Include vertex input state
        graphicsPipelineDescOut->setVertexShaderComponentTypes(
            glExecutable.getNonBuiltinAttribLocationsMask(), glExecutable.getAttributesTypeMask());

        // Include fragment output state
        gl::BlendStateExt::ColorMaskStorage::Type colorMask =
            gl::BlendStateExt::ColorMaskStorage::GetReplicatedValue(
                gl::BlendStateExt::PackColorMask(true, true, true, true),
                gl::BlendStateExt::ColorMaskStorage::GetMask(gl::IMPLEMENTATION_MAX_DRAW_BUFFERS));
        graphicsPipelineDescOut->setColorWriteMasks(colorMask, {}, drawBuffers);
    }
}

void GetPipelineCacheData(ContextVk *contextVk,
                          const vk::PipelineCache &pipelineCache,
                          angle::MemoryBuffer *cacheDataOut)
{
    ASSERT(pipelineCache.valid() || contextVk->getState().isGLES1() ||
           !contextVk->getFeatures().warmUpPipelineCacheAtLink.enabled ||
           !contextVk->getFeatures().hasEffectivePipelineCacheSerialization.enabled);
    if (!pipelineCache.valid() ||
        !contextVk->getFeatures().hasEffectivePipelineCacheSerialization.enabled)
    {
        return;
    }

    // Extract the pipeline data.  If failed, or empty, it's simply not stored on disk.
    size_t pipelineCacheSize = 0;
    VkResult result =
        pipelineCache.getCacheData(contextVk->getDevice(), &pipelineCacheSize, nullptr);
    if (result != VK_SUCCESS || pipelineCacheSize == 0)
    {
        return;
    }

    if (contextVk->getFeatures().enablePipelineCacheDataCompression.enabled)
    {
        std::vector<uint8_t> pipelineCacheData(pipelineCacheSize);
        result = pipelineCache.getCacheData(contextVk->getDevice(), &pipelineCacheSize,
                                            pipelineCacheData.data());
        if (result != VK_SUCCESS && result != VK_INCOMPLETE)
        {
            return;
        }

        // Compress it.
        if (!angle::CompressBlob(pipelineCacheData.size(), pipelineCacheData.data(), cacheDataOut))
        {
            cacheDataOut->clear();
        }
    }
    else
    {
        if (!cacheDataOut->resize(pipelineCacheSize))
        {
            ERR() << "Failed to allocate memory for pipeline cache data.";
            return;
        }
        result = pipelineCache.getCacheData(contextVk->getDevice(), &pipelineCacheSize,
                                            cacheDataOut->data());
        if (result != VK_SUCCESS && result != VK_INCOMPLETE)
        {
            cacheDataOut->clear();
        }
    }
}

vk::SpecializationConstants MakeSpecConsts(ProgramTransformOptions transformOptions,
                                           const vk::GraphicsPipelineDesc &desc)
{
    vk::SpecializationConstants specConsts;

    specConsts.surfaceRotation = transformOptions.surfaceRotation;
    specConsts.dither          = desc.getEmulatedDitherControl();

    return specConsts;
}

vk::GraphicsPipelineSubset GetWarmUpSubset(const angle::FeaturesVk &features)
{
    // Only build the shaders subset of the pipeline if VK_EXT_graphics_pipeline_library is
    // supported.
    return features.supportsGraphicsPipelineLibrary.enabled ? vk::GraphicsPipelineSubset::Shaders
                                                            : vk::GraphicsPipelineSubset::Complete;
}

angle::Result UpdateFullTexturesDescriptorSet(vk::ErrorContext *context,
                                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                              const vk::WriteDescriptorDescs &writeDescriptorDescs,
                                              UpdateDescriptorSetsBuilder *updateBuilder,
                                              const gl::ProgramExecutable &executable,
                                              const gl::ActiveTextureArray<TextureVk *> &textures,
                                              const gl::SamplerBindingVector &samplers,
                                              VkDescriptorSet descriptorSet)
{
    vk::Renderer *renderer                                 = context->getRenderer();
    const std::vector<gl::SamplerBinding> &samplerBindings = executable.getSamplerBindings();
    const std::vector<GLuint> &samplerBoundTextureUnits = executable.getSamplerBoundTextureUnits();
    const std::vector<gl::LinkedUniform> &uniforms      = executable.getUniforms();
    const gl::ActiveTextureTypeArray &textureTypes      = executable.getActiveSamplerTypes();

    // Allocate VkWriteDescriptorSet and initialize the data structure
    VkWriteDescriptorSet *writeDescriptorSets =
        updateBuilder->allocWriteDescriptorSets(writeDescriptorDescs.size());
    for (uint32_t writeIndex = 0; writeIndex < writeDescriptorDescs.size(); ++writeIndex)
    {
        ASSERT(writeDescriptorDescs[writeIndex].descriptorCount > 0);

        VkWriteDescriptorSet &writeSet = writeDescriptorSets[writeIndex];
        writeSet.descriptorCount       = writeDescriptorDescs[writeIndex].descriptorCount;
        writeSet.descriptorType =
            static_cast<VkDescriptorType>(writeDescriptorDescs[writeIndex].descriptorType);
        writeSet.dstArrayElement  = 0;
        writeSet.dstBinding       = writeIndex;
        writeSet.dstSet           = descriptorSet;
        writeSet.pBufferInfo      = nullptr;
        writeSet.pImageInfo       = nullptr;
        writeSet.pNext            = nullptr;
        writeSet.pTexelBufferView = nullptr;
        writeSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // Always allocate VkDescriptorImageInfo. In less common case that descriptorType is
        // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, this will not used.
        writeSet.pImageInfo = updateBuilder->allocDescriptorImageInfos(
            writeDescriptorDescs[writeIndex].descriptorCount);
    }

    for (uint32_t samplerIndex = 0; samplerIndex < samplerBindings.size(); ++samplerIndex)
    {
        uint32_t uniformIndex = executable.getUniformIndexFromSamplerIndex(samplerIndex);
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];
        if (samplerUniform.activeShaders().none())
        {
            continue;
        }

        const gl::ShaderType firstShaderType = samplerUniform.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            variableInfoMap.getVariableById(firstShaderType, samplerUniform.getId(firstShaderType));

        const gl::SamplerBinding &samplerBinding = samplerBindings[samplerIndex];
        uint32_t arraySize = static_cast<uint32_t>(samplerBinding.textureUnitsCount);

        VkWriteDescriptorSet &writeSet = writeDescriptorSets[info.binding];
        // Now fill pImageInfo or pTexelBufferView for writeSet
        for (uint32_t arrayElement = 0; arrayElement < arraySize; ++arrayElement)
        {
            GLuint textureUnit =
                samplerBinding.getTextureUnit(samplerBoundTextureUnits, arrayElement);
            TextureVk *textureVk = textures[textureUnit];

            if (textureTypes[textureUnit] == gl::TextureType::Buffer)
            {
                ASSERT(writeSet.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
                const vk::BufferView *view = nullptr;
                ANGLE_TRY(
                    textureVk->getBufferView(context, nullptr, &samplerBinding, false, &view));

                VkBufferView &bufferView  = updateBuilder->allocBufferView();
                bufferView                = view->getHandle();
                writeSet.pTexelBufferView = &bufferView;
            }
            else
            {
                ASSERT(writeSet.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                bool isSamplerExternalY2Y =
                    samplerBinding.samplerType == GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT;
                gl::Sampler *sampler       = samplers[textureUnit].get();
                const SamplerVk *samplerVk = sampler ? vk::GetImpl(sampler) : nullptr;
                const vk::SamplerHelper &samplerHelper =
                    samplerVk ? samplerVk->getSampler()
                              : textureVk->getSampler(isSamplerExternalY2Y);
                const gl::SamplerState &samplerState =
                    sampler ? sampler->getSamplerState() : textureVk->getState().getSamplerState();

                vk::ImageLayout imageLayout    = textureVk->getImage().getCurrentImageLayout();
                const vk::ImageView &imageView = textureVk->getReadImageView(
                    samplerState.getSRGBDecode(), samplerUniform.isTexelFetchStaticUse(),
                    isSamplerExternalY2Y);

                VkDescriptorImageInfo *imageInfo = const_cast<VkDescriptorImageInfo *>(
                    &writeSet.pImageInfo[arrayElement + samplerUniform.getOuterArrayOffset()]);
                imageInfo->imageLayout = ConvertImageLayoutToVkImageLayout(renderer, imageLayout);
                imageInfo->imageView   = imageView.getHandle();
                imageInfo->sampler     = samplerHelper.get().getHandle();
            }
        }
    }

    return angle::Result::Continue;
}
}  // namespace

class ProgramExecutableVk::WarmUpTaskCommon : public vk::ErrorContext, public LinkSubTask
{
  public:
    WarmUpTaskCommon(vk::Renderer *renderer) : vk::ErrorContext(renderer) {}
    WarmUpTaskCommon(vk::Renderer *renderer,
                     ProgramExecutableVk *executableVk,
                     vk::PipelineRobustness pipelineRobustness,
                     vk::PipelineProtectedAccess pipelineProtectedAccess)
        : vk::ErrorContext(renderer),
          mExecutableVk(executableVk),
          mPipelineRobustness(pipelineRobustness),
          mPipelineProtectedAccess(pipelineProtectedAccess)
    {}
    ~WarmUpTaskCommon() override = default;

    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override
    {
        mErrorCode     = result;
        mErrorFile     = file;
        mErrorFunction = function;
        mErrorLine     = line;
    }

    void operator()() override { UNREACHABLE(); }

    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        ContextVk *contextVk = vk::GetImpl(context);
        return getResultImpl(contextVk, infoLog);
    }

    angle::Result getResultImpl(ContextVk *contextVk, gl::InfoLog &infoLog)
    {
        // Forward any errors
        if (mErrorCode != VK_SUCCESS)
        {
            contextVk->handleError(mErrorCode, mErrorFile, mErrorFunction, mErrorLine);
            return angle::Result::Stop;
        }

        // Accumulate relevant perf counters
        const angle::VulkanPerfCounters &from = getPerfCounters();
        angle::VulkanPerfCounters &to         = contextVk->getPerfCounters();

        to.pipelineCreationCacheHits += from.pipelineCreationCacheHits;
        to.pipelineCreationCacheMisses += from.pipelineCreationCacheMisses;
        to.pipelineCreationTotalCacheHitsDurationNs +=
            from.pipelineCreationTotalCacheHitsDurationNs;
        to.pipelineCreationTotalCacheMissesDurationNs +=
            from.pipelineCreationTotalCacheMissesDurationNs;

        return angle::Result::Continue;
    }

  protected:
    void mergeProgramExecutablePipelineCacheToRenderer()
    {
        angle::Result mergeResult = mExecutableVk->mergePipelineCacheToRenderer(this);

        // Treat error during merge as non fatal, log it and move on
        if (mergeResult != angle::Result::Continue)
        {
            INFO() << "Error while merging to Renderer's pipeline cache";
        }
    }

    // The front-end ensures that the program is not modified while the subtask is running, so it is
    // safe to directly access the executable from this parallel job.  Note that this is the reason
    // why the front-end does not let the parallel job continue when a relink happens or the first
    // draw with this program.
    ProgramExecutableVk *mExecutableVk               = nullptr;
    const vk::PipelineRobustness mPipelineRobustness = vk::PipelineRobustness::NonRobust;
    const vk::PipelineProtectedAccess mPipelineProtectedAccess =
        vk::PipelineProtectedAccess::Unprotected;

    // Error handling
    VkResult mErrorCode        = VK_SUCCESS;
    const char *mErrorFile     = nullptr;
    const char *mErrorFunction = nullptr;
    unsigned int mErrorLine    = 0;
};

class ProgramExecutableVk::WarmUpComputeTask : public WarmUpTaskCommon
{
  public:
    WarmUpComputeTask(vk::Renderer *renderer,
                      ProgramExecutableVk *executableVk,
                      vk::PipelineRobustness pipelineRobustness,
                      vk::PipelineProtectedAccess pipelineProtectedAccess)
        : WarmUpTaskCommon(renderer, executableVk, pipelineRobustness, pipelineProtectedAccess)
    {}
    ~WarmUpComputeTask() override = default;

    void operator()() override
    {
        angle::Result result = mExecutableVk->warmUpComputePipelineCache(this, mPipelineRobustness,
                                                                         mPipelineProtectedAccess);
        ASSERT((result == angle::Result::Continue) == (mErrorCode == VK_SUCCESS));

        mergeProgramExecutablePipelineCacheToRenderer();
    }
};

using SharedRenderPass = vk::AtomicRefCounted<vk::RenderPass>;
class ProgramExecutableVk::WarmUpGraphicsTask : public WarmUpTaskCommon
{
  public:
    WarmUpGraphicsTask(vk::Renderer *renderer,
                       ProgramExecutableVk *executableVk,
                       vk::PipelineRobustness pipelineRobustness,
                       vk::PipelineProtectedAccess pipelineProtectedAccess,
                       vk::GraphicsPipelineSubset subset,
                       const bool isSurfaceRotated,
                       const vk::GraphicsPipelineDesc &graphicsPipelineDesc,
                       SharedRenderPass *compatibleRenderPass,
                       vk::PipelineHelper *placeholderPipelineHelper)
        : WarmUpTaskCommon(renderer, executableVk, pipelineRobustness, pipelineProtectedAccess),
          mPipelineSubset(subset),
          mIsSurfaceRotated(isSurfaceRotated),
          mGraphicsPipelineDesc(graphicsPipelineDesc),
          mWarmUpPipelineHelper(placeholderPipelineHelper),
          mCompatibleRenderPass(compatibleRenderPass)
    {
        ASSERT(mCompatibleRenderPass);
        mCompatibleRenderPass->addRef();
    }
    ~WarmUpGraphicsTask() override = default;

    void operator()() override
    {
        angle::Result result = mExecutableVk->warmUpGraphicsPipelineCache(
            this, mPipelineRobustness, mPipelineProtectedAccess, mPipelineSubset, mIsSurfaceRotated,
            mGraphicsPipelineDesc, mCompatibleRenderPass->get(), mWarmUpPipelineHelper);
        ASSERT((result == angle::Result::Continue) == (mErrorCode == VK_SUCCESS));

        // Release reference to shared renderpass. If this is the last reference -
        // 1. merge ProgramExecutableVk's pipeline cache into the Renderer's cache
        // 2. cleanup temporary renderpass
        //
        // Note: with dynamic rendering, |mCompatibleRenderPass| holds a VK_NULL_HANDLE, and it's
        // just used as a ref count for this purpose.
        const bool isLastWarmUpTask = mCompatibleRenderPass->getAndReleaseRef() == 1;
        if (isLastWarmUpTask)
        {
            mergeProgramExecutablePipelineCacheToRenderer();

            mCompatibleRenderPass->get().destroy(getDevice());
            SafeDelete(mCompatibleRenderPass);
        }
    }

  private:
    vk::GraphicsPipelineSubset mPipelineSubset;
    bool mIsSurfaceRotated;
    vk::GraphicsPipelineDesc mGraphicsPipelineDesc;
    vk::PipelineHelper *mWarmUpPipelineHelper;

    // Temporary objects to clean up at the end
    SharedRenderPass *mCompatibleRenderPass;
};

// ShaderInfo implementation.
ShaderInfo::ShaderInfo() {}

ShaderInfo::~ShaderInfo() = default;

angle::Result ShaderInfo::initShaders(vk::ErrorContext *context,
                                      const gl::ShaderBitSet &linkedShaderStages,
                                      const gl::ShaderMap<const angle::spirv::Blob *> &spirvBlobs,
                                      const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                      bool isGLES1)
{
    clear();

    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        if (spirvBlobs[shaderType] != nullptr)
        {
            mSpirvBlobs[shaderType] = *spirvBlobs[shaderType];
        }
    }

    // Assert that SPIR-V transformation is correct, even if the test never issues a draw call.
    // Don't validate GLES1 programs because they are always created right before a draw, so they
    // will naturally be validated.  This improves GLES1 test run times.
    if (!isGLES1)
    {
        ASSERT(ValidateTransformedSpirV(context, linkedShaderStages, variableInfoMap, mSpirvBlobs));
    }

    mIsInitialized = true;
    return angle::Result::Continue;
}

void ShaderInfo::initShaderFromProgram(gl::ShaderType shaderType,
                                       const ShaderInfo &programShaderInfo)
{
    mSpirvBlobs[shaderType] = programShaderInfo.mSpirvBlobs[shaderType];
    mIsInitialized          = true;
}

void ShaderInfo::clear()
{
    for (angle::spirv::Blob &spirvBlob : mSpirvBlobs)
    {
        spirvBlob.clear();
    }
    mIsInitialized = false;
}

void ShaderInfo::load(gl::BinaryInputStream *stream)
{
    clear();

    // Read in shader codes for all shader types
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->readVector(&mSpirvBlobs[shaderType]);
    }

    mIsInitialized = true;
}

void ShaderInfo::save(gl::BinaryOutputStream *stream)
{
    ASSERT(valid());

    // Write out shader codes for all shader types
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->writeVector(mSpirvBlobs[shaderType]);
    }
}

// ProgramInfo implementation.
ProgramInfo::ProgramInfo() {}

ProgramInfo::~ProgramInfo() = default;

angle::Result ProgramInfo::initProgram(vk::ErrorContext *context,
                                       gl::ShaderType shaderType,
                                       bool isLastPreFragmentStage,
                                       bool isTransformFeedbackProgram,
                                       const ShaderInfo &shaderInfo,
                                       ProgramTransformOptions optionBits,
                                       const ShaderInterfaceVariableInfoMap &variableInfoMap)
{
    const gl::ShaderMap<angle::spirv::Blob> &originalSpirvBlobs = shaderInfo.getSpirvBlobs();
    const angle::spirv::Blob &originalSpirvBlob                 = originalSpirvBlobs[shaderType];
    gl::ShaderMap<angle::spirv::Blob> transformedSpirvBlobs;
    angle::spirv::Blob &transformedSpirvBlob = transformedSpirvBlobs[shaderType];

    SpvTransformOptions options;
    options.shaderType               = shaderType;
    options.isLastPreFragmentStage   = isLastPreFragmentStage;
    options.isTransformFeedbackStage = isLastPreFragmentStage && isTransformFeedbackProgram &&
                                       !optionBits.removeTransformFeedbackEmulation;
    options.isTransformFeedbackEmulated = context->getFeatures().emulateTransformFeedback.enabled;
    options.isMultisampledFramebufferFetch =
        optionBits.multiSampleFramebufferFetch && shaderType == gl::ShaderType::Fragment;
    options.enableSampleShading = optionBits.enableSampleShading;
    options.removeDepthStencilInput =
        optionBits.removeDepthStencilInput && shaderType == gl::ShaderType::Fragment;

    options.useSpirvVaryingPrecisionFixer =
        context->getFeatures().varyingsRequireMatchingPrecisionInSpirv.enabled;

    ANGLE_TRY(
        SpvTransformSpirvCode(options, variableInfoMap, originalSpirvBlob, &transformedSpirvBlob));
    ANGLE_TRY(vk::InitShaderModule(context, &mShaders[shaderType], transformedSpirvBlob.data(),
                                   transformedSpirvBlob.size() * sizeof(uint32_t)));

    mProgramHelper.setShader(shaderType, mShaders[shaderType]);

    return angle::Result::Continue;
}

void ProgramInfo::release(ContextVk *contextVk)
{
    mProgramHelper.release(contextVk);

    for (vk::ShaderModulePtr &shader : mShaders)
    {
        shader.reset();
    }
}

ProgramExecutableVk::ProgramExecutableVk(const gl::ProgramExecutable *executable)
    : ProgramExecutableImpl(executable),
      mImmutableSamplersMaxDescriptorCount(1),
      mUniformBufferDescriptorType(VK_DESCRIPTOR_TYPE_MAX_ENUM),
      mDynamicUniformDescriptorOffsets{},
      mValidGraphicsPermutations{},
      mValidComputePermutations{}
{
    for (std::shared_ptr<BufferAndLayout> &defaultBlock : mDefaultUniformBlocks)
    {
        defaultBlock = std::make_shared<BufferAndLayout>();
    }
}

ProgramExecutableVk::~ProgramExecutableVk()
{
    ASSERT(!mPipelineCache.valid());
}

void ProgramExecutableVk::destroy(const gl::Context *context)
{
    reset(vk::GetImpl(context));
}

void ProgramExecutableVk::resetLayout(ContextVk *contextVk)
{
    if (!mPipelineLayout)
    {
        ASSERT(mValidGraphicsPermutations.none());
        ASSERT(mValidComputePermutations.none());
        return;
    }

    waitForPostLinkTasksImpl(contextVk);

    for (auto &descriptorSetLayout : mDescriptorSetLayouts)
    {
        descriptorSetLayout.reset();
    }
    mImmutableSamplersMaxDescriptorCount = 1;
    mImmutableSamplerIndexMap.clear();

    for (vk::DescriptorSetPointer &descriptorSet : mDescriptorSets)
    {
        descriptorSet.reset();
    }

    for (vk::DynamicDescriptorPoolPointer &pool : mDynamicDescriptorPools)
    {
        pool.reset();
    }

    // Initialize with an invalid BufferSerial
    mCurrentDefaultUniformBufferSerial = vk::BufferSerial();

    for (size_t index : mValidGraphicsPermutations)
    {
        mCompleteGraphicsPipelines[index].release(contextVk);
        mShadersGraphicsPipelines[index].release(contextVk);

        // Program infos and pipeline layout must be released after pipelines are; they might be
        // having pending jobs that are referencing them.
        mGraphicsProgramInfos[index].release(contextVk);
    }
    mValidGraphicsPermutations.reset();

    mComputePipelines.release(contextVk);
    mComputeProgramInfo.release(contextVk);
    mValidComputePermutations.reset();

    mPipelineLayout.reset();

    contextVk->onProgramExecutableReset(this);
}

void ProgramExecutableVk::reset(ContextVk *contextVk)
{
    resetLayout(contextVk);

    if (mPipelineCache.valid())
    {
        mPipelineCache.destroy(contextVk->getDevice());
    }
}

angle::Result ProgramExecutableVk::initializePipelineCache(vk::ErrorContext *context,
                                                           bool compressed,
                                                           const std::vector<uint8_t> &pipelineData)
{
    ASSERT(!mPipelineCache.valid());

    size_t dataSize            = pipelineData.size();
    const uint8_t *dataPointer = pipelineData.data();

    angle::MemoryBuffer uncompressedData;
    if (compressed)
    {
        if (!angle::DecompressBlob(dataPointer, dataSize, kMaxLocalPipelineCacheSize,
                                   &uncompressedData))
        {
            return angle::Result::Stop;
        }
        dataSize    = uncompressedData.size();
        dataPointer = uncompressedData.data();
    }

    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.initialDataSize = dataSize;
    pipelineCacheCreateInfo.pInitialData    = dataPointer;

    ANGLE_VK_TRY(context, mPipelineCache.init(context->getDevice(), pipelineCacheCreateInfo));

    // Merge the pipeline cache into Renderer's.
    if (context->getFeatures().mergeProgramPipelineCachesToGlobalCache.enabled)
    {
        ANGLE_TRY(context->getRenderer()->mergeIntoPipelineCache(context, mPipelineCache));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::ensurePipelineCacheInitialized(vk::ErrorContext *context)
{
    if (!mPipelineCache.valid())
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        ANGLE_VK_TRY(context, mPipelineCache.init(context->getDevice(), pipelineCacheCreateInfo));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::load(ContextVk *contextVk,
                                        bool isSeparable,
                                        gl::BinaryInputStream *stream,
                                        egl::CacheGetResult *resultOut)
{
    mVariableInfoMap.load(stream);
    mOriginalShaderInfo.load(stream);

    // Deserializes the uniformLayout data of mDefaultUniformBlocks
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->readVector(&mDefaultUniformBlocks[shaderType]->uniformLayout);
    }

    // Deserializes required uniform block memory sizes
    gl::ShaderMap<size_t> requiredBufferSize;
    stream->readPackedEnumMap(&requiredBufferSize);

    if (!isSeparable)
    {
        size_t compressedPipelineDataSize = 0;
        stream->readInt<size_t>(&compressedPipelineDataSize);

        std::vector<uint8_t> compressedPipelineData(compressedPipelineDataSize);
        if (compressedPipelineDataSize > 0)
        {
            bool compressedData = false;
            stream->readBool(&compressedData);
            stream->readBytes(compressedPipelineData.data(), compressedPipelineDataSize);
            // Initialize the pipeline cache based on cached data.
            ANGLE_TRY(initializePipelineCache(contextVk, compressedData, compressedPipelineData));
        }
    }

    // Initialize and resize the mDefaultUniformBlocks' memory
    ANGLE_TRY(resizeUniformBlockMemory(contextVk, requiredBufferSize));

    resetLayout(contextVk);
    ANGLE_TRY(createPipelineLayout(contextVk, &contextVk->getPipelineLayoutCache(),
                                   &contextVk->getDescriptorSetLayoutCache(), nullptr));

    ANGLE_TRY(initializeDescriptorPools(contextVk, &contextVk->getDescriptorSetLayoutCache(),
                                        &contextVk->getMetaDescriptorPools()));

    *resultOut = egl::CacheGetResult::Success;
    return angle::Result::Continue;
}

void ProgramExecutableVk::save(ContextVk *contextVk,
                               bool isSeparable,
                               gl::BinaryOutputStream *stream)
{
    mVariableInfoMap.save(stream);
    mOriginalShaderInfo.save(stream);

    // Serializes the uniformLayout data of mDefaultUniformBlocks
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->writeVector(mDefaultUniformBlocks[shaderType]->uniformLayout);
    }

    // Serializes required uniform block memory sizes
    gl::ShaderMap<size_t> uniformDataSize;
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        uniformDataSize[shaderType] = mDefaultUniformBlocks[shaderType]->uniformData.size();
    }
    stream->writePackedEnumMap(uniformDataSize);

    // Need to wait for warm up tasks to complete.
    waitForPostLinkTasksImpl(contextVk);

    // Compress and save mPipelineCache.  Separable programs don't warm up the cache, while program
    // pipelines do.  However, currently ANGLE doesn't sync program pipelines to cache.  ANGLE could
    // potentially use VK_EXT_graphics_pipeline_library to create separate pipelines for
    // pre-rasterization and fragment subsets, but currently those subsets are bundled together.
    if (!isSeparable)
    {
        angle::MemoryBuffer cacheData;

        GetPipelineCacheData(contextVk, mPipelineCache, &cacheData);
        stream->writeInt(cacheData.size());
        if (cacheData.size() > 0)
        {
            stream->writeBool(contextVk->getFeatures().enablePipelineCacheDataCompression.enabled);
            stream->writeBytes(cacheData.data(), cacheData.size());
        }
    }
}

void ProgramExecutableVk::clearVariableInfoMap()
{
    mVariableInfoMap.clear();
}

angle::Result ProgramExecutableVk::getPipelineCacheWarmUpTasks(
    vk::Renderer *renderer,
    vk::PipelineRobustness pipelineRobustness,
    vk::PipelineProtectedAccess pipelineProtectedAccess,
    std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut)
{
    ASSERT(!postLinkSubTasksOut || postLinkSubTasksOut->empty());

    const vk::GraphicsPipelineSubset subset = GetWarmUpSubset(renderer->getFeatures());

    bool isCompute                                        = false;
    angle::FixedVector<bool, 2> surfaceRotationVariations = {false};
    vk::GraphicsPipelineDesc *graphicsPipelineDesc        = nullptr;
    vk::RenderPass compatibleRenderPass;

    WarmUpTaskCommon prepForWarmUpContext(renderer);
    ANGLE_TRY(prepareForWarmUpPipelineCache(
        &prepForWarmUpContext, pipelineRobustness, pipelineProtectedAccess, subset, &isCompute,
        &surfaceRotationVariations, &graphicsPipelineDesc, &compatibleRenderPass));

    std::vector<std::shared_ptr<rx::LinkSubTask>> warmUpSubTasks;
    if (isCompute)
    {
        ASSERT(!compatibleRenderPass.valid());

        warmUpSubTasks.push_back(std::make_shared<WarmUpComputeTask>(
            renderer, this, pipelineRobustness, pipelineProtectedAccess));
    }
    else
    {
        ProgramTransformOptions transformOptions = {};
        SharedRenderPass *sharedRenderPass = new SharedRenderPass(std::move(compatibleRenderPass));
        for (bool surfaceRotation : surfaceRotationVariations)
        {
            // Add a placeholder entry in GraphicsPipelineCache
            transformOptions.surfaceRotation   = surfaceRotation;
            const uint8_t programIndex         = transformOptions.permutationIndex;
            vk::PipelineHelper *pipelineHelper = nullptr;
            if (subset == vk::GraphicsPipelineSubset::Complete)
            {
                CompleteGraphicsPipelineCache &pipelines = mCompleteGraphicsPipelines[programIndex];
                pipelines.populate(mWarmUpGraphicsPipelineDesc, vk::Pipeline(), &pipelineHelper);
            }
            else
            {
                ASSERT(subset == vk::GraphicsPipelineSubset::Shaders);
                ShadersGraphicsPipelineCache &pipelines = mShadersGraphicsPipelines[programIndex];
                pipelines.populate(mWarmUpGraphicsPipelineDesc, vk::Pipeline(), &pipelineHelper);
            }

            warmUpSubTasks.push_back(std::make_shared<WarmUpGraphicsTask>(
                renderer, this, pipelineRobustness, pipelineProtectedAccess, subset,
                surfaceRotation, *graphicsPipelineDesc, sharedRenderPass, pipelineHelper));
        }
    }

    // If the caller hasn't provided a valid async task container, inline the warmUp tasks.
    if (postLinkSubTasksOut)
    {
        *postLinkSubTasksOut = std::move(warmUpSubTasks);
    }
    else
    {
        for (std::shared_ptr<rx::LinkSubTask> &task : warmUpSubTasks)
        {
            (*task)();
        }
        warmUpSubTasks.clear();
    }

    ASSERT(warmUpSubTasks.empty());

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::prepareForWarmUpPipelineCache(
    vk::ErrorContext *context,
    vk::PipelineRobustness pipelineRobustness,
    vk::PipelineProtectedAccess pipelineProtectedAccess,
    vk::GraphicsPipelineSubset subset,
    bool *isComputeOut,
    angle::FixedVector<bool, 2> *surfaceRotationVariationsOut,
    vk::GraphicsPipelineDesc **graphicsPipelineDescOut,
    vk::RenderPass *renderPassOut)
{
    ASSERT(isComputeOut);
    ASSERT(surfaceRotationVariationsOut);
    ASSERT(graphicsPipelineDescOut);
    ASSERT(renderPassOut);
    ASSERT(context->getFeatures().warmUpPipelineCacheAtLink.enabled);

    ANGLE_TRY(ensurePipelineCacheInitialized(context));

    *isComputeOut        = false;
    const bool isCompute = mExecutable->hasLinkedShaderStage(gl::ShaderType::Compute);
    if (isCompute)
    {
        // Initialize compute program.
        vk::ComputePipelineOptions pipelineOptions =
            vk::GetComputePipelineOptions(pipelineRobustness, pipelineProtectedAccess);
        ANGLE_TRY(
            initComputeProgram(context, &mComputeProgramInfo, mVariableInfoMap, pipelineOptions));

        *isComputeOut = true;
        return angle::Result::Continue;
    }

    // It is only at drawcall time that we will have complete information required to build the
    // graphics pipeline descriptor. Use the most "commonly seen" state values and create the
    // pipeline.
    gl::PrimitiveMode mode = (mExecutable->hasLinkedShaderStage(gl::ShaderType::TessControl) ||
                              mExecutable->hasLinkedShaderStage(gl::ShaderType::TessEvaluation))
                                 ? gl::PrimitiveMode::Patches
                                 : gl::PrimitiveMode::TriangleStrip;
    SetupDefaultPipelineState(context, *mExecutable, mode, pipelineRobustness,
                              pipelineProtectedAccess, subset, &mWarmUpGraphicsPipelineDesc);

    // Create a temporary compatible RenderPass.  The render pass cache in ContextVk cannot be used
    // because this function may be called from a worker thread.
    vk::AttachmentOpsArray ops;
    RenderPassCache::InitializeOpsForCompatibleRenderPass(
        mWarmUpGraphicsPipelineDesc.getRenderPassDesc(), &ops);
    if (!context->getFeatures().preferDynamicRendering.enabled)
    {
        ANGLE_TRY(RenderPassCache::MakeRenderPass(
            context, mWarmUpGraphicsPipelineDesc.getRenderPassDesc(), ops, renderPassOut, nullptr));
    }

    *graphicsPipelineDescOut = &mWarmUpGraphicsPipelineDesc;

    // Variations that definitely matter:
    //
    // - PreRotation: It's a boolean specialization constant
    // - Depth correction: It's a SPIR-V transformation
    //
    // There are a number of states that are not currently dynamic (and may never be, such as sample
    // shading), but pre-creating shaders for them is impractical.  Most such state is likely unused
    // by most applications, but variations can be added here for certain apps that are known to
    // benefit from it.
    *surfaceRotationVariationsOut = {false};
    if (context->getFeatures().enablePreRotateSurfaces.enabled &&
        !context->getFeatures().preferDriverUniformOverSpecConst.enabled)
    {
        surfaceRotationVariationsOut->push_back(true);
    }

    ProgramTransformOptions transformOptions = {};
    for (bool rotation : *surfaceRotationVariationsOut)
    {
        // Initialize graphics programs.
        transformOptions.surfaceRotation = rotation;
        ANGLE_TRY(initGraphicsShaderPrograms(context, transformOptions));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::warmUpComputePipelineCache(
    vk::ErrorContext *context,
    vk::PipelineRobustness pipelineRobustness,
    vk::PipelineProtectedAccess pipelineProtectedAccess)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ProgramExecutableVk::warmUpComputePipelineCache");

    // This method assumes that all the state necessary to create a compute pipeline has already
    // been setup by the caller. Assert that all required state is valid so all that is left will
    // be the call to `vkCreateComputePipelines`

    // Make sure the shader module for compute shader stage is valid.
    ASSERT(mComputeProgramInfo.valid(gl::ShaderType::Compute));

    // No synchronization necessary since mPipelineCache is internally synchronized.
    vk::PipelineCacheAccess pipelineCache;
    pipelineCache.init(&mPipelineCache, nullptr);

    // There is no state associated with compute programs, so only one pipeline needs creation
    // to warm up the cache.
    vk::PipelineHelper *pipeline = nullptr;
    ANGLE_TRY(getOrCreateComputePipeline(context, &pipelineCache, PipelineSource::WarmUp,
                                         pipelineRobustness, pipelineProtectedAccess, &pipeline));

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::warmUpGraphicsPipelineCache(
    vk::ErrorContext *context,
    vk::PipelineRobustness pipelineRobustness,
    vk::PipelineProtectedAccess pipelineProtectedAccess,
    vk::GraphicsPipelineSubset subset,
    const bool isSurfaceRotated,
    const vk::GraphicsPipelineDesc &graphicsPipelineDesc,
    const vk::RenderPass &renderPass,
    vk::PipelineHelper *placeholderPipelineHelper)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "ProgramExecutableVk::warmUpGraphicsPipelineCache");

    ASSERT(placeholderPipelineHelper && !placeholderPipelineHelper->valid());

    // No synchronization necessary since mPipelineCache is internally synchronized.
    vk::PipelineCacheAccess pipelineCache;
    pipelineCache.init(&mPipelineCache, nullptr);

    const vk::GraphicsPipelineDesc *descPtr  = nullptr;
    ProgramTransformOptions transformOptions = {};
    transformOptions.surfaceRotation         = isSurfaceRotated;

    ANGLE_TRY(createGraphicsPipelineImpl(context, transformOptions, subset, &pipelineCache,
                                         PipelineSource::WarmUp, graphicsPipelineDesc, renderPass,
                                         &descPtr, &placeholderPipelineHelper));

    ASSERT(placeholderPipelineHelper->valid());
    return angle::Result::Continue;
}

void ProgramExecutableVk::waitForPostLinkTasksImpl(ContextVk *contextVk)
{
    const std::vector<std::shared_ptr<rx::LinkSubTask>> &postLinkSubTasks =
        mExecutable->getPostLinkSubTasks();

    if (postLinkSubTasks.empty())
    {
        return;
    }

    // Wait for all post-link tasks to finish
    angle::WaitableEvent::WaitMany(&mExecutable->getPostLinkSubTaskWaitableEvents());

    // Get results and clean up
    for (const std::shared_ptr<rx::LinkSubTask> &task : postLinkSubTasks)
    {
        WarmUpTaskCommon *warmUpTask = static_cast<WarmUpTaskCommon *>(task.get());

        // As these tasks can be run post-link, their results are ignored.  Failure is harmless, but
        // more importantly the error (effectively due to a link event) may not be allowed through
        // the entry point that results in this call.
        gl::InfoLog infoLog;
        angle::Result result = warmUpTask->getResultImpl(contextVk, infoLog);
        if (result != angle::Result::Continue)
        {
            ANGLE_PERF_WARNING(contextVk->getDebug(), GL_DEBUG_SEVERITY_LOW,
                               "Post-link task unexpectedly failed. Performance may degrade, or "
                               "device may soon be lost");
        }
    }

    mExecutable->onPostLinkTasksComplete();
}

void ProgramExecutableVk::waitForGraphicsPostLinkTasks(
    ContextVk *contextVk,
    const vk::GraphicsPipelineDesc &currentGraphicsPipelineDesc)
{
    ASSERT(mExecutable->hasLinkedShaderStage(gl::ShaderType::Vertex));

    if (mExecutable->getPostLinkSubTasks().empty())
    {
        return;
    }

    const vk::GraphicsPipelineSubset subset = GetWarmUpSubset(contextVk->getFeatures());

    if (!mWarmUpGraphicsPipelineDesc.keyEqual(currentGraphicsPipelineDesc, subset))
    {
        // The GraphicsPipelineDesc used for warm up differs from the one used by the draw call.
        // There is no need to wait for the warm up tasks to complete.
        ANGLE_PERF_WARNING(
            contextVk->getDebug(), GL_DEBUG_SEVERITY_LOW,
            "GraphicsPipelineDesc used for warm up differs from the one used by draw.");

        // If the warm up tasks are finished anyway, let |waitForPostLinkTasksImpl| clean them up.
        if (!angle::WaitableEvent::AllReady(&mExecutable->getPostLinkSubTaskWaitableEvents()))
        {
            return;
        }
    }

    waitForPostLinkTasksImpl(contextVk);
}

angle::Result ProgramExecutableVk::mergePipelineCacheToRenderer(vk::ErrorContext *context) const
{
    // Merge the cache with Renderer's
    if (context->getFeatures().mergeProgramPipelineCachesToGlobalCache.enabled)
    {
        ANGLE_TRACE_EVENT0("gpu.angle", "ProgramExecutableVk::mergePipelineCacheToRenderer");
        ANGLE_TRY(context->getRenderer()->mergeIntoPipelineCache(context, mPipelineCache));
    }

    return angle::Result::Continue;
}

void ProgramExecutableVk::addInterfaceBlockDescriptorSetDesc(
    const std::vector<gl::InterfaceBlock> &blocks,
    gl::ShaderBitSet shaderTypes,
    VkDescriptorType descType,
    vk::DescriptorSetLayoutDesc *descOut)
{
    for (uint32_t bufferIndex = 0, arraySize = 0; bufferIndex < blocks.size();
         bufferIndex += arraySize)
    {
        gl::InterfaceBlock block = blocks[bufferIndex];
        arraySize                = GetInterfaceBlockArraySize(blocks, bufferIndex);

        if (block.activeShaders().none())
        {
            continue;
        }

        const gl::ShaderType firstShaderType = block.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            mVariableInfoMap.getVariableById(firstShaderType, block.getId(firstShaderType));

        const VkShaderStageFlags activeStages = gl_vk::GetShaderStageFlags(info.activeStages);

        descOut->addBinding(info.binding, descType, arraySize, activeStages, nullptr);
    }
}

void ProgramExecutableVk::addAtomicCounterBufferDescriptorSetDesc(
    const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers,
    vk::DescriptorSetLayoutDesc *descOut)
{
    if (atomicCounterBuffers.empty())
    {
        return;
    }

    const ShaderInterfaceVariableInfo &info =
        mVariableInfoMap.getAtomicCounterInfo(atomicCounterBuffers[0].getFirstActiveShaderType());
    VkShaderStageFlags activeStages = gl_vk::GetShaderStageFlags(info.activeStages);

    // A single storage buffer array is used for all stages for simplicity.
    descOut->addBinding(info.binding, vk::kStorageBufferDescriptorType,
                        gl::IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, activeStages,
                        nullptr);
}

void ProgramExecutableVk::addImageDescriptorSetDesc(vk::DescriptorSetLayoutDesc *descOut)
{
    const std::vector<gl::ImageBinding> &imageBindings = mExecutable->getImageBindings();
    const std::vector<gl::LinkedUniform> &uniforms     = mExecutable->getUniforms();

    for (uint32_t imageIndex = 0; imageIndex < imageBindings.size(); ++imageIndex)
    {
        uint32_t uniformIndex = mExecutable->getUniformIndexFromImageIndex(imageIndex);
        const gl::LinkedUniform &imageUniform = uniforms[uniformIndex];

        // 2D arrays are split into multiple 1D arrays when generating LinkedUniforms. Since they
        // are flattened into one array, ignore the nonzero elements and expand the array to the
        // total array size.
        if (imageUniform.activeShaders().none() || imageUniform.getOuterArrayOffset() > 0)
        {
            ASSERT(gl::SamplerNameContainsNonZeroArrayElement(
                mExecutable->getUniformNameByIndex(uniformIndex)));
            continue;
        }

        ASSERT(!gl::SamplerNameContainsNonZeroArrayElement(
            mExecutable->getUniformNameByIndex(uniformIndex)));

        // The front-end always binds array image units sequentially.
        const gl::ImageBinding &imageBinding = imageBindings[imageIndex];
        uint32_t arraySize = static_cast<uint32_t>(imageBinding.boundImageUnits.size());
        arraySize *= imageUniform.getOuterArraySizeProduct();

        const gl::ShaderType firstShaderType = imageUniform.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info =
            mVariableInfoMap.getVariableById(firstShaderType, imageUniform.getId(firstShaderType));

        const VkShaderStageFlags activeStages = gl_vk::GetShaderStageFlags(info.activeStages);

        const VkDescriptorType descType = imageBinding.textureType == gl::TextureType::Buffer
                                              ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                                              : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descOut->addBinding(info.binding, descType, arraySize, activeStages, nullptr);
    }
}

void ProgramExecutableVk::addInputAttachmentDescriptorSetDesc(vk::ErrorContext *context,
                                                              vk::DescriptorSetLayoutDesc *descOut)
{
    if (!mExecutable->getLinkedShaderStages()[gl::ShaderType::Fragment])
    {
        return;
    }

    if (mExecutable->usesDepthFramebufferFetch())
    {
        const uint32_t depthBinding =
            mVariableInfoMap
                .getVariableById(gl::ShaderType::Fragment, sh::vk::spirv::kIdDepthInputAttachment)
                .binding;
        descOut->addBinding(depthBinding, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1,
                            VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    }

    if (mExecutable->usesStencilFramebufferFetch())
    {
        const uint32_t stencilBinding =
            mVariableInfoMap
                .getVariableById(gl::ShaderType::Fragment, sh::vk::spirv::kIdStencilInputAttachment)
                .binding;
        descOut->addBinding(stencilBinding, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1,
                            VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
    }

    if (!mExecutable->usesColorFramebufferFetch())
    {
        return;
    }

    const uint32_t firstInputAttachment =
        static_cast<uint32_t>(mExecutable->getFragmentInoutIndices().first());

    const ShaderInterfaceVariableInfo &baseInfo = mVariableInfoMap.getVariableById(
        gl::ShaderType::Fragment, sh::vk::spirv::kIdInputAttachment0 + firstInputAttachment);

    uint32_t baseBinding = baseInfo.binding - firstInputAttachment;

    const uint32_t maxColorInputAttachmentCount =
        context->getRenderer()->getMaxColorInputAttachmentCount();
    for (uint32_t colorIndex = 0; colorIndex < maxColorInputAttachmentCount; ++colorIndex)
    {
        descOut->addBinding(baseBinding, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1,
                            VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
        baseBinding++;
    }
}

angle::Result ProgramExecutableVk::addTextureDescriptorSetDesc(
    vk::ErrorContext *context,
    const gl::ActiveTextureArray<TextureVk *> *activeTextures,
    vk::DescriptorSetLayoutDesc *descOut)
{
    const std::vector<gl::SamplerBinding> &samplerBindings = mExecutable->getSamplerBindings();
    const std::vector<gl::LinkedUniform> &uniforms         = mExecutable->getUniforms();
    const std::vector<GLuint> &samplerBoundTextureUnits =
        mExecutable->getSamplerBoundTextureUnits();

    for (uint32_t samplerIndex = 0; samplerIndex < samplerBindings.size(); ++samplerIndex)
    {
        uint32_t uniformIndex = mExecutable->getUniformIndexFromSamplerIndex(samplerIndex);
        const gl::LinkedUniform &samplerUniform = uniforms[uniformIndex];

        // 2D arrays are split into multiple 1D arrays when generating LinkedUniforms. Since they
        // are flattened into one array, ignore the nonzero elements and expand the array to the
        // total array size.
        if (samplerUniform.activeShaders().none() || samplerUniform.getOuterArrayOffset() > 0)
        {
            ASSERT(gl::SamplerNameContainsNonZeroArrayElement(
                mExecutable->getUniformNameByIndex(uniformIndex)));
            continue;
        }

        ASSERT(!gl::SamplerNameContainsNonZeroArrayElement(
            mExecutable->getUniformNameByIndex(uniformIndex)));

        // The front-end always binds array sampler units sequentially.
        const gl::SamplerBinding &samplerBinding = samplerBindings[samplerIndex];
        uint32_t arraySize = static_cast<uint32_t>(samplerBinding.textureUnitsCount);
        arraySize *= samplerUniform.getOuterArraySizeProduct();

        const gl::ShaderType firstShaderType    = samplerUniform.getFirstActiveShaderType();
        const ShaderInterfaceVariableInfo &info = mVariableInfoMap.getVariableById(
            firstShaderType, samplerUniform.getId(firstShaderType));

        const VkShaderStageFlags activeStages = gl_vk::GetShaderStageFlags(info.activeStages);

        // TODO: https://issuetracker.google.com/issues/158215272: how do we handle array of
        // immutable samplers?
        GLuint textureUnit = samplerBinding.getTextureUnit(samplerBoundTextureUnits, 0);
        if (activeTextures != nullptr &&
            (*activeTextures)[textureUnit]->getImage().hasImmutableSampler())
        {
            ASSERT(samplerBinding.textureUnitsCount == 1);

            // In the case of samplerExternal2DY2YEXT, we need
            // samplerYcbcrConversion object with IDENTITY conversion model
            bool isSamplerExternalY2Y =
                samplerBinding.samplerType == GL_SAMPLER_EXTERNAL_2D_Y2Y_EXT;

            // Always take the texture's sampler, that's only way to get to yuv conversion for
            // externalFormat
            const TextureVk *textureVk          = (*activeTextures)[textureUnit];
            const vk::Sampler &immutableSampler = textureVk->getSampler(isSamplerExternalY2Y).get();
            descOut->addBinding(info.binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, arraySize,
                                activeStages, &immutableSampler);
            const vk::ImageHelper &image = textureVk->getImage();
            const vk::YcbcrConversionDesc ycbcrConversionDesc =
                isSamplerExternalY2Y ? image.getY2YConversionDesc()
                                     : image.getYcbcrConversionDesc();
            mImmutableSamplerIndexMap[ycbcrConversionDesc] = samplerIndex;
            // The Vulkan spec has the following note -
            // All descriptors in a binding use the same maximum
            // combinedImageSamplerDescriptorCount descriptors to allow implementations to use a
            // uniform stride for dynamic indexing of the descriptors in the binding.
            uint64_t externalFormat        = image.getExternalFormat();
            uint32_t formatDescriptorCount = 0;

            vk::Renderer *renderer = context->getRenderer();

            if (externalFormat != 0)
            {
                ANGLE_TRY(renderer->getFormatDescriptorCountForExternalFormat(
                    context, externalFormat, &formatDescriptorCount));
            }
            else
            {
                VkFormat vkFormat = image.getActualVkFormat(renderer);
                ASSERT(vkFormat != 0);
                ANGLE_TRY(renderer->getFormatDescriptorCountForVkFormat(context, vkFormat,
                                                                        &formatDescriptorCount));
            }

            ASSERT(formatDescriptorCount > 0);
            mImmutableSamplersMaxDescriptorCount =
                std::max(mImmutableSamplersMaxDescriptorCount, formatDescriptorCount);
        }
        else
        {
            const VkDescriptorType descType = samplerBinding.textureType == gl::TextureType::Buffer
                                                  ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
                                                  : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descOut->addBinding(info.binding, descType, arraySize, activeStages, nullptr);
        }
    }

    return angle::Result::Continue;
}

void ProgramExecutableVk::initializeWriteDescriptorDesc(vk::ErrorContext *context)
{
    const gl::ShaderBitSet &linkedShaderStages = mExecutable->getLinkedShaderStages();

    // Update mShaderResourceWriteDescriptorDescBuilder
    mShaderResourceWriteDescriptorDescs.reset();
    mShaderResourceWriteDescriptorDescs.updateShaderBuffers(
        mVariableInfoMap, mExecutable->getUniformBlocks(), getUniformBufferDescriptorType());
    mShaderResourceWriteDescriptorDescs.updateShaderBuffers(
        mVariableInfoMap, mExecutable->getShaderStorageBlocks(), getStorageBufferDescriptorType());
    mShaderResourceWriteDescriptorDescs.updateAtomicCounters(
        mVariableInfoMap, mExecutable->getAtomicCounterBuffers());
    mShaderResourceWriteDescriptorDescs.updateImages(*mExecutable, mVariableInfoMap);
    mShaderResourceWriteDescriptorDescs.updateDynamicDescriptorsCount();

    // Update mTextureWriteDescriptors
    mTextureWriteDescriptorDescs.reset();
    mTextureWriteDescriptorDescs.updateExecutableActiveTextures(mVariableInfoMap, *mExecutable);
    mTextureWriteDescriptorDescs.updateDynamicDescriptorsCount();

    // Update mDefaultUniformWriteDescriptors
    mDefaultUniformWriteDescriptorDescs.reset();
    mDefaultUniformWriteDescriptorDescs.updateDefaultUniform(linkedShaderStages, mVariableInfoMap,
                                                             *mExecutable);
    mDefaultUniformWriteDescriptorDescs.updateDynamicDescriptorsCount();

    mDefaultUniformAndXfbWriteDescriptorDescs.reset();
    if (mExecutable->hasTransformFeedbackOutput() &&
        context->getFeatures().emulateTransformFeedback.enabled)
    {
        // Update mDefaultUniformAndXfbWriteDescriptorDescs for the emulation code path.
        mDefaultUniformAndXfbWriteDescriptorDescs.updateDefaultUniform(
            linkedShaderStages, mVariableInfoMap, *mExecutable);
        if (linkedShaderStages[gl::ShaderType::Vertex])
        {
            mDefaultUniformAndXfbWriteDescriptorDescs.updateTransformFeedbackWrite(mVariableInfoMap,
                                                                                   *mExecutable);
        }
        mDefaultUniformAndXfbWriteDescriptorDescs.updateDynamicDescriptorsCount();
    }
    else
    {
        // Otherwise it will be the same as default uniform
        mDefaultUniformAndXfbWriteDescriptorDescs = mDefaultUniformWriteDescriptorDescs;
    }
}

ProgramTransformOptions ProgramExecutableVk::getTransformOptions(
    ContextVk *contextVk,
    const vk::GraphicsPipelineDesc &desc)
{
    ProgramTransformOptions transformOptions = {};

    transformOptions.surfaceRotation = desc.getSurfaceRotation();
    transformOptions.removeTransformFeedbackEmulation =
        contextVk->getFeatures().emulateTransformFeedback.enabled &&
        !contextVk->getState().isTransformFeedbackActiveUnpaused();
    FramebufferVk *drawFrameBuffer = vk::GetImpl(contextVk->getState().getDrawFramebuffer());
    const bool hasDepthStencilFramebufferFetch =
        mExecutable->usesDepthFramebufferFetch() || mExecutable->usesStencilFramebufferFetch();
    const bool hasFramebufferFetch =
        mExecutable->usesColorFramebufferFetch() || hasDepthStencilFramebufferFetch;
    const bool isMultisampled                    = drawFrameBuffer->getSamples() > 1;
    transformOptions.multiSampleFramebufferFetch = hasFramebufferFetch && isMultisampled;
    transformOptions.enableSampleShading =
        contextVk->getState().isSampleShadingEnabled() && isMultisampled;
    transformOptions.removeDepthStencilInput =
        hasDepthStencilFramebufferFetch &&
        drawFrameBuffer->getDepthStencilRenderTarget() == nullptr;

    return transformOptions;
}

angle::Result ProgramExecutableVk::initGraphicsShaderPrograms(
    vk::ErrorContext *context,
    ProgramTransformOptions transformOptions)
{
    ASSERT(mExecutable->hasLinkedShaderStage(gl::ShaderType::Vertex));

    const uint8_t programIndex                = transformOptions.permutationIndex;
    ProgramInfo &programInfo                  = mGraphicsProgramInfos[programIndex];
    const gl::ShaderBitSet linkedShaderStages = mExecutable->getLinkedShaderStages();
    gl::ShaderType lastPreFragmentStage       = gl::GetLastPreFragmentStage(linkedShaderStages);

    const bool isTransformFeedbackProgram =
        !mExecutable->getLinkedTransformFeedbackVaryings().empty();

    for (gl::ShaderType shaderType : linkedShaderStages)
    {
        ANGLE_TRY(initGraphicsShaderProgram(context, shaderType, shaderType == lastPreFragmentStage,
                                            isTransformFeedbackProgram, transformOptions,
                                            &programInfo, mVariableInfoMap));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::initProgramThenCreateGraphicsPipeline(
    vk::ErrorContext *context,
    ProgramTransformOptions transformOptions,
    vk::GraphicsPipelineSubset pipelineSubset,
    vk::PipelineCacheAccess *pipelineCache,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::RenderPass &compatibleRenderPass,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    ANGLE_TRY(initGraphicsShaderPrograms(context, transformOptions));

    return createGraphicsPipelineImpl(context, transformOptions, pipelineSubset, pipelineCache,
                                      source, desc, compatibleRenderPass, descPtrOut, pipelineOut);
}

angle::Result ProgramExecutableVk::createGraphicsPipelineImpl(
    vk::ErrorContext *context,
    ProgramTransformOptions transformOptions,
    vk::GraphicsPipelineSubset pipelineSubset,
    vk::PipelineCacheAccess *pipelineCache,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::RenderPass &compatibleRenderPass,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    // This method assumes that all the state necessary to create a graphics pipeline has already
    // been setup by the caller. Assert that all required state is valid so all that is left will
    // be the call to `vkCreateGraphicsPipelines`

    // Make sure program index is within range
    const uint8_t programIndex = transformOptions.permutationIndex;
    ASSERT(programIndex >= 0 && programIndex < ProgramTransformOptions::kPermutationCount);

    // Make sure the shader modules for all linked shader stages are valid.
    ProgramInfo &programInfo = mGraphicsProgramInfos[programIndex];
    for (gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
    {
        ASSERT(programInfo.valid(shaderType));
    }

    // Generate spec consts, a change in which results in a new pipeline.
    vk::SpecializationConstants specConsts = MakeSpecConsts(transformOptions, desc);

    // Choose appropriate pipeline cache based on pipeline subset
    if (pipelineSubset == vk::GraphicsPipelineSubset::Complete)
    {
        CompleteGraphicsPipelineCache &pipelines = mCompleteGraphicsPipelines[programIndex];
        return programInfo.getShaderProgram().createGraphicsPipeline(
            context, &pipelines, pipelineCache, compatibleRenderPass, getPipelineLayout(), source,
            desc, specConsts, descPtrOut, pipelineOut);
    }
    else
    {
        // Vertex input and fragment output subsets are independent of shaders, and are not created
        // through the program executable.
        ASSERT(pipelineSubset == vk::GraphicsPipelineSubset::Shaders);

        ShadersGraphicsPipelineCache &pipelines = mShadersGraphicsPipelines[programIndex];
        return programInfo.getShaderProgram().createGraphicsPipeline(
            context, &pipelines, pipelineCache, compatibleRenderPass, getPipelineLayout(), source,
            desc, specConsts, descPtrOut, pipelineOut);
    }
}

angle::Result ProgramExecutableVk::getGraphicsPipeline(ContextVk *contextVk,
                                                       vk::GraphicsPipelineSubset pipelineSubset,
                                                       const vk::GraphicsPipelineDesc &desc,
                                                       const vk::GraphicsPipelineDesc **descPtrOut,
                                                       vk::PipelineHelper **pipelineOut)
{
    ProgramTransformOptions transformOptions = getTransformOptions(contextVk, desc);

    ANGLE_TRY(initGraphicsShaderPrograms(contextVk, transformOptions));

    const uint8_t programIndex = transformOptions.permutationIndex;

    *descPtrOut  = nullptr;
    *pipelineOut = nullptr;

    if (pipelineSubset == vk::GraphicsPipelineSubset::Complete)
    {
        mCompleteGraphicsPipelines[programIndex].getPipeline(desc, descPtrOut, pipelineOut);
    }
    else
    {
        // Vertex input and fragment output subsets are independent of shaders, and are not created
        // through the program executable.
        ASSERT(pipelineSubset == vk::GraphicsPipelineSubset::Shaders);

        mShadersGraphicsPipelines[programIndex].getPipeline(desc, descPtrOut, pipelineOut);
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::createGraphicsPipeline(
    ContextVk *contextVk,
    vk::GraphicsPipelineSubset pipelineSubset,
    vk::PipelineCacheAccess *pipelineCache,
    PipelineSource source,
    const vk::GraphicsPipelineDesc &desc,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    ProgramTransformOptions transformOptions = getTransformOptions(contextVk, desc);

    // When creating monolithic pipelines, the renderer's pipeline cache is used as passed in.
    // When creating the shaders subset of pipelines, the program's own pipeline cache is used.
    vk::PipelineCacheAccess perProgramPipelineCache;
    const bool useProgramPipelineCache = pipelineSubset == vk::GraphicsPipelineSubset::Shaders;
    if (useProgramPipelineCache)
    {
        ANGLE_TRY(ensurePipelineCacheInitialized(contextVk));

        perProgramPipelineCache.init(&mPipelineCache, nullptr);
        pipelineCache = &perProgramPipelineCache;
    }

    // Pull in a compatible RenderPass.
    const vk::RenderPass *compatibleRenderPass = nullptr;
    ANGLE_TRY(contextVk->getCompatibleRenderPass(desc.getRenderPassDesc(), &compatibleRenderPass));

    ANGLE_TRY(initProgramThenCreateGraphicsPipeline(
        contextVk, transformOptions, pipelineSubset, pipelineCache, source, desc,
        *compatibleRenderPass, descPtrOut, pipelineOut));

    if (useProgramPipelineCache &&
        contextVk->getFeatures().mergeProgramPipelineCachesToGlobalCache.enabled)
    {
        ANGLE_TRY(contextVk->getRenderer()->mergeIntoPipelineCache(contextVk, mPipelineCache));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::linkGraphicsPipelineLibraries(
    ContextVk *contextVk,
    vk::PipelineCacheAccess *pipelineCache,
    const vk::GraphicsPipelineDesc &desc,
    vk::PipelineHelper *vertexInputPipeline,
    vk::PipelineHelper *shadersPipeline,
    vk::PipelineHelper *fragmentOutputPipeline,
    const vk::GraphicsPipelineDesc **descPtrOut,
    vk::PipelineHelper **pipelineOut)
{
    ProgramTransformOptions transformOptions = getTransformOptions(contextVk, desc);
    const uint8_t programIndex               = transformOptions.permutationIndex;

    ANGLE_TRY(mCompleteGraphicsPipelines[programIndex].linkLibraries(
        contextVk, pipelineCache, desc, getPipelineLayout(), vertexInputPipeline, shadersPipeline,
        fragmentOutputPipeline, descPtrOut, pipelineOut));

    // If monolithic pipelines are preferred over libraries, create a task so that it can be created
    // asynchronously.
    if (contextVk->getFeatures().preferMonolithicPipelinesOverLibraries.enabled)
    {
        vk::SpecializationConstants specConsts = MakeSpecConsts(transformOptions, desc);

        mGraphicsProgramInfos[programIndex].getShaderProgram().createMonolithicPipelineCreationTask(
            contextVk, pipelineCache, desc, getPipelineLayout(), specConsts, *pipelineOut);
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::getOrCreateComputePipeline(
    vk::ErrorContext *context,
    vk::PipelineCacheAccess *pipelineCache,
    PipelineSource source,
    vk::PipelineRobustness pipelineRobustness,
    vk::PipelineProtectedAccess pipelineProtectedAccess,
    vk::PipelineHelper **pipelineOut)
{
    ASSERT(mExecutable->hasLinkedShaderStage(gl::ShaderType::Compute));

    vk::ComputePipelineOptions pipelineOptions =
        vk::GetComputePipelineOptions(pipelineRobustness, pipelineProtectedAccess);
    ANGLE_TRY(initComputeProgram(context, &mComputeProgramInfo, mVariableInfoMap, pipelineOptions));

    return mComputeProgramInfo.getShaderProgram().getOrCreateComputePipeline(
        context, &mComputePipelines, pipelineCache, getPipelineLayout(), pipelineOptions, source,
        pipelineOut, nullptr, nullptr);
}

angle::Result ProgramExecutableVk::createPipelineLayout(
    vk::ErrorContext *context,
    PipelineLayoutCache *pipelineLayoutCache,
    DescriptorSetLayoutCache *descriptorSetLayoutCache,
    gl::ActiveTextureArray<TextureVk *> *activeTextures)
{
    const gl::ShaderBitSet &linkedShaderStages = mExecutable->getLinkedShaderStages();

    // Store a reference to the pipeline and descriptor set layouts. This will create them if they
    // don't already exist in the cache.

    // Default uniforms and transform feedback:
    mDefaultUniformAndXfbSetDesc          = {};
    uint32_t numDefaultUniformDescriptors = 0;
    for (gl::ShaderType shaderType : linkedShaderStages)
    {
        const ShaderInterfaceVariableInfo &info =
            mVariableInfoMap.getDefaultUniformInfo(shaderType);
        // Note that currently the default uniform block is added unconditionally.
        ASSERT(info.activeStages[shaderType]);

        mDefaultUniformAndXfbSetDesc.addBinding(info.binding,
                                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1,
                                                gl_vk::kShaderStageMap[shaderType], nullptr);
        numDefaultUniformDescriptors++;
    }

    gl::ShaderType linkedTransformFeedbackStage = mExecutable->getLinkedTransformFeedbackStage();
    bool hasXfbVaryings = linkedTransformFeedbackStage != gl::ShaderType::InvalidEnum &&
                          !mExecutable->getLinkedTransformFeedbackVaryings().empty();
    if (context->getFeatures().emulateTransformFeedback.enabled && hasXfbVaryings)
    {
        size_t xfbBufferCount = mExecutable->getTransformFeedbackBufferCount();
        for (uint32_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
        {
            const uint32_t binding = mVariableInfoMap.getEmulatedXfbBufferBinding(bufferIndex);
            ASSERT(binding != std::numeric_limits<uint32_t>::max());

            mDefaultUniformAndXfbSetDesc.addBinding(binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                                    VK_SHADER_STAGE_VERTEX_BIT, nullptr);
        }
    }

    ANGLE_TRY(descriptorSetLayoutCache->getDescriptorSetLayout(
        context, mDefaultUniformAndXfbSetDesc,
        &mDescriptorSetLayouts[DescriptorSetIndex::UniformsAndXfb]));

    // Uniform and storage buffers, atomic counter buffers and images:
    mShaderResourceSetDesc = {};

    // Count the number of active uniform buffer descriptors.
    uint32_t numActiveUniformBufferDescriptors    = 0;
    const std::vector<gl::InterfaceBlock> &blocks = mExecutable->getUniformBlocks();
    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size();)
    {
        const gl::InterfaceBlock &block = blocks[bufferIndex];
        const uint32_t arraySize        = GetInterfaceBlockArraySize(blocks, bufferIndex);
        bufferIndex += arraySize;

        if (block.activeShaders().any())
        {
            numActiveUniformBufferDescriptors += arraySize;
        }
    }

    // Decide if we should use dynamic or fixed descriptor types.
    VkPhysicalDeviceLimits limits = context->getRenderer()->getPhysicalDeviceProperties().limits;
    uint32_t totalDynamicUniformBufferCount =
        numActiveUniformBufferDescriptors + numDefaultUniformDescriptors;
    if (totalDynamicUniformBufferCount <= limits.maxDescriptorSetUniformBuffersDynamic)
    {
        mUniformBufferDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }
    else
    {
        mUniformBufferDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }

    addInterfaceBlockDescriptorSetDesc(mExecutable->getUniformBlocks(), linkedShaderStages,
                                       mUniformBufferDescriptorType, &mShaderResourceSetDesc);
    addInterfaceBlockDescriptorSetDesc(mExecutable->getShaderStorageBlocks(), linkedShaderStages,
                                       vk::kStorageBufferDescriptorType, &mShaderResourceSetDesc);
    addAtomicCounterBufferDescriptorSetDesc(mExecutable->getAtomicCounterBuffers(),
                                            &mShaderResourceSetDesc);
    addImageDescriptorSetDesc(&mShaderResourceSetDesc);
    addInputAttachmentDescriptorSetDesc(context, &mShaderResourceSetDesc);

    ANGLE_TRY(descriptorSetLayoutCache->getDescriptorSetLayout(
        context, mShaderResourceSetDesc,
        &mDescriptorSetLayouts[DescriptorSetIndex::ShaderResource]));

    // Textures:
    mTextureSetDesc = {};
    ANGLE_TRY(addTextureDescriptorSetDesc(context, activeTextures, &mTextureSetDesc));

    ANGLE_TRY(descriptorSetLayoutCache->getDescriptorSetLayout(
        context, mTextureSetDesc, &mDescriptorSetLayouts[DescriptorSetIndex::Texture]));

    // Create pipeline layout with these 3 descriptor sets.
    vk::PipelineLayoutDesc pipelineLayoutDesc;
    pipelineLayoutDesc.updateDescriptorSetLayout(DescriptorSetIndex::UniformsAndXfb,
                                                 mDefaultUniformAndXfbSetDesc);
    pipelineLayoutDesc.updateDescriptorSetLayout(DescriptorSetIndex::ShaderResource,
                                                 mShaderResourceSetDesc);
    pipelineLayoutDesc.updateDescriptorSetLayout(DescriptorSetIndex::Texture, mTextureSetDesc);

    // Set up driver uniforms as push constants. The size is set for a graphics pipeline, as there
    // are more driver uniforms for a graphics pipeline than there are for a compute pipeline. As
    // for the shader stages, both graphics and compute stages are used.
    VkShaderStageFlags pushConstantShaderStageFlags =
        context->getRenderer()->getSupportedVulkanShaderStageMask();

    uint32_t pushConstantSize = GetDriverUniformSize(context, PipelineType::Graphics);
    pipelineLayoutDesc.updatePushConstantRange(pushConstantShaderStageFlags, 0, pushConstantSize);

    ANGLE_TRY(pipelineLayoutCache->getPipelineLayout(context, pipelineLayoutDesc,
                                                     mDescriptorSetLayouts, &mPipelineLayout));

    mDynamicUniformDescriptorOffsets.clear();
    mDynamicUniformDescriptorOffsets.resize(mExecutable->getLinkedShaderStageCount(), 0);

    initializeWriteDescriptorDesc(context);

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::initializeDescriptorPools(
    vk::ErrorContext *context,
    DescriptorSetLayoutCache *descriptorSetLayoutCache,
    vk::DescriptorSetArray<vk::MetaDescriptorPool> *metaDescriptorPools)
{
    ANGLE_TRY((*metaDescriptorPools)[DescriptorSetIndex::UniformsAndXfb].bindCachedDescriptorPool(
        context, mDefaultUniformAndXfbSetDesc, 1, descriptorSetLayoutCache,
        &mDynamicDescriptorPools[DescriptorSetIndex::UniformsAndXfb]));
    ANGLE_TRY((*metaDescriptorPools)[DescriptorSetIndex::Texture].bindCachedDescriptorPool(
        context, mTextureSetDesc, mImmutableSamplersMaxDescriptorCount, descriptorSetLayoutCache,
        &mDynamicDescriptorPools[DescriptorSetIndex::Texture]));
    return (*metaDescriptorPools)[DescriptorSetIndex::ShaderResource].bindCachedDescriptorPool(
        context, mShaderResourceSetDesc, 1, descriptorSetLayoutCache,
        &mDynamicDescriptorPools[DescriptorSetIndex::ShaderResource]);
}

void ProgramExecutableVk::resolvePrecisionMismatch(const gl::ProgramMergedVaryings &mergedVaryings)
{
    for (const gl::ProgramVaryingRef &mergedVarying : mergedVaryings)
    {
        if (!mergedVarying.frontShader || !mergedVarying.backShader)
        {
            continue;
        }

        GLenum frontPrecision = mergedVarying.frontShader->precision;
        GLenum backPrecision  = mergedVarying.backShader->precision;
        if (frontPrecision == backPrecision)
        {
            continue;
        }

        ASSERT(frontPrecision >= GL_LOW_FLOAT && frontPrecision <= GL_HIGH_INT);
        ASSERT(backPrecision >= GL_LOW_FLOAT && backPrecision <= GL_HIGH_INT);

        if (frontPrecision > backPrecision)
        {
            // The output is higher precision than the input
            ShaderInterfaceVariableInfo &info = mVariableInfoMap.getMutable(
                mergedVarying.frontShaderStage, mergedVarying.frontShader->id);
            info.varyingIsOutput     = true;
            info.useRelaxedPrecision = true;
        }
        else
        {
            // The output is lower precision than the input, adjust the input
            ASSERT(backPrecision > frontPrecision);
            ShaderInterfaceVariableInfo &info = mVariableInfoMap.getMutable(
                mergedVarying.backShaderStage, mergedVarying.backShader->id);
            info.varyingIsInput      = true;
            info.useRelaxedPrecision = true;
        }
    }
}

angle::Result ProgramExecutableVk::getOrAllocateDescriptorSet(
    vk::Context *context,
    uint32_t currentFrame,
    UpdateDescriptorSetsBuilder *updateBuilder,
    const vk::DescriptorSetDescBuilder &descriptorSetDesc,
    const vk::WriteDescriptorDescs &writeDescriptorDescs,
    DescriptorSetIndex setIndex,
    vk::SharedDescriptorSetCacheKey *newSharedCacheKeyOut)
{
    vk::Renderer *renderer = context->getRenderer();

    if (renderer->getFeatures().descriptorSetCache.enabled)
    {
        ANGLE_TRY(mDynamicDescriptorPools[setIndex]->getOrAllocateDescriptorSet(
            context, currentFrame, descriptorSetDesc.getDesc(), *mDescriptorSetLayouts[setIndex],
            &mDescriptorSets[setIndex], newSharedCacheKeyOut));
        ASSERT(mDescriptorSets[setIndex]);

        if (*newSharedCacheKeyOut)
        {
            ASSERT((*newSharedCacheKeyOut)->valid());
            // Cache miss. A new cache entry has been created.
            descriptorSetDesc.updateDescriptorSet(renderer, writeDescriptorDescs, updateBuilder,
                                                  mDescriptorSets[setIndex]->getDescriptorSet());
        }
    }
    else
    {
        ANGLE_TRY(mDynamicDescriptorPools[setIndex]->allocateDescriptorSet(
            context, *mDescriptorSetLayouts[setIndex], &mDescriptorSets[setIndex]));
        ASSERT(mDescriptorSets[setIndex]);

        descriptorSetDesc.updateDescriptorSet(renderer, writeDescriptorDescs, updateBuilder,
                                              mDescriptorSets[setIndex]->getDescriptorSet());
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::updateShaderResourcesDescriptorSet(
    vk::Context *context,
    uint32_t currentFrame,
    UpdateDescriptorSetsBuilder *updateBuilder,
    const vk::WriteDescriptorDescs &writeDescriptorDescs,
    const vk::DescriptorSetDescBuilder &shaderResourcesDesc,
    vk::SharedDescriptorSetCacheKey *newSharedCacheKeyOut)
{
    if (!mDynamicDescriptorPools[DescriptorSetIndex::ShaderResource])
    {
        (*newSharedCacheKeyOut).reset();
        return angle::Result::Continue;
    }

    ANGLE_TRY(getOrAllocateDescriptorSet(context, currentFrame, updateBuilder, shaderResourcesDesc,
                                         writeDescriptorDescs, DescriptorSetIndex::ShaderResource,
                                         newSharedCacheKeyOut));

    size_t numOffsets = writeDescriptorDescs.getDynamicDescriptorSetCount();
    mDynamicShaderResourceDescriptorOffsets.resize(numOffsets);
    if (numOffsets > 0)
    {
        memcpy(mDynamicShaderResourceDescriptorOffsets.data(),
               shaderResourcesDesc.getDynamicOffsets(), numOffsets * sizeof(uint32_t));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableVk::updateUniformsAndXfbDescriptorSet(
    vk::Context *context,
    uint32_t currentFrame,
    UpdateDescriptorSetsBuilder *updateBuilder,
    const vk::WriteDescriptorDescs &writeDescriptorDescs,
    vk::BufferHelper *defaultUniformBuffer,
    vk::DescriptorSetDescBuilder *uniformsAndXfbDesc,
    vk::SharedDescriptorSetCacheKey *sharedCacheKeyOut)
{
    mCurrentDefaultUniformBufferSerial =
        defaultUniformBuffer ? defaultUniformBuffer->getBufferSerial() : vk::kInvalidBufferSerial;

    return getOrAllocateDescriptorSet(context, currentFrame, updateBuilder, *uniformsAndXfbDesc,
                                      writeDescriptorDescs, DescriptorSetIndex::UniformsAndXfb,
                                      sharedCacheKeyOut);
}

angle::Result ProgramExecutableVk::updateTexturesDescriptorSet(
    vk::Context *context,
    uint32_t currentFrame,
    const gl::ActiveTextureArray<TextureVk *> &textures,
    const gl::SamplerBindingVector &samplers,
    PipelineType pipelineType,
    UpdateDescriptorSetsBuilder *updateBuilder)
{
    if (context->getFeatures().descriptorSetCache.enabled)
    {
        vk::SharedDescriptorSetCacheKey newSharedCacheKey;

        // We use textureSerial to optimize texture binding updates. Each permutation of a
        // {VkImage/VkSampler} generates a unique serial. These object ids are combined to form a
        // unique signature for each descriptor set. This allows us to keep a cache of descriptor
        // sets and avoid calling vkAllocateDesctiporSets each texture update.
        vk::DescriptorSetDescBuilder descriptorBuilder;
        descriptorBuilder.updatePreCacheActiveTextures(context, *mExecutable, textures, samplers);

        ANGLE_TRY(mDynamicDescriptorPools[DescriptorSetIndex::Texture]->getOrAllocateDescriptorSet(
            context, currentFrame, descriptorBuilder.getDesc(),
            *mDescriptorSetLayouts[DescriptorSetIndex::Texture],
            &mDescriptorSets[DescriptorSetIndex::Texture], &newSharedCacheKey));
        ASSERT(mDescriptorSets[DescriptorSetIndex::Texture]);

        if (newSharedCacheKey)
        {
            ASSERT(newSharedCacheKey->valid());
            ANGLE_TRY(UpdateFullTexturesDescriptorSet(
                context, mVariableInfoMap, mTextureWriteDescriptorDescs, updateBuilder,
                *mExecutable, textures, samplers,
                mDescriptorSets[DescriptorSetIndex::Texture]->getDescriptorSet()));

            const gl::ActiveTextureMask &activeTextureMask = mExecutable->getActiveSamplersMask();
            for (size_t textureUnit : activeTextureMask)
            {
                ASSERT(textures[textureUnit] != nullptr);
                textures[textureUnit]->onNewDescriptorSet(newSharedCacheKey);
            }
        }
    }
    else
    {
        ANGLE_TRY(mDynamicDescriptorPools[DescriptorSetIndex::Texture]->allocateDescriptorSet(
            context, *mDescriptorSetLayouts[DescriptorSetIndex::Texture],
            &mDescriptorSets[DescriptorSetIndex::Texture]));
        ASSERT(mDescriptorSets[DescriptorSetIndex::Texture]);

        ANGLE_TRY(UpdateFullTexturesDescriptorSet(
            context, mVariableInfoMap, mTextureWriteDescriptorDescs, updateBuilder, *mExecutable,
            textures, samplers, mDescriptorSets[DescriptorSetIndex::Texture]->getDescriptorSet()));
    }

    return angle::Result::Continue;
}

template <typename CommandBufferT>
angle::Result ProgramExecutableVk::bindDescriptorSets(
    vk::ErrorContext *context,
    uint32_t currentFrame,
    vk::CommandBufferHelperCommon *commandBufferHelper,
    CommandBufferT *commandBuffer,
    PipelineType pipelineType)
{
    // Can probably use better dirty bits here.

    // Find the maximum non-null descriptor set.  This is used in conjunction with a driver
    // workaround to bind empty descriptor sets only for gaps in between 0 and max and avoid
    // binding unnecessary empty descriptor sets for the sets beyond max.
    DescriptorSetIndex lastNonNullDescriptorSetIndex = DescriptorSetIndex::InvalidEnum;
    for (DescriptorSetIndex descriptorSetIndex : angle::AllEnums<DescriptorSetIndex>())
    {
        if (mDescriptorSets[descriptorSetIndex])
        {
            lastNonNullDescriptorSetIndex = descriptorSetIndex;
        }
    }

    const VkPipelineBindPoint pipelineBindPoint = pipelineType == PipelineType::Compute
                                                      ? VK_PIPELINE_BIND_POINT_COMPUTE
                                                      : VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (DescriptorSetIndex descriptorSetIndex : angle::AllEnums<DescriptorSetIndex>())
    {
        if (ToUnderlying(descriptorSetIndex) > ToUnderlying(lastNonNullDescriptorSetIndex))
        {
            continue;
        }

        if (!mDescriptorSets[descriptorSetIndex])
        {
            continue;
        }

        VkDescriptorSet descSet = mDescriptorSets[descriptorSetIndex]->getDescriptorSet();
        ASSERT(descSet != VK_NULL_HANDLE);

        // Default uniforms are encompassed in a block per shader stage, and they are assigned
        // through dynamic uniform buffers (requiring dynamic offsets).  No other descriptor
        // requires a dynamic offset.
        if (descriptorSetIndex == DescriptorSetIndex::UniformsAndXfb)
        {
            commandBuffer->bindDescriptorSets(
                getPipelineLayout(), pipelineBindPoint, descriptorSetIndex, 1, &descSet,
                static_cast<uint32_t>(mDynamicUniformDescriptorOffsets.size()),
                mDynamicUniformDescriptorOffsets.data());
        }
        else if (descriptorSetIndex == DescriptorSetIndex::ShaderResource)
        {
            commandBuffer->bindDescriptorSets(
                getPipelineLayout(), pipelineBindPoint, descriptorSetIndex, 1, &descSet,
                static_cast<uint32_t>(mDynamicShaderResourceDescriptorOffsets.size()),
                mDynamicShaderResourceDescriptorOffsets.data());
        }
        else
        {
            commandBuffer->bindDescriptorSets(getPipelineLayout(), pipelineBindPoint,
                                              descriptorSetIndex, 1, &descSet, 0, nullptr);
        }

        commandBufferHelper->retainResource(mDescriptorSets[descriptorSetIndex].get());
        mDescriptorSets[descriptorSetIndex]->updateLastUsedFrame(currentFrame);
    }

    return angle::Result::Continue;
}

template angle::Result ProgramExecutableVk::bindDescriptorSets<vk::priv::SecondaryCommandBuffer>(
    vk::ErrorContext *context,
    uint32_t currentFrame,
    vk::CommandBufferHelperCommon *commandBufferHelper,
    vk::priv::SecondaryCommandBuffer *commandBuffer,
    PipelineType pipelineType);
template angle::Result ProgramExecutableVk::bindDescriptorSets<vk::VulkanSecondaryCommandBuffer>(
    vk::ErrorContext *context,
    uint32_t currentFrame,
    vk::CommandBufferHelperCommon *commandBufferHelper,
    vk::VulkanSecondaryCommandBuffer *commandBuffer,
    PipelineType pipelineType);

void ProgramExecutableVk::setAllDefaultUniformsDirty()
{
    mDefaultUniformBlocksDirty.reset();
    for (gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
    {
        if (!mDefaultUniformBlocks[shaderType]->uniformData.empty())
        {
            mDefaultUniformBlocksDirty.set(shaderType);
        }
    }
}

angle::Result ProgramExecutableVk::updateUniforms(vk::Context *context,
                                                  uint32_t currentFrame,
                                                  UpdateDescriptorSetsBuilder *updateBuilder,
                                                  vk::BufferHelper *emptyBuffer,
                                                  vk::DynamicBuffer *defaultUniformStorage,
                                                  bool isTransformFeedbackActiveUnpaused,
                                                  TransformFeedbackVk *transformFeedbackVk)
{
    ASSERT(mDefaultUniformBlocksDirty.any());

    vk::BufferHelper *defaultUniformBuffer;
    bool anyNewBufferAllocated          = false;
    gl::ShaderMap<VkDeviceSize> offsets = {};  // offset to the beginning of bufferData
    uint32_t offsetIndex                = 0;
    size_t requiredSpace;

    // We usually only update uniform data for shader stages that are actually dirty. But when the
    // buffer for uniform data have switched, because all shader stages are using the same buffer,
    // we then must update uniform data for all shader stages to keep all shader stages' uniform
    // data in the same buffer.
    requiredSpace = calcUniformUpdateRequiredSpace(context, &offsets);
    ASSERT(requiredSpace > 0);

    // Allocate space from dynamicBuffer. Always try to allocate from the current buffer first.
    // If that failed, we deal with fall out and try again.
    if (!defaultUniformStorage->allocateFromCurrentBuffer(requiredSpace, &defaultUniformBuffer))
    {
        setAllDefaultUniformsDirty();

        requiredSpace = calcUniformUpdateRequiredSpace(context, &offsets);
        ANGLE_TRY(defaultUniformStorage->allocate(context, requiredSpace, &defaultUniformBuffer,
                                                  &anyNewBufferAllocated));
    }

    ASSERT(defaultUniformBuffer);

    uint8_t *bufferData       = defaultUniformBuffer->getMappedMemory();
    VkDeviceSize bufferOffset = defaultUniformBuffer->getOffset();
    for (gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
    {
        if (mDefaultUniformBlocksDirty[shaderType])
        {
            const angle::MemoryBuffer &uniformData = mDefaultUniformBlocks[shaderType]->uniformData;
            memcpy(&bufferData[offsets[shaderType]], uniformData.data(), uniformData.size());
            mDynamicUniformDescriptorOffsets[offsetIndex] =
                static_cast<uint32_t>(bufferOffset + offsets[shaderType]);
            mDefaultUniformBlocksDirty.reset(shaderType);
        }
        ++offsetIndex;
    }
    ANGLE_TRY(defaultUniformBuffer->flush(context->getRenderer()));

    // Because the uniform buffers are per context, we can't rely on dynamicBuffer's allocate
    // function to tell us if you have got a new buffer or not. Other program's use of the buffer
    // might already pushed dynamicBuffer to a new buffer. We record which buffer (represented by
    // the unique BufferSerial number) we were using with the current descriptor set and then we
    // use that recorded BufferSerial compare to the current uniform buffer to quickly detect if
    // there is a buffer switch or not. We need to retrieve from the descriptor set cache or
    // allocate a new descriptor set whenever there is uniform buffer switch.
    if (mCurrentDefaultUniformBufferSerial != defaultUniformBuffer->getBufferSerial())
    {
        // We need to reinitialize the descriptor sets if we newly allocated buffers since we can't
        // modify the descriptor sets once initialized.
        const vk::WriteDescriptorDescs &writeDescriptorDescs =
            getDefaultUniformWriteDescriptorDescs(transformFeedbackVk);

        vk::DescriptorSetDescBuilder uniformsAndXfbDesc(
            writeDescriptorDescs.getTotalDescriptorCount());
        uniformsAndXfbDesc.updateUniformsAndXfb(
            context, *mExecutable, writeDescriptorDescs, defaultUniformBuffer, *emptyBuffer,
            isTransformFeedbackActiveUnpaused,
            mExecutable->hasTransformFeedbackOutput() ? transformFeedbackVk : nullptr);

        vk::SharedDescriptorSetCacheKey newSharedCacheKey;
        ANGLE_TRY(updateUniformsAndXfbDescriptorSet(context, currentFrame, updateBuilder,
                                                    writeDescriptorDescs, defaultUniformBuffer,
                                                    &uniformsAndXfbDesc, &newSharedCacheKey));
        if (newSharedCacheKey)
        {
            defaultUniformBuffer->getBufferBlock()->onNewDescriptorSet(newSharedCacheKey);
            if (mExecutable->hasTransformFeedbackOutput() &&
                context->getFeatures().emulateTransformFeedback.enabled)
            {
                transformFeedbackVk->onNewDescriptorSet(*mExecutable, newSharedCacheKey);
            }
        }
    }

    return angle::Result::Continue;
}

size_t ProgramExecutableVk::calcUniformUpdateRequiredSpace(
    vk::ErrorContext *context,
    gl::ShaderMap<VkDeviceSize> *uniformOffsets) const
{
    size_t requiredSpace = 0;
    for (gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
    {
        if (mDefaultUniformBlocksDirty[shaderType])
        {
            (*uniformOffsets)[shaderType] = requiredSpace;
            requiredSpace += getDefaultUniformAlignedSize(context, shaderType);
        }
    }
    return requiredSpace;
}

void ProgramExecutableVk::onProgramBind()
{
    // Because all programs share default uniform buffers, when we switch programs, we have to
    // re-update all uniform data. We could do more tracking to avoid update if the context's
    // current uniform buffer is still the same buffer we last time used and buffer has not been
    // recycled. But statistics gathered on gfxbench shows that app always update uniform data on
    // program bind anyway, so not really worth it to add more tracking logic here.
    //
    // Note: if this is changed, PPO uniform checks need to be updated as well
    setAllDefaultUniformsDirty();
}

angle::Result ProgramExecutableVk::resizeUniformBlockMemory(
    vk::ErrorContext *context,
    const gl::ShaderMap<size_t> &requiredBufferSize)
{
    for (gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
    {
        if (requiredBufferSize[shaderType] > 0)
        {
            if (!mDefaultUniformBlocks[shaderType]->uniformData.resize(
                    requiredBufferSize[shaderType]))
            {
                ANGLE_VK_CHECK(context, false, VK_ERROR_OUT_OF_HOST_MEMORY);
            }

            // Initialize uniform buffer memory to zero by default.
            mDefaultUniformBlocks[shaderType]->uniformData.fill(0);
            mDefaultUniformBlocksDirty.set(shaderType);
        }
    }

    return angle::Result::Continue;
}

void ProgramExecutableVk::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    SetUniform(mExecutable, location, count, v, GL_FLOAT, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    SetUniform(mExecutable, location, count, v, GL_FLOAT_VEC2, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    SetUniform(mExecutable, location, count, v, GL_FLOAT_VEC3, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    SetUniform(mExecutable, location, count, v, GL_FLOAT_VEC4, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    const gl::VariableLocation &locationInfo = mExecutable->getUniformLocations()[location];
    const gl::LinkedUniform &linkedUniform   = mExecutable->getUniforms()[locationInfo.index];
    if (linkedUniform.isSampler())
    {
        // We could potentially cache some indexing here. For now this is a no-op since the mapping
        // is handled entirely in ContextVk.
        return;
    }

    SetUniform(mExecutable, location, count, v, GL_INT, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    SetUniform(mExecutable, location, count, v, GL_INT_VEC2, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    SetUniform(mExecutable, location, count, v, GL_INT_VEC3, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    SetUniform(mExecutable, location, count, v, GL_INT_VEC4, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    SetUniform(mExecutable, location, count, v, GL_UNSIGNED_INT, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    SetUniform(mExecutable, location, count, v, GL_UNSIGNED_INT_VEC2, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    SetUniform(mExecutable, location, count, v, GL_UNSIGNED_INT_VEC3, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    SetUniform(mExecutable, location, count, v, GL_UNSIGNED_INT_VEC4, &mDefaultUniformBlocks,
               &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix2fv(GLint location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *value)
{
    SetUniformMatrixfv<2, 2>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix3fv(GLint location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *value)
{
    SetUniformMatrixfv<3, 3>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix4fv(GLint location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *value)
{
    SetUniformMatrixfv<4, 4>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix2x3fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    SetUniformMatrixfv<2, 3>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix3x2fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    SetUniformMatrixfv<3, 2>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix2x4fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    SetUniformMatrixfv<2, 4>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix4x2fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    SetUniformMatrixfv<4, 2>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix3x4fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    SetUniformMatrixfv<3, 4>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::setUniformMatrix4x3fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    SetUniformMatrixfv<4, 3>(mExecutable, location, count, transpose, value, &mDefaultUniformBlocks,
                             &mDefaultUniformBlocksDirty);
}

void ProgramExecutableVk::getUniformfv(const gl::Context *context,
                                       GLint location,
                                       GLfloat *params) const
{
    GetUniform(mExecutable, location, params, GL_FLOAT, &mDefaultUniformBlocks);
}

void ProgramExecutableVk::getUniformiv(const gl::Context *context,
                                       GLint location,
                                       GLint *params) const
{
    GetUniform(mExecutable, location, params, GL_INT, &mDefaultUniformBlocks);
}

void ProgramExecutableVk::getUniformuiv(const gl::Context *context,
                                        GLint location,
                                        GLuint *params) const
{
    GetUniform(mExecutable, location, params, GL_UNSIGNED_INT, &mDefaultUniformBlocks);
}
}  // namespace rx
