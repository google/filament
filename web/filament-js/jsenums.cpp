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

#include <filament/BufferObject.h>
#include <filament/Camera.h>
#include <filament/ColorGrading.h>
#include <filament/Color.h>
#include <filament/Frustum.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/RenderTarget.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <ktxreader/Ktx2Reader.h>

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace filament;

EMSCRIPTEN_BINDINGS(jsenums) {

enum_<RgbType>("RgbType")
    .value("sRGB", RgbType::sRGB)
    .value("LINEAR", RgbType::LINEAR);

enum_<RgbaType>("RgbaType")
    .value("sRGB", RgbaType::sRGB)
    .value("LINEAR", RgbaType::LINEAR)
    .value("PREMULTIPLIED_sRGB", RgbaType::PREMULTIPLIED_sRGB)
    .value("PREMULTIPLIED_LINEAR", RgbaType::PREMULTIPLIED_LINEAR);

enum_<VertexAttribute>("VertexAttribute")
    .value("POSITION", POSITION)
    .value("TANGENTS", TANGENTS)
    .value("COLOR", COLOR)
    .value("UV0", UV0)
    .value("UV1", UV1)
    .value("BONE_INDICES", BONE_INDICES)
    .value("BONE_WEIGHTS", BONE_WEIGHTS)
    .value("CUSTOM0", CUSTOM0)
    .value("CUSTOM1", CUSTOM1)
    .value("CUSTOM2", CUSTOM2)
    .value("CUSTOM3", CUSTOM3)
    .value("CUSTOM4", CUSTOM4)
    .value("CUSTOM5", CUSTOM5)
    .value("CUSTOM6", CUSTOM6)
    .value("CUSTOM7", CUSTOM7)
    .value("MORPH_POSITION_0", MORPH_POSITION_0)
    .value("MORPH_POSITION_1", MORPH_POSITION_1)
    .value("MORPH_POSITION_2", MORPH_POSITION_2)
    .value("MORPH_POSITION_3", MORPH_POSITION_3)
    .value("MORPH_TANGENTS_0", MORPH_TANGENTS_0)
    .value("MORPH_TANGENTS_1", MORPH_TANGENTS_1)
    .value("MORPH_TANGENTS_2", MORPH_TANGENTS_2)
    .value("MORPH_TANGENTS_3", MORPH_TANGENTS_3);

enum_<BufferObject::BindingType>("BufferObject$BindingType")
    .value("VERTEX", BufferObject::BindingType::VERTEX);

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

enum_<LightManager::Type>("LightManager$Type")
    .value("SUN", LightManager::Type::SUN)
    .value("DIRECTIONAL", LightManager::Type::DIRECTIONAL)
    .value("POINT", LightManager::Type::POINT)
    .value("FOCUSED_SPOT", LightManager::Type::FOCUSED_SPOT)
    .value("SPOT", LightManager::Type::SPOT);

enum_<RenderableManager::PrimitiveType>("RenderableManager$PrimitiveType")
    .value("POINTS", RenderableManager::PrimitiveType::POINTS)
    .value("LINES", RenderableManager::PrimitiveType::LINES)
    .value("LINE_STRIP", RenderableManager::PrimitiveType::LINE_STRIP)
    .value("TRIANGLES", RenderableManager::PrimitiveType::TRIANGLES)
    .value("TRIANGLE_STRIP", RenderableManager::PrimitiveType::TRIANGLE_STRIP);

enum_<View::AmbientOcclusion>("View$AmbientOcclusion")
    .value("NONE", View::AmbientOcclusion::NONE)
    .value("SSAO", View::AmbientOcclusion::SSAO);

enum_<Camera::Fov>("Camera$Fov")
    .value("VERTICAL", Camera::Fov::VERTICAL)
    .value("HORIZONTAL", Camera::Fov::HORIZONTAL);

enum_<Camera::Projection>("Camera$Projection")
    .value("PERSPECTIVE", Camera::Projection::PERSPECTIVE)
    .value("ORTHO", Camera::Projection::ORTHO);

enum_<ColorGrading::QualityLevel>("ColorGrading$QualityLevel")
    .value("LOW", ColorGrading::QualityLevel::LOW)
    .value("MEDIUM", ColorGrading::QualityLevel::MEDIUM)
    .value("HIGH", ColorGrading::QualityLevel::HIGH)
    .value("ULTRA", ColorGrading::QualityLevel::ULTRA);

enum_<ColorGrading::ToneMapping>("ColorGrading$ToneMapping")
    .value("LINEAR", ColorGrading::ToneMapping::LINEAR)
    .value("ACES_LEGACY", ColorGrading::ToneMapping::ACES_LEGACY)
    .value("ACES", ColorGrading::ToneMapping::ACES)
    .value("FILMIC", ColorGrading::ToneMapping::FILMIC)
    .value("DISPLAY_RANGE", ColorGrading::ToneMapping::DISPLAY_RANGE);

enum_<ColorGrading::LutFormat>("ColorGrading$LutFormat")
    .value("INTEGER", ColorGrading::LutFormat::INTEGER)
    .value("FLOAT", ColorGrading::LutFormat::FLOAT);

enum_<Frustum::Plane>("Frustum$Plane")
    .value("LEFT", Frustum::Plane::LEFT)
    .value("RIGHT", Frustum::Plane::RIGHT)
    .value("BOTTOM", Frustum::Plane::BOTTOM)
    .value("TOP", Frustum::Plane::TOP)
    .value("FAR", Frustum::Plane::FAR)
    .value("NEAR", Frustum::Plane::NEAR);

enum_<Texture::Sampler>("Texture$Sampler") // aka backend::SamplerType
    .value("SAMPLER_2D", Texture::Sampler::SAMPLER_2D)
    .value("SAMPLER_CUBEMAP", Texture::Sampler::SAMPLER_CUBEMAP)
    .value("SAMPLER_EXTERNAL", Texture::Sampler::SAMPLER_EXTERNAL);

enum_<Texture::InternalFormat>("Texture$InternalFormat") // aka backend::TextureFormat
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
    .value("RGBA32I", Texture::InternalFormat::RGBA32I)
    .value("EAC_R11", Texture::InternalFormat::EAC_R11)
    .value("EAC_R11_SIGNED", Texture::InternalFormat::EAC_R11_SIGNED)
    .value("EAC_RG11", Texture::InternalFormat::EAC_RG11)
    .value("EAC_RG11_SIGNED", Texture::InternalFormat::EAC_RG11_SIGNED)
    .value("ETC2_RGB8", Texture::InternalFormat::ETC2_RGB8)
    .value("ETC2_SRGB8", Texture::InternalFormat::ETC2_SRGB8)
    .value("ETC2_RGB8_A1", Texture::InternalFormat::ETC2_RGB8_A1)
    .value("ETC2_SRGB8_A1", Texture::InternalFormat::ETC2_SRGB8_A1)
    .value("ETC2_EAC_RGBA8", Texture::InternalFormat::ETC2_EAC_RGBA8)
    .value("ETC2_EAC_SRGBA8", Texture::InternalFormat::ETC2_EAC_SRGBA8)
    .value("DXT1_RGB", Texture::InternalFormat::DXT1_RGB)
    .value("DXT1_RGBA", Texture::InternalFormat::DXT1_RGBA)
    .value("DXT3_RGBA", Texture::InternalFormat::DXT3_RGBA)
    .value("DXT5_RGBA", Texture::InternalFormat::DXT5_RGBA)
    .value("DXT1_SRGB", Texture::InternalFormat::DXT1_SRGB)
    .value("DXT1_SRGBA", Texture::InternalFormat::DXT1_SRGBA)
    .value("DXT3_SRGBA", Texture::InternalFormat::DXT3_SRGBA)
    .value("DXT5_SRGBA", Texture::InternalFormat::DXT5_SRGBA)
    .value("RGBA_ASTC_4x4", Texture::InternalFormat::RGBA_ASTC_4x4)
    .value("RGBA_ASTC_5x4", Texture::InternalFormat::RGBA_ASTC_5x4)
    .value("RGBA_ASTC_5x5", Texture::InternalFormat::RGBA_ASTC_5x5)
    .value("RGBA_ASTC_6x5", Texture::InternalFormat::RGBA_ASTC_6x5)
    .value("RGBA_ASTC_6x6", Texture::InternalFormat::RGBA_ASTC_6x6)
    .value("RGBA_ASTC_8x5", Texture::InternalFormat::RGBA_ASTC_8x5)
    .value("RGBA_ASTC_8x6", Texture::InternalFormat::RGBA_ASTC_8x6)
    .value("RGBA_ASTC_8x8", Texture::InternalFormat::RGBA_ASTC_8x8)
    .value("RGBA_ASTC_10x5", Texture::InternalFormat::RGBA_ASTC_10x5)
    .value("RGBA_ASTC_10x6", Texture::InternalFormat::RGBA_ASTC_10x6)
    .value("RGBA_ASTC_10x8", Texture::InternalFormat::RGBA_ASTC_10x8)
    .value("RGBA_ASTC_10x10", Texture::InternalFormat::RGBA_ASTC_10x10)
    .value("RGBA_ASTC_12x10", Texture::InternalFormat::RGBA_ASTC_12x10)
    .value("RGBA_ASTC_12x12", Texture::InternalFormat::RGBA_ASTC_12x12)
    .value("SRGB8_ALPHA8_ASTC_4x4", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_4x4)
    .value("SRGB8_ALPHA8_ASTC_5x4", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_5x4)
    .value("SRGB8_ALPHA8_ASTC_5x5", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_5x5)
    .value("SRGB8_ALPHA8_ASTC_6x5", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_6x5)
    .value("SRGB8_ALPHA8_ASTC_6x6", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_6x6)
    .value("SRGB8_ALPHA8_ASTC_8x5", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_8x5)
    .value("SRGB8_ALPHA8_ASTC_8x6", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_8x6)
    .value("SRGB8_ALPHA8_ASTC_8x8", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_8x8)
    .value("SRGB8_ALPHA8_ASTC_10x5", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_10x5)
    .value("SRGB8_ALPHA8_ASTC_10x6", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_10x6)
    .value("SRGB8_ALPHA8_ASTC_10x8", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_10x8)
    .value("SRGB8_ALPHA8_ASTC_10x10", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_10x10)
    .value("SRGB8_ALPHA8_ASTC_12x10", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_12x10)
    .value("SRGB8_ALPHA8_ASTC_12x12", Texture::InternalFormat::SRGB8_ALPHA8_ASTC_12x12);

enum_<Texture::Usage>("Texture$Usage") // aka backend::TextureUsage
    .value("DEFAULT", Texture::Usage::DEFAULT)
    .value("COLOR_ATTACHMENT", Texture::Usage::COLOR_ATTACHMENT)
    .value("DEPTH_ATTACHMENT", Texture::Usage::DEPTH_ATTACHMENT)
    .value("STENCIL_ATTACHMENT", Texture::Usage::STENCIL_ATTACHMENT)
    .value("UPLOADABLE", Texture::Usage::UPLOADABLE)
    .value("SAMPLEABLE", Texture::Usage::SAMPLEABLE)
    .value("BLIT_SRC", Texture::Usage::BLIT_SRC)
    .value("BLIT_DST", Texture::Usage::BLIT_DST)
    .value("SUBPASS_INPUT", Texture::Usage::SUBPASS_INPUT);

enum_<Texture::CubemapFace>("Texture$CubemapFace") // aka backend::TextureCubemapFace
    .value("POSITIVE_X", Texture::CubemapFace::POSITIVE_X)
    .value("NEGATIVE_X", Texture::CubemapFace::NEGATIVE_X)
    .value("POSITIVE_Y", Texture::CubemapFace::POSITIVE_Y)
    .value("NEGATIVE_Y", Texture::CubemapFace::NEGATIVE_Y)
    .value("POSITIVE_Z", Texture::CubemapFace::POSITIVE_Z)
    .value("NEGATIVE_Z", Texture::CubemapFace::NEGATIVE_Z);

enum_<RenderTarget::AttachmentPoint>("RenderTarget$AttachmentPoint")
    .value("COLOR0", RenderTarget::AttachmentPoint::COLOR0)
    .value("COLOR1", RenderTarget::AttachmentPoint::COLOR1)
    .value("COLOR2", RenderTarget::AttachmentPoint::COLOR2)
    .value("COLOR3", RenderTarget::AttachmentPoint::COLOR3)
    .value("COLOR", RenderTarget::AttachmentPoint::COLOR)
    .value("DEPTH", RenderTarget::AttachmentPoint::DEPTH);

enum_<backend::PixelDataFormat>("PixelDataFormat")
    .value("R", backend::PixelDataFormat::R)
    .value("R_INTEGER", backend::PixelDataFormat::R_INTEGER)
    .value("RG", backend::PixelDataFormat::RG)
    .value("RG_INTEGER", backend::PixelDataFormat::RG_INTEGER)
    .value("RGB", backend::PixelDataFormat::RGB)
    .value("RGB_INTEGER", backend::PixelDataFormat::RGB_INTEGER)
    .value("RGBA", backend::PixelDataFormat::RGBA)
    .value("RGBA_INTEGER", backend::PixelDataFormat::RGBA_INTEGER)
    .value("DEPTH_COMPONENT", backend::PixelDataFormat::DEPTH_COMPONENT)
    .value("DEPTH_STENCIL", backend::PixelDataFormat::DEPTH_STENCIL)
    .value("ALPHA", backend::PixelDataFormat::ALPHA);

enum_<backend::PixelDataType>("PixelDataType")
    .value("UBYTE", backend::PixelDataType::UBYTE)
    .value("BYTE", backend::PixelDataType::BYTE)
    .value("USHORT", backend::PixelDataType::USHORT)
    .value("SHORT", backend::PixelDataType::SHORT)
    .value("UINT", backend::PixelDataType::UINT)
    .value("INT", backend::PixelDataType::INT)
    .value("HALF", backend::PixelDataType::HALF)
    .value("FLOAT", backend::PixelDataType::FLOAT)
    .value("UINT_10F_11F_11F_REV", backend::PixelDataType::UINT_10F_11F_11F_REV)
    .value("USHORT_565", backend::PixelDataType::USHORT_565);

enum_<backend::CompressedPixelDataType>("CompressedPixelDataType")
    .value("EAC_R11", backend::CompressedPixelDataType::EAC_R11)
    .value("EAC_R11_SIGNED", backend::CompressedPixelDataType::EAC_R11_SIGNED)
    .value("EAC_RG11", backend::CompressedPixelDataType::EAC_RG11)
    .value("EAC_RG11_SIGNED", backend::CompressedPixelDataType::EAC_RG11_SIGNED)
    .value("ETC2_RGB8", backend::CompressedPixelDataType::ETC2_RGB8)
    .value("ETC2_SRGB8", backend::CompressedPixelDataType::ETC2_SRGB8)
    .value("ETC2_RGB8_A1", backend::CompressedPixelDataType::ETC2_RGB8_A1)
    .value("ETC2_SRGB8_A1", backend::CompressedPixelDataType::ETC2_SRGB8_A1)
    .value("ETC2_EAC_RGBA8", backend::CompressedPixelDataType::ETC2_EAC_RGBA8)
    .value("ETC2_EAC_SRGBA8", backend::CompressedPixelDataType::ETC2_EAC_SRGBA8)
    .value("DXT1_RGB", backend::CompressedPixelDataType::DXT1_RGB)
    .value("DXT1_RGBA", backend::CompressedPixelDataType::DXT1_RGBA)
    .value("DXT3_RGBA", backend::CompressedPixelDataType::DXT3_RGBA)
    .value("DXT5_RGBA", backend::CompressedPixelDataType::DXT5_RGBA)
    .value("DXT1_SRGB", backend::CompressedPixelDataType::DXT1_SRGB)
    .value("DXT1_SRGBA", backend::CompressedPixelDataType::DXT1_SRGBA)
    .value("DXT3_SRGBA", backend::CompressedPixelDataType::DXT3_SRGBA)
    .value("DXT5_SRGBA", backend::CompressedPixelDataType::DXT5_SRGBA)
    .value("RGBA_ASTC_4x4", backend::CompressedPixelDataType::RGBA_ASTC_4x4)
    .value("RGBA_ASTC_5x4", backend::CompressedPixelDataType::RGBA_ASTC_5x4)
    .value("RGBA_ASTC_5x5", backend::CompressedPixelDataType::RGBA_ASTC_5x5)
    .value("RGBA_ASTC_6x5", backend::CompressedPixelDataType::RGBA_ASTC_6x5)
    .value("RGBA_ASTC_6x6", backend::CompressedPixelDataType::RGBA_ASTC_6x6)
    .value("RGBA_ASTC_8x5", backend::CompressedPixelDataType::RGBA_ASTC_8x5)
    .value("RGBA_ASTC_8x6", backend::CompressedPixelDataType::RGBA_ASTC_8x6)
    .value("RGBA_ASTC_8x8", backend::CompressedPixelDataType::RGBA_ASTC_8x8)
    .value("RGBA_ASTC_10x5", backend::CompressedPixelDataType::RGBA_ASTC_10x5)
    .value("RGBA_ASTC_10x6", backend::CompressedPixelDataType::RGBA_ASTC_10x6)
    .value("RGBA_ASTC_10x8", backend::CompressedPixelDataType::RGBA_ASTC_10x8)
    .value("RGBA_ASTC_10x10", backend::CompressedPixelDataType::RGBA_ASTC_10x10)
    .value("RGBA_ASTC_12x10", backend::CompressedPixelDataType::RGBA_ASTC_12x10)
    .value("RGBA_ASTC_12x12", backend::CompressedPixelDataType::RGBA_ASTC_12x12)
    .value("SRGB8_ALPHA8_ASTC_4x4", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_4x4)
    .value("SRGB8_ALPHA8_ASTC_5x4", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_5x4)
    .value("SRGB8_ALPHA8_ASTC_5x5", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_5x5)
    .value("SRGB8_ALPHA8_ASTC_6x5", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_6x5)
    .value("SRGB8_ALPHA8_ASTC_6x6", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_6x6)
    .value("SRGB8_ALPHA8_ASTC_8x5", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_8x5)
    .value("SRGB8_ALPHA8_ASTC_8x6", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_8x6)
    .value("SRGB8_ALPHA8_ASTC_8x8", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_8x8)
    .value("SRGB8_ALPHA8_ASTC_10x5", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x5)
    .value("SRGB8_ALPHA8_ASTC_10x6", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x6)
    .value("SRGB8_ALPHA8_ASTC_10x8", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x8)
    .value("SRGB8_ALPHA8_ASTC_10x10", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_10x10)
    .value("SRGB8_ALPHA8_ASTC_12x10", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_12x10)
    .value("SRGB8_ALPHA8_ASTC_12x12", backend::CompressedPixelDataType::SRGB8_ALPHA8_ASTC_12x12);

enum_<backend::SamplerWrapMode>("WrapMode")
    .value("CLAMP_TO_EDGE", backend::SamplerWrapMode::CLAMP_TO_EDGE)
    .value("REPEAT", backend::SamplerWrapMode::REPEAT)
    .value("MIRRORED_REPEAT", backend::SamplerWrapMode::MIRRORED_REPEAT);

enum_<backend::SamplerMinFilter>("MinFilter")
    .value("NEAREST", backend::SamplerMinFilter::NEAREST)
    .value("LINEAR", backend::SamplerMinFilter::LINEAR)
    .value("NEAREST_MIPMAP_NEAREST", backend::SamplerMinFilter::NEAREST_MIPMAP_NEAREST)
    .value("LINEAR_MIPMAP_NEAREST", backend::SamplerMinFilter::LINEAR_MIPMAP_NEAREST)
    .value("NEAREST_MIPMAP_LINEAR", backend::SamplerMinFilter::NEAREST_MIPMAP_LINEAR)
    .value("LINEAR_MIPMAP_LINEAR", backend::SamplerMinFilter::LINEAR_MIPMAP_LINEAR);

enum_<TextureSampler::CompareMode>("CompareMode")
    .value("NONE", TextureSampler::CompareMode::NONE)
    .value("COMPARE_TO_TEXTURE", TextureSampler::CompareMode::COMPARE_TO_TEXTURE);

enum_<TextureSampler::CompareFunc>("CompareFunc")
    .value("LESS_EQUAL", TextureSampler::CompareFunc::LE)
    .value("GREATER_EQUAL", TextureSampler::CompareFunc::GE)
    .value("LESS", TextureSampler::CompareFunc::L)
    .value("GREATER", TextureSampler::CompareFunc::G)
    .value("EQUAL", TextureSampler::CompareFunc::E)
    .value("NOT_EQUAL", TextureSampler::CompareFunc::NE)
    .value("ALWAYS", TextureSampler::CompareFunc::A)
    .value("NEVER", TextureSampler::CompareFunc::N);

enum_<backend::SamplerMagFilter>("MagFilter")
    .value("NEAREST", backend::SamplerMagFilter::NEAREST)
    .value("LINEAR", backend::SamplerMagFilter::LINEAR);

enum_<backend::CullingMode>("CullingMode")
    .value("NONE", backend::CullingMode::NONE)
    .value("FRONT", backend::CullingMode::FRONT)
    .value("BACK", backend::CullingMode::BACK)
    .value("FRONT_AND_BACK", backend::CullingMode::FRONT_AND_BACK);

enum_<filament::TransparencyMode>("TransparencyMode")
    .value("DEFAULT", filament::TransparencyMode::DEFAULT)
    .value("TWO_PASSES_ONE_SIDE", filament::TransparencyMode::TWO_PASSES_ONE_SIDE)
    .value("TWO_PASSES_TWO_SIDES", filament::TransparencyMode::TWO_PASSES_TWO_SIDES);

enum_<backend::FeatureLevel>("FeatureLevel")
    .value("FEATURE_LEVEL_1", backend::FeatureLevel::FEATURE_LEVEL_1)
    .value("FEATURE_LEVEL_2", backend::FeatureLevel::FEATURE_LEVEL_2);

enum_<backend::StencilOperation>("StencilOperation")
    .value("KEEP", backend::StencilOperation::KEEP)
    .value("ZERO", backend::StencilOperation::ZERO)
    .value("REPLACE", backend::StencilOperation::REPLACE)
    .value("INCR_CLAMP", backend::StencilOperation::INCR)
    .value("INCR_WRAP", backend::StencilOperation::INCR_WRAP)
    .value("DECR_CLAMP", backend::StencilOperation::DECR)
    .value("DECR_WRAP", backend::StencilOperation::DECR_WRAP)
    .value("INVERT", backend::StencilOperation::INVERT);

enum_<backend::StencilFace>("StencilFace")
    .value("FRONT", backend::StencilFace::FRONT)
    .value("BACK", backend::StencilFace::BACK)
    .value("FRONT_AND_BACK", backend::StencilFace::FRONT_AND_BACK);

enum_<ktxreader::Ktx2Reader::TransferFunction>("Ktx2Reader$TransferFunction")
    .value("LINEAR", ktxreader::Ktx2Reader::TransferFunction::LINEAR)
    .value("sRGB", ktxreader::Ktx2Reader::TransferFunction::sRGB);

enum_<ktxreader::Ktx2Reader::Result>("Ktx2Reader$Result")
    .value("SUCCESS", ktxreader::Ktx2Reader::Result::SUCCESS)
    .value("COMPRESSED_TRANSCODE_FAILURE", ktxreader::Ktx2Reader::Result::COMPRESSED_TRANSCODE_FAILURE)
    .value("UNCOMPRESSED_TRANSCODE_FAILURE", ktxreader::Ktx2Reader::Result::UNCOMPRESSED_TRANSCODE_FAILURE)
    .value("FORMAT_UNSUPPORTED", ktxreader::Ktx2Reader::Result::FORMAT_UNSUPPORTED)
    .value("FORMAT_ALREADY_REQUESTED", ktxreader::Ktx2Reader::Result::FORMAT_ALREADY_REQUESTED);

}
