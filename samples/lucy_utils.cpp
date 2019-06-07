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

#include "lucy_utils.h"

#include <filamat/MaterialBuilder.h>

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/IndirectLight.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <geometry/SurfaceOrientation.h>

#include <stb_image.h>

using namespace filament;
using namespace filament::math;
using namespace utils;

using filament::geometry::SurfaceOrientation;

#define FILTER_SIZE 9

namespace LucyUtils {

constexpr float M_PIf = float(M_PI);

Entity createQuad(Engine* engine, Texture* tex, ImageOp op, Texture* secondary) {

    static VertexBuffer* vb = [](Engine& engine) {
        struct OverlayVertex {
            float2 position;
            float2 uv;
            quath tangent;
        };
        static OverlayVertex verts[4] = {
            {{0, 0}, {0, 0}, {0, 0, 0, 1}},
            {{1, 0}, {1, 0}, {0, 0, 0, 1}},
            {{0, 1}, {0, 1}, {0, 0, 0, 1}},
            {{1, 1}, {1, 1}, {0, 0, 0, 1}}
        };
        static float3 normals[4] = {
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
            {0, 0, 1},
        };
        SurfaceOrientation::Builder()
            .vertexCount(4)
            .normals(normals)
            .build()
            .getQuats(2 + (quath*) verts, 4, 24);
        auto vb = VertexBuffer::Builder()
            .vertexCount(4)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 24)
            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 24)
            .attribute(VertexAttribute::TANGENTS, 0, VertexBuffer::AttributeType::HALF4, 16, 24)
            .build(engine);
        vb->setBufferAt(engine, 0, VertexBuffer::BufferDescriptor(verts, sizeof(verts), nullptr));
        return vb;
    }(*engine);

    static IndexBuffer* ib = [](Engine& engine) {
        static constexpr uint16_t indices[6] = { 2, 1, 0, 1, 2, 3 };
        auto ib = IndexBuffer::Builder()
            .indexCount(6)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(engine);
        ib->setBuffer(engine, IndexBuffer::BufferDescriptor(indices, sizeof(indices), nullptr));
        return ib;
    }(*engine);

    static Material* blit = [](Engine& engine) {
        filamat::Package pkg = filamat::MaterialBuilder()
            .name("blit")
            .material(R"SHADER(
                void material(inout MaterialInputs material) {
                    prepareMaterial(material);
                    material.baseColor = texture(materialParams_color, getUV0());
                }
            )SHADER")
            .require(VertexAttribute::UV0)
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
            .targetApi(filamat::MaterialBuilder::TargetApi::OPENGL)
            .shading(Shading::UNLIT)
            .depthWrite(false)
            .depthCulling(false)
            .build();
        return Material::Builder().package(pkg.getData(), pkg.getSize()).build(engine);
    }(*engine);

    static Material* mix = [](Engine& engine) {
        filamat::Package pkg = filamat::MaterialBuilder()
            .name("mix")
            .material(R"SHADER(
                void material(inout MaterialInputs material) {
                    prepareMaterial(material);
                    vec4 primary = texture(materialParams_color, getUV0());
                    vec4 blurred = texture(materialParams_secondary, getUV0());
                    // HACK: this is a crude bloom effect
                    float L = max(0.0, max(blurred.r, max(blurred.g, blurred.b)) - 1.0);
                    material.baseColor = mix(primary, blurred, min(1.0, L));
                }
            )SHADER")
            .require(VertexAttribute::UV0)
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "secondary")
            .targetApi(filamat::MaterialBuilder::TargetApi::OPENGL)
            .shading(Shading::UNLIT)
            .depthWrite(false)
            .depthCulling(false)
            .build();
        return Material::Builder().package(pkg.getData(), pkg.getSize()).build(engine);
    }(*engine);

    static const float4* weights = []() {
        static float4 weights[FILTER_SIZE * 2];
        float4* hweights = weights;
        float4* vweights = weights + FILTER_SIZE;
        static const float radius = 2;
        const auto filter = [](float t) {
            t /= 2.0;
            if (t >= 1.0) return 0.0f;
            const float scale = 1.0f / std::sqrt(0.5f * M_PIf);
            return std::exp(-2.0f * t * t) * scale;
        };
        constexpr float pixelWidth = 2.0f / float(FILTER_SIZE);
        float sum = 0.0f;
        for (int i = 0; i < FILTER_SIZE; i++) {
            float x = -1.0f + pixelWidth / 2.0f + pixelWidth * i;
            hweights[i].x = vweights[i].x = filter(std::abs(x));
            hweights[i].y = vweights[i].z = radius * (i - (FILTER_SIZE - 1) / 2);
            hweights[i].z = vweights[i].y = 0.0f;
            sum += weights[i].x;
        }
        for (int i = 0; i < FILTER_SIZE; i++) {
            hweights[i].x /= sum;
            vweights[i].x /= sum;
        }
        return weights;
    }();
    static const float4* hweights = weights;
    static const float4* vweights = weights + FILTER_SIZE;

    static Material* blur = [](Engine& engine) {
        std::string txt = R"SHADER(
            void material(inout MaterialInputs material) {
                prepareMaterial(material);
                float2 uv = gl_FragCoord.xy;
                vec4 c = vec4(0);
                for (int i = 0; i < FILTER_SIZE; i++) {
                    float2 st = (uv + materialParams.weights[i].yz) * getResolution().zw;
                    c += texture(materialParams_color, st) * materialParams.weights[i].x;
                }
                material.baseColor = c;
            }
        )SHADER";
        const std::string from("FILTER_SIZE");
        const std::string to = std::to_string(FILTER_SIZE);
        size_t pos = txt.find(from);
        txt.replace(pos, from.length(), to.c_str());
        filamat::Package pkg = filamat::MaterialBuilder()
            .name("hblur")
            .material(txt.c_str())
            .require(VertexAttribute::UV0)
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
            .parameter(filamat::MaterialBuilder::UniformType::FLOAT4, FILTER_SIZE, "weights")
            .targetApi(filamat::MaterialBuilder::TargetApi::OPENGL)
            .shading(Shading::UNLIT)
            .depthWrite(false)
            .depthCulling(false)
            .build();
        return Material::Builder().package(pkg.getData(), pkg.getSize()).build(engine);
    }(*engine);

    TextureSampler::MinFilter minfilt;
    TextureSampler::MagFilter magfilt;
    MaterialInstance* matinstance;
    switch (op) {
        case BLIT:
            minfilt = TextureSampler::MinFilter::LINEAR;
            magfilt = TextureSampler::MagFilter::LINEAR;
            matinstance = blit->createInstance();
            break;
        case HBLUR:
            minfilt = TextureSampler::MinFilter::NEAREST;
            magfilt = TextureSampler::MagFilter::NEAREST;
            matinstance = blur->createInstance();
            matinstance->setParameter("weights", hweights, FILTER_SIZE);
            break;
        case VBLUR:
            minfilt = TextureSampler::MinFilter::NEAREST;
            magfilt = TextureSampler::MagFilter::NEAREST;
            matinstance = blur->createInstance();
            matinstance->setParameter("weights", vweights, FILTER_SIZE);
            break;
        case MIX:
            minfilt = TextureSampler::MinFilter::LINEAR;
            magfilt = TextureSampler::MagFilter::LINEAR;
            matinstance = mix->createInstance();
            matinstance->setParameter("secondary", secondary, TextureSampler(minfilt, magfilt));
            break;
    }
    matinstance->setParameter("color", tex, TextureSampler(minfilt, magfilt));

    Entity entity = EntityManager::get().create();
    RenderableManager::Builder(1)
        .boundingBox({{ 0, 0, 0 }, { 9000, 9000, 9000 }})
        .material(0, matinstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib)
        .build(*engine, entity);

    return entity;
}

Entity createDisk(Engine* engine, Texture* reflection) {
    constexpr int nslices = 64;
    constexpr int nverts = nslices + 2;

    static float4 verts[nverts];
    int i = 0;
    while (i < nslices + 1) {
        float theta = 2.0f * M_PI * i / nslices;
        verts[i++] = float4 {
            std::cos(theta), std::sin(theta),
            0.5 + 0.5 * std::cos(theta), 0.5 + 0.5 * std::sin(theta)
        };
    }
    verts[i] = {0.0f, 0.0f, 0.5f, 0.5f};

    static uint16_t indices[nslices * 3];
    for (int i = 0, j = 0; i < nslices; ++i) {
        indices[j++] = nverts - 1;
        indices[j++] = i;
        indices[j++] = (i + 1) % (nslices + 1);
    }

    VertexBuffer* vb = VertexBuffer::Builder()
        .vertexCount(nverts)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 16)
        .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 16)
        .build(*engine);

    vb->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(verts, sizeof(verts), nullptr));

    IndexBuffer* ib = IndexBuffer::Builder()
        .indexCount(nslices * 3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*engine);

    ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(indices, sizeof(indices), nullptr));

    filamat::Package pkg = filamat::MaterialBuilder()
        .name("podium")
        .material(R"SHADER(
            void material(inout MaterialInputs material) {
                vec3 n = texture(materialParams_normal, getUV0()).xyz * 2.0 - 1.0;

                // The blue tiles normal map is very harsh, so we soften the normal vector.
                material.normal = normalize(n + vec3(0, 0, 5));

                prepareMaterial(material);
                material.baseColor = texture(materialParams_color, getUV0());
                material.roughness = texture(materialParams_roughness, getUV0()).r;
                material.ambientOcclusion = texture(materialParams_ao, getUV0()).r;

                float2 uv = gl_FragCoord.xy * getResolution().zw; uv.y = 1.0 - uv.y;
                vec3 reflectance = texture(materialParams_reflection, uv).xyz;

                // HACK: blend the reflection with the baseColor.
                material.baseColor.rgb = mix(reflectance, material.baseColor.rgb, 0.75);
            }
        )SHADER")
        .require(VertexAttribute::UV0)
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "normal")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "roughness")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "ao")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "reflection")
        .targetApi(filamat::MaterialBuilder::TargetApi::OPENGL)
        .specularAntiAliasing(true)
        .shading(Shading::LIT)
        .build();

    Material* mat = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    MaterialInstance* matinstance = mat->createInstance();

    auto createTexture4 = [engine](const uint8_t* data, int size, bool srgb) {
        int width, height, nchan;
        auto texels = stbi_load_from_memory(data, size, &width, &height, &nchan, 4);
        Texture::PixelBufferDescriptor buffer(texels, size_t(width * height * 4),
                Texture::Format::RGBA, Texture::Type::UBYTE,
                (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
        auto tex = Texture::Builder()
            .width(uint32_t(width)).height(uint32_t(height)).levels(1)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(srgb ? Texture::InternalFormat::SRGB8_A8 : Texture::InternalFormat::RGBA8)
            .build(*engine);
        tex->setImage(*engine, 0, std::move(buffer));
        tex->generateMipmaps(*engine);
        return tex;
    };

    auto createTexture1 = [engine](const uint8_t* data, int size) {
        int width, height, nchan;
        auto texels = stbi_load_from_memory(data, size, &width, &height, &nchan, 1);
        Texture::PixelBufferDescriptor buffer(texels, size_t(width * height),
                Texture::Format::R, Texture::Type::UBYTE,
                (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);
        auto tex = Texture::Builder()
            .width(uint32_t(width)).height(uint32_t(height)).levels(1)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(Texture::InternalFormat::R8)
            .build(*engine);
        tex->setImage(*engine, 0, std::move(buffer));
        tex->generateMipmaps(*engine);
        return tex;
    };

    auto normal = createTexture4(RESOURCE_ARGS(BLUE_TILES_01_NORMAL), false);
    auto color = createTexture4(RESOURCE_ARGS(BLUE_TILES_01_COLOR), true);
    auto roughness = createTexture1(RESOURCE_ARGS(BLUE_TILES_01_ROUGHNESS));
    auto ao = createTexture1(RESOURCE_ARGS(BLUE_TILES_01_AO));

    TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
            TextureSampler::MagFilter::LINEAR);

    matinstance->setParameter("normal", normal, sampler);
    matinstance->setParameter("color", color, sampler);
    matinstance->setParameter("roughness", roughness, sampler);
    matinstance->setParameter("ao", ao, sampler);
    matinstance->setParameter("reflection", reflection,
            TextureSampler(TextureSampler::MagFilter::LINEAR));

    auto entity = EntityManager::get().create();
    RenderableManager::Builder(1)
        .boundingBox({{ -1, -1, 1 }, { 1, 1, 1 }})
        .material(0, matinstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib)
        .culling(false)
        .receiveShadows(true)
        .castShadows(false)
        .build(*engine, entity);

    auto& tcm = engine->getTransformManager();
    tcm.create(entity);

    return entity;
}

mat4f fitIntoUnitCube(const Aabb& bounds) {
    auto minpt = bounds.min;
    auto maxpt = bounds.max;
    float maxExtent = 0;
    maxExtent = std::max(maxpt.x - minpt.x, maxpt.y - minpt.y);
    maxExtent = std::max(maxExtent, maxpt.z - minpt.z);
    float scaleFactor = 2.0f / maxExtent;
    float3 center = (minpt + maxpt) / 2.0f;
    return mat4f::scaling(float3(scaleFactor)) * mat4f::translation(-center);
}

} // namespace LucyUtils
