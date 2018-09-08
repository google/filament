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

#ifndef TNT_CFILAMENT_API_H
#define TNT_CFILAMENT_API_H

#define CFILAMENT_SKIP_DECL

// Rewire the declarations to use the C++ variants directly
// For pointer types and enums this should not affect the ABI
#include <math/vec3.h>
#include <math/mat4.h>
#include <filament/Color.h>
#include <filament/Box.h>

using FMat4 = math::mat4;
using FMat4f = math::mat4f;
using FFloat3 = math::float3;
using FFloat4 = math::float4;
using FBox = filament::Box;
using FLinearColor = filament::LinearColor;
using FLinearColorA = filament::LinearColorA;

// We cannot use forward declarations for nested enums and classes, sadly.
#include <filament/driver/DriverEnums.h>
#include <filament/Camera.h>
#include <filament/Fence.h>
#include <filament/MaterialEnums.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/IndirectLight.h>
#include <filament/RenderableManager.h>
#include <filament/Skybox.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Stream.h>
#include <filament/Texture.h>

using FIndexBufferBuilder = filament::IndexBuffer::Builder;
using FLightManagerBuilder = filament::LightManager::Builder;
using FIndirectLightBuilder = filament::IndirectLight::Builder;
using FRenderableManagerBuilder = filament::RenderableManager::Builder;
using FRenderableManagerBone = filament::RenderableManager::Bone;
using FVertexBufferBuilder = filament::VertexBuffer::Builder;
using FSkyboxBuilder = filament::Skybox::Builder;
using FStreamBuilder = filament::Stream::Builder;
using FTextureBuilder = filament::Texture::Builder;

using FCamera = filament::Camera;
using FCameraProjection = filament::Camera::Projection;
using FCameraFov = filament::Camera::Fov;
using FFence = filament::Fence;
using FFenceType = filament::Fence::Type;
using FFenceStatus = filament::Fence::FenceStatus;
using FFenceMode = filament::Fence::Mode;

using FMaterialShading = filament::Shading;
using FMaterialInterpolation = filament::Interpolation;
using FMaterialBlendingMode = filament::BlendingMode;
using FMaterialVertexDomain = filament::VertexDomain;
using FMaterialCullingMode = filament::driver::CullingMode;
using FMaterialParameterType = filament::driver::UniformType;
using FMaterialSamplerType = filament::driver::SamplerType;
using FMaterialPrecision = filament::driver::Precision;
using FPixelDataFormat = filament::driver::PixelDataFormat;
using FPixelDataType = filament::driver::PixelDataType;
using FTextureFormat = filament::driver::TextureFormat;
using FSamplerType = filament::driver::SamplerType;
using FTextureUsage = filament::driver::TextureUsage;
using FTextureCubemapFace = filament::driver::TextureCubemapFace;

using FIndexBufferIndexType = filament::IndexBuffer::IndexType;

using FLightManagerType = filament::LightManager::Type;

using FPrimitiveType = filament::driver::PrimitiveType;
using FElementType = filament::driver::ElementType;
using FVertexAttribute = filament::VertexAttribute;
using FAntiAliasing = filament::View::AntiAliasing;
using FDepthPrepass = filament::View::DepthPrepass;

namespace filament {
    class Engine;

    class SwapChain;

    class View;

    class Renderer;

    class Scene;

    class Stream;

    class IndexBuffer;

    class VertexBuffer;

    class IndirectLight;

    class Material;

    class MaterialInstance;

    class Skybox;

    class Texture;

    class TransformManager;

    class LightManager;

    class RenderableManager;

    class Frustum;
}

using FBackend = filament::driver::Backend;
using FEngine = filament::Engine;
using FSwapChain = filament::SwapChain;
using FView = filament::View;
using FRenderer = filament::Renderer;
using FScene = filament::Scene;
using FStream = filament::Stream;
using FIndexBuffer = filament::IndexBuffer;
using FVertexBuffer = filament::VertexBuffer;
using FIndirectLight = filament::IndirectLight;
using FMaterial = filament::Material;
using FMaterialInstance = filament::MaterialInstance;
using FSkybox = filament::Skybox;
using FTexture = filament::Texture;
using FTransformManager = filament::TransformManager;
using FLightManager = filament::LightManager;
using FRenderableManager = filament::RenderableManager;
using FSamplerMagFilter = filament::driver::SamplerMagFilter;
using FSamplerMinFilter = filament::driver::SamplerMinFilter;
using FSamplerWrapMode = filament::driver::SamplerWrapMode;
using FSamplerCompareMode = filament::driver::SamplerCompareMode;
using FSamplerCompareFunc = filament::driver::SamplerCompareFunc;
using FTextureSampler = uint32_t;
using FFrustum = filament::Frustum;
using FBool = uint8_t;

using FEntity = uint32_t;
static_assert(sizeof(uint32_t) == sizeof(FEntity), "Size of Entity should be 32-bit");

#include <cfilament.h>

#include <utils/Entity.h>

inline utils::Entity convertEntity(FEntity entity) {
    return *reinterpret_cast<utils::Entity *>(&entity);
}

#endif // TNT_CFILAMENT_API_H
