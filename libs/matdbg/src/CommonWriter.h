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

#include <filaflat/BlobDictionary.h>
#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>
#include <filaflat/ShaderBuilder.h>
#include <filaflat/Unflattener.h>

#include <filament/MaterialChunkType.h>
#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <matdbg/JsonWriter.h>
#include <matdbg/ShaderInfo.h>

namespace filament {
namespace matdbg {

template<typename T>
static bool read(const filaflat::ChunkContainer& container, filamat::ChunkType type,
        T* value) noexcept {
    if (!container.hasChunk(type)) {
        return false;
    }

    filaflat::Unflattener unflattener(container.getChunkStart(type), container.getChunkEnd(type));
    return unflattener.read(value);
}


template<typename T>
static const char* toString(T value);

template<>
const char* toString(Shading shadingModel) {
    switch (shadingModel) {
        case Shading::UNLIT: return "unlit";
        case Shading::LIT: return "lit";
        case Shading::SUBSURFACE: return "subsurface";
        case Shading::CLOTH: return "cloth";
        case Shading::SPECULAR_GLOSSINESS: return "specularGlossiness";
    }
}

template<>
const char* toString(BlendingMode blendingMode) {
    switch (blendingMode) {
        case BlendingMode::OPAQUE: return "opaque";
        case BlendingMode::TRANSPARENT: return "transparent";
        case BlendingMode::ADD: return "add";
        case BlendingMode::MASKED: return "masked";
        case BlendingMode::FADE: return "fade";
        case BlendingMode::MULTIPLY: return "multiply";
        case BlendingMode::SCREEN: return "screen";
    }
}

template<>
const char* toString(Interpolation interpolation) {
    switch (interpolation) {
        case Interpolation::SMOOTH: return "smooth";
        case Interpolation::FLAT: return "flat";
    }
}

template<>
const char* toString(VertexDomain domain) {
    switch (domain) {
        case VertexDomain::OBJECT: return "object";
        case VertexDomain::WORLD: return "world";
        case VertexDomain::VIEW: return "view";
        case VertexDomain::DEVICE: return "device";
    }
}

template<>
const char* toString(backend::CullingMode cullingMode) {
    switch (cullingMode) {
        case backend::CullingMode::NONE: return "none";
        case backend::CullingMode::FRONT: return "front";
        case backend::CullingMode::BACK: return "back";
        case backend::CullingMode::FRONT_AND_BACK: return "front & back";
    }
}

template<>
const char* toString(TransparencyMode transparencyMode) {
    switch (transparencyMode) {
        case TransparencyMode::DEFAULT: return "default";
        case TransparencyMode::TWO_PASSES_ONE_SIDE: return "two passes, one side";
        case TransparencyMode::TWO_PASSES_TWO_SIDES: return "two passes, two sides";
    }
}

template<>
const char* toString(VertexAttribute attribute) {
    switch (attribute) {
        case VertexAttribute::POSITION: return "position";
        case VertexAttribute::TANGENTS: return "tangents";
        case VertexAttribute::COLOR: return "color";
        case VertexAttribute::UV0: return "uv0";
        case VertexAttribute::UV1: return "uv1";
        case VertexAttribute::BONE_INDICES: return "bone indices";
        case VertexAttribute::BONE_WEIGHTS: return "bone weights";
        case VertexAttribute::CUSTOM0: return "custom0";
        case VertexAttribute::CUSTOM1: return "custom1";
        case VertexAttribute::CUSTOM2: return "custom2";
        case VertexAttribute::CUSTOM3: return "custom3";
        case VertexAttribute::CUSTOM4: return "custom4";
        case VertexAttribute::CUSTOM5: return "custom5";
        case VertexAttribute::CUSTOM6: return "custom6";
        case VertexAttribute::CUSTOM7: return "custom7";
    }
    return "--";
}

template<>
const char* toString(bool value) {
    return value ? "true" : "false";
}

template<>
const char* toString(backend::ShaderType stage) {
    switch (stage) {
        case backend::ShaderType::VERTEX: return "vs";
        case backend::ShaderType::FRAGMENT: return "fs";
        default: break;
    }
    return "--";
}

template<>
const char* toString(backend::ShaderModel model) {
    switch (model) {
        case backend::ShaderModel::UNKNOWN: return "--";
        case backend::ShaderModel::GL_ES_30: return "gles30";
        case backend::ShaderModel::GL_CORE_41: return "gl41";
    }
}

template<>
const char* toString(backend::UniformType type) {
    switch (type) {
        case backend::UniformType::BOOL:   return "bool";
        case backend::UniformType::BOOL2:  return "bool2";
        case backend::UniformType::BOOL3:  return "bool3";
        case backend::UniformType::BOOL4:  return "bool4";
        case backend::UniformType::FLOAT:  return "float";
        case backend::UniformType::FLOAT2: return "float2";
        case backend::UniformType::FLOAT3: return "float3";
        case backend::UniformType::FLOAT4: return "float4";
        case backend::UniformType::INT:    return "int";
        case backend::UniformType::INT2:   return "int2";
        case backend::UniformType::INT3:   return "int3";
        case backend::UniformType::INT4:   return "int4";
        case backend::UniformType::UINT:   return "uint";
        case backend::UniformType::UINT2:  return "uint2";
        case backend::UniformType::UINT3:  return "uint3";
        case backend::UniformType::UINT4:  return "uint4";
        case backend::UniformType::MAT3:   return "float3x3";
        case backend::UniformType::MAT4:   return "float4x4";
    }
}

template<>
const char* toString(backend::SamplerType type) {
    switch (type) {
        case backend::SamplerType::SAMPLER_2D: return "sampler2D";
        case backend::SamplerType::SAMPLER_CUBEMAP: return "samplerCubemap";
        case backend::SamplerType::SAMPLER_EXTERNAL: return "samplerExternal";
    }
}

template<>
const char* toString(backend::Precision precision) {
    switch (precision) {
        case backend::Precision::LOW: return "lowp";
        case backend::Precision::MEDIUM: return "mediump";
        case backend::Precision::HIGH: return "highp";
        case backend::Precision::DEFAULT: return "default";
    }
}

template<>
const char* toString(backend::SamplerFormat format) {
    switch (format) {
        case backend::SamplerFormat::INT: return "int";
        case backend::SamplerFormat::UINT: return "uint";
        case backend::SamplerFormat::FLOAT: return "float";
        case backend::SamplerFormat::SHADOW: return "shadow";
    }
}

} // namespace matdbg
} // namespace filament
