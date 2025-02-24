//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexDataManager.h: Defines the VertexDataManager, a class that
// runs the Buffer translation process.

#include "libANGLE/renderer/d3d/VertexDataManager.h"

#include "common/bitset_utils.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Program.h"
#include "libANGLE/State.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"
#include "libANGLE/renderer/d3d/VertexBuffer.h"

using namespace angle;

namespace rx
{
namespace
{
enum
{
    INITIAL_STREAM_BUFFER_SIZE = 1024 * 1024
};
// This has to be at least 4k or else it fails on ATI cards.
enum
{
    CONSTANT_VERTEX_BUFFER_SIZE = 4096
};

// Warning: ensure the binding matches attrib.bindingIndex before using these functions.
int64_t GetMaxAttributeByteOffsetForDraw(const gl::VertexAttribute &attrib,
                                         const gl::VertexBinding &binding,
                                         int64_t elementCount)
{
    CheckedNumeric<int64_t> stride = ComputeVertexAttributeStride(attrib, binding);
    CheckedNumeric<int64_t> offset = ComputeVertexAttributeOffset(attrib, binding);
    CheckedNumeric<int64_t> size   = ComputeVertexAttributeTypeSize(attrib);

    ASSERT(elementCount > 0);

    CheckedNumeric<int64_t> result =
        stride * (CheckedNumeric<int64_t>(elementCount) - 1) + size + offset;
    return result.ValueOrDefault(std::numeric_limits<int64_t>::max());
}

// Warning: ensure the binding matches attrib.bindingIndex before using these functions.
int ElementsInBuffer(const gl::VertexAttribute &attrib,
                     const gl::VertexBinding &binding,
                     unsigned int size)
{
    angle::CheckedNumeric<int64_t> bufferSize(size);
    angle::CheckedNumeric<int64_t> stride      = ComputeVertexAttributeStride(attrib, binding);
    angle::CheckedNumeric<int64_t> offset      = ComputeVertexAttributeOffset(attrib, binding);
    angle::CheckedNumeric<int64_t> elementSize = ComputeVertexAttributeTypeSize(attrib);

    auto elementsInBuffer    = (bufferSize - (offset % stride) + (stride - elementSize)) / stride;
    auto elementsInBufferInt = elementsInBuffer.Cast<int>();

    return elementsInBufferInt.ValueOrDefault(0);
}

// Warning: you should ensure binding really matches attrib.bindingIndex before using this function.
bool DirectStoragePossible(const gl::Context *context,
                           const gl::VertexAttribute &attrib,
                           const gl::VertexBinding &binding)
{
    // Current value attribs may not use direct storage.
    if (!attrib.enabled)
    {
        return false;
    }

    gl::Buffer *buffer = binding.getBuffer().get();
    if (!buffer)
    {
        return false;
    }

    BufferD3D *bufferD3D = GetImplAs<BufferD3D>(buffer);
    ASSERT(bufferD3D);
    if (!bufferD3D->supportsDirectBinding())
    {
        return false;
    }

    // Alignment restrictions: In D3D, vertex data must be aligned to the format stride, or to a
    // 4-byte boundary, whichever is smaller. (Undocumented, and experimentally confirmed)
    size_t alignment = 4;

    // TODO(jmadill): add VertexFormatCaps
    BufferFactoryD3D *factory = bufferD3D->getFactory();

    angle::FormatID vertexFormatID = attrib.format->id;

    // CPU-converted vertex data must be converted (naturally).
    if ((factory->getVertexConversionType(vertexFormatID) & VERTEX_CONVERT_CPU) != 0)
    {
        return false;
    }

    if (attrib.format->vertexAttribType != gl::VertexAttribType::Float)
    {
        unsigned int elementSize = 0;
        angle::Result error =
            factory->getVertexSpaceRequired(context, attrib, binding, 1, 0, 0, &elementSize);
        ASSERT(error == angle::Result::Continue);
        alignment = std::min<size_t>(elementSize, 4);
    }

    GLintptr offset = ComputeVertexAttributeOffset(attrib, binding);
    // Final alignment check - unaligned data must be converted.
    return (static_cast<size_t>(ComputeVertexAttributeStride(attrib, binding)) % alignment == 0) &&
           (static_cast<size_t>(offset) % alignment == 0);
}
}  // anonymous namespace

TranslatedAttribute::TranslatedAttribute()
    : active(false),
      attribute(nullptr),
      binding(nullptr),
      currentValueType(gl::VertexAttribType::InvalidEnum),
      baseOffset(0),
      usesFirstVertexOffset(false),
      stride(0),
      vertexBuffer(),
      storage(nullptr),
      serial(0),
      divisor(0)
{}

TranslatedAttribute::TranslatedAttribute(const TranslatedAttribute &other) = default;

angle::Result TranslatedAttribute::computeOffset(const gl::Context *context,
                                                 GLint startVertex,
                                                 unsigned int *offsetOut) const
{
    if (!usesFirstVertexOffset)
    {
        *offsetOut = baseOffset;
        return angle::Result::Continue;
    }

    CheckedNumeric<unsigned int> offset(baseOffset);
    CheckedNumeric<unsigned int> checkedStride(stride);

    offset += checkedStride * static_cast<unsigned int>(startVertex);
    ANGLE_CHECK_GL_MATH(GetImplAs<ContextD3D>(context), offset.IsValid());
    *offsetOut = offset.ValueOrDie();
    return angle::Result::Continue;
}

// Warning: you should ensure binding really matches attrib.bindingIndex before using this function.
VertexStorageType ClassifyAttributeStorage(const gl::Context *context,
                                           const gl::VertexAttribute &attrib,
                                           const gl::VertexBinding &binding)
{
    // If attribute is disabled, we use the current value.
    if (!attrib.enabled)
    {
        return VertexStorageType::CURRENT_VALUE;
    }

    // If specified with immediate data, we must use dynamic storage.
    gl::Buffer *buffer = binding.getBuffer().get();
    if (!buffer)
    {
        return VertexStorageType::DYNAMIC;
    }

    // Check if the buffer supports direct storage.
    if (DirectStoragePossible(context, attrib, binding))
    {
        return VertexStorageType::DIRECT;
    }

    // Otherwise the storage is static or dynamic.
    BufferD3D *bufferD3D = GetImplAs<BufferD3D>(buffer);
    ASSERT(bufferD3D);
    switch (bufferD3D->getUsage())
    {
        case D3DBufferUsage::DYNAMIC:
            return VertexStorageType::DYNAMIC;
        case D3DBufferUsage::STATIC:
            return VertexStorageType::STATIC;
        default:
            UNREACHABLE();
            return VertexStorageType::UNKNOWN;
    }
}

VertexDataManager::CurrentValueState::CurrentValueState(BufferFactoryD3D *factory)
    : buffer(new StreamingVertexBufferInterface(factory)), offset(0)
{
    data.Values.FloatValues[0] = std::numeric_limits<float>::quiet_NaN();
    data.Values.FloatValues[1] = std::numeric_limits<float>::quiet_NaN();
    data.Values.FloatValues[2] = std::numeric_limits<float>::quiet_NaN();
    data.Values.FloatValues[3] = std::numeric_limits<float>::quiet_NaN();
    data.Type                  = gl::VertexAttribType::Float;
}

VertexDataManager::CurrentValueState::CurrentValueState(CurrentValueState &&other)
{
    std::swap(buffer, other.buffer);
    std::swap(data, other.data);
    std::swap(offset, other.offset);
}

VertexDataManager::CurrentValueState::~CurrentValueState() {}

VertexDataManager::VertexDataManager(BufferFactoryD3D *factory)
    : mFactory(factory), mStreamingBuffer(factory)
{
    mCurrentValueCache.reserve(gl::MAX_VERTEX_ATTRIBS);
    for (int currentValueIndex = 0; currentValueIndex < gl::MAX_VERTEX_ATTRIBS; ++currentValueIndex)
    {
        mCurrentValueCache.emplace_back(factory);
    }
}

VertexDataManager::~VertexDataManager() {}

angle::Result VertexDataManager::initialize(const gl::Context *context)
{
    return mStreamingBuffer.initialize(context, INITIAL_STREAM_BUFFER_SIZE);
}

void VertexDataManager::deinitialize()
{
    mStreamingBuffer.reset();
    mCurrentValueCache.clear();
}

angle::Result VertexDataManager::prepareVertexData(
    const gl::Context *context,
    GLint start,
    GLsizei count,
    std::vector<TranslatedAttribute> *translatedAttribs,
    GLsizei instances)
{
    const gl::State &state                  = context->getState();
    const gl::ProgramExecutable *executable = state.getProgramExecutable();
    const gl::VertexArray *vertexArray      = state.getVertexArray();
    const auto &vertexAttributes            = vertexArray->getVertexAttributes();
    const auto &vertexBindings              = vertexArray->getVertexBindings();

    mDynamicAttribsMaskCache.reset();

    translatedAttribs->clear();

    for (size_t attribIndex = 0; attribIndex < vertexAttributes.size(); ++attribIndex)
    {
        // Skip attrib locations the program doesn't use.
        if (!executable->isAttribLocationActive(attribIndex))
            continue;

        const auto &attrib  = vertexAttributes[attribIndex];
        const auto &binding = vertexBindings[attrib.bindingIndex];

        // Resize automatically puts in empty attribs
        translatedAttribs->resize(attribIndex + 1);

        TranslatedAttribute *translated = &(*translatedAttribs)[attribIndex];
        auto currentValueData           = state.getVertexAttribCurrentValue(attribIndex);

        // Record the attribute now
        translated->active           = true;
        translated->attribute        = &attrib;
        translated->binding          = &binding;
        translated->currentValueType = currentValueData.Type;
        translated->divisor          = binding.getDivisor();

        switch (ClassifyAttributeStorage(context, attrib, binding))
        {
            case VertexStorageType::STATIC:
            {
                // Store static attribute.
                ANGLE_TRY(StoreStaticAttrib(context, translated));
                break;
            }
            case VertexStorageType::DYNAMIC:
                // Dynamic attributes must be handled together.
                mDynamicAttribsMaskCache.set(attribIndex);
                break;
            case VertexStorageType::DIRECT:
                // Update translated data for direct attributes.
                StoreDirectAttrib(context, translated);
                break;
            case VertexStorageType::CURRENT_VALUE:
            {
                ANGLE_TRY(storeCurrentValue(context, currentValueData, translated, attribIndex));
                break;
            }
            default:
                UNREACHABLE();
                break;
        }
    }

    if (mDynamicAttribsMaskCache.none())
    {
        return angle::Result::Continue;
    }

    // prepareVertexData is only called by Renderer9 which don't support baseInstance
    ANGLE_TRY(storeDynamicAttribs(context, translatedAttribs, mDynamicAttribsMaskCache, start,
                                  count, instances, 0u));

    PromoteDynamicAttribs(context, *translatedAttribs, mDynamicAttribsMaskCache, count);

    return angle::Result::Continue;
}

// static
void VertexDataManager::StoreDirectAttrib(const gl::Context *context,
                                          TranslatedAttribute *directAttrib)
{
    ASSERT(directAttrib->attribute && directAttrib->binding);
    const auto &attrib  = *directAttrib->attribute;
    const auto &binding = *directAttrib->binding;

    gl::Buffer *buffer = binding.getBuffer().get();
    ASSERT(buffer);
    BufferD3D *bufferD3D = GetImplAs<BufferD3D>(buffer);

    ASSERT(DirectStoragePossible(context, attrib, binding));
    directAttrib->vertexBuffer.set(nullptr);
    directAttrib->storage = bufferD3D;
    directAttrib->serial  = bufferD3D->getSerial();
    directAttrib->stride = static_cast<unsigned int>(ComputeVertexAttributeStride(attrib, binding));
    directAttrib->baseOffset =
        static_cast<unsigned int>(ComputeVertexAttributeOffset(attrib, binding));

    // Instanced vertices do not apply the 'start' offset
    directAttrib->usesFirstVertexOffset = (binding.getDivisor() == 0);
}

// static
angle::Result VertexDataManager::StoreStaticAttrib(const gl::Context *context,
                                                   TranslatedAttribute *translated)
{
    ASSERT(translated->attribute && translated->binding);
    const auto &attrib  = *translated->attribute;
    const auto &binding = *translated->binding;

    gl::Buffer *buffer = binding.getBuffer().get();
    ASSERT(buffer && attrib.enabled && !DirectStoragePossible(context, attrib, binding));
    BufferD3D *bufferD3D = GetImplAs<BufferD3D>(buffer);

    // Compute source data pointer
    const uint8_t *sourceData = nullptr;
    const int offset          = static_cast<int>(ComputeVertexAttributeOffset(attrib, binding));

    ANGLE_TRY(bufferD3D->getData(context, &sourceData));

    if (sourceData)
    {
        sourceData += offset;
    }

    unsigned int streamOffset = 0;

    translated->storage = nullptr;
    ANGLE_TRY(bufferD3D->getFactory()->getVertexSpaceRequired(context, attrib, binding, 1, 0, 0,
                                                              &translated->stride));

    auto *staticBuffer = bufferD3D->getStaticVertexBuffer(attrib, binding);
    ASSERT(staticBuffer);

    if (staticBuffer->empty())
    {
        // Convert the entire buffer
        int totalCount =
            ElementsInBuffer(attrib, binding, static_cast<unsigned int>(bufferD3D->getSize()));
        int startIndex = offset / static_cast<int>(ComputeVertexAttributeStride(attrib, binding));

        if (totalCount > 0)
        {
            ANGLE_TRY(staticBuffer->storeStaticAttribute(context, attrib, binding, -startIndex,
                                                         totalCount, 0, sourceData));
        }
    }

    unsigned int firstElementOffset =
        (static_cast<unsigned int>(offset) /
         static_cast<unsigned int>(ComputeVertexAttributeStride(attrib, binding))) *
        translated->stride;

    VertexBuffer *vertexBuffer = staticBuffer->getVertexBuffer();

    CheckedNumeric<unsigned int> checkedOffset(streamOffset);
    checkedOffset += firstElementOffset;

    ANGLE_CHECK_GL_MATH(GetImplAs<ContextD3D>(context), checkedOffset.IsValid());

    translated->vertexBuffer.set(vertexBuffer);
    translated->serial     = vertexBuffer->getSerial();
    translated->baseOffset = streamOffset + firstElementOffset;

    // Instanced vertices do not apply the 'start' offset
    translated->usesFirstVertexOffset = (binding.getDivisor() == 0);

    return angle::Result::Continue;
}

angle::Result VertexDataManager::storeDynamicAttribs(
    const gl::Context *context,
    std::vector<TranslatedAttribute> *translatedAttribs,
    const gl::AttributesMask &dynamicAttribsMask,
    GLint start,
    size_t count,
    GLsizei instances,
    GLuint baseInstance)
{
    // Instantiating this class will ensure the streaming buffer is never left mapped.
    class StreamingBufferUnmapper final : NonCopyable
    {
      public:
        StreamingBufferUnmapper(StreamingVertexBufferInterface *streamingBuffer)
            : mStreamingBuffer(streamingBuffer)
        {
            ASSERT(mStreamingBuffer);
        }
        ~StreamingBufferUnmapper() { mStreamingBuffer->getVertexBuffer()->hintUnmapResource(); }

      private:
        StreamingVertexBufferInterface *mStreamingBuffer;
    };

    // Will trigger unmapping on return.
    StreamingBufferUnmapper localUnmapper(&mStreamingBuffer);

    // Reserve the required space for the dynamic buffers.
    for (auto attribIndex : dynamicAttribsMask)
    {
        const auto &dynamicAttrib = (*translatedAttribs)[attribIndex];
        ANGLE_TRY(
            reserveSpaceForAttrib(context, dynamicAttrib, start, count, instances, baseInstance));
    }

    // Store dynamic attributes
    for (auto attribIndex : dynamicAttribsMask)
    {
        auto *dynamicAttrib = &(*translatedAttribs)[attribIndex];
        ANGLE_TRY(
            storeDynamicAttrib(context, dynamicAttrib, start, count, instances, baseInstance));
    }

    return angle::Result::Continue;
}

void VertexDataManager::PromoteDynamicAttribs(
    const gl::Context *context,
    const std::vector<TranslatedAttribute> &translatedAttribs,
    const gl::AttributesMask &dynamicAttribsMask,
    size_t count)
{
    for (auto attribIndex : dynamicAttribsMask)
    {
        const auto &dynamicAttrib = translatedAttribs[attribIndex];
        ASSERT(dynamicAttrib.attribute && dynamicAttrib.binding);
        const auto &binding = *dynamicAttrib.binding;

        gl::Buffer *buffer = binding.getBuffer().get();
        if (buffer)
        {
            // Note: this multiplication can overflow. It should not be a security problem.
            BufferD3D *bufferD3D = GetImplAs<BufferD3D>(buffer);
            size_t typeSize      = ComputeVertexAttributeTypeSize(*dynamicAttrib.attribute);
            bufferD3D->promoteStaticUsage(context, count * typeSize);
        }
    }
}

angle::Result VertexDataManager::reserveSpaceForAttrib(const gl::Context *context,
                                                       const TranslatedAttribute &translatedAttrib,
                                                       GLint start,
                                                       size_t count,
                                                       GLsizei instances,
                                                       GLuint baseInstance)
{
    ASSERT(translatedAttrib.attribute && translatedAttrib.binding);
    const auto &attrib  = *translatedAttrib.attribute;
    const auto &binding = *translatedAttrib.binding;

    ASSERT(!DirectStoragePossible(context, attrib, binding));

    gl::Buffer *buffer   = binding.getBuffer().get();
    BufferD3D *bufferD3D = buffer ? GetImplAs<BufferD3D>(buffer) : nullptr;
    ASSERT(!bufferD3D || bufferD3D->getStaticVertexBuffer(attrib, binding) == nullptr);

    // Make sure we always pass at least one instance count to gl::ComputeVertexBindingElementCount.
    // Even if this is not an instanced draw call, some attributes can still be instanced if they
    // have a non-zero divisor.
    size_t totalCount = gl::ComputeVertexBindingElementCount(
        binding.getDivisor(), count, static_cast<size_t>(std::max(instances, 1)));
    // TODO(jiajia.qin@intel.com): force the index buffer to clamp any out of range indices instead
    // of invalid operation here.
    if (bufferD3D)
    {
        // Vertices do not apply the 'start' offset when the divisor is non-zero even when doing
        // a non-instanced draw call
        GLint firstVertexIndex = binding.getDivisor() > 0
                                     ? UnsignedCeilDivide(baseInstance, binding.getDivisor())
                                     : start;
        int64_t maxVertexCount =
            static_cast<int64_t>(firstVertexIndex) + static_cast<int64_t>(totalCount);

        int64_t maxByte = GetMaxAttributeByteOffsetForDraw(attrib, binding, maxVertexCount);

        ASSERT(bufferD3D->getSize() <= static_cast<size_t>(std::numeric_limits<int64_t>::max()));
        ANGLE_CHECK(GetImplAs<ContextD3D>(context),
                    maxByte <= static_cast<int64_t>(bufferD3D->getSize()),
                    "Vertex buffer is not big enough for the draw call.", GL_INVALID_OPERATION);
    }
    return mStreamingBuffer.reserveVertexSpace(context, attrib, binding, totalCount, instances,
                                               baseInstance);
}

angle::Result VertexDataManager::storeDynamicAttrib(const gl::Context *context,
                                                    TranslatedAttribute *translated,
                                                    GLint start,
                                                    size_t count,
                                                    GLsizei instances,
                                                    GLuint baseInstance)
{
    ASSERT(translated->attribute && translated->binding);
    const auto &attrib  = *translated->attribute;
    const auto &binding = *translated->binding;

    gl::Buffer *buffer = binding.getBuffer().get();
    ASSERT(buffer || attrib.pointer);
    ASSERT(attrib.enabled);

    BufferD3D *storage = buffer ? GetImplAs<BufferD3D>(buffer) : nullptr;

    // Instanced vertices do not apply the 'start' offset
    GLint firstVertexIndex =
        (binding.getDivisor() > 0 ? UnsignedCeilDivide(baseInstance, binding.getDivisor()) : start);

    // Compute source data pointer
    const uint8_t *sourceData = nullptr;

    if (buffer)
    {
        ANGLE_TRY(storage->getData(context, &sourceData));
        sourceData += static_cast<int>(ComputeVertexAttributeOffset(attrib, binding));
    }
    else
    {
        // Attributes using client memory ignore the VERTEX_ATTRIB_BINDING state.
        // https://www.opengl.org/registry/specs/ARB/vertex_attrib_binding.txt
        sourceData = static_cast<const uint8_t *>(attrib.pointer);
    }

    unsigned int streamOffset = 0;

    translated->storage = nullptr;
    ANGLE_TRY(
        mFactory->getVertexSpaceRequired(context, attrib, binding, 1, 0, 0, &translated->stride));

    size_t totalCount = gl::ComputeVertexBindingElementCount(
        binding.getDivisor(), count, static_cast<size_t>(std::max(instances, 1)));

    ANGLE_TRY(mStreamingBuffer.storeDynamicAttribute(
        context, attrib, binding, translated->currentValueType, firstVertexIndex,
        static_cast<GLsizei>(totalCount), instances, baseInstance, &streamOffset, sourceData));

    VertexBuffer *vertexBuffer = mStreamingBuffer.getVertexBuffer();

    translated->vertexBuffer.set(vertexBuffer);
    translated->serial                = vertexBuffer->getSerial();
    translated->baseOffset            = streamOffset;
    translated->usesFirstVertexOffset = false;

    return angle::Result::Continue;
}

angle::Result VertexDataManager::storeCurrentValue(
    const gl::Context *context,
    const gl::VertexAttribCurrentValueData &currentValue,
    TranslatedAttribute *translated,
    size_t attribIndex)
{
    CurrentValueState *cachedState         = &mCurrentValueCache[attribIndex];
    StreamingVertexBufferInterface &buffer = *cachedState->buffer;

    if (buffer.getBufferSize() == 0)
    {
        ANGLE_TRY(buffer.initialize(context, CONSTANT_VERTEX_BUFFER_SIZE));
    }

    if (cachedState->data != currentValue)
    {
        ASSERT(translated->attribute && translated->binding);
        const auto &attrib  = *translated->attribute;
        const auto &binding = *translated->binding;

        ANGLE_TRY(buffer.reserveVertexSpace(context, attrib, binding, 1, 0, 0));

        const uint8_t *sourceData =
            reinterpret_cast<const uint8_t *>(currentValue.Values.FloatValues);
        unsigned int streamOffset;
        ANGLE_TRY(buffer.storeDynamicAttribute(context, attrib, binding, currentValue.Type, 0, 1, 0,
                                               0, &streamOffset, sourceData));

        buffer.getVertexBuffer()->hintUnmapResource();

        cachedState->data   = currentValue;
        cachedState->offset = streamOffset;
    }

    translated->vertexBuffer.set(buffer.getVertexBuffer());

    translated->storage               = nullptr;
    translated->serial                = buffer.getSerial();
    translated->divisor               = 0;
    translated->stride                = 0;
    translated->baseOffset            = static_cast<unsigned int>(cachedState->offset);
    translated->usesFirstVertexOffset = false;

    return angle::Result::Continue;
}

// VertexBufferBinding implementation
VertexBufferBinding::VertexBufferBinding() : mBoundVertexBuffer(nullptr) {}

VertexBufferBinding::VertexBufferBinding(const VertexBufferBinding &other)
    : mBoundVertexBuffer(other.mBoundVertexBuffer)
{
    if (mBoundVertexBuffer)
    {
        mBoundVertexBuffer->addRef();
    }
}

VertexBufferBinding::~VertexBufferBinding()
{
    if (mBoundVertexBuffer)
    {
        mBoundVertexBuffer->release();
    }
}

VertexBufferBinding &VertexBufferBinding::operator=(const VertexBufferBinding &other)
{
    mBoundVertexBuffer = other.mBoundVertexBuffer;
    if (mBoundVertexBuffer)
    {
        mBoundVertexBuffer->addRef();
    }
    return *this;
}

void VertexBufferBinding::set(VertexBuffer *vertexBuffer)
{
    if (mBoundVertexBuffer == vertexBuffer)
        return;

    if (mBoundVertexBuffer)
    {
        mBoundVertexBuffer->release();
    }
    if (vertexBuffer)
    {
        vertexBuffer->addRef();
    }

    mBoundVertexBuffer = vertexBuffer;
}

VertexBuffer *VertexBufferBinding::get() const
{
    return mBoundVertexBuffer;
}

}  // namespace rx
