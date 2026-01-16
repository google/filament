/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! \file

#ifndef TNT_FILAMENT_VERTEXBUFFER_H
#define TNT_FILAMENT_VERTEXBUFFER_H

#include <filament/FilamentAPI.h>
#include <filament/MaterialEnums.h>

#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/StaticString.h>

#include <functional>
#include <stddef.h>
#include <stdint.h>

namespace filament {

class FVertexBuffer;

class BufferObject;
class Engine;

/**
 * Holds a set of buffers that define the geometry of a Renderable.
 *
 * The geometry of the Renderable itself is defined by a set of vertex attributes such as
 * position, color, normals, tangents, etc...
 *
 * There is no need to have a 1-to-1 mapping between attributes and buffer. A buffer can hold the
 * data of several attributes -- attributes are then referred as being "interleaved".
 *
 * The buffers themselves are GPU resources, therefore mutating their data can be relatively slow.
 * For this reason, it is best to separate the constant data from the dynamic data into multiple
 * buffers.
 *
 * It is possible, and even encouraged, to use a single vertex buffer for several Renderables.
 *
 * @see IndexBuffer, RenderableManager
 */
class UTILS_PUBLIC VertexBuffer : public FilamentAPI {
    struct BuilderDetails;

public:
    using AttributeType = backend::ElementType;
    using BufferDescriptor = backend::BufferDescriptor;
    using AsyncCompletionCallback =
            std::function<void(VertexBuffer* UTILS_NONNULL, void* UTILS_NULLABLE)>;
    using AsyncCallId = backend::AsyncCallId;


    class Builder : public BuilderBase<BuilderDetails>, public BuilderNameMixin<Builder> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Defines how many buffers will be created in this vertex buffer set. These buffers are
         * later referenced by index from 0 to \p bufferCount - 1.
         *
         * This call is mandatory. The default is 0.
         *
         * @param bufferCount Number of buffers in this vertex buffer set. The maximum value is 8.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& bufferCount(uint8_t bufferCount) noexcept;

        /**
         * Size of each buffer in the set in vertex.
         *
         * @param vertexCount Number of vertices in each buffer in this set.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& vertexCount(uint32_t vertexCount) noexcept;

        /**
         * Allows buffers to be swapped out and shared using BufferObject.
         *
         * If buffer objects mode is enabled, clients must call setBufferObjectAt rather than
         * setBufferAt. This allows sharing of data between VertexBuffer objects, but it may
         * slightly increase the memory footprint of Filament's internal bookkeeping.
         *
         * @param enabled If true, enables buffer object mode.  False by default.
         */
        Builder& enableBufferObjects(bool enabled = true) noexcept;

        /**
         * Sets up an attribute for this vertex buffer set.
         *
         * Using \p byteOffset and \p byteStride, attributes can be interleaved in the same buffer.
         *
         * @param attribute The attribute to set up.
         * @param bufferIndex  The index of the buffer containing the data for this attribute. Must
         *                     be between 0 and bufferCount() - 1.
         * @param attributeType The type of the attribute data (e.g. byte, float3, etc...)
         * @param byteOffset Offset in *bytes* into the buffer \p bufferIndex
         * @param byteStride Stride in *bytes* to the next element of this attribute. When set to
         *                   zero the attribute size, as defined by \p attributeType is used.
         *
         * @return A reference to this Builder for chaining calls.
         *
         * @warning VertexAttribute::TANGENTS must be specified as a quaternion and is how normals
         *          are specified.
         *
         * @warning Not all backends support 3-component attributes that are not floats. For help
         *          with conversion, see geometry::Transcoder.
         *
         * @see VertexAttribute
         *
         * This is a no-op if the \p attribute is an invalid enum.
         * This is a no-op if the \p bufferIndex is out of bounds.
         *
         */
        Builder& attribute(VertexAttribute attribute, uint8_t bufferIndex,
                AttributeType attributeType,
                uint32_t byteOffset = 0, uint8_t byteStride = 0) noexcept;

        /**
         * Sets whether a given attribute should be normalized. By default attributes are not
         * normalized. A normalized attribute is mapped between 0 and 1 in the shader. This applies
         * only to integer types.
         *
         * @param attribute Enum of the attribute to set the normalization flag to.
         * @param normalize true to automatically normalize the given attribute.
         * @return A reference to this Builder for chaining calls.
         *
         * This is a no-op if the \p attribute is an invalid enum.
         */
        Builder& normalized(VertexAttribute attribute, bool normalize = true) noexcept;

        /**
         * Sets advanced skinning mode. Bone data, indices and weights will be
         * set in RenderableManager:Builder:boneIndicesAndWeights methods.
         * Works with or without buffer objects.
         *
         * @param enabled If true, enables advanced skinning mode. False by default.
         *
         * @return A reference to this Builder for chaining calls.
         *
         * @see RenderableManager:Builder:boneIndicesAndWeights
         */
        Builder& advancedSkinning(bool enabled) noexcept;

        /**
         * Associate an optional name with this VertexBuffer for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible. The name is
         * truncated to a maximum of 128 characters.
         *
         * The name string is copied during this method so clients may free its memory after
         * the function returns.
         *
         * @param name A string to identify this VertexBuffer
         * @param len Length of name, should be less than or equal to 128
         * @return This Builder, for chaining calls.
         * @deprecated Use name(utils::StaticString const&) instead.
         */
        UTILS_DEPRECATED
        Builder& name(const char* UTILS_NONNULL name, size_t len) noexcept;

        /**
         * Associate an optional name with this VertexBuffer for debugging purposes.
         *
         * name will show in error messages and should be kept as short as possible.
         *
         * @param name A string literal to identify this VertexBuffer
         * @return This Builder, for chaining calls.
         */
        Builder& name(utils::StaticString const& name) noexcept;

        /**
         * Specifies a callback that will execute once the resource's data has been fully allocated
         * within the GPU memory. This enables the resource creation process to be handled
         * asynchronously.
         *
         * Any asynchronous calls made during a resource's asynchronous creation (using this method)
         * are safe because they are queued and executed in sequence. However, invoking regular
         * methods on the same resource before it's fully ready is unsafe and may cause undefined
         * behavior. Users can call the `isCreationComplete()` method for the resource to confirm
         * when the resource is ready for regular API calls.
         *
         * To use this method, the engine must be configured for asynchronous operation. Otherwise,
         * calling async method will cause the program to terminate.
         *
         * @param handler Handler to dispatch the callback or nullptr for the default handler
         * @param callback A function to be called upon the completion of an asynchronous creation.
         * @param user The custom data that will be passed as the second argument to the `callback`.
         * @return This Builder, for chaining calls.
         */
        Builder& async(backend::CallbackHandler* UTILS_NULLABLE handler,
                AsyncCompletionCallback callback = nullptr,
                void* UTILS_NULLABLE user = nullptr) noexcept;

        /**
         * Creates the VertexBuffer object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
         *
         * @return pointer to the newly created object.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        VertexBuffer* UTILS_NONNULL build(Engine& engine);

    private:
        friend class FVertexBuffer;
    };

    /**
     * Returns the vertex count.
     * @return Number of vertices in this vertex buffer set.
     */
    size_t getVertexCount() const noexcept;

    /**
     * copy-initializes the specified buffer from the given buffer data.
     *
     * Do not use this if you called enableBufferObjects() on the Builder.
     *
     * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
     * @param bufferIndex Index of the buffer to initialize. Must be between 0
     *                    and Builder::bufferCount() - 1.
     * @param buffer A BufferDescriptor representing the data used to initialize the buffer at
     *               index \p bufferIndex. BufferDescriptor points to raw, untyped data that will
     *               be copied as-is into the buffer.
     * @param byteOffset Offset in *bytes* into the buffer at index \p bufferIndex of this vertex
     *                   buffer set.  Must be multiple of 4.
     */
    void setBufferAt(Engine& engine, uint8_t bufferIndex, BufferDescriptor&& buffer,
            uint32_t byteOffset = 0);

    /**
     * An asynchronous version of `setBufferAt()`.
     * Asynchronously copy-initializes the specified buffer from the given buffer data.
     *
     * Users can call the `Engine::cancelAsyncCall()` method with the returned ID to cancel the
     * asynchronous call.
     *
     * To use this method, the engine must be configured for asynchronous operation. Otherwise,
     * calling async method will cause the program to terminate.
     *
     * Do not use this if you called enableBufferObjects() on the Builder.
     *
     * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
     * @param bufferIndex Index of the buffer to initialize. Must be between 0
     *                    and Builder::bufferCount() - 1.
     * @param buffer A BufferDescriptor representing the data used to initialize the buffer at
     *               index \p bufferIndex. BufferDescriptor points to raw, untyped data that will
     *               be copied as-is into the buffer.
     * @param byteOffset Offset in *bytes* into the buffer at index \p bufferIndex of this vertex
     *                   buffer set.  Must be multiple of 4.
     * @param handler Handler to dispatch the callback or nullptr for the default handler
     * @param callback A function to be called upon the completion of an asynchronous creation.
     * @param user The custom data that will be passed as the second argument to the `callback`.
     *
     * @return An ID that the caller can use to cancel the operation.
     */
    AsyncCallId setBufferAtAsync(Engine& engine, uint8_t bufferIndex, BufferDescriptor&& buffer,
            uint32_t byteOffset, backend::CallbackHandler* UTILS_NULLABLE handler,
            AsyncCompletionCallback callback, void* UTILS_NULLABLE user = nullptr);

    /**
     * Swaps in the given buffer object.
     *
     * To use this, you must first call enableBufferObjects() on the Builder.
     *
     * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
     * @param bufferIndex Index of the buffer to initialize. Must be between 0
     *                    and Builder::bufferCount() - 1.
     * @param bufferObject The handle to the GPU data that will be used in this buffer slot.
     */
    void setBufferObjectAt(Engine& engine, uint8_t bufferIndex,
            BufferObject const*  UTILS_NONNULL bufferObject);

    /**
     * An asynchronous version of `setBufferObjectAt()`.
     * Swaps in the given buffer object.
     *
     * Users can call the `Engine::cancelAsyncCall()` method with the returned ID to cancel the
     * asynchronous call.
     *
     * To use this method, the engine must be configured for asynchronous operation. Otherwise,
     * calling async method will cause the program to terminate.
     *
     * To use this, you must first call enableBufferObjects() on the Builder.
     *
     * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
     * @param bufferIndex Index of the buffer to initialize. Must be between 0
     *                    and Builder::bufferCount() - 1.
     * @param bufferObject The handle to the GPU data that will be used in this buffer slot.
     * @param handler   Handler to dispatch the callback or nullptr for the default handler
     * @param callback  A function to be called upon the completion of an asynchronous creation.
     * @param user      The custom data that will be passed as the second argument to the `callback`.
     *
     * @return An ID that the caller can use to cancel the operation.
     */
    AsyncCallId setBufferObjectAtAsync(Engine& engine, uint8_t bufferIndex,
            BufferObject const*  UTILS_NONNULL bufferObject,
            backend::CallbackHandler* UTILS_NULLABLE handler,
            AsyncCompletionCallback callback, void* UTILS_NULLABLE user = nullptr);

    /**
     * This non-blocking method checks if the resource has finished creation. If the resource
     * creation was initiated asynchronously, it will return true only after all related
     * asynchronous tasks are complete. If the resource was created normally without using async
     * method, it will always return true.
     *
     * @return Whether the resource is created.
     *
     * @see Builder::async()
     */
    bool isCreationComplete() const noexcept;

protected:
    // prevent heap allocation
    ~VertexBuffer() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_VERTEXBUFFER_H
