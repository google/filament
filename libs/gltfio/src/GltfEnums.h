/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef GLTFIO_GLTFENUMS_H
#define GLTFIO_GLTFENUMS_H

#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>

#include <cgltf.h>

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_REPEAT                         0x2901
#define GL_MIRRORED_REPEAT                0x8370
#define GL_CLAMP_TO_EDGE                  0x812F

inline filament::TextureSampler::WrapMode getWrapMode(cgltf_int wrap) {
    switch (wrap) {
        case GL_REPEAT:
            return filament::TextureSampler::WrapMode::REPEAT;
        case GL_MIRRORED_REPEAT:
            return filament::TextureSampler::WrapMode::MIRRORED_REPEAT;
        case GL_CLAMP_TO_EDGE:
            return filament::TextureSampler::WrapMode::CLAMP_TO_EDGE;
    }
    return filament::TextureSampler::WrapMode::REPEAT;
}

inline filament::TextureSampler::MinFilter getMinFilter(cgltf_int minFilter) {
    switch (minFilter) {
        case GL_NEAREST:
            return filament::TextureSampler::MinFilter::NEAREST;
        case GL_LINEAR:
            return filament::TextureSampler::MinFilter::LINEAR;
        case GL_NEAREST_MIPMAP_NEAREST:
            return filament::TextureSampler::MinFilter::NEAREST_MIPMAP_NEAREST;
        case GL_LINEAR_MIPMAP_NEAREST:
            return filament::TextureSampler::MinFilter::LINEAR_MIPMAP_NEAREST;
        case GL_NEAREST_MIPMAP_LINEAR:
            return filament::TextureSampler::MinFilter::NEAREST_MIPMAP_LINEAR;
        case GL_LINEAR_MIPMAP_LINEAR:
            return filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR;
    }
    return filament::TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR;
}

inline filament::TextureSampler::MagFilter getMagFilter(cgltf_int magFilter) {
    switch (magFilter) {
        case GL_NEAREST:
            return filament::TextureSampler::MagFilter::NEAREST;
        case GL_LINEAR:
            return filament::TextureSampler::MagFilter::LINEAR;
    }
    return filament::TextureSampler::MagFilter::LINEAR;
}

inline bool getVertexAttrType(cgltf_attribute_type atype, filament::VertexAttribute* attrType) {
    switch (atype) {
        case cgltf_attribute_type_position:
            *attrType = filament::VertexAttribute::POSITION;
            return true;
        case cgltf_attribute_type_texcoord:
            *attrType = filament::VertexAttribute::UV0;
            return true;
        case cgltf_attribute_type_color:
            *attrType = filament::VertexAttribute::COLOR;
            return true;
        case cgltf_attribute_type_joints:
            *attrType = filament::VertexAttribute::BONE_INDICES;
            return true;
        case cgltf_attribute_type_weights:
            *attrType = filament::VertexAttribute::BONE_WEIGHTS;
            return true;
        case cgltf_attribute_type_normal:
        case cgltf_attribute_type_tangent:
        default:
            return false;
    }
}

inline bool getIndexType(cgltf_component_type ctype, filament::IndexBuffer::IndexType* itype) {
    switch (ctype) {
        case cgltf_component_type_r_8u:
        case cgltf_component_type_r_16u:
            *itype = filament::IndexBuffer::IndexType::USHORT;
            return true;
        case cgltf_component_type_r_32u:
            *itype = filament::IndexBuffer::IndexType::UINT;
            return true;
        default:
            break;
    }
    return false;
}

inline bool getPrimitiveType(cgltf_primitive_type in,
        filament::RenderableManager::PrimitiveType* out) {
    switch (in) {
        case cgltf_primitive_type_points:
            *out = filament::RenderableManager::PrimitiveType::POINTS;
            return true;
        case cgltf_primitive_type_lines:
            *out = filament::RenderableManager::PrimitiveType::LINES;
            return true;
        case cgltf_primitive_type_line_strip:
            *out = filament::RenderableManager::PrimitiveType::LINE_STRIP;
            return true;
        case cgltf_primitive_type_triangles:
            *out = filament::RenderableManager::PrimitiveType::TRIANGLES;
            return true;
        case cgltf_primitive_type_triangle_strip:
            *out = filament::RenderableManager::PrimitiveType::TRIANGLE_STRIP;
            return true;
        case cgltf_primitive_type_line_loop:
        case cgltf_primitive_type_triangle_fan:
        case cgltf_primitive_type_max_enum:
            return false;
    }
    return false;
}

// This converts a cgltf component type into a Filament Attribute type.
//
// This function has two out parameters. One result is a safe "permitted type" which we know is
// universally accepted across GPU's and backends, but may require conversion (see Transcoder). The
// other result is the "actual type" which requires no conversion.
//
// Returns false if the given component type is invalid.
inline bool getElementType(cgltf_type type, cgltf_component_type ctype,
        filament::VertexBuffer::AttributeType* permitType,
        filament::VertexBuffer::AttributeType* actualType) {
    switch (type) {
	    case cgltf_type_scalar:
            switch (ctype) {
                case cgltf_component_type_r_8:
                    *permitType = filament::VertexBuffer::AttributeType::BYTE;
                    *actualType = filament::VertexBuffer::AttributeType::BYTE;
                    return true;
                case cgltf_component_type_r_8u:
                    *permitType = filament::VertexBuffer::AttributeType::UBYTE;
                    *actualType = filament::VertexBuffer::AttributeType::UBYTE;
                    return true;
                case cgltf_component_type_r_16:
                    *permitType = filament::VertexBuffer::AttributeType::SHORT;
                    *actualType = filament::VertexBuffer::AttributeType::SHORT;
                    return true;
                case cgltf_component_type_r_16u:
                    *permitType = filament::VertexBuffer::AttributeType::USHORT;
                    *actualType = filament::VertexBuffer::AttributeType::USHORT;
                    return true;
                case cgltf_component_type_r_32u:
                    *permitType = filament::VertexBuffer::AttributeType::UINT;
                    *actualType = filament::VertexBuffer::AttributeType::UINT;
                    return true;
                case cgltf_component_type_r_32f:
                    *permitType = filament::VertexBuffer::AttributeType::FLOAT;
                    *actualType = filament::VertexBuffer::AttributeType::FLOAT;
                    return true;
                default:
                    return false;
            }
            break;
	    case cgltf_type_vec2:
            switch (ctype) {
                case cgltf_component_type_r_8:
                    *permitType = filament::VertexBuffer::AttributeType::BYTE2;
                    *actualType = filament::VertexBuffer::AttributeType::BYTE2;
                    return true;
                case cgltf_component_type_r_8u:
                    *permitType = filament::VertexBuffer::AttributeType::UBYTE2;
                    *actualType = filament::VertexBuffer::AttributeType::UBYTE2;
                    return true;
                case cgltf_component_type_r_16:
                    *permitType = filament::VertexBuffer::AttributeType::SHORT2;
                    *actualType = filament::VertexBuffer::AttributeType::SHORT2;
                    return true;
                case cgltf_component_type_r_16u:
                    *permitType = filament::VertexBuffer::AttributeType::USHORT2;
                    *actualType = filament::VertexBuffer::AttributeType::USHORT2;
                    return true;
                case cgltf_component_type_r_32f:
                    *permitType = filament::VertexBuffer::AttributeType::FLOAT2;
                    *actualType = filament::VertexBuffer::AttributeType::FLOAT2;
                    return true;
                default:
                    return false;
            }
            break;
	    case cgltf_type_vec3:
            switch (ctype) {
                case cgltf_component_type_r_8:
                    *permitType = filament::VertexBuffer::AttributeType::FLOAT3;
                    *actualType = filament::VertexBuffer::AttributeType::BYTE3;
                    return true;
                case cgltf_component_type_r_8u:
                    *permitType = filament::VertexBuffer::AttributeType::FLOAT3;
                    *actualType = filament::VertexBuffer::AttributeType::UBYTE3;
                    return true;
                case cgltf_component_type_r_16:
                    *permitType = filament::VertexBuffer::AttributeType::SHORT3;
                    *actualType = filament::VertexBuffer::AttributeType::SHORT3;
                    return true;
                case cgltf_component_type_r_16u:
                    *permitType = filament::VertexBuffer::AttributeType::FLOAT3;
                    *actualType = filament::VertexBuffer::AttributeType::USHORT3;
                    return true;
                case cgltf_component_type_r_32f:
                    *permitType = filament::VertexBuffer::AttributeType::FLOAT3;
                    *actualType = filament::VertexBuffer::AttributeType::FLOAT3;
                    return true;
                default:
                    return false;
            }
            break;
	    case cgltf_type_vec4:
            switch (ctype) {
                case cgltf_component_type_r_8:
                    *permitType = filament::VertexBuffer::AttributeType::BYTE4;
                    *actualType = filament::VertexBuffer::AttributeType::BYTE4;
                    return true;
                case cgltf_component_type_r_8u:
                    *permitType = filament::VertexBuffer::AttributeType::UBYTE4;
                    *actualType = filament::VertexBuffer::AttributeType::UBYTE4;
                    return true;
                case cgltf_component_type_r_16:
                    *permitType = filament::VertexBuffer::AttributeType::SHORT4;
                    *actualType = filament::VertexBuffer::AttributeType::SHORT4;
                    return true;
                case cgltf_component_type_r_16u:
                    *permitType = filament::VertexBuffer::AttributeType::USHORT4;
                    *actualType = filament::VertexBuffer::AttributeType::USHORT4;
                    return true;
                case cgltf_component_type_r_32f:
                    *permitType = filament::VertexBuffer::AttributeType::FLOAT4;
                    *actualType = filament::VertexBuffer::AttributeType::FLOAT4;
                    return true;
                default:
                    return false;
            }
            break;
        default:
            return false;
    }
    return false;
}

#endif // GLTFIO_GLTFENUMS_H
