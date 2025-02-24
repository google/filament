//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramExecutableVk.h: Collects the information and interfaces common to both ProgramVks and
// ProgramPipelineVks in order to execute/draw with either.

#ifndef LIBANGLE_RENDERER_VULKAN_PROGRAMEXECUTABLEVK_H_
#define LIBANGLE_RENDERER_VULKAN_PROGRAMEXECUTABLEVK_H_

#include "common/bitset_utils.h"
#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/InfoLog.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/renderer/ProgramExecutableImpl.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ShaderInterfaceVariableInfoMap.h"
#include "libANGLE/renderer/vulkan/spv_utils.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ShaderInfo final : angle::NonCopyable
{
  public:
    ShaderInfo();
    ~ShaderInfo();

    angle::Result initShaders(vk::ErrorContext *context,
                              const gl::ShaderBitSet &linkedShaderStages,
                              const gl::ShaderMap<const angle::spirv::Blob *> &spirvBlobs,
                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                              bool isGLES1);
    void initShaderFromProgram(gl::ShaderType shaderType, const ShaderInfo &programShaderInfo);
    void clear();

    ANGLE_INLINE bool valid() const { return mIsInitialized; }

    const gl::ShaderMap<angle::spirv::Blob> &getSpirvBlobs() const { return mSpirvBlobs; }

    // Save and load implementation for GLES Program Binary support.
    void load(gl::BinaryInputStream *stream);
    void save(gl::BinaryOutputStream *stream);

  private:
    gl::ShaderMap<angle::spirv::Blob> mSpirvBlobs;
    bool mIsInitialized = false;
};

union ProgramTransformOptions final
{
    struct
    {
        uint8_t surfaceRotation : 1;
        uint8_t removeTransformFeedbackEmulation : 1;
        uint8_t multiSampleFramebufferFetch : 1;
        uint8_t enableSampleShading : 1;
        uint8_t removeDepthStencilInput : 1;
        uint8_t reserved : 3;  // must initialize to zero
    };
    uint8_t permutationIndex;
    static constexpr uint32_t kPermutationCount = 0x1 << 5;
};
static_assert(sizeof(ProgramTransformOptions) == 1, "Size check failed");
static_assert(static_cast<int>(SurfaceRotation::EnumCount) <= 8, "Size check failed");

class ProgramInfo final : angle::NonCopyable
{
  public:
    ProgramInfo();
    ~ProgramInfo();

    angle::Result initProgram(vk::ErrorContext *context,
                              gl::ShaderType shaderType,
                              bool isLastPreFragmentStage,
                              bool isTransformFeedbackProgram,
                              const ShaderInfo &shaderInfo,
                              ProgramTransformOptions optionBits,
                              const ShaderInterfaceVariableInfoMap &variableInfoMap);
    void release(ContextVk *contextVk);

    ANGLE_INLINE bool valid(gl::ShaderType shaderType) const
    {
        return mProgramHelper.valid(shaderType);
    }

    vk::ShaderProgramHelper &getShaderProgram() { return mProgramHelper; }

  private:
    vk::ShaderProgramHelper mProgramHelper;
    vk::ShaderModuleMap mShaders;
};

using ImmutableSamplerIndexMap = angle::HashMap<vk::YcbcrConversionDesc, uint32_t>;

class ProgramExecutableVk : public ProgramExecutableImpl
{
  public:
    ProgramExecutableVk(const gl::ProgramExecutable *executable);
    ~ProgramExecutableVk() override;

    void destroy(const gl::Context *context) override;

    void save(ContextVk *contextVk, bool isSeparable, gl::BinaryOutputStream *stream);
    angle::Result load(ContextVk *contextVk,
                       bool isSeparable,
                       gl::BinaryInputStream *stream,
                       egl::CacheGetResult *resultOut);

    void setUniform1fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform2fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform3fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform4fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform1iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform2iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform3iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform4iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform1uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform2uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform3uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform4uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniformMatrix2fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix3fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix4fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix2x3fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix3x2fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix2x4fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix4x2fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix3x4fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix4x3fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;

    void getUniformfv(const gl::Context *context, GLint location, GLfloat *params) const override;
    void getUniformiv(const gl::Context *context, GLint location, GLint *params) const override;
    void getUniformuiv(const gl::Context *context, GLint location, GLuint *params) const override;

    void clearVariableInfoMap();

    vk::BufferSerial getCurrentDefaultUniformBufferSerial() const
    {
        return mCurrentDefaultUniformBufferSerial;
    }

    // Get the graphics pipeline if already created.
    angle::Result getGraphicsPipeline(ContextVk *contextVk,
                                      vk::GraphicsPipelineSubset pipelineSubset,
                                      const vk::GraphicsPipelineDesc &desc,
                                      const vk::GraphicsPipelineDesc **descPtrOut,
                                      vk::PipelineHelper **pipelineOut);

    angle::Result createGraphicsPipeline(ContextVk *contextVk,
                                         vk::GraphicsPipelineSubset pipelineSubset,
                                         vk::PipelineCacheAccess *pipelineCache,
                                         PipelineSource source,
                                         const vk::GraphicsPipelineDesc &desc,
                                         const vk::GraphicsPipelineDesc **descPtrOut,
                                         vk::PipelineHelper **pipelineOut);

    angle::Result linkGraphicsPipelineLibraries(ContextVk *contextVk,
                                                vk::PipelineCacheAccess *pipelineCache,
                                                const vk::GraphicsPipelineDesc &desc,
                                                vk::PipelineHelper *vertexInputPipeline,
                                                vk::PipelineHelper *shadersPipeline,
                                                vk::PipelineHelper *fragmentOutputPipeline,
                                                const vk::GraphicsPipelineDesc **descPtrOut,
                                                vk::PipelineHelper **pipelineOut);

    angle::Result getOrCreateComputePipeline(vk::ErrorContext *context,
                                             vk::PipelineCacheAccess *pipelineCache,
                                             PipelineSource source,
                                             vk::PipelineRobustness pipelineRobustness,
                                             vk::PipelineProtectedAccess pipelineProtectedAccess,
                                             vk::PipelineHelper **pipelineOut);

    const vk::PipelineLayout &getPipelineLayout() const { return *mPipelineLayout; }
    void resetLayout(ContextVk *contextVk);
    angle::Result createPipelineLayout(vk::ErrorContext *context,
                                       PipelineLayoutCache *pipelineLayoutCache,
                                       DescriptorSetLayoutCache *descriptorSetLayoutCache,
                                       gl::ActiveTextureArray<TextureVk *> *activeTextures);
    angle::Result initializeDescriptorPools(
        vk::ErrorContext *context,
        DescriptorSetLayoutCache *descriptorSetLayoutCache,
        vk::DescriptorSetArray<vk::MetaDescriptorPool> *metaDescriptorPools);

    angle::Result updateTexturesDescriptorSet(vk::Context *context,
                                              uint32_t currentFrame,
                                              const gl::ActiveTextureArray<TextureVk *> &textures,
                                              const gl::SamplerBindingVector &samplers,
                                              PipelineType pipelineType,
                                              UpdateDescriptorSetsBuilder *updateBuilder);

    angle::Result updateShaderResourcesDescriptorSet(
        vk::Context *context,
        uint32_t currentFrame,
        UpdateDescriptorSetsBuilder *updateBuilder,
        const vk::WriteDescriptorDescs &writeDescriptorDescs,
        const vk::DescriptorSetDescBuilder &shaderResourcesDesc,
        vk::SharedDescriptorSetCacheKey *newSharedCacheKeyOut);

    angle::Result updateUniformsAndXfbDescriptorSet(
        vk::Context *context,
        uint32_t currentFrame,
        UpdateDescriptorSetsBuilder *updateBuilder,
        const vk::WriteDescriptorDescs &writeDescriptorDescs,
        vk::BufferHelper *defaultUniformBuffer,
        vk::DescriptorSetDescBuilder *uniformsAndXfbDesc,
        vk::SharedDescriptorSetCacheKey *sharedCacheKeyOut);

    template <typename CommandBufferT>
    angle::Result bindDescriptorSets(vk::ErrorContext *context,
                                     uint32_t currentFrame,
                                     vk::CommandBufferHelperCommon *commandBufferHelper,
                                     CommandBufferT *commandBuffer,
                                     PipelineType pipelineType);

    bool usesDynamicUniformBufferDescriptors() const
    {
        return mUniformBufferDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }
    VkDescriptorType getUniformBufferDescriptorType() const { return mUniformBufferDescriptorType; }
    bool usesDynamicShaderStorageBufferDescriptors() const { return false; }
    VkDescriptorType getStorageBufferDescriptorType() const
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    VkDescriptorType getAtomicCounterBufferDescriptorType() const
    {
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    bool usesDynamicAtomicCounterBufferDescriptors() const { return false; }

    bool areImmutableSamplersCompatible(
        const ImmutableSamplerIndexMap &immutableSamplerIndexMap) const
    {
        return (mImmutableSamplerIndexMap == immutableSamplerIndexMap);
    }

    size_t getDefaultUniformAlignedSize(vk::ErrorContext *context, gl::ShaderType shaderType) const
    {
        vk::Renderer *renderer = context->getRenderer();
        size_t alignment       = static_cast<size_t>(
            renderer->getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment);
        return roundUp(mDefaultUniformBlocks[shaderType]->uniformData.size(), alignment);
    }

    std::shared_ptr<BufferAndLayout> &getSharedDefaultUniformBlock(gl::ShaderType shaderType)
    {
        return mDefaultUniformBlocks[shaderType];
    }

    bool updateAndCheckDirtyUniforms()
    {
        if (ANGLE_LIKELY(!mExecutable->IsPPO()))
        {
            return mDefaultUniformBlocksDirty.any();
        }

        const auto &ppoExecutables = mExecutable->getPPOProgramExecutables();
        for (gl::ShaderType shaderType : mExecutable->getLinkedShaderStages())
        {
            ProgramExecutableVk *executableVk = vk::GetImpl(ppoExecutables[shaderType].get());
            if (executableVk->mDefaultUniformBlocksDirty.test(shaderType))
            {
                mDefaultUniformBlocksDirty.set(shaderType);
                // Note: this relies on onProgramBind marking everything as dirty
                executableVk->mDefaultUniformBlocksDirty.reset(shaderType);
            }
        }

        return mDefaultUniformBlocksDirty.any();
    }

    void setAllDefaultUniformsDirty();
    angle::Result updateUniforms(vk::Context *context,
                                 uint32_t currentFrame,
                                 UpdateDescriptorSetsBuilder *updateBuilder,
                                 vk::BufferHelper *emptyBuffer,
                                 vk::DynamicBuffer *defaultUniformStorage,
                                 bool isTransformFeedbackActiveUnpaused,
                                 TransformFeedbackVk *transformFeedbackVk);
    void onProgramBind();

    const ShaderInterfaceVariableInfoMap &getVariableInfoMap() const { return mVariableInfoMap; }

    angle::Result warmUpPipelineCache(vk::Renderer *renderer,
                                      vk::PipelineRobustness pipelineRobustness,
                                      vk::PipelineProtectedAccess pipelineProtectedAccess)
    {
        return getPipelineCacheWarmUpTasks(renderer, pipelineRobustness, pipelineProtectedAccess,
                                           nullptr);
    }
    angle::Result getPipelineCacheWarmUpTasks(
        vk::Renderer *renderer,
        vk::PipelineRobustness pipelineRobustness,
        vk::PipelineProtectedAccess pipelineProtectedAccess,
        std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut);

    void waitForPostLinkTasks(const gl::Context *context) override
    {
        ContextVk *contextVk = vk::GetImpl(context);
        waitForPostLinkTasksImpl(contextVk);
    }
    void waitForComputePostLinkTasks(ContextVk *contextVk)
    {
        ASSERT(mExecutable->hasLinkedShaderStage(gl::ShaderType::Compute));
        waitForPostLinkTasksImpl(contextVk);
    }
    void waitForGraphicsPostLinkTasks(ContextVk *contextVk,
                                      const vk::GraphicsPipelineDesc &currentGraphicsPipelineDesc);

    angle::Result mergePipelineCacheToRenderer(vk::ErrorContext *context) const;

    const vk::WriteDescriptorDescs &getShaderResourceWriteDescriptorDescs() const
    {
        return mShaderResourceWriteDescriptorDescs;
    }
    const vk::WriteDescriptorDescs &getDefaultUniformWriteDescriptorDescs(
        TransformFeedbackVk *transformFeedbackVk) const
    {
        return transformFeedbackVk == nullptr ? mDefaultUniformWriteDescriptorDescs
                                              : mDefaultUniformAndXfbWriteDescriptorDescs;
    }

    const vk::WriteDescriptorDescs &getTextureWriteDescriptorDescs() const
    {
        return mTextureWriteDescriptorDescs;
    }
    // The following functions are for internal use of programs, including from a threaded link job:
    angle::Result resizeUniformBlockMemory(vk::ErrorContext *context,
                                           const gl::ShaderMap<size_t> &requiredBufferSize);
    void resolvePrecisionMismatch(const gl::ProgramMergedVaryings &mergedVaryings);
    angle::Result initShaders(vk::ErrorContext *context,
                              const gl::ShaderBitSet &linkedShaderStages,
                              const gl::ShaderMap<const angle::spirv::Blob *> &spirvBlobs,
                              bool isGLES1)
    {
        return mOriginalShaderInfo.initShaders(context, linkedShaderStages, spirvBlobs,
                                               mVariableInfoMap, isGLES1);
    }
    void assignAllSpvLocations(vk::ErrorContext *context,
                               const gl::ProgramState &programState,
                               const gl::ProgramLinkedResources &resources)
    {
        SpvSourceOptions options = SpvCreateSourceOptions(
            context->getFeatures(), context->getRenderer()->getMaxColorInputAttachmentCount());
        SpvAssignAllLocations(options, programState, resources, &mVariableInfoMap);
    }

  private:
    class WarmUpTaskCommon;
    class WarmUpComputeTask;
    class WarmUpGraphicsTask;

    friend class ProgramVk;
    friend class ProgramPipelineVk;

    void reset(ContextVk *contextVk);

    void addInterfaceBlockDescriptorSetDesc(const std::vector<gl::InterfaceBlock> &blocks,
                                            gl::ShaderBitSet shaderTypes,
                                            VkDescriptorType descType,
                                            vk::DescriptorSetLayoutDesc *descOut);
    void addAtomicCounterBufferDescriptorSetDesc(
        const std::vector<gl::AtomicCounterBuffer> &atomicCounterBuffers,
        vk::DescriptorSetLayoutDesc *descOut);
    void addImageDescriptorSetDesc(vk::DescriptorSetLayoutDesc *descOut);
    void addInputAttachmentDescriptorSetDesc(vk::ErrorContext *context,
                                             vk::DescriptorSetLayoutDesc *descOut);
    angle::Result addTextureDescriptorSetDesc(
        vk::ErrorContext *context,
        const gl::ActiveTextureArray<TextureVk *> *activeTextures,
        vk::DescriptorSetLayoutDesc *descOut);

    size_t calcUniformUpdateRequiredSpace(vk::ErrorContext *context,
                                          gl::ShaderMap<VkDeviceSize> *uniformOffsets) const;

    ANGLE_INLINE angle::Result initProgram(vk::ErrorContext *context,
                                           gl::ShaderType shaderType,
                                           bool isLastPreFragmentStage,
                                           bool isTransformFeedbackProgram,
                                           ProgramTransformOptions optionBits,
                                           ProgramInfo *programInfo,
                                           const ShaderInterfaceVariableInfoMap &variableInfoMap)
    {
        ASSERT(mOriginalShaderInfo.valid());

        // Create the program pipeline.  This is done lazily and once per combination of
        // specialization constants.
        if (!programInfo->valid(shaderType))
        {
            ANGLE_TRY(programInfo->initProgram(context, shaderType, isLastPreFragmentStage,
                                               isTransformFeedbackProgram, mOriginalShaderInfo,
                                               optionBits, variableInfoMap));
        }
        ASSERT(programInfo->valid(shaderType));

        return angle::Result::Continue;
    }

    ANGLE_INLINE angle::Result initGraphicsShaderProgram(
        vk::ErrorContext *context,
        gl::ShaderType shaderType,
        bool isLastPreFragmentStage,
        bool isTransformFeedbackProgram,
        ProgramTransformOptions optionBits,
        ProgramInfo *programInfo,
        const ShaderInterfaceVariableInfoMap &variableInfoMap)
    {
        mValidGraphicsPermutations.set(optionBits.permutationIndex);
        return initProgram(context, shaderType, isLastPreFragmentStage, isTransformFeedbackProgram,
                           optionBits, programInfo, variableInfoMap);
    }

    ANGLE_INLINE angle::Result initComputeProgram(
        vk::ErrorContext *context,
        ProgramInfo *programInfo,
        const ShaderInterfaceVariableInfoMap &variableInfoMap,
        const vk::ComputePipelineOptions &pipelineOptions)
    {
        mValidComputePermutations.set(pipelineOptions.permutationIndex);
        ProgramTransformOptions optionBits = {};
        return initProgram(context, gl::ShaderType::Compute, false, false, optionBits, programInfo,
                           variableInfoMap);
    }

    ProgramTransformOptions getTransformOptions(ContextVk *contextVk,
                                                const vk::GraphicsPipelineDesc &desc);
    angle::Result initGraphicsShaderPrograms(vk::ErrorContext *context,
                                             ProgramTransformOptions transformOptions);
    angle::Result initProgramThenCreateGraphicsPipeline(vk::ErrorContext *context,
                                                        ProgramTransformOptions transformOptions,
                                                        vk::GraphicsPipelineSubset pipelineSubset,
                                                        vk::PipelineCacheAccess *pipelineCache,
                                                        PipelineSource source,
                                                        const vk::GraphicsPipelineDesc &desc,
                                                        const vk::RenderPass &compatibleRenderPass,
                                                        const vk::GraphicsPipelineDesc **descPtrOut,
                                                        vk::PipelineHelper **pipelineOut);
    angle::Result createGraphicsPipelineImpl(vk::ErrorContext *context,
                                             ProgramTransformOptions transformOptions,
                                             vk::GraphicsPipelineSubset pipelineSubset,
                                             vk::PipelineCacheAccess *pipelineCache,
                                             PipelineSource source,
                                             const vk::GraphicsPipelineDesc &desc,
                                             const vk::RenderPass &compatibleRenderPass,
                                             const vk::GraphicsPipelineDesc **descPtrOut,
                                             vk::PipelineHelper **pipelineOut);
    angle::Result prepareForWarmUpPipelineCache(
        vk::ErrorContext *context,
        vk::PipelineRobustness pipelineRobustness,
        vk::PipelineProtectedAccess pipelineProtectedAccess,
        vk::GraphicsPipelineSubset subset,
        bool *isComputeOut,
        angle::FixedVector<bool, 2> *surfaceRotationVariationsOut,
        vk::GraphicsPipelineDesc **graphicsPipelineDescOut,
        vk::RenderPass *renderPassOut);
    angle::Result warmUpComputePipelineCache(vk::ErrorContext *context,
                                             vk::PipelineRobustness pipelineRobustness,
                                             vk::PipelineProtectedAccess pipelineProtectedAccess);
    angle::Result warmUpGraphicsPipelineCache(vk::ErrorContext *context,
                                              vk::PipelineRobustness pipelineRobustness,
                                              vk::PipelineProtectedAccess pipelineProtectedAccess,
                                              vk::GraphicsPipelineSubset subset,
                                              const bool isSurfaceRotated,
                                              const vk::GraphicsPipelineDesc &graphicsPipelineDesc,
                                              const vk::RenderPass &renderPass,
                                              vk::PipelineHelper *placeholderPipelineHelper);
    void waitForPostLinkTasksImpl(ContextVk *contextVk);

    angle::Result getOrAllocateDescriptorSet(vk::Context *context,
                                             uint32_t currentFrame,
                                             UpdateDescriptorSetsBuilder *updateBuilder,
                                             const vk::DescriptorSetDescBuilder &descriptorSetDesc,
                                             const vk::WriteDescriptorDescs &writeDescriptorDescs,
                                             DescriptorSetIndex setIndex,
                                             vk::SharedDescriptorSetCacheKey *newSharedCacheKeyOut);

    // When loading from cache / binary, initialize the pipeline cache with given data.  Otherwise
    // the cache is lazily created as needed.
    angle::Result initializePipelineCache(vk::ErrorContext *context,
                                          bool compressed,
                                          const std::vector<uint8_t> &pipelineData);
    angle::Result ensurePipelineCacheInitialized(vk::ErrorContext *context);

    void initializeWriteDescriptorDesc(vk::ErrorContext *context);

    // Descriptor sets and pools for shader resources for this program.
    vk::DescriptorSetArray<vk::DescriptorSetPointer> mDescriptorSets;
    vk::DescriptorSetArray<vk::DynamicDescriptorPoolPointer> mDynamicDescriptorPools;
    vk::BufferSerial mCurrentDefaultUniformBufferSerial;

    // We keep a reference to the pipeline and descriptor set layouts. This ensures they don't get
    // deleted while this program is in use.
    uint32_t mImmutableSamplersMaxDescriptorCount;
    ImmutableSamplerIndexMap mImmutableSamplerIndexMap;
    vk::PipelineLayoutPtr mPipelineLayout;
    vk::DescriptorSetLayoutPointerArray mDescriptorSetLayouts;

    // A set of dynamic offsets used with vkCmdBindDescriptorSets for the default uniform buffers.
    VkDescriptorType mUniformBufferDescriptorType;
    gl::ShaderVector<uint32_t> mDynamicUniformDescriptorOffsets;
    std::vector<uint32_t> mDynamicShaderResourceDescriptorOffsets;

    ShaderInterfaceVariableInfoMap mVariableInfoMap;

    static_assert((ProgramTransformOptions::kPermutationCount == 32),
                  "ProgramTransformOptions::kPermutationCount must be 32.");
    angle::BitSet32<ProgramTransformOptions::kPermutationCount> mValidGraphicsPermutations;

    static_assert((vk::ComputePipelineOptions::kPermutationCount == 4),
                  "ComputePipelineOptions::kPermutationCount must be 4.");
    angle::BitSet8<vk::ComputePipelineOptions::kPermutationCount> mValidComputePermutations;

    // We store all permutations of surface rotation and transformed SPIR-V programs here. We may
    // need some LRU algorithm to free least used programs to reduce the number of programs.
    ProgramInfo mGraphicsProgramInfos[ProgramTransformOptions::kPermutationCount];
    ProgramInfo mComputeProgramInfo;

    // Pipeline caches.  The pipelines are tightly coupled with the shaders they are created for, so
    // they live in the program executable.  With VK_EXT_graphics_pipeline_library, the pipeline is
    // divided in subsets; the "shaders" subset is created based on the shaders, so its cache lives
    // in the program executable.  The "vertex input" and "fragment output" pipelines are
    // independent, and live in the context.
    CompleteGraphicsPipelineCache
        mCompleteGraphicsPipelines[ProgramTransformOptions::kPermutationCount];
    ShadersGraphicsPipelineCache
        mShadersGraphicsPipelines[ProgramTransformOptions::kPermutationCount];
    ComputePipelineCache mComputePipelines;

    DefaultUniformBlockMap mDefaultUniformBlocks;
    gl::ShaderBitSet mDefaultUniformBlocksDirty;

    ShaderInfo mOriginalShaderInfo;

    // The pipeline cache specific to this program executable.  Currently:
    //
    // - This is used during warm up (at link time)
    // - The contents are merged to Renderer's pipeline cache immediately after warm up
    // - The contents are returned as part of program binary
    // - Draw-time pipeline creation uses Renderer's cache
    //
    // Without VK_EXT_graphics_pipeline_library, this cache is not used for draw-time pipeline
    // creations to allow reuse of other blobs that are independent of the actual shaders; vertex
    // input fetch, fragment output and blend.
    //
    // With VK_EXT_graphics_pipeline_library, this cache is used for the "shaders" subset of the
    // pipeline.
    vk::PipelineCache mPipelineCache;

    vk::GraphicsPipelineDesc mWarmUpGraphicsPipelineDesc;

    // The "layout" information for descriptorSets
    vk::WriteDescriptorDescs mShaderResourceWriteDescriptorDescs;
    vk::WriteDescriptorDescs mTextureWriteDescriptorDescs;
    vk::WriteDescriptorDescs mDefaultUniformWriteDescriptorDescs;
    vk::WriteDescriptorDescs mDefaultUniformAndXfbWriteDescriptorDescs;

    vk::DescriptorSetLayoutDesc mShaderResourceSetDesc;
    vk::DescriptorSetLayoutDesc mTextureSetDesc;
    vk::DescriptorSetLayoutDesc mDefaultUniformAndXfbSetDesc;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMEXECUTABLEVK_H_
