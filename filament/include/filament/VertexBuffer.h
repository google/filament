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

#ifndef TNT_FILAMENT_VERTEXBUFFER_H
#define TNT_FILAMENT_VERTEXBUFFER_H

#include <filament/EngineEnums.h>
#include <filament/FilamentAPI.h>

#include <filament/driver/BufferDescriptor.h>
#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>

namespace filament {

namespace details {
class FVertexBuffer;
} // namespace details

class Engine;

/**
 * Holds a set of buffers that define the geometry of a Renderable.
 */
class UTILS_PUBLIC VertexBuffer : public FilamentAPI {
    struct BuilderDetails;

public:
    using AttributeType = driver::ElementType;
    using BufferDescriptor = driver::BufferDescriptor;

    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        Builder& vertexCount(uint32_t vertexCount) noexcept;
        Builder& bufferCount(uint8_t bufferCount) noexcept;

        // no-op if attribute is an invalid enum
        // no-op if bufferIndex is out of bounds
        Builder& attribute(VertexAttribute attribute, uint8_t bufferIndex,
                AttributeType attributeType,
                uint32_t byteOffset = 0,
                uint8_t byteStride = 0) noexcept;     // default is attribute size

        // no-op if attribute is an invalid enum
        Builder& normalized(VertexAttribute attribute) noexcept;

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
        friend class details::FVertexBuffer;
        struct AttributeData {
            uint32_t offset = 0;
            uint8_t stride = 0;
            uint8_t buffer = 0;
            AttributeType type = AttributeType::FLOAT4;
            uint8_t flags = 0x0; // See Driver::Attribute
        };
    };

    /**
     * Returns the vertex count.
     */
    size_t getVertexCount() const noexcept;

    /**
     * Moves the given buffer data into the slot at the given index.
     *
     * Does nothing if bufferIndex >= bufferCount.
     */
    void setBufferAt(Engine& engine, uint8_t bufferIndex,
            BufferDescriptor&& buffer,
            uint32_t byteOffset = 0,
            uint32_t byteSize = 0);

    /**
     * Specifies the quaternion type for the "populateTangentQuaternions" utility.
     */
    enum QuatType {
        HALF4,  // 2 bytes per component as half-floats (8 bytes per quat)
        SHORT4, // 2 bytes per component as normalized integers (8 bytes per quat)
        FLOAT4, // 4 bytes per component as floats (16 bytes per quat)
    };

    /**
     * Specifies the parameters for the "populateTangentQuaternions" utility.
     */
    struct QuatTangentContext {
        QuatType quatType;            // required
        size_t quatCount;             // required
        void* outBuffer;              // required
        size_t outStride;             // required stride in bytes
        const math::float3* normals;  // required source data
        size_t normalsStride;         // optional stride in bytes (assumes packed)
        const math::float4* tangents; // optional source data
        size_t tangentsStride;        // optional stride in bytes (assumes packed)
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
     * Note that some applications and file formats (e.g. Blender and glTF) use mikktspace, which
     * consumes full topology information and produces an unindexed mesh, so it cannot be used here.
     * This function exists for simple use cases only.
     */
    static void populateTangentQuaternions(const QuatTangentContext& ctx);
};

} // namespace filament

#endif // TNT_FILAMENT_VERTEXBUFFER_H
