// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_API_COMMON_VERTEX_PULLING_CONFIG_H_
#define SRC_TINT_API_COMMON_VERTEX_PULLING_CONFIG_H_

#include <cstdint>
#include <vector>

#include "src/tint/utils/reflection/reflection.h"

namespace tint {

/// Describes the format of data in a vertex buffer.
enum class VertexFormat : uint8_t {
    kUint8,            // uint8
    kUint8x2,          // uint8x2
    kUint8x4,          // uint8x4
    kSint8,            // sint8
    kSint8x2,          // sint8x2
    kSint8x4,          // sint8x4
    kUnorm8,           // unorm8
    kUnorm8x2,         // unorm8x2
    kUnorm8x4,         // unorm8x4
    kSnorm8,           // snorm8
    kSnorm8x2,         // snorm8x2
    kSnorm8x4,         // snorm8x4
    kUint16,           // uint16
    kUint16x2,         // uint16x2
    kUint16x4,         // uint16x4
    kSint16,           // sint16
    kSint16x2,         // sint16x2
    kSint16x4,         // sint16x4
    kUnorm16,          // unorm16
    kUnorm16x2,        // unorm16x2
    kUnorm16x4,        // unorm16x4
    kSnorm16,          // snorm16
    kSnorm16x2,        // snorm16x2
    kSnorm16x4,        // snorm16x4
    kFloat16,          // float16
    kFloat16x2,        // float16x2
    kFloat16x4,        // float16x4
    kFloat32,          // float32
    kFloat32x2,        // float32x2
    kFloat32x3,        // float32x3
    kFloat32x4,        // float32x4
    kUint32,           // uint32
    kUint32x2,         // uint32x2
    kUint32x3,         // uint32x3
    kUint32x4,         // uint32x4
    kSint32,           // sint32
    kSint32x2,         // sint32x2
    kSint32x3,         // sint32x3
    kSint32x4,         // sint32x4
    kUnorm10_10_10_2,  // unorm10-10-10-2
    kUnorm8x4BGRA,     // unorm8x4-bgra
    kSnorm10_10_10_2,  // snorm10-10-10-2
};

/// @param out the stream to write to
/// @param format the vertex format
/// @returns @p out so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, VertexFormat format) {
    switch (format) {
        case VertexFormat::kUint8:
            out << "uint8";
            break;
        case VertexFormat::kUint8x2:
            out << "uint8x2";
            break;
        case VertexFormat::kUint8x4:
            out << "uint8x4";
            break;
        case VertexFormat::kSint8:
            out << "sint8";
            break;
        case VertexFormat::kSint8x2:
            out << "sint8x2";
            break;
        case VertexFormat::kSint8x4:
            out << "sint8x4";
            break;
        case VertexFormat::kUnorm8:
            out << "unorm8";
            break;
        case VertexFormat::kUnorm8x2:
            out << "unorm8x2";
            break;
        case VertexFormat::kUnorm8x4:
            out << "unorm8x4";
            break;
        case VertexFormat::kSnorm8:
            out << "snorm8";
            break;
        case VertexFormat::kSnorm8x2:
            out << "snorm8x2";
            break;
        case VertexFormat::kSnorm8x4:
            out << "snorm8x4";
            break;
        case VertexFormat::kUint16:
            out << "uint16";
            break;
        case VertexFormat::kUint16x2:
            out << "uint16x2";
            break;
        case VertexFormat::kUint16x4:
            out << "uint16x4";
            break;
        case VertexFormat::kSint16:
            out << "sint16";
            break;
        case VertexFormat::kSint16x2:
            out << "sint16x2";
            break;
        case VertexFormat::kSint16x4:
            out << "sint16x4";
            break;
        case VertexFormat::kUnorm16:
            out << "unorm16";
            break;
        case VertexFormat::kUnorm16x2:
            out << "unorm16x2";
            break;
        case VertexFormat::kUnorm16x4:
            out << "unorm16x4";
            break;
        case VertexFormat::kSnorm16:
            out << "snorm16";
            break;
        case VertexFormat::kSnorm16x2:
            out << "snorm16x2";
            break;
        case VertexFormat::kSnorm16x4:
            out << "snorm16x4";
            break;
        case VertexFormat::kFloat16:
            out << "float16";
            break;
        case VertexFormat::kFloat16x2:
            out << "float16x2";
            break;
        case VertexFormat::kFloat16x4:
            out << "float16x4";
            break;
        case VertexFormat::kFloat32:
            out << "float32";
            break;
        case VertexFormat::kFloat32x2:
            out << "float32x2";
            break;
        case VertexFormat::kFloat32x3:
            out << "float32x3";
            break;
        case VertexFormat::kFloat32x4:
            out << "float32x4";
            break;
        case VertexFormat::kUint32:
            out << "uint32";
            break;
        case VertexFormat::kUint32x2:
            out << "uint32x2";
            break;
        case VertexFormat::kUint32x3:
            out << "uint32x3";
            break;
        case VertexFormat::kUint32x4:
            out << "uint32x4";
            break;
        case VertexFormat::kSint32:
            out << "sint32";
            break;
        case VertexFormat::kSint32x2:
            out << "sint32x2";
            break;
        case VertexFormat::kSint32x3:
            out << "sint32x3";
            break;
        case VertexFormat::kSint32x4:
            out << "sint32x4";
            break;
        case VertexFormat::kUnorm10_10_10_2:
            out << "unorm10_10_10_2";
            break;
        case VertexFormat::kUnorm8x4BGRA:
            out << "unorm8x4BGRA";
            break;
        case VertexFormat::kSnorm10_10_10_2:
            out << "snorm10_10_10_2";
            break;
    }
    return out;
}

/// Describes if a vertex attribute increments with vertex index or instance index.
enum class VertexStepMode : uint8_t { kVertex, kInstance };

/// @param out the stream to write to
/// @param mode the step mode
/// @returns @p out so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, VertexStepMode mode) {
    switch (mode) {
        case VertexStepMode::kVertex:
            out << "vertex";
            break;
        case VertexStepMode::kInstance:
            out << "instance";
            break;
    }
    return out;
}

/// Describes a vertex attribute within a buffer
struct VertexAttributeDescriptor {
    /// The format of the attribute.
    VertexFormat format = VertexFormat::kUint8;
    /// The byte offset of the attribute in the buffer.
    uint32_t offset = 0;
    /// The shader location used for the attribute.
    uint32_t shader_location = 0;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(VertexAttributeDescriptor, format, offset, shader_location);
    TINT_REFLECT_HASH_CODE(VertexAttributeDescriptor);

    bool operator==(const VertexAttributeDescriptor&) const = default;
};

/// Describes a buffer containing multiple vertex attributes
struct VertexBufferLayoutDescriptor {
    /// Constructor
    VertexBufferLayoutDescriptor();
    /// Constructor
    /// @param in_array_stride the array stride in bytes of the in buffer
    /// @param in_step_mode the step mode of the in buffer
    /// @param in_attributes the in attributes
    VertexBufferLayoutDescriptor(uint32_t in_array_stride,
                                 VertexStepMode in_step_mode,
                                 std::vector<VertexAttributeDescriptor> in_attributes);
    /// The array stride in bytes used in the in buffer.
    uint32_t array_stride = 0u;
    /// The input step mode used.
    VertexStepMode step_mode = VertexStepMode::kVertex;
    /// The vertex attributes.
    std::vector<VertexAttributeDescriptor> attributes;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(VertexBufferLayoutDescriptor, array_stride, step_mode, attributes);
    TINT_REFLECT_HASH_CODE(VertexBufferLayoutDescriptor);

    bool operator==(const VertexBufferLayoutDescriptor&) const = default;
};

/// Configuration options that control the vertex pulling transform.
struct VertexPullingConfig {
    /// The vertex state descriptor, containing info about attributes.
    std::vector<VertexBufferLayoutDescriptor> vertex_state;

    /// The "group" we will put all our vertex buffers into (as storage buffers).
    /// Default to 4 as it is past the limits of user-accessible groups.
    uint32_t pulling_group = 4u;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(VertexPullingConfig, vertex_state, pulling_group);
    TINT_REFLECT_HASH_CODE(VertexPullingConfig);

    bool operator==(const VertexPullingConfig&) const = default;
};

/// Reflection for VertexFormat.
TINT_REFLECT_ENUM_RANGE(tint::VertexFormat, kUint8, kSnorm10_10_10_2);

/// Reflection for VertexStepMode.
TINT_REFLECT_ENUM_RANGE(tint::VertexStepMode, kVertex, kInstance);

}  // namespace tint

#endif  // SRC_TINT_API_COMMON_VERTEX_PULLING_CONFIG_H_
