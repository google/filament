//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutableMtl.cpp: Implementation of ProgramExecutableMtl.

#include "libANGLE/renderer/metal/ProgramExecutableMtl.h"

#include "libANGLE/renderer/metal/BufferMtl.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/TextureMtl.h"
#include "libANGLE/renderer/metal/blocklayoutMetal.h"
#include "libANGLE/renderer/metal/renderermtl_utils.h"

namespace rx
{
namespace
{
#define SHADER_ENTRY_NAME @"main0"

bool CompareBlockInfo(const sh::BlockMemberInfo &a, const sh::BlockMemberInfo &b)
{
    return a.offset < b.offset;
}

size_t GetAlignmentOfUniformGroup(sh::BlockLayoutMap *blockLayoutMap)
{
    size_t align = 1;
    for (auto layoutIter = blockLayoutMap->begin(); layoutIter != blockLayoutMap->end();
         ++layoutIter)
    {
        align = std::max(mtl::GetMetalAlignmentForGLType(layoutIter->second.type), align);
    }
    return align;
}

void InitDefaultUniformBlock(const std::vector<sh::Uniform> &uniforms,
                             sh::BlockLayoutMap *blockLayoutMapOut,
                             size_t *blockSizeOut)
{
    if (uniforms.empty())
    {
        *blockSizeOut = 0;
        return;
    }

    mtl::BlockLayoutEncoderMTL blockEncoder;
    sh::GetActiveUniformBlockInfo(uniforms, "", &blockEncoder, blockLayoutMapOut);
    size_t blockAlign = GetAlignmentOfUniformGroup(blockLayoutMapOut);
    size_t blockSize  = roundUp(blockEncoder.getCurrentOffset(), blockAlign);

    // TODO(jmadill): I think we still need a valid block for the pipeline even if zero sized.
    if (blockSize == 0)
    {
        *blockSizeOut = 0;
        return;
    }

    *blockSizeOut = blockSize;
    return;
}

template <typename T>
class [[nodiscard]] ScopedAutoClearVector
{
  public:
    ScopedAutoClearVector(std::vector<T> *array) : mArray(*array) {}
    ~ScopedAutoClearVector() { mArray.clear(); }

  private:
    std::vector<T> &mArray;
};

inline void memcpy_guarded(void *dst, const void *src, const void *maxSrcPtr, size_t size)
{
    size_t bytesAvailable = maxSrcPtr > src ? (const uint8_t *)maxSrcPtr - (const uint8_t *)src : 0;
    size_t bytesToCopy    = std::min(size, bytesAvailable);
    size_t bytesToZero    = size - bytesToCopy;

    if (bytesToCopy)
        memcpy(dst, src, bytesToCopy);
    if (bytesToZero)
        memset((uint8_t *)dst + bytesToCopy, 0, bytesToZero);
}

// Copy matrix one column at a time
inline void copy_matrix(void *dst,
                        const void *src,
                        const void *maxSrcPtr,
                        size_t srcStride,
                        size_t dstStride,
                        GLenum type)
{
    size_t elemSize      = mtl::GetMetalSizeForGLType(gl::VariableComponentType(type));
    const size_t dstRows = gl::VariableRowCount(type);
    const size_t dstCols = gl::VariableColumnCount(type);

    for (size_t col = 0; col < dstCols; col++)
    {
        size_t srcOffset = col * srcStride;
        memcpy_guarded(((uint8_t *)dst) + dstStride * col, (const uint8_t *)src + srcOffset,
                       maxSrcPtr, elemSize * dstRows);
    }
}

// Copy matrix one element at a time to transpose.
inline void copy_matrix_row_major(void *dst,
                                  const void *src,
                                  const void *maxSrcPtr,
                                  size_t srcStride,
                                  size_t dstStride,
                                  GLenum type)
{
    size_t elemSize      = mtl::GetMetalSizeForGLType(gl::VariableComponentType(type));
    const size_t dstRows = gl::VariableRowCount(type);
    const size_t dstCols = gl::VariableColumnCount(type);

    for (size_t col = 0; col < dstCols; col++)
    {
        for (size_t row = 0; row < dstRows; row++)
        {
            size_t srcOffset = row * srcStride + col * elemSize;
            memcpy_guarded((uint8_t *)dst + dstStride * col + row * elemSize,
                           (const uint8_t *)src + srcOffset, maxSrcPtr, elemSize);
        }
    }
}
// TODO(angleproject:7979) Upgrade ANGLE Uniform buffer remapper to compute shaders
angle::Result ConvertUniformBufferData(ContextMtl *contextMtl,
                                       const UBOConversionInfo &blockConversionInfo,
                                       mtl::BufferPool *dynamicBuffer,
                                       const uint8_t *sourceData,
                                       size_t sizeToCopy,
                                       mtl::BufferRef *bufferOut,
                                       size_t *bufferOffsetOut)
{
    uint8_t *dst             = nullptr;
    const uint8_t *maxSrcPtr = sourceData + sizeToCopy;
    dynamicBuffer->releaseInFlightBuffers(contextMtl);

    // When converting a UBO buffer, we convert all of the data
    // supplied in a buffer at once (sizeToCopy = bufferMtl->size() - initial offset).
    // It's possible that a buffer could represent multiple instances of
    // a uniform block, so we loop over the number of block conversions we intend
    // to do.
    size_t numBlocksToCopy =
        (sizeToCopy + blockConversionInfo.stdSize() - 1) / blockConversionInfo.stdSize();
    size_t bytesToAllocate = numBlocksToCopy * blockConversionInfo.metalSize();
    ANGLE_TRY(dynamicBuffer->allocate(contextMtl, bytesToAllocate, &dst, bufferOut, bufferOffsetOut,
                                      nullptr));

    const std::vector<sh::BlockMemberInfo> &stdConversions = blockConversionInfo.stdInfo();
    const std::vector<sh::BlockMemberInfo> &mtlConversions = blockConversionInfo.metalInfo();
    for (size_t i = 0; i < numBlocksToCopy; ++i)
    {
        auto stdIterator = stdConversions.begin();
        auto mtlIterator = mtlConversions.begin();

        while (stdIterator != stdConversions.end())
        {
            for (int arraySize = 0; arraySize < stdIterator->arraySize; ++arraySize)
            {
                // For every entry in an array, calculate the offset based off of the
                // array element size.

                // Offset of a single entry is
                // blockIndex*blockSize + arrayOffset*arraySize + offset of field in base struct.
                // Fields are copied per block, per member, per array entry of member.

                size_t stdArrayOffset = stdIterator->arrayStride * arraySize;
                size_t mtlArrayOffset = mtlIterator->arrayStride * arraySize;

                if (gl::IsMatrixType(mtlIterator->type))
                {

                    void *dstMat = dst + mtlIterator->offset + mtlArrayOffset +
                                   blockConversionInfo.metalSize() * i;
                    const void *srcMat = sourceData + stdIterator->offset + stdArrayOffset +
                                         blockConversionInfo.stdSize() * i;
                    // Transpose matricies into column major order, if they're row major encoded.
                    if (stdIterator->isRowMajorMatrix)
                    {
                        copy_matrix_row_major(dstMat, srcMat, maxSrcPtr, stdIterator->matrixStride,
                                              mtlIterator->matrixStride, mtlIterator->type);
                    }
                    else
                    {
                        copy_matrix(dstMat, srcMat, maxSrcPtr, stdIterator->matrixStride,
                                    mtlIterator->matrixStride, mtlIterator->type);
                    }
                }
                // Compress bool from four bytes to one byte because bool values in GLSL
                // are uint-sized: ES 3.0 Section 2.12.6.3 "Uniform Buffer Object Storage".
                // Bools in metal are byte-sized. (Metal shading language spec Table 2.2)
                else if (gl::VariableComponentType(mtlIterator->type) == GL_BOOL)
                {
                    for (int boolCol = 0; boolCol < gl::VariableComponentCount(mtlIterator->type);
                         boolCol++)
                    {
                        const uint8_t *srcBool =
                            (sourceData + stdIterator->offset + stdArrayOffset +
                             blockConversionInfo.stdSize() * i +
                             gl::VariableComponentSize(GL_BOOL) * boolCol);
                        unsigned int srcValue =
                            srcBool < maxSrcPtr ? *((unsigned int *)(srcBool)) : 0;
                        uint8_t *dstBool = dst + mtlIterator->offset + mtlArrayOffset +
                                           blockConversionInfo.metalSize() * i +
                                           sizeof(bool) * boolCol;
                        *dstBool = (srcValue != 0);
                    }
                }
                else
                {
                    memcpy_guarded(dst + mtlIterator->offset + mtlArrayOffset +
                                       blockConversionInfo.metalSize() * i,
                                   sourceData + stdIterator->offset + stdArrayOffset +
                                       blockConversionInfo.stdSize() * i,
                                   maxSrcPtr, mtl::GetMetalSizeForGLType(mtlIterator->type));
                }
            }
            ++stdIterator;
            ++mtlIterator;
        }
    }

    ANGLE_TRY(dynamicBuffer->commit(contextMtl));
    return angle::Result::Continue;
}

constexpr size_t PipelineParametersToFragmentShaderVariantIndex(bool multisampledRendering,
                                                                bool allowFragDepthWrite)
{
    const size_t index = (allowFragDepthWrite << 1) | multisampledRendering;
    ASSERT(index < kFragmentShaderVariants);
    return index;
}

void InitArgumentBufferEncoder(mtl::Context *context,
                               id<MTLFunction> function,
                               uint32_t bufferIndex,
                               ProgramArgumentBufferEncoderMtl *encoder)
{
    encoder->metalArgBufferEncoder =
        mtl::adoptObjCPtr([function newArgumentEncoderWithBufferIndex:bufferIndex]);
    if (encoder->metalArgBufferEncoder)
    {
        encoder->bufferPool.initialize(context, encoder->metalArgBufferEncoder.get().encodedLength,
                                       mtl::kArgumentBufferOffsetAlignment, 0);
    }
}

template <typename T>
void UpdateDefaultUniformBlockWithElementSize(GLsizei count,
                                              uint32_t arrayIndex,
                                              int componentCount,
                                              const T *v,
                                              size_t baseElementSize,
                                              const sh::BlockMemberInfo &layoutInfo,
                                              angle::MemoryBuffer *uniformData)
{
    const int elementSize = (int)(baseElementSize * componentCount);

    uint8_t *dst = uniformData->data() + layoutInfo.offset;
    if (layoutInfo.arrayStride == 0 || layoutInfo.arrayStride == elementSize)
    {
        uint32_t arrayOffset = arrayIndex * layoutInfo.arrayStride;
        uint8_t *writePtr    = dst + arrayOffset;
        ASSERT(writePtr + (elementSize * count) <= uniformData->data() + uniformData->size());
        memcpy(writePtr, v, elementSize * count);
    }
    else
    {
        // Have to respect the arrayStride between each element of the array.
        int maxIndex = arrayIndex + count;
        for (int writeIndex = arrayIndex, readIndex = 0; writeIndex < maxIndex;
             writeIndex++, readIndex++)
        {
            const int arrayOffset = writeIndex * layoutInfo.arrayStride;
            uint8_t *writePtr     = dst + arrayOffset;
            const T *readPtr      = v + (readIndex * componentCount);
            ASSERT(writePtr + elementSize <= uniformData->data() + uniformData->size());
            memcpy(writePtr, readPtr, elementSize);
        }
    }
}
template <typename T>
void ReadFromDefaultUniformBlock(int componentCount,
                                 uint32_t arrayIndex,
                                 T *dst,
                                 size_t elementSize,
                                 const sh::BlockMemberInfo &layoutInfo,
                                 const angle::MemoryBuffer *uniformData)
{
    ReadFromDefaultUniformBlockWithElementSize(componentCount, arrayIndex, dst, sizeof(T),
                                               layoutInfo, uniformData);
}

void ReadFromDefaultUniformBlockWithElementSize(int componentCount,
                                                uint32_t arrayIndex,
                                                void *dst,
                                                size_t baseElementSize,
                                                const sh::BlockMemberInfo &layoutInfo,
                                                const angle::MemoryBuffer *uniformData)
{
    ASSERT(layoutInfo.offset != -1);

    const size_t elementSize = (baseElementSize * componentCount);
    const uint8_t *source    = uniformData->data() + layoutInfo.offset;

    if (layoutInfo.arrayStride == 0 || (size_t)layoutInfo.arrayStride == elementSize)
    {
        const uint8_t *readPtr = source + arrayIndex * layoutInfo.arrayStride;
        memcpy(dst, readPtr, elementSize);
    }
    else
    {
        // Have to respect the arrayStride between each element of the array.
        const int arrayOffset  = arrayIndex * layoutInfo.arrayStride;
        const uint8_t *readPtr = source + arrayOffset;
        memcpy(dst, readPtr, elementSize);
    }
}

class Std140BlockLayoutEncoderFactory : public gl::CustomBlockLayoutEncoderFactory
{
  public:
    sh::BlockLayoutEncoder *makeEncoder() override { return new sh::Std140BlockEncoder(); }
};

class Std430BlockLayoutEncoderFactory : public gl::CustomBlockLayoutEncoderFactory
{
  public:
    sh::BlockLayoutEncoder *makeEncoder() override { return new sh::Std430BlockEncoder(); }
};

class StdMTLBLockLayoutEncoderFactory : public gl::CustomBlockLayoutEncoderFactory
{
  public:
    sh::BlockLayoutEncoder *makeEncoder() override { return new mtl::BlockLayoutEncoderMTL(); }
};
}  // anonymous namespace

angle::Result CreateMslShaderLib(mtl::Context *context,
                                 gl::InfoLog &infoLog,
                                 mtl::TranslatedShaderInfo *translatedMslInfo,
                                 const std::map<std::string, std::string> &substitutionMacros)
{
    ANGLE_MTL_OBJC_SCOPE
    {
        mtl::LibraryCache &libraryCache = context->getDisplay()->getLibraryCache();

        // Convert to actual binary shader
        mtl::AutoObjCPtr<NSError *> err = nil;
        const bool disableFastMath =
            context->getDisplay()->getFeatures().intelDisableFastMath.enabled ||
            translatedMslInfo->hasIsnanOrIsinf;
        const bool usesInvariance       = translatedMslInfo->hasInvariant;
        translatedMslInfo->metalLibrary = libraryCache.getOrCompileShaderLibrary(
            context->getDisplay(), translatedMslInfo->metalShaderSource, substitutionMacros,
            disableFastMath, usesInvariance, &err);
        if (err || !translatedMslInfo->metalLibrary)
        {
            infoLog << "Internal error while linking shader. MSL compilation error:\n"
                    << (err ? err.get().localizedDescription.UTF8String : "unknown error")
                    << ".\nTranslated source:\n"
                    << *(translatedMslInfo->metalShaderSource);
            ANGLE_MTL_CHECK(context, translatedMslInfo->metalLibrary, err);
        }
        return angle::Result::Continue;
    }
}
DefaultUniformBlockMtl::DefaultUniformBlockMtl() {}

DefaultUniformBlockMtl::~DefaultUniformBlockMtl() = default;

ProgramExecutableMtl::ProgramExecutableMtl(const gl::ProgramExecutable *executable)
    : ProgramExecutableImpl(executable), mProgramHasFlatAttributes(false), mShadowCompareModes{}
{
    mCurrentShaderVariants.fill(nullptr);

    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mMslShaderTranslateInfo[shaderType].reset();
    }
    mMslXfbOnlyVertexShaderInfo.reset();
}

ProgramExecutableMtl::~ProgramExecutableMtl() {}

void ProgramExecutableMtl::destroy(const gl::Context *context)
{
    auto contextMtl = mtl::GetImpl(context);
    reset(contextMtl);
}

void ProgramExecutableMtl::reset(ContextMtl *context)
{
    mProgramHasFlatAttributes = false;

    for (auto &block : mDefaultUniformBlocks)
    {
        block.uniformLayout.clear();
    }

    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mMslShaderTranslateInfo[shaderType].reset();
        mCurrentShaderVariants[shaderType] = nullptr;
    }
    mMslXfbOnlyVertexShaderInfo.reset();

    for (ProgramShaderObjVariantMtl &var : mVertexShaderVariants)
    {
        var.reset(context);
    }
    for (ProgramShaderObjVariantMtl &var : mFragmentShaderVariants)
    {
        var.reset(context);
    }

    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        if (mDefaultUniformBufferPools[shaderType])
        {
            mDefaultUniformBufferPools[shaderType]->destroy(context);
            mDefaultUniformBufferPools[shaderType].reset();
        }
    }
}

angle::Result ProgramExecutableMtl::load(ContextMtl *contextMtl, gl::BinaryInputStream *stream)
{
    loadTranslatedShaders(stream);
    loadShaderInternalInfo(stream);
    ANGLE_TRY(loadDefaultUniformBlocksInfo(contextMtl, stream));
    return loadInterfaceBlockInfo(stream);
}

void ProgramExecutableMtl::save(gl::BinaryOutputStream *stream)
{
    saveTranslatedShaders(stream);
    saveShaderInternalInfo(stream);
    saveDefaultUniformBlocksInfo(stream);
    saveInterfaceBlockInfo(stream);
}

void ProgramExecutableMtl::saveInterfaceBlockInfo(gl::BinaryOutputStream *stream)
{
    // Serializes the uniformLayout data of mDefaultUniformBlocks
    // First, save the number of Ib's to process
    stream->writeInt<unsigned int>((unsigned int)mUniformBlockConversions.size());
    // Next, iterate through all of the conversions.
    for (auto conversion : mUniformBlockConversions)
    {
        // Write the name of the conversion
        stream->writeString(conversion.first);
        // Write the number of entries in the conversion
        const UBOConversionInfo &conversionInfo = conversion.second;
        stream->writeVector(conversionInfo.stdInfo());
        stream->writeVector(conversionInfo.metalInfo());
        stream->writeInt<size_t>(conversionInfo.stdSize());
        stream->writeInt<size_t>(conversionInfo.metalSize());
    }
}

angle::Result ProgramExecutableMtl::loadInterfaceBlockInfo(gl::BinaryInputStream *stream)
{
    mUniformBlockConversions.clear();
    // First, load the number of Ib's to process
    uint32_t numBlocks = stream->readInt<uint32_t>();
    // Next, iterate through all of the conversions.
    for (uint32_t nBlocks = 0; nBlocks < numBlocks; ++nBlocks)
    {
        // Read the name of the conversion
        std::string blockName = stream->readString();
        // Read the number of entries in the conversion
        std::vector<sh::BlockMemberInfo> stdInfo, metalInfo;
        stream->readVector(&stdInfo);
        stream->readVector(&metalInfo);
        size_t stdSize   = stream->readInt<size_t>();
        size_t metalSize = stream->readInt<size_t>();
        mUniformBlockConversions.insert(
            {blockName, UBOConversionInfo(stdInfo, metalInfo, stdSize, metalSize)});
    }
    return angle::Result::Continue;
}

void ProgramExecutableMtl::saveDefaultUniformBlocksInfo(gl::BinaryOutputStream *stream)
{
    // Serializes the uniformLayout data of mDefaultUniformBlocks
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->writeVector(mDefaultUniformBlocks[shaderType].uniformLayout);
    }

    // Serializes required uniform block memory sizes
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->writeInt(mDefaultUniformBlocks[shaderType].uniformData.size());
    }
}

angle::Result ProgramExecutableMtl::loadDefaultUniformBlocksInfo(mtl::Context *context,
                                                                 gl::BinaryInputStream *stream)
{
    gl::ShaderMap<size_t> requiredBufferSize;
    requiredBufferSize.fill(0);
    // Deserializes the uniformLayout data of mDefaultUniformBlocks
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->readVector(&mDefaultUniformBlocks[shaderType].uniformLayout);
    }

    // Deserializes required uniform block memory sizes
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        requiredBufferSize[shaderType] = stream->readInt<size_t>();
    }

    return resizeDefaultUniformBlocksMemory(context, requiredBufferSize);
}

void ProgramExecutableMtl::saveShaderInternalInfo(gl::BinaryOutputStream *stream)
{
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        stream->writeInt<int>(mMslShaderTranslateInfo[shaderType].hasUBOArgumentBuffer);
        for (const mtl::SamplerBinding &binding :
             mMslShaderTranslateInfo[shaderType].actualSamplerBindings)
        {
            stream->writeInt<uint32_t>(binding.textureBinding);
            stream->writeInt<uint32_t>(binding.samplerBinding);
        }
        for (int rwTextureBinding : mMslShaderTranslateInfo[shaderType].actualImageBindings)
        {
            stream->writeInt<int>(rwTextureBinding);
        }

        for (uint32_t uboBinding : mMslShaderTranslateInfo[shaderType].actualUBOBindings)
        {
            stream->writeInt<uint32_t>(uboBinding);
        }
        stream->writeBool(mMslShaderTranslateInfo[shaderType].hasInvariant);
    }
    for (size_t xfbBindIndex = 0; xfbBindIndex < mtl::kMaxShaderXFBs; xfbBindIndex++)
    {
        stream->writeInt(
            mMslShaderTranslateInfo[gl::ShaderType::Vertex].actualXFBBindings[xfbBindIndex]);
    }

    // Write out XFB info.
    {
        stream->writeInt<int>(mMslXfbOnlyVertexShaderInfo.hasUBOArgumentBuffer);
        for (mtl::SamplerBinding &binding : mMslXfbOnlyVertexShaderInfo.actualSamplerBindings)
        {
            stream->writeInt<uint32_t>(binding.textureBinding);
            stream->writeInt<uint32_t>(binding.samplerBinding);
        }
        for (int rwTextureBinding : mMslXfbOnlyVertexShaderInfo.actualImageBindings)
        {
            stream->writeInt<int>(rwTextureBinding);
        }

        for (uint32_t &uboBinding : mMslXfbOnlyVertexShaderInfo.actualUBOBindings)
        {
            stream->writeInt<uint32_t>(uboBinding);
        }
        for (size_t xfbBindIndex = 0; xfbBindIndex < mtl::kMaxShaderXFBs; xfbBindIndex++)
        {
            stream->writeInt(mMslXfbOnlyVertexShaderInfo.actualXFBBindings[xfbBindIndex]);
        }
    }

    stream->writeBool(mProgramHasFlatAttributes);
}

void ProgramExecutableMtl::loadShaderInternalInfo(gl::BinaryInputStream *stream)
{
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        mMslShaderTranslateInfo[shaderType].hasUBOArgumentBuffer = stream->readInt<int>() != 0;
        for (mtl::SamplerBinding &binding :
             mMslShaderTranslateInfo[shaderType].actualSamplerBindings)
        {
            binding.textureBinding = stream->readInt<uint32_t>();
            binding.samplerBinding = stream->readInt<uint32_t>();
        }
        for (int &rwTextureBinding : mMslShaderTranslateInfo[shaderType].actualImageBindings)
        {
            rwTextureBinding = stream->readInt<int>();
        }

        for (uint32_t &uboBinding : mMslShaderTranslateInfo[shaderType].actualUBOBindings)
        {
            uboBinding = stream->readInt<uint32_t>();
        }
        mMslShaderTranslateInfo[shaderType].hasInvariant = stream->readBool();
    }

    for (size_t xfbBindIndex = 0; xfbBindIndex < mtl::kMaxShaderXFBs; xfbBindIndex++)
    {
        stream->readInt(
            &mMslShaderTranslateInfo[gl::ShaderType::Vertex].actualXFBBindings[xfbBindIndex]);
    }
    // Load Transform Feedback info
    {
        mMslXfbOnlyVertexShaderInfo.hasUBOArgumentBuffer = stream->readInt<int>() != 0;
        for (mtl::SamplerBinding &binding : mMslXfbOnlyVertexShaderInfo.actualSamplerBindings)
        {
            binding.textureBinding = stream->readInt<uint32_t>();
            binding.samplerBinding = stream->readInt<uint32_t>();
        }
        for (int &rwTextureBinding : mMslXfbOnlyVertexShaderInfo.actualImageBindings)
        {
            rwTextureBinding = stream->readInt<int>();
        }

        for (uint32_t &uboBinding : mMslXfbOnlyVertexShaderInfo.actualUBOBindings)
        {
            uboBinding = stream->readInt<uint32_t>();
        }
        for (size_t xfbBindIndex = 0; xfbBindIndex < mtl::kMaxShaderXFBs; xfbBindIndex++)
        {
            stream->readInt(&mMslXfbOnlyVertexShaderInfo.actualXFBBindings[xfbBindIndex]);
        }
        mMslXfbOnlyVertexShaderInfo.metalLibrary = nullptr;
    }

    mProgramHasFlatAttributes = stream->readBool();
}

void ProgramExecutableMtl::saveTranslatedShaders(gl::BinaryOutputStream *stream)
{
    auto writeTranslatedSource = [](gl::BinaryOutputStream *stream,
                                    const mtl::TranslatedShaderInfo &shaderInfo) {
        const std::string &source =
            shaderInfo.metalShaderSource ? *shaderInfo.metalShaderSource : std::string();
        stream->writeString(source);
    };

    // Write out shader sources for all shader types
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        writeTranslatedSource(stream, mMslShaderTranslateInfo[shaderType]);
    }
    writeTranslatedSource(stream, mMslXfbOnlyVertexShaderInfo);
}

void ProgramExecutableMtl::loadTranslatedShaders(gl::BinaryInputStream *stream)
{
    auto readTranslatedSource = [](gl::BinaryInputStream *stream,
                                   mtl::TranslatedShaderInfo &shaderInfo) {
        std::string source = stream->readString();
        if (!source.empty())
        {
            shaderInfo.metalShaderSource = std::make_shared<const std::string>(std::move(source));
        }
        else
        {
            shaderInfo.metalShaderSource = nullptr;
        }
    };

    // Read in shader sources for all shader types
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        readTranslatedSource(stream, mMslShaderTranslateInfo[shaderType]);
    }
    readTranslatedSource(stream, mMslXfbOnlyVertexShaderInfo);
}

void ProgramExecutableMtl::linkUpdateHasFlatAttributes(
    const gl::SharedCompiledShaderState &vertexShader)
{
    mProgramHasFlatAttributes = false;

    const auto &programInputs = mExecutable->getProgramInputs();
    for (auto &attribute : programInputs)
    {
        if (attribute.getInterpolation() == sh::INTERPOLATION_FLAT)
        {
            mProgramHasFlatAttributes = true;
            return;
        }
    }

    const auto &flatVaryings = vertexShader->outputVaryings;
    for (auto &attribute : flatVaryings)
    {
        if (attribute.interpolation == sh::INTERPOLATION_FLAT)
        {
            mProgramHasFlatAttributes = true;
            return;
        }
    }
}

angle::Result ProgramExecutableMtl::initDefaultUniformBlocks(
    mtl::Context *context,
    const gl::ShaderMap<gl::SharedCompiledShaderState> &shaders)
{
    // Process vertex and fragment uniforms into std140 packing.
    gl::ShaderMap<sh::BlockLayoutMap> layoutMap;
    gl::ShaderMap<size_t> requiredBufferSize;
    requiredBufferSize.fill(0);

    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        const gl::SharedCompiledShaderState &shader = shaders[shaderType];
        if (shader)
        {
            const std::vector<sh::Uniform> &uniforms = shader->uniforms;
            InitDefaultUniformBlock(uniforms, &layoutMap[shaderType],
                                    &requiredBufferSize[shaderType]);
            // Set up block conversion buffer
            initUniformBlocksRemapper(shader);
        }
    }

    // Init the default block layout info.
    const auto &uniforms         = mExecutable->getUniforms();
    const auto &uniformNames     = mExecutable->getUniformNames();
    const auto &uniformLocations = mExecutable->getUniformLocations();
    for (size_t locSlot = 0; locSlot < uniformLocations.size(); ++locSlot)
    {
        const gl::VariableLocation &location = uniformLocations[locSlot];
        gl::ShaderMap<sh::BlockMemberInfo> layoutInfo;

        if (location.used() && !location.ignored)
        {
            const gl::LinkedUniform &uniform = uniforms[location.index];
            if (uniform.isInDefaultBlock() && !uniform.isSampler() && !uniform.isImage())
            {
                std::string uniformName = uniformNames[location.index];
                if (uniform.isArray())
                {
                    // Gets the uniform name without the [0] at the end.
                    uniformName = gl::ParseResourceName(uniformName, nullptr);
                }

                bool found = false;

                for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
                {
                    auto it = layoutMap[shaderType].find(uniformName);
                    if (it != layoutMap[shaderType].end())
                    {
                        found                  = true;
                        layoutInfo[shaderType] = it->second;
                    }
                }

                ASSERT(found);
            }
        }

        for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
        {
            mDefaultUniformBlocks[shaderType].uniformLayout.push_back(layoutInfo[shaderType]);
        }
    }

    return resizeDefaultUniformBlocksMemory(context, requiredBufferSize);
}

angle::Result ProgramExecutableMtl::resizeDefaultUniformBlocksMemory(
    mtl::Context *context,
    const gl::ShaderMap<size_t> &requiredBufferSize)
{
    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        if (requiredBufferSize[shaderType] > 0)
        {
            ASSERT(requiredBufferSize[shaderType] <= mtl::kDefaultUniformsMaxSize);

            ANGLE_CHECK_GL_ALLOC(context, mDefaultUniformBlocks[shaderType].uniformData.resize(
                                              requiredBufferSize[shaderType]));
            // Initialize uniform buffer memory to zero by default.
            mDefaultUniformBlocks[shaderType].uniformData.fill(0);
            mDefaultUniformBlocksDirty.set(shaderType);
        }
    }

    return angle::Result::Continue;
}

// TODO(angleproject:7979) Upgrade ANGLE Uniform buffer remapper to compute shaders
void ProgramExecutableMtl::initUniformBlocksRemapper(const gl::SharedCompiledShaderState &shader)
{
    std::unordered_map<std::string, UBOConversionInfo> conversionMap;
    const std::vector<sh::InterfaceBlock> ibs = shader->uniformBlocks;
    for (size_t i = 0; i < ibs.size(); ++i)
    {

        const sh::InterfaceBlock &ib = ibs[i];
        if (mUniformBlockConversions.find(ib.name) == mUniformBlockConversions.end())
        {
            mtl::BlockLayoutEncoderMTL metalEncoder;
            sh::BlockLayoutEncoder *encoder;
            switch (ib.layout)
            {
                case sh::BLOCKLAYOUT_PACKED:
                case sh::BLOCKLAYOUT_SHARED:
                case sh::BLOCKLAYOUT_STD140:
                {
                    Std140BlockLayoutEncoderFactory factory;
                    encoder = factory.makeEncoder();
                }
                break;
                case sh::BLOCKLAYOUT_STD430:
                {
                    Std430BlockLayoutEncoderFactory factory;
                    encoder = factory.makeEncoder();
                }
                break;
            }
            sh::BlockLayoutMap blockLayoutMapOut, stdMapOut;

            sh::GetInterfaceBlockInfo(ib.fields, "", &metalEncoder, &blockLayoutMapOut);
            sh::GetInterfaceBlockInfo(ib.fields, "", encoder, &stdMapOut);

            auto stdIterator = stdMapOut.begin();
            auto mtlIterator = blockLayoutMapOut.begin();

            std::vector<sh::BlockMemberInfo> stdConversions, mtlConversions;
            while (stdIterator != stdMapOut.end())
            {
                stdConversions.push_back(stdIterator->second);
                mtlConversions.push_back(mtlIterator->second);
                stdIterator++;
                mtlIterator++;
            }
            std::sort(stdConversions.begin(), stdConversions.end(), CompareBlockInfo);
            std::sort(mtlConversions.begin(), mtlConversions.end(), CompareBlockInfo);

            size_t stdSize    = encoder->getCurrentOffset();
            size_t metalAlign = GetAlignmentOfUniformGroup(&blockLayoutMapOut);
            size_t metalSize  = roundUp(metalEncoder.getCurrentOffset(), metalAlign);

            conversionMap.insert(
                {ib.name, UBOConversionInfo(stdConversions, mtlConversions, stdSize, metalSize)});
            SafeDelete(encoder);
        }
    }
    mUniformBlockConversions.insert(conversionMap.begin(), conversionMap.end());
}

mtl::BufferPool *ProgramExecutableMtl::getBufferPool(ContextMtl *context, gl::ShaderType shaderType)
{
    auto &pool = mDefaultUniformBufferPools[shaderType];
    if (pool == nullptr)
    {
        DefaultUniformBlockMtl &uniformBlock = mDefaultUniformBlocks[shaderType];

        // Size each buffer to hold 10 draw calls worth of uniform updates before creating extra
        // buffers. This number was chosen loosely to balance the size of buffers versus the total
        // number allocated. Without any sub-allocation, the total buffer count can reach the
        // thousands when many draw calls are issued with the same program.
        size_t bufferSize =
            std::max(uniformBlock.uniformData.size() * 10, mtl::kDefaultUniformsMaxSize * 2);

        pool.reset(new mtl::BufferPool(false));

        // Allow unbounded growth of the buffer count. Doing a full CPU/GPU sync waiting for new
        // uniform uploads has catastrophic performance cost.
        pool->initialize(context, bufferSize, mtl::kUniformBufferSettingOffsetMinAlignment, 0);
    }
    return pool.get();
}

angle::Result ProgramExecutableMtl::setupDraw(const gl::Context *glContext,
                                              mtl::RenderCommandEncoder *cmdEncoder,
                                              const mtl::RenderPipelineDesc &pipelineDesc,
                                              bool pipelineDescChanged,
                                              bool forceTexturesSetting,
                                              bool uniformBuffersDirty)
{
    ContextMtl *context = mtl::GetImpl(glContext);

    if (pipelineDescChanged)
    {
        id<MTLFunction> vertexShader = nil;
        ANGLE_TRY(
            getSpecializedShader(context, gl::ShaderType::Vertex, pipelineDesc, &vertexShader));

        id<MTLFunction> fragmentShader = nil;
        ANGLE_TRY(
            getSpecializedShader(context, gl::ShaderType::Fragment, pipelineDesc, &fragmentShader));

        mtl::AutoObjCPtr<id<MTLRenderPipelineState>> pipelineState;
        ANGLE_TRY(context->getPipelineCache().getRenderPipeline(
            context, vertexShader, fragmentShader, pipelineDesc, &pipelineState));

        cmdEncoder->setRenderPipelineState(pipelineState);

        // We need to rebind uniform buffers & textures also
        mDefaultUniformBlocksDirty.set();
        mSamplerBindingsDirty.set();

        // Cache current shader variant references for easier querying.
        mCurrentShaderVariants[gl::ShaderType::Vertex] =
            &mVertexShaderVariants[pipelineDesc.rasterizationType];

        const bool multisampledRendering = pipelineDesc.outputDescriptor.rasterSampleCount > 1;
        const bool allowFragDepthWrite =
            pipelineDesc.outputDescriptor.depthAttachmentPixelFormat != 0;
        mCurrentShaderVariants[gl::ShaderType::Fragment] =
            pipelineDesc.rasterizationEnabled()
                ? &mFragmentShaderVariants[PipelineParametersToFragmentShaderVariantIndex(
                      multisampledRendering, allowFragDepthWrite)]
                : nullptr;
    }

    ANGLE_TRY(commitUniforms(context, cmdEncoder));
    ANGLE_TRY(updateTextures(glContext, cmdEncoder, forceTexturesSetting));

    if (uniformBuffersDirty || pipelineDescChanged)
    {
        ANGLE_TRY(updateUniformBuffers(context, cmdEncoder, pipelineDesc));
    }

    if (pipelineDescChanged)
    {
        ANGLE_TRY(updateXfbBuffers(context, cmdEncoder, pipelineDesc));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::getSpecializedShader(
    ContextMtl *context,
    gl::ShaderType shaderType,
    const mtl::RenderPipelineDesc &renderPipelineDesc,
    id<MTLFunction> *shaderOut)
{
    static_assert(YES == 1, "YES should have value of 1");

    mtl::TranslatedShaderInfo *translatedMslInfo = &mMslShaderTranslateInfo[shaderType];
    ProgramShaderObjVariantMtl *shaderVariant;
    mtl::AutoObjCPtr<MTLFunctionConstantValues *> funcConstants;

    if (shaderType == gl::ShaderType::Vertex)
    {
        // For vertex shader, we need to create 3 variants, one with emulated rasterization
        // discard, one with true rasterization discard and one without.
        shaderVariant = &mVertexShaderVariants[renderPipelineDesc.rasterizationType];
        if (shaderVariant->metalShader)
        {
            // Already created.
            *shaderOut = shaderVariant->metalShader;
            return angle::Result::Continue;
        }

        if (renderPipelineDesc.rasterizationType == mtl::RenderPipelineRasterization::Disabled)
        {
            // Special case: XFB output only vertex shader.
            ASSERT(!mExecutable->getLinkedTransformFeedbackVaryings().empty());
            translatedMslInfo = &mMslXfbOnlyVertexShaderInfo;
            if (!translatedMslInfo->metalLibrary)
            {
                // Lazily compile XFB only shader
                gl::InfoLog infoLog;
                ANGLE_TRY(CreateMslShaderLib(context, infoLog, &mMslXfbOnlyVertexShaderInfo,
                                             {{"TRANSFORM_FEEDBACK_ENABLED", "1"}}));
                translatedMslInfo->metalLibrary.get().label = @"TransformFeedback";
            }
        }

        ANGLE_MTL_OBJC_SCOPE
        {
            BOOL emulateDiscard = renderPipelineDesc.rasterizationType ==
                                  mtl::RenderPipelineRasterization::EmulatedDiscard;

            NSString *discardEnabledStr =
                [NSString stringWithUTF8String:sh::mtl::kRasterizerDiscardEnabledConstName];

            funcConstants = mtl::adoptObjCPtr([[MTLFunctionConstantValues alloc] init]);
            [funcConstants setConstantValue:&emulateDiscard
                                       type:MTLDataTypeBool
                                   withName:discardEnabledStr];
        }
    }  // if (shaderType == gl::ShaderType::Vertex)
    else if (shaderType == gl::ShaderType::Fragment)
    {
        // For fragment shader, we need to create 4 variants,
        // combining multisampled rendering and depth write enabled states.
        const bool multisampledRendering =
            renderPipelineDesc.outputDescriptor.rasterSampleCount > 1;
        const bool allowFragDepthWrite =
            renderPipelineDesc.outputDescriptor.depthAttachmentPixelFormat != 0;
        shaderVariant = &mFragmentShaderVariants[PipelineParametersToFragmentShaderVariantIndex(
            multisampledRendering, allowFragDepthWrite)];
        if (shaderVariant->metalShader)
        {
            // Already created.
            *shaderOut = shaderVariant->metalShader;
            return angle::Result::Continue;
        }

        ANGLE_MTL_OBJC_SCOPE
        {
            NSString *multisampledRenderingStr =
                [NSString stringWithUTF8String:sh::mtl::kMultisampledRenderingConstName];

            NSString *depthWriteEnabledStr =
                [NSString stringWithUTF8String:sh::mtl::kDepthWriteEnabledConstName];

            funcConstants = mtl::adoptObjCPtr([[MTLFunctionConstantValues alloc] init]);
            [funcConstants setConstantValue:&multisampledRendering
                                       type:MTLDataTypeBool
                                   withName:multisampledRenderingStr];
            [funcConstants setConstantValue:&allowFragDepthWrite
                                       type:MTLDataTypeBool
                                   withName:depthWriteEnabledStr];
        }

    }  // gl::ShaderType::Fragment
    else
    {
        UNREACHABLE();
        return angle::Result::Stop;
    }
    [funcConstants
        setConstantValue:&(context->getDisplay()->getFeatures().allowSamplerCompareGradient.enabled)
                    type:MTLDataTypeBool
                withName:@"ANGLEUseSampleCompareGradient"];
    [funcConstants
        setConstantValue:&(context->getDisplay()->getFeatures().emulateAlphaToCoverage.enabled)
                    type:MTLDataTypeBool
                withName:@"ANGLEEmulateAlphaToCoverage"];
    [funcConstants
        setConstantValue:&(context->getDisplay()->getFeatures().writeHelperSampleMask.enabled)
                    type:MTLDataTypeBool
                withName:@"ANGLEWriteHelperSampleMask"];
    ANGLE_TRY(CreateMslShader(context, translatedMslInfo->metalLibrary, SHADER_ENTRY_NAME,
                              funcConstants.get(), &shaderVariant->metalShader));

    // Store reference to the translated source for easily querying mapped bindings later.
    shaderVariant->translatedSrcInfo = translatedMslInfo;

    // Initialize argument buffer encoder if required
    if (translatedMslInfo->hasUBOArgumentBuffer)
    {
        InitArgumentBufferEncoder(context, shaderVariant->metalShader,
                                  mtl::kUBOArgumentBufferBindingIndex,
                                  &shaderVariant->uboArgBufferEncoder);
    }

    *shaderOut = shaderVariant->metalShader;

    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::commitUniforms(ContextMtl *context,
                                                   mtl::RenderCommandEncoder *cmdEncoder)
{
    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        if (!mDefaultUniformBlocksDirty[shaderType] || !mCurrentShaderVariants[shaderType])
        {
            continue;
        }
        DefaultUniformBlockMtl &uniformBlock = mDefaultUniformBlocks[shaderType];

        if (!uniformBlock.uniformData.size())
        {
            continue;
        }

        // If we exceed the default inline max size, try to allocate a buffer
        bool needsCommitUniform = true;
        if (needsCommitUniform && uniformBlock.uniformData.size() <= mtl::kInlineConstDataMaxSize)
        {
            ASSERT(uniformBlock.uniformData.size() <= mtl::kInlineConstDataMaxSize);
            cmdEncoder->setBytes(shaderType, uniformBlock.uniformData.data(),
                                 uniformBlock.uniformData.size(),
                                 mtl::kDefaultUniformsBindingIndex);
        }
        else if (needsCommitUniform)
        {
            mtl::BufferPool *bufferPool = getBufferPool(context, shaderType);
            bufferPool->releaseInFlightBuffers(context);

            ASSERT(uniformBlock.uniformData.size() <= mtl::kDefaultUniformsMaxSize);
            mtl::BufferRef mtlBufferOut;
            size_t offsetOut;
            uint8_t *ptrOut;
            // Allocate a new Uniform buffer
            ANGLE_TRY(bufferPool->allocate(context, uniformBlock.uniformData.size(), &ptrOut,
                                           &mtlBufferOut, &offsetOut));
            // Copy the uniform result
            memcpy(ptrOut, uniformBlock.uniformData.data(), uniformBlock.uniformData.size());
            // Commit
            ANGLE_TRY(bufferPool->commit(context));
            // Set buffer
            cmdEncoder->setBuffer(shaderType, mtlBufferOut, (uint32_t)offsetOut,
                                  mtl::kDefaultUniformsBindingIndex);
        }

        mDefaultUniformBlocksDirty.reset(shaderType);
    }
    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::updateTextures(const gl::Context *glContext,
                                                   mtl::RenderCommandEncoder *cmdEncoder,
                                                   bool forceUpdate)
{
    ContextMtl *contextMtl                          = mtl::GetImpl(glContext);
    const auto &glState                             = glContext->getState();
    const gl::ActiveTexturesCache &completeTextures = glState.getActiveTexturesCache();

    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        if ((!mSamplerBindingsDirty[shaderType] && !forceUpdate) ||
            !mCurrentShaderVariants[shaderType])
        {
            continue;
        }

        const mtl::TranslatedShaderInfo &shaderInfo =
            mCurrentShaderVariants[shaderType]->translatedSrcInfo
                ? *mCurrentShaderVariants[shaderType]->translatedSrcInfo
                : mMslShaderTranslateInfo[shaderType];
        bool hasDepthSampler = false;

        for (uint32_t textureIndex = 0; textureIndex < mExecutable->getSamplerBindings().size();
             ++textureIndex)
        {
            const gl::SamplerBinding &samplerBinding =
                mExecutable->getSamplerBindings()[textureIndex];
            const mtl::SamplerBinding &mslBinding = shaderInfo.actualSamplerBindings[textureIndex];
            if (mslBinding.textureBinding >= mtl::kMaxShaderSamplers)
            {
                // No binding assigned
                continue;
            }

            gl::TextureType textureType = samplerBinding.textureType;

            for (uint32_t arrayElement = 0; arrayElement < samplerBinding.textureUnitsCount;
                 ++arrayElement)
            {
                GLuint textureUnit = samplerBinding.getTextureUnit(
                    mExecutable->getSamplerBoundTextureUnits(), arrayElement);
                gl::Texture *texture = completeTextures[textureUnit];
                gl::Sampler *sampler = contextMtl->getState().getSampler(textureUnit);
                uint32_t textureSlot = mslBinding.textureBinding + arrayElement;
                uint32_t samplerSlot = mslBinding.samplerBinding + arrayElement;
                if (!texture)
                {
                    ANGLE_TRY(contextMtl->getIncompleteTexture(glContext, textureType,
                                                               samplerBinding.format, &texture));
                }
                const gl::SamplerState *samplerState =
                    sampler ? &sampler->getSamplerState() : &texture->getSamplerState();
                TextureMtl *textureMtl = mtl::GetImpl(texture);
                if (samplerBinding.format == gl::SamplerFormat::Shadow)
                {
                    hasDepthSampler                  = true;
                    mShadowCompareModes[textureSlot] = mtl::MslGetShaderShadowCompareMode(
                        samplerState->getCompareMode(), samplerState->getCompareFunc());
                }
                ANGLE_TRY(textureMtl->bindToShader(glContext, cmdEncoder, shaderType, sampler,
                                                   textureSlot, samplerSlot));
            }  // for array elements
        }      // for sampler bindings

        if (hasDepthSampler)
        {
            cmdEncoder->setData(shaderType, mShadowCompareModes,
                                mtl::kShadowSamplerCompareModesBindingIndex);
        }

        for (const gl::ImageBinding &imageBinding : mExecutable->getImageBindings())
        {
            if (imageBinding.boundImageUnits.size() != 1)
            {
                UNIMPLEMENTED();
                continue;
            }

            int glslImageBinding    = imageBinding.boundImageUnits[0];
            int mtlRWTextureBinding = shaderInfo.actualImageBindings[glslImageBinding];
            ASSERT(mtlRWTextureBinding < static_cast<int>(mtl::kMaxShaderImages));
            if (mtlRWTextureBinding < 0)
            {
                continue;  // The program does not have an image bound at this unit.
            }

            const gl::ImageUnit &imageUnit = glState.getImageUnit(glslImageBinding);
            TextureMtl *textureMtl         = mtl::GetImpl(imageUnit.texture.get());
            if (imageUnit.layered)
            {
                UNIMPLEMENTED();
                continue;
            }
            ANGLE_TRY(textureMtl->bindToShaderImage(
                glContext, cmdEncoder, shaderType, static_cast<uint32_t>(mtlRWTextureBinding),
                imageUnit.level, imageUnit.layer, imageUnit.format));
        }
    }  // for shader types

    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::updateUniformBuffers(
    ContextMtl *context,
    mtl::RenderCommandEncoder *cmdEncoder,
    const mtl::RenderPipelineDesc &pipelineDesc)
{
    const std::vector<gl::InterfaceBlock> &blocks = mExecutable->getUniformBlocks();
    if (blocks.empty())
    {
        return angle::Result::Continue;
    }

    // This array is only used inside this function and its callees.
    ScopedAutoClearVector<uint32_t> scopeArrayClear(&mArgumentBufferRenderStageUsages);
    ScopedAutoClearVector<std::pair<mtl::BufferRef, uint32_t>> scopeArrayClear2(
        &mLegalizedOffsetedUniformBuffers);
    mArgumentBufferRenderStageUsages.resize(blocks.size());
    mLegalizedOffsetedUniformBuffers.resize(blocks.size());

    ANGLE_TRY(legalizeUniformBufferOffsets(context));

    const gl::State &glState = context->getState();

    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        if (!mCurrentShaderVariants[shaderType])
        {
            continue;
        }

        if (mCurrentShaderVariants[shaderType]->translatedSrcInfo->hasUBOArgumentBuffer)
        {
            ANGLE_TRY(encodeUniformBuffersInfoArgumentBuffer(context, cmdEncoder, shaderType));
        }
        else
        {
            ANGLE_TRY(bindUniformBuffersToDiscreteSlots(context, cmdEncoder, shaderType));
        }
    }  // for shader types

    // After encode the uniform buffers into an argument buffer, we need to tell Metal that
    // the buffers are being used by what shader stages.
    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
    {
        const GLuint binding = mExecutable->getUniformBlockBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            glState.getIndexedUniformBuffer(binding);
        if (bufferBinding.get() == nullptr)
        {
            continue;
        }

        // Remove any other stages other than vertex and fragment.
        uint32_t stages = mArgumentBufferRenderStageUsages[bufferIndex] &
                          (MTLRenderStageVertex | MTLRenderStageFragment);

        if (stages == 0)
        {
            continue;
        }

        cmdEncoder->useResource(mLegalizedOffsetedUniformBuffers[bufferIndex].first,
                                MTLResourceUsageRead, static_cast<MTLRenderStages>(stages));
    }

    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::legalizeUniformBufferOffsets(ContextMtl *context)
{
    const gl::State &glState                      = context->getState();
    const std::vector<gl::InterfaceBlock> &blocks = mExecutable->getUniformBlocks();

    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
    {
        const gl::InterfaceBlock &block = blocks[bufferIndex];
        const GLuint binding            = mExecutable->getUniformBlockBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            glState.getIndexedUniformBuffer(binding);

        if (bufferBinding.get() == nullptr)
        {
            continue;
        }

        BufferMtl *bufferMtl = mtl::GetImpl(bufferBinding.get());
        size_t srcOffset     = std::min<size_t>(bufferBinding.getOffset(), bufferMtl->size());
        ASSERT(mUniformBlockConversions.find(block.name) != mUniformBlockConversions.end());
        const UBOConversionInfo &conversionInfo = mUniformBlockConversions.at(block.name);

        size_t spaceAvailable  = bufferMtl->size() - srcOffset;
        bool haveSpaceInBuffer = conversionInfo.metalSize() <= spaceAvailable;
        if (conversionInfo.needsConversion() || !haveSpaceInBuffer)
        {

            UniformConversionBufferMtl *conversion =
                (UniformConversionBufferMtl *)bufferMtl->getUniformConversionBuffer(
                    context, std::pair<size_t, size_t>(bufferIndex, srcOffset),
                    conversionInfo.stdSize());
            // Has the content of the buffer has changed since last conversion?
            if (conversion->dirty)
            {
                const uint8_t *srcBytes = bufferMtl->getBufferDataReadOnly(context);
                srcBytes += conversion->initialSrcOffset();
                size_t sizeToCopy = bufferMtl->size() - conversion->initialSrcOffset();

                ANGLE_TRY(ConvertUniformBufferData(
                    context, conversionInfo, &conversion->data, srcBytes, sizeToCopy,
                    &conversion->convertedBuffer, &conversion->convertedOffset));

                conversion->dirty = false;
            }
            // Calculate offset in new block.
            size_t dstOffsetSource = srcOffset - conversion->initialSrcOffset();
            ASSERT(dstOffsetSource % conversionInfo.stdSize() == 0);
            unsigned int numBlocksToOffset =
                (unsigned int)(dstOffsetSource / conversionInfo.stdSize());
            size_t bytesToOffset = numBlocksToOffset * conversionInfo.metalSize();

            mLegalizedOffsetedUniformBuffers[bufferIndex].first = conversion->convertedBuffer;
            mLegalizedOffsetedUniformBuffers[bufferIndex].second =
                static_cast<uint32_t>(conversion->convertedOffset + bytesToOffset);
            // Ensure that the converted info can fit in the buffer.
            ASSERT(conversion->convertedOffset + bytesToOffset + conversionInfo.metalSize() <=
                   conversion->convertedBuffer->size());
        }
        else
        {
            mLegalizedOffsetedUniformBuffers[bufferIndex].first = bufferMtl->getCurrentBuffer();
            mLegalizedOffsetedUniformBuffers[bufferIndex].second =
                static_cast<uint32_t>(bufferBinding.getOffset());
        }
    }
    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::bindUniformBuffersToDiscreteSlots(
    ContextMtl *context,
    mtl::RenderCommandEncoder *cmdEncoder,
    gl::ShaderType shaderType)
{
    const gl::State &glState                      = context->getState();
    const std::vector<gl::InterfaceBlock> &blocks = mExecutable->getUniformBlocks();
    const mtl::TranslatedShaderInfo &shaderInfo =
        *mCurrentShaderVariants[shaderType]->translatedSrcInfo;

    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
    {
        const gl::InterfaceBlock &block = blocks[bufferIndex];
        const GLuint binding            = mExecutable->getUniformBlockBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            glState.getIndexedUniformBuffer(binding);

        if (bufferBinding.get() == nullptr || !block.activeShaders().test(shaderType))
        {
            continue;
        }

        uint32_t actualBufferIdx = shaderInfo.actualUBOBindings[bufferIndex];

        if (actualBufferIdx >= mtl::kMaxShaderBuffers)
        {
            continue;
        }

        mtl::BufferRef mtlBuffer = mLegalizedOffsetedUniformBuffers[bufferIndex].first;
        uint32_t offset          = mLegalizedOffsetedUniformBuffers[bufferIndex].second;
        cmdEncoder->setBuffer(shaderType, mtlBuffer, offset, actualBufferIdx);
    }
    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::encodeUniformBuffersInfoArgumentBuffer(
    ContextMtl *context,
    mtl::RenderCommandEncoder *cmdEncoder,
    gl::ShaderType shaderType)
{
    const gl::State &glState                      = context->getState();
    const std::vector<gl::InterfaceBlock> &blocks = mExecutable->getUniformBlocks();

    ASSERT(mCurrentShaderVariants[shaderType]->translatedSrcInfo);
    const mtl::TranslatedShaderInfo &shaderInfo =
        *mCurrentShaderVariants[shaderType]->translatedSrcInfo;

    // Encode all uniform buffers into an argument buffer.
    ProgramArgumentBufferEncoderMtl &bufferEncoder =
        mCurrentShaderVariants[shaderType]->uboArgBufferEncoder;

    mtl::BufferRef argumentBuffer;
    size_t argumentBufferOffset;
    bufferEncoder.bufferPool.releaseInFlightBuffers(context);
    ANGLE_TRY(bufferEncoder.bufferPool.allocate(
        context, bufferEncoder.metalArgBufferEncoder.get().encodedLength, nullptr, &argumentBuffer,
        &argumentBufferOffset));

    // MTLArgumentEncoder is modifying the buffer indirectly on CPU. We need to call map()
    // so that the buffer's data changes could be flushed to the GPU side later.
    ANGLE_UNUSED_VARIABLE(argumentBuffer->mapWithOpt(context, /*readonly=*/false, /*noSync=*/true));

    [bufferEncoder.metalArgBufferEncoder setArgumentBuffer:argumentBuffer->get()
                                                    offset:argumentBufferOffset];

    constexpr gl::ShaderMap<MTLRenderStages> kShaderStageMap = {
        {gl::ShaderType::Vertex, MTLRenderStageVertex},
        {gl::ShaderType::Fragment, MTLRenderStageFragment},
    };

    auto mtlRenderStage = kShaderStageMap[shaderType];

    for (uint32_t bufferIndex = 0; bufferIndex < blocks.size(); ++bufferIndex)
    {
        const gl::InterfaceBlock &block = blocks[bufferIndex];
        const GLuint binding            = mExecutable->getUniformBlockBinding(bufferIndex);
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            glState.getIndexedUniformBuffer(binding);

        if (bufferBinding.get() == nullptr || !block.activeShaders().test(shaderType))
        {
            continue;
        }

        mArgumentBufferRenderStageUsages[bufferIndex] |= mtlRenderStage;

        uint32_t actualBufferIdx = shaderInfo.actualUBOBindings[bufferIndex];
        if (actualBufferIdx >= mtl::kMaxShaderBuffers)
        {
            continue;
        }

        mtl::BufferRef mtlBuffer = mLegalizedOffsetedUniformBuffers[bufferIndex].first;
        uint32_t offset          = mLegalizedOffsetedUniformBuffers[bufferIndex].second;
        [bufferEncoder.metalArgBufferEncoder setBuffer:mtlBuffer->get()
                                                offset:offset
                                               atIndex:actualBufferIdx];
    }

    // Flush changes made by MTLArgumentEncoder to GPU.
    argumentBuffer->unmapAndFlushSubset(context, argumentBufferOffset,
                                        bufferEncoder.metalArgBufferEncoder.get().encodedLength);

    cmdEncoder->setBuffer(shaderType, argumentBuffer, static_cast<uint32_t>(argumentBufferOffset),
                          mtl::kUBOArgumentBufferBindingIndex);
    return angle::Result::Continue;
}

angle::Result ProgramExecutableMtl::updateXfbBuffers(ContextMtl *context,
                                                     mtl::RenderCommandEncoder *cmdEncoder,
                                                     const mtl::RenderPipelineDesc &pipelineDesc)
{
    const gl::State &glState                 = context->getState();
    gl::TransformFeedback *transformFeedback = glState.getCurrentTransformFeedback();

    if (pipelineDesc.rasterizationEnabled() || !glState.isTransformFeedbackActiveUnpaused() ||
        ANGLE_UNLIKELY(!transformFeedback))
    {
        // XFB output can only be used with rasterization disabled.
        return angle::Result::Continue;
    }

    size_t xfbBufferCount = glState.getProgramExecutable()->getTransformFeedbackBufferCount();

    ASSERT(xfbBufferCount > 0);
    ASSERT(mExecutable->getTransformFeedbackBufferMode() != GL_INTERLEAVED_ATTRIBS ||
           xfbBufferCount == 1);

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        uint32_t actualBufferIdx = mMslXfbOnlyVertexShaderInfo.actualXFBBindings[bufferIndex];

        if (actualBufferIdx >= mtl::kMaxShaderBuffers)
        {
            continue;
        }

        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding =
            transformFeedback->getIndexedBuffer(bufferIndex);
        gl::Buffer *buffer = bufferBinding.get();
        ASSERT((bufferBinding.getOffset() % 4) == 0);
        ASSERT(buffer != nullptr);

        BufferMtl *bufferMtl = mtl::GetImpl(buffer);

        // Use offset=0, actual offset will be set in Driver Uniform inside ContextMtl.
        cmdEncoder->setBufferForWrite(gl::ShaderType::Vertex, bufferMtl->getCurrentBuffer(), 0,
                                      actualBufferIdx);
    }

    return angle::Result::Continue;
}

template <typename T>
void ProgramExecutableMtl::setUniformImpl(GLint location,
                                          GLsizei count,
                                          const T *v,
                                          GLenum entryPointType)
{
    const std::vector<gl::VariableLocation> &uniformLocations = mExecutable->getUniformLocations();
    const gl::VariableLocation &locationInfo                  = uniformLocations[location];

    const std::vector<gl::LinkedUniform> &linkedUniforms = mExecutable->getUniforms();
    const gl::LinkedUniform &linkedUniform               = linkedUniforms[locationInfo.index];

    if (linkedUniform.isSampler())
    {
        // Sampler binding has changed.
        mSamplerBindingsDirty.set();
        return;
    }

    if (linkedUniform.getType() == entryPointType)
    {
        for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
        {
            DefaultUniformBlockMtl &uniformBlock  = mDefaultUniformBlocks[shaderType];
            const sh::BlockMemberInfo &layoutInfo = uniformBlock.uniformLayout[location];

            // Assume an offset of -1 means the block is unused.
            if (layoutInfo.offset == -1)
            {
                continue;
            }

            const GLint componentCount    = (GLint)linkedUniform.getElementComponents();
            const GLint baseComponentSize = (GLint)mtl::GetMetalSizeForGLType(
                gl::VariableComponentType(linkedUniform.getType()));
            UpdateDefaultUniformBlockWithElementSize(count, locationInfo.arrayIndex, componentCount,
                                                     v, baseComponentSize, layoutInfo,
                                                     &uniformBlock.uniformData);
            mDefaultUniformBlocksDirty.set(shaderType);
        }
    }
    else
    {
        for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
        {
            DefaultUniformBlockMtl &uniformBlock  = mDefaultUniformBlocks[shaderType];
            const sh::BlockMemberInfo &layoutInfo = uniformBlock.uniformLayout[location];

            // Assume an offset of -1 means the block is unused.
            if (layoutInfo.offset == -1)
            {
                continue;
            }

            const GLint componentCount = linkedUniform.getElementComponents();

            ASSERT(linkedUniform.getType() == gl::VariableBoolVectorType(entryPointType));

            GLint initialArrayOffset =
                locationInfo.arrayIndex * layoutInfo.arrayStride + layoutInfo.offset;
            for (GLint i = 0; i < count; i++)
            {
                GLint elementOffset = i * layoutInfo.arrayStride + initialArrayOffset;
                bool *dest =
                    reinterpret_cast<bool *>(uniformBlock.uniformData.data() + elementOffset);
                const T *source = v + i * componentCount;

                for (int c = 0; c < componentCount; c++)
                {
                    dest[c] = (source[c] == static_cast<T>(0)) ? GL_FALSE : GL_TRUE;
                }
            }

            mDefaultUniformBlocksDirty.set(shaderType);
        }
    }
}

template <typename T>
void ProgramExecutableMtl::getUniformImpl(GLint location, T *v, GLenum entryPointType) const
{
    const gl::VariableLocation &locationInfo = mExecutable->getUniformLocations()[location];
    const gl::LinkedUniform &linkedUniform   = mExecutable->getUniforms()[locationInfo.index];

    ASSERT(!linkedUniform.isSampler());

    const gl::ShaderType shaderType = linkedUniform.getFirstActiveShaderType();
    ASSERT(shaderType != gl::ShaderType::InvalidEnum);

    const DefaultUniformBlockMtl &uniformBlock = mDefaultUniformBlocks[shaderType];
    const sh::BlockMemberInfo &layoutInfo      = uniformBlock.uniformLayout[location];

    ASSERT(linkedUniform.getUniformTypeInfo().componentType == entryPointType ||
           linkedUniform.getUniformTypeInfo().componentType ==
               gl::VariableBoolVectorType(entryPointType));
    const GLint baseComponentSize =
        (GLint)mtl::GetMetalSizeForGLType(gl::VariableComponentType(linkedUniform.getType()));

    if (gl::IsMatrixType(linkedUniform.getType()))
    {
        const uint8_t *ptrToElement = uniformBlock.uniformData.data() + layoutInfo.offset +
                                      (locationInfo.arrayIndex * layoutInfo.arrayStride);
        mtl::GetMatrixUniformMetal(linkedUniform.getType(), v,
                                   reinterpret_cast<const T *>(ptrToElement), false);
    }
    // Decompress bool from one byte to four bytes because bool values in GLSL
    // are uint-sized: ES 3.0 Section 2.12.6.3 "Uniform Buffer Object Storage".
    else if (gl::VariableComponentType(linkedUniform.getType()) == GL_BOOL)
    {
        bool bVals[4] = {0};
        ReadFromDefaultUniformBlockWithElementSize(
            linkedUniform.getElementComponents(), locationInfo.arrayIndex, bVals, baseComponentSize,
            layoutInfo, &uniformBlock.uniformData);
        for (int bCol = 0; bCol < linkedUniform.getElementComponents(); ++bCol)
        {
            unsigned int data = bVals[bCol];
            *(v + bCol)       = static_cast<T>(data);
        }
    }
    else
    {

        assert(baseComponentSize == sizeof(T));
        ReadFromDefaultUniformBlockWithElementSize(linkedUniform.getElementComponents(),
                                                   locationInfo.arrayIndex, v, baseComponentSize,
                                                   layoutInfo, &uniformBlock.uniformData);
    }
}

void ProgramExecutableMtl::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT);
}

void ProgramExecutableMtl::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT_VEC2);
}

void ProgramExecutableMtl::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT_VEC3);
}

void ProgramExecutableMtl::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    setUniformImpl(location, count, v, GL_FLOAT_VEC4);
}

void ProgramExecutableMtl::setUniform1iv(GLint startLocation, GLsizei count, const GLint *v)
{
    setUniformImpl(startLocation, count, v, GL_INT);
}

void ProgramExecutableMtl::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    setUniformImpl(location, count, v, GL_INT_VEC2);
}

void ProgramExecutableMtl::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    setUniformImpl(location, count, v, GL_INT_VEC3);
}

void ProgramExecutableMtl::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    setUniformImpl(location, count, v, GL_INT_VEC4);
}

void ProgramExecutableMtl::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformImpl(location, count, v, GL_UNSIGNED_INT);
}

void ProgramExecutableMtl::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformImpl(location, count, v, GL_UNSIGNED_INT_VEC2);
}

void ProgramExecutableMtl::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformImpl(location, count, v, GL_UNSIGNED_INT_VEC3);
}

void ProgramExecutableMtl::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    setUniformImpl(location, count, v, GL_UNSIGNED_INT_VEC4);
}

template <int cols, int rows>
void ProgramExecutableMtl::setUniformMatrixfv(GLint location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *value)
{
    const gl::VariableLocation &locationInfo = mExecutable->getUniformLocations()[location];
    const gl::LinkedUniform &linkedUniform   = mExecutable->getUniforms()[locationInfo.index];

    for (gl::ShaderType shaderType : gl::kAllGLES2ShaderTypes)
    {
        DefaultUniformBlockMtl &uniformBlock  = mDefaultUniformBlocks[shaderType];
        const sh::BlockMemberInfo &layoutInfo = uniformBlock.uniformLayout[location];

        // Assume an offset of -1 means the block is unused.
        if (layoutInfo.offset == -1)
        {
            continue;
        }

        mtl::SetFloatUniformMatrixMetal<cols, rows>::Run(
            locationInfo.arrayIndex, linkedUniform.getBasicTypeElementCount(), count, transpose,
            value, uniformBlock.uniformData.data() + layoutInfo.offset);

        mDefaultUniformBlocksDirty.set(shaderType);
    }
}

void ProgramExecutableMtl::setUniformMatrix2fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    setUniformMatrixfv<2, 2>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix3fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    setUniformMatrixfv<3, 3>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix4fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    setUniformMatrixfv<4, 4>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix2x3fv(GLint location,
                                                 GLsizei count,
                                                 GLboolean transpose,
                                                 const GLfloat *value)
{
    setUniformMatrixfv<2, 3>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix3x2fv(GLint location,
                                                 GLsizei count,
                                                 GLboolean transpose,
                                                 const GLfloat *value)
{
    setUniformMatrixfv<3, 2>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix2x4fv(GLint location,
                                                 GLsizei count,
                                                 GLboolean transpose,
                                                 const GLfloat *value)
{
    setUniformMatrixfv<2, 4>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix4x2fv(GLint location,
                                                 GLsizei count,
                                                 GLboolean transpose,
                                                 const GLfloat *value)
{
    setUniformMatrixfv<4, 2>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix3x4fv(GLint location,
                                                 GLsizei count,
                                                 GLboolean transpose,
                                                 const GLfloat *value)
{
    setUniformMatrixfv<3, 4>(location, count, transpose, value);
}

void ProgramExecutableMtl::setUniformMatrix4x3fv(GLint location,
                                                 GLsizei count,
                                                 GLboolean transpose,
                                                 const GLfloat *value)
{
    setUniformMatrixfv<4, 3>(location, count, transpose, value);
}

void ProgramExecutableMtl::getUniformfv(const gl::Context *context,
                                        GLint location,
                                        GLfloat *params) const
{
    getUniformImpl(location, params, GL_FLOAT);
}

void ProgramExecutableMtl::getUniformiv(const gl::Context *context,
                                        GLint location,
                                        GLint *params) const
{
    getUniformImpl(location, params, GL_INT);
}

void ProgramExecutableMtl::getUniformuiv(const gl::Context *context,
                                         GLint location,
                                         GLuint *params) const
{
    getUniformImpl(location, params, GL_UNSIGNED_INT);
}
}  // namespace rx
