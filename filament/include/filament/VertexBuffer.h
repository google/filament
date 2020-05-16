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

namespace filament {

class FVertexBuffer;

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

    class Builder : public BuilderBase<BuilderDetails> {
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
         * Creates the VertexBuffer object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
         *
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occurred.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        VertexBuffer* build(Engine& engine);

    private:
        friend class FVertexBuffer;
    };

    /**
     * Returns the vertex count.
     * @return Number of vertices in this vertex buffer set.
     */
    size_t getVertexCount() const noexcept;

    /**
     * Asynchronously copy-initializes the specified buffer from the given buffer data.
     *
     * @param engine Reference to the filament::Engine to associate this VertexBuffer with.
     * @param bufferIndex Index of the buffer to initialize. Must be between 0
     *                    and Builder::bufferCount() - 1.
     * @param buffer A BufferDescriptor representing the data used to initialize the buffer at
     *               index \p bufferIndex. BufferDescriptor points to raw, untyped data that will
     *               be copied as-is into the buffer.
     * @param byteOffset Offset in *bytes* into the buffer at index \p bufferIndex of this vertex
     *                   buffer set.
     */
    void setBufferAt(Engine& engine, uint8_t bufferIndex, BufferDescriptor&& buffer,
            uint32_t byteOffset = 0);

    /**
     * Specifies the quaternion type for the "populateTangentQuaternions" utility.
     */
    enum QuatType {
        HALF4,  //!< 2 bytes per component as half-floats (8 bytes per quat)
        SHORT4, //!< 2 bytes per component as normalized integers (8 bytes per quat)
        FLOAT4, //!< 4 bytes per component as floats (16 bytes per quat)
    };

    /**
     * Specifies the parameters for the "populateTangentQuaternions" utility.
     */
    struct QuatTangentContext {
        QuatType quatType;                      //!< desired quaternion type (required)
        size_t quatCount;                       //!< number of quaternions (required)
        void* outBuffer;                        //!< pre-allocated output buffer (required)
        size_t outStride;                       //!< desired stride in bytes (optional)
        const math::float3* normals;  //!< source normals (required)
        size_t normalsStride;                   //!< normals stride in bytes (optional)
        const math::float4* tangents; //!< source tangents (optional)
        size_t tangentsStride;                  //!< tangents stride in bytes (optional)
    };

    /**
     * Convenience function that consumes normal vectors (and, optionally, tangent vectors) and
     * produces quaternions that can be passed into a TANGENTS buffer.
     *
     * The given output buffer must be preallocated with at least quatCount * outStride bytes.
     *
     * Normals are required but tangents are optional, in which case this function tries to generate
     * reasonable tangents. The given normals should be unit length.
     *
     * If supplied, the tangent vectors should be unit length and should be orthogonal to the
     * normals. The w component of the tangent is a sign (-1 or +1) indicating handedness of the
     * basis.
     *
     * @param ctx An initialized QuatTangentContext structure.
     *
     * @deprecated Instead please use filament::geometry::SurfaceOrientation from libgeometry, it
     * has additional capabilities and a daisy-chain API. Be sure to explicitly link libgeometry
     * since its dependency might be removed in future versions of libfilament.
     */
    static void populateTangentQuaternions(const QuatTangentContext& ctx);
};

} // namespace filament

#endif // TNT_FILAMENT_VERTEXBUFFER_H
