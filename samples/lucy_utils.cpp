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

#define FILTER_SIZE 17
#define SAMPLE_COUNT (1 + FILTER_SIZE / 2)

namespace LucyUtils {

constexpr float M_PIf = float(M_PI);

Entity createQuad(Engine* engine, Texture* tex, ImageOp op, Texture* secondary) {

    static VertexBuffer* vertexBuffer = [](Engine& engine) {
        struct OverlayVertex {
            float2 position;
            float2 uv;
        };
        static OverlayVertex verts[4] = {
            {{0, 0}, {0, 0} },
            {{1, 0}, {1, 0} },
            {{0, 1}, {0, 1} },
            {{1, 1}, {1, 1} }
        };
        auto vb = VertexBuffer::Builder()
            .vertexCount(4)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 16)
            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 16)
            .build(engine);
        vb->setBufferAt(engine, 0, VertexBuffer::BufferDescriptor(verts, sizeof(verts), nullptr));
        return vb;
    }(*engine);

    static IndexBuffer* indexBuffer = [](Engine& engine) {
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
                    vec2 uv = uvToRenderTargetUV(getUV0());
                    material.baseColor = texture(materialParams_color, uv);
                }
            )SHADER")
            .require(VertexAttribute::UV0)
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
            .targetApi(filamat::targetApiFromBackend(engine.getBackend()))
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
                    vec2 uv = uvToRenderTargetUV(getUV0());
                    vec4 primary = texture(materialParams_color, uv);
                    vec4 blurred = texture(materialParams_secondary, uv);
                    // HACK: this is a crude bloom effect
                    float L = max(0.0, max(blurred.r, max(blurred.g, blurred.b)) - 1.0);
                    material.baseColor = mix(primary, blurred, min(1.0, L));
                }
            )SHADER")
            .require(VertexAttribute::UV0)
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "secondary")
            .targetApi(filamat::targetApiFromBackend(engine.getBackend()))
            .shading(Shading::UNLIT)
            .depthWrite(false)
            .depthCulling(false)
            .build();
        return Material::Builder().package(pkg.getData(), pkg.getSize()).build(engine);
    }(*engine);

    // Compute the "weights" array, which is composed of two consective sequences of 4-tuples:
    //       [WEIGHT, OFFSET_X, OFFSET_Y, DONT_CARE]
    // The first sequence is for the horizontal pass, the second sequence is for the vertical pass.
    static const float4* weights = []() {
        static float4 weights[SAMPLE_COUNT * 2];
        float4* hweights = weights;
        float4* vweights = weights + SAMPLE_COUNT;
        const auto filter = [](float t) {
            t /= 2.0;
            if (t >= 1.0) return 0.0f;
            const float scale = 1.0f / std::sqrt(0.5f * M_PIf);
            return std::exp(-2.0f * t * t) * scale;
        };
        constexpr float pixelWidth = 2.0f / float(FILTER_SIZE);
        float sum = 0.0f;
        for (int s = 0; s < SAMPLE_COUNT; s++) {

            // Determine which two texels to sample from.
            int i, j;
            if (s < SAMPLE_COUNT / 2) {
                i = s * 2;
                j = i + 1;
            } else if (s == SAMPLE_COUNT / 2) {
                i = j = s * 2;
            } else {
                j = s * 2;
                i = j - 1;
            }

            // Determine the normalized (x,y) along the Gaussian curve for each of the two samples.
            float weighti = filter(std::abs(-1.0f + pixelWidth / 2.0f + pixelWidth * i));
            float weightj = filter(std::abs(-1.0f + pixelWidth / 2.0f + pixelWidth * j));
            float offseti = i - (FILTER_SIZE - 1) / 2;

            // Leverage hardware interpolation by sampling between the texel centers.
            // Nudge the left sample rightward by an amount proportional to its weight.
            const float offset = offseti + weightj / (weighti + weightj);
            const float weight = weighti + weightj;

            hweights[s].x = vweights[s].x = weight;
            hweights[s].y = vweights[s].z = offset;
            hweights[s].z = vweights[s].y = 0.0f;
            sum += weights[s].x;
        }
        for (int s = 0; s < SAMPLE_COUNT; s++) {
            hweights[s].x /= sum;
            vweights[s].x /= sum;
        }
        return weights;
    }();
    static const float4* hweights = weights;
    static const float4* vweights = weights + SAMPLE_COUNT;

    static Material* blur = [](Engine& engine) {
        std::string txt = R"SHADER(
            void material(inout MaterialInputs material) {
                prepareMaterial(material);
                float2 uv = uvToRenderTargetUV(getUV0());
                vec4 c = vec4(0);
                for (int i = 0; i < SAMPLE_COUNT; i++) {
                    float2 st = uv + materialParams.weights[i].yz * getResolution().zw;
                    c += texture(materialParams_color, st) * materialParams.weights[i].x;
                }
                material.baseColor = c;
            }
        )SHADER";
        const std::string from("SAMPLE_COUNT");
        const std::string to = std::to_string(SAMPLE_COUNT);
        size_t pos = txt.find(from);
        txt.replace(pos, from.length(), to.c_str());
        filamat::Package pkg = filamat::MaterialBuilder()
            .name("blur")
            .material(txt.c_str())
            .require(VertexAttribute::UV0)
            .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
            .parameter(filamat::MaterialBuilder::UniformType::FLOAT4, SAMPLE_COUNT, "weights")
            .targetApi(filamat::targetApiFromBackend(engine.getBackend()))
            .shading(Shading::UNLIT)
            .depthWrite(false)
            .depthCulling(false)
            .build();
        return Material::Builder().package(pkg.getData(), pkg.getSize()).build(engine);
    }(*engine);

    TextureSampler::MinFilter minFilter = TextureSampler::MinFilter::LINEAR;
    TextureSampler::MagFilter magFilter = TextureSampler::MagFilter::LINEAR;
    TextureSampler sampler(minFilter, magFilter);

    MaterialInstance* matInstance;
    switch (op) {
        case BLIT:
            matInstance = blit->createInstance();
            break;
        case HBLUR:
            matInstance = blur->createInstance();
            matInstance->setParameter("weights", hweights, SAMPLE_COUNT);
            break;
        case VBLUR:
            matInstance = blur->createInstance();
            matInstance->setParameter("weights", vweights, SAMPLE_COUNT);
            break;
        case MIX:
            matInstance = mix->createInstance();
            matInstance->setParameter("secondary", secondary, sampler);
            break;
    }
    matInstance->setParameter("color", tex, sampler);

    Entity entity = EntityManager::get().create();
    RenderableManager::Builder(1)
        .boundingBox({{ 0, 0, 0 }, { 9000, 9000, 9000 }})
        .material(0, matInstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer)
        .build(*engine, entity);

    return entity;
}

Entity createDisk(Engine* engine, Texture* reflection) {
    constexpr int nslices = 64;
    constexpr int nverts = nslices + 2;

    static float4 verts[nverts];
    int slice = 0;
    while (slice < nslices + 1) {
        float theta = 2.0f * M_PI * slice / nslices;
        verts[slice++] = float4 {
            std::cos(theta), std::sin(theta),
            0.5 + 0.5 * std::cos(theta), 0.5 + 0.5 * std::sin(theta)
        };
    }
    verts[slice] = {0.0f, 0.0f, 0.5f, 0.5f};

    static quath quats[nverts];
    static float3 normals[1] = { float3(0, 0, 1) };
    SurfaceOrientation::Builder().vertexCount(1).normals(normals).build().getQuats(quats, 1);
    for (int i = 1; i < nverts; i++) {
        quats[i] = quats[0];
    }

    static uint16_t indices[nslices * 3];
    for (int slice = 0, j = 0; slice < nslices; ++slice) {
        indices[j++] = nverts - 1;
        indices[j++] = slice;
        indices[j++] = (slice + 1) % (nslices + 1);
    }

    VertexBuffer* vbuffer = VertexBuffer::Builder()
        .vertexCount(nverts)
        .bufferCount(2)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 16)
        .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 16)
        .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::HALF4)
        .build(*engine);

    vbuffer->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(verts, sizeof(verts), nullptr));
    vbuffer->setBufferAt(*engine, 1, VertexBuffer::BufferDescriptor(quats, sizeof(quats), nullptr));

    IndexBuffer* ibuffer = IndexBuffer::Builder()
        .indexCount(nslices * 3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*engine);

    ibuffer->setBuffer(*engine, IndexBuffer::BufferDescriptor(indices, sizeof(indices), nullptr));

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
                vec3 reflection = texture(materialParams_reflection, uv).xyz;

                // HACK: blend the reflection with the baseColor.
                material.baseColor.rgb = mix(reflection, material.baseColor.rgb, 0.75);
            }
        )SHADER")
        .require(VertexAttribute::UV0)
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "normal")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "color")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "roughness")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "ao")
        .parameter(filamat::MaterialBuilder::SamplerType::SAMPLER_2D, "reflection")
        .targetApi(filamat::targetApiFromBackend(engine->getBackend()))
        .specularAntiAliasing(true)
        .shading(Shading::LIT)
        .build();

    Material* material = Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
    MaterialInstance* matInstance = material->createInstance();

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
    auto occlusion = createTexture1(RESOURCE_ARGS(BLUE_TILES_01_AO));

    TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
            TextureSampler::MagFilter::LINEAR);

    matInstance->setParameter("normal", normal, sampler);
    matInstance->setParameter("color", color, sampler);
    matInstance->setParameter("roughness", roughness, sampler);
    matInstance->setParameter("ao", occlusion, sampler);
    matInstance->setParameter("reflection", reflection,
            TextureSampler(TextureSampler::MagFilter::LINEAR));

    auto entity = EntityManager::get().create();
    RenderableManager::Builder(1)
        .boundingBox({{ -1, -1, 1 }, { 1, 1, 1 }})
        .material(0, matInstance)
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vbuffer, ibuffer)
        .culling(false)
        .receiveShadows(true)
        .castShadows(false)
        .build(*engine, entity);

    auto& tcm = engine->getTransformManager();
    tcm.create(entity);

    return entity;
}

mat4f fitIntoUnitCube(const Aabb& bounds) {
    auto minPoint = bounds.min;
    auto maxPoint = bounds.max;
    float maxExtent = 0;
    maxExtent = std::max(maxPoint.x - minPoint.x, maxPoint.y - minPoint.y);
    maxExtent = std::max(maxExtent, maxPoint.z - minPoint.z);
    float scaleFactor = 2.0f / maxExtent;
    float3 center = (minPoint + maxPoint) / 2.0f;
    return mat4f::scaling(float3(scaleFactor)) * mat4f::translation(-center);
}

} // namespace LucyUtils
