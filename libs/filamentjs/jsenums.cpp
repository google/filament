/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <filament/Camera.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Texture.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace filament;

EMSCRIPTEN_BINDINGS(jsenums) {

enum_<VertexAttribute>("VertexAttribute")
    .value("POSITION", POSITION)
    .value("TANGENTS", TANGENTS)
    .value("COLOR", COLOR)
    .value("UV0", UV0)
    .value("UV1", UV1)
    .value("BONE_INDICES", BONE_INDICES)
    .value("BONE_WEIGHTS", BONE_WEIGHTS);

 enum_<VertexBuffer::AttributeType>("VertexBuffer$AttributeType")
    .value("BYTE", VertexBuffer::AttributeType::BYTE)
    .value("BYTE2", VertexBuffer::AttributeType::BYTE2)
    .value("BYTE3", VertexBuffer::AttributeType::BYTE3)
    .value("BYTE4", VertexBuffer::AttributeType::BYTE4)
    .value("UBYTE", VertexBuffer::AttributeType::UBYTE)
    .value("UBYTE2", VertexBuffer::AttributeType::UBYTE2)
    .value("UBYTE3", VertexBuffer::AttributeType::UBYTE3)
    .value("UBYTE4", VertexBuffer::AttributeType::UBYTE4)
    .value("SHORT", VertexBuffer::AttributeType::SHORT)
    .value("SHORT2", VertexBuffer::AttributeType::SHORT2)
    .value("SHORT3", VertexBuffer::AttributeType::SHORT3)
    .value("SHORT4", VertexBuffer::AttributeType::SHORT4)
    .value("USHORT", VertexBuffer::AttributeType::USHORT)
    .value("USHORT2", VertexBuffer::AttributeType::USHORT2)
    .value("USHORT3", VertexBuffer::AttributeType::USHORT3)
    .value("USHORT4", VertexBuffer::AttributeType::USHORT4)
    .value("INT", VertexBuffer::AttributeType::INT)
    .value("UINT", VertexBuffer::AttributeType::UINT)
    .value("FLOAT", VertexBuffer::AttributeType::FLOAT)
    .value("FLOAT2", VertexBuffer::AttributeType::FLOAT2)
    .value("FLOAT3", VertexBuffer::AttributeType::FLOAT3)
    .value("FLOAT4", VertexBuffer::AttributeType::FLOAT4)
    .value("HALF", VertexBuffer::AttributeType::HALF)
    .value("HALF2", VertexBuffer::AttributeType::HALF2)
    .value("HALF3", VertexBuffer::AttributeType::HALF3)
    .value("HALF4", VertexBuffer::AttributeType::HALF4);

 enum_<IndexBuffer::IndexType>("IndexBuffer$IndexType")
    .value("USHORT", IndexBuffer::IndexType::USHORT)
    .value("UINT", IndexBuffer::IndexType::UINT);

 enum_<RenderableManager::PrimitiveType>("RenderableManager$PrimitiveType")
    .value("POINTS", RenderableManager::PrimitiveType::POINTS)
    .value("LINES", RenderableManager::PrimitiveType::LINES)
    .value("TRIANGLES", RenderableManager::PrimitiveType::TRIANGLES)
    .value("NONE", RenderableManager::PrimitiveType::NONE);

 enum_<View::DepthPrepass>("View$DepthPrepass")
    .value("DEFAULT", View::DepthPrepass::DEFAULT)
    .value("DISABLED", View::DepthPrepass::DISABLED)
    .value("ENABLED", View::DepthPrepass::ENABLED);

 enum_<Camera::Projection>("Camera$Projection")
    .value("PERSPECTIVE", Camera::Projection::PERSPECTIVE)
    .value("ORTHO", Camera::Projection::ORTHO);

 enum_<Texture::Sampler>("Texture$Sampler") // aka driver::SamplerType
    .value("SAMPLER_2D", Texture::Sampler::SAMPLER_2D)
    .value("SAMPLER_CUBEMAP", Texture::Sampler::SAMPLER_CUBEMAP)
    .value("SAMPLER_EXTERNAL", Texture::Sampler::SAMPLER_EXTERNAL);

  enum_<Texture::InternalFormat>("Texture$InternalFormat") // aka driver::TextureFormat
    .value("R8", Texture::InternalFormat::R8)
    .value("R8_SNORM", Texture::InternalFormat::R8_SNORM)
    .value("R8UI", Texture::InternalFormat::R8UI)
    .value("R8I", Texture::InternalFormat::R8I)
    .value("STENCIL8", Texture::InternalFormat::STENCIL8)
    .value("R16F", Texture::InternalFormat::R16F)
    .value("R16UI", Texture::InternalFormat::R16UI)
    .value("R16I", Texture::InternalFormat::R16I)
    .value("RG8", Texture::InternalFormat::RG8)
    .value("RG8_SNORM", Texture::InternalFormat::RG8_SNORM)
    .value("RG8UI", Texture::InternalFormat::RG8UI)
    .value("RG8I", Texture::InternalFormat::RG8I)
    .value("RGB565", Texture::InternalFormat::RGB565)
    .value("RGB9_E5", Texture::InternalFormat::RGB9_E5)
    .value("RGB5_A1", Texture::InternalFormat::RGB5_A1)
    .value("RGBA4", Texture::InternalFormat::RGBA4)
    .value("DEPTH16", Texture::InternalFormat::DEPTH16)
    .value("RGB8", Texture::InternalFormat::RGB8)
    .value("SRGB8", Texture::InternalFormat::SRGB8)
    .value("RGB8_SNORM", Texture::InternalFormat::RGB8_SNORM)
    .value("RGB8UI", Texture::InternalFormat::RGB8UI)
    .value("RGB8I", Texture::InternalFormat::RGB8I)
    .value("DEPTH24", Texture::InternalFormat::DEPTH24)
    .value("R32F", Texture::InternalFormat::R32F)
    .value("R32UI", Texture::InternalFormat::R32UI)
    .value("R32I", Texture::InternalFormat::R32I)
    .value("RG16F", Texture::InternalFormat::RG16F)
    .value("RG16UI", Texture::InternalFormat::RG16UI)
    .value("RG16I", Texture::InternalFormat::RG16I)
    .value("R11F_G11F_B10F", Texture::InternalFormat::R11F_G11F_B10F)
    .value("RGBA8", Texture::InternalFormat::RGBA8)
    .value("SRGB8_A8", Texture::InternalFormat::SRGB8_A8)
    .value("RGBA8_SNORM", Texture::InternalFormat::RGBA8_SNORM)
    .value("UNUSED", Texture::InternalFormat::UNUSED)
    .value("RGB10_A2", Texture::InternalFormat::RGB10_A2)
    .value("RGBA8UI", Texture::InternalFormat::RGBA8UI)
    .value("RGBA8I", Texture::InternalFormat::RGBA8I)
    .value("DEPTH32F", Texture::InternalFormat::DEPTH32F)
    .value("DEPTH24_STENCIL8", Texture::InternalFormat::DEPTH24_STENCIL8)
    .value("DEPTH32F_STENCIL8", Texture::InternalFormat::DEPTH32F_STENCIL8)
    .value("RGB16F", Texture::InternalFormat::RGB16F)
    .value("RGB16UI", Texture::InternalFormat::RGB16UI)
    .value("RGB16I", Texture::InternalFormat::RGB16I)
    .value("RG32F", Texture::InternalFormat::RG32F)
    .value("RG32UI", Texture::InternalFormat::RG32UI)
    .value("RG32I", Texture::InternalFormat::RG32I)
    .value("RGBA16F", Texture::InternalFormat::RGBA16F)
    .value("RGBA16UI", Texture::InternalFormat::RGBA16UI)
    .value("RGBA16I", Texture::InternalFormat::RGBA16I)
    .value("RGB32F", Texture::InternalFormat::RGB32F)
    .value("RGB32UI", Texture::InternalFormat::RGB32UI)
    .value("RGB32I", Texture::InternalFormat::RGB32I)
    .value("RGBA32F", Texture::InternalFormat::RGBA32F)
    .value("RGBA32UI", Texture::InternalFormat::RGBA32UI)
    .value("RGBA32I", Texture::InternalFormat::RGBA32I);

  enum_<Texture::Usage>("Texture$Usage") // aka driver::TextureUsage
    .value("DEFAULT", Texture::Usage::DEFAULT)
    .value("COLOR_ATTACHMENT", Texture::Usage::COLOR_ATTACHMENT)
    .value("DEPTH_ATTACHMENT", Texture::Usage::DEPTH_ATTACHMENT);

  enum_<driver::PixelDataFormat>("PixelDataFormat")
    .value("R", driver::PixelDataFormat::R)
    .value("R_INTEGER", driver::PixelDataFormat::R_INTEGER)
    .value("RG", driver::PixelDataFormat::RG)
    .value("RG_INTEGER", driver::PixelDataFormat::RG_INTEGER)
    .value("RGB", driver::PixelDataFormat::RGB)
    .value("RGB_INTEGER", driver::PixelDataFormat::RGB_INTEGER)
    .value("RGBA", driver::PixelDataFormat::RGBA)
    .value("RGBA_INTEGER", driver::PixelDataFormat::RGBA_INTEGER)
    .value("RGBM", driver::PixelDataFormat::RGBM)
    .value("DEPTH_COMPONENT", driver::PixelDataFormat::DEPTH_COMPONENT)
    .value("DEPTH_STENCIL", driver::PixelDataFormat::DEPTH_STENCIL)
    .value("ALPHA", driver::PixelDataFormat::ALPHA);

  enum_<driver::PixelDataType>("PixelDataType")
    .value("UBYTE", driver::PixelDataType::UBYTE)
    .value("BYTE", driver::PixelDataType::BYTE)
    .value("USHORT", driver::PixelDataType::USHORT)
    .value("SHORT", driver::PixelDataType::SHORT)
    .value("UINT", driver::PixelDataType::UINT)
    .value("INT", driver::PixelDataType::INT)
    .value("HALF", driver::PixelDataType::HALF)
    .value("FLOAT", driver::PixelDataType::FLOAT);

enum_<driver::SamplerWrapMode>("WrapMode")
    .value("CLAMP_TO_EDGE", driver::SamplerWrapMode::CLAMP_TO_EDGE)
    .value("REPEAT", driver::SamplerWrapMode::REPEAT)
    .value("MIRRORED_REPEAT", driver::SamplerWrapMode::MIRRORED_REPEAT);

enum_<driver::SamplerMinFilter>("MinFilter")
    .value("NEAREST", driver::SamplerMinFilter::NEAREST)
    .value("LINEAR", driver::SamplerMinFilter::LINEAR)
    .value("NEAREST_MIPMAP_NEAREST", driver::SamplerMinFilter::NEAREST_MIPMAP_NEAREST)
    .value("LINEAR_MIPMAP_NEAREST", driver::SamplerMinFilter::LINEAR_MIPMAP_NEAREST)
    .value("NEAREST_MIPMAP_LINEAR", driver::SamplerMinFilter::NEAREST_MIPMAP_LINEAR)
    .value("LINEAR_MIPMAP_LINEAR", driver::SamplerMinFilter::LINEAR_MIPMAP_LINEAR);

enum_<driver::SamplerMagFilter>("MagFilter")
    .value("NEAREST", driver::SamplerMagFilter::NEAREST)
    .value("LINEAR", driver::SamplerMagFilter::LINEAR);

}
