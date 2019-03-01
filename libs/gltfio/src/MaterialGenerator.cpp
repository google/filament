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

#include "MaterialGenerator.h"

#include <filamat/MaterialBuilder.h>

#include <utils/Log.h>

#include <string>

using namespace filamat;
using namespace filament;
using namespace utils;

namespace gltfio {
namespace details {

bool MaterialGenerator::EqualFn::operator()(const MaterialKey& k1, const MaterialKey& k2) const {
    return
        (k1.doubleSided == k2.doubleSided) &&
        (k1.unlit == k2.unlit) &&
        (k1.hasVertexColors == k2.hasVertexColors) &&
        (k1.hasBaseColorTexture == k2.hasBaseColorTexture) &&
        (k1.hasMetallicRoughnessTexture == k2.hasMetallicRoughnessTexture) &&
        (k1.hasNormalTexture == k2.hasNormalTexture) &&
        (k1.hasOcclusionTexture == k2.hasOcclusionTexture) &&
        (k1.hasEmissiveTexture == k2.hasEmissiveTexture) &&
        (k1.alphaMode == k2.alphaMode) &&
        (k1.baseColorUV == k2.baseColorUV) &&
        (k1.metallicRoughnessUV == k2.metallicRoughnessUV) &&
        (k1.emissiveUV == k2.emissiveUV) &&
        (k1.aoUV == k2.aoUV) &&
        (k1.normalUV == k2.normalUV) &&
        (k1.alphaMaskThreshold == k2.alphaMaskThreshold);
}

MaterialGenerator::MaterialGenerator(Engine* engine) : mEngine(engine) {
    MaterialBuilder::init();
}

MaterialGenerator::~MaterialGenerator() {
    MaterialBuilder::shutdown();
}

size_t MaterialGenerator::getMaterialsCount() const noexcept {
    return mMaterials.size();
}

const Material* const* MaterialGenerator::getMaterials() const noexcept {
    return mMaterials.data();
}

void MaterialGenerator::destroyMaterials() {
    for (auto& iter : mCache) {
        mEngine->destroy(iter.second);
    }
    mMaterials.clear();
    mCache.clear();
}

static std::string shaderFromKey(const MaterialKey& config, const UvMap& uvmap) {
    const auto normalUV = std::to_string(uvmap[config.normalUV] - 1);
    const auto baseColorUV = std::to_string(uvmap[config.baseColorUV] - 1);
    const auto metallicRoughnessUV = std::to_string(uvmap[config.metallicRoughnessUV] - 1);
    const auto emissiveUV = std::to_string(uvmap[config.emissiveUV] - 1);
    const auto aoUV = std::to_string(uvmap[config.aoUV] - 1);

    std::string shader = "void material(inout MaterialInputs material) {\n";

    if (config.hasNormalTexture && !config.unlit) {
        shader += "float2 normalUV = getUV" + normalUV + "();\n";
        if (config.hasTextureTransforms) {
            shader += "normalUV = (vec3(normalUV, 1.0) * materialParams.normalUvMatrix).xy;\n";
        }
        shader += R"SHADER(
            material.normal = texture(materialParams_normalMap, normalUV).xyz * 2.0 - 1.0;
            material.normal.y = -material.normal.y;
            material.normal.xy *= materialParams.normalScale;
        )SHADER";
    }

    shader += R"SHADER(
        prepareMaterial(material);
        material.baseColor = materialParams.baseColorFactor;
    )SHADER";

    if (config.hasBaseColorTexture) {
        shader += "float2 baseColorUV = getUV" + baseColorUV + "();\n";
        if (config.hasTextureTransforms) {
            shader += "baseColorUV = (vec3(baseColorUV, 1.0) * materialParams.baseColorUvMatrix).xy;\n";
        }
        shader += R"SHADER(
            material.baseColor *= texture(materialParams_baseColorMap, baseColorUV);
        )SHADER";
    }

    if (config.alphaMode == AlphaMode::TRANSPARENT) {
        shader += R"SHADER(
            material.baseColor.rgb *= material.baseColor.a;
        )SHADER";
    }

    if (config.hasVertexColors) {
        shader += "material.baseColor *= getColor();\n";
    }

    if (!config.unlit) {
        shader += R"SHADER(
            material.roughness = materialParams.roughnessFactor;
            material.metallic = materialParams.metallicFactor;
            material.emissive.rgb = materialParams.emissiveFactor.rgb;
        )SHADER";
        if (config.hasMetallicRoughnessTexture) {
            shader += "float2 metallicRoughnessUV = getUV" + metallicRoughnessUV + "();\n";
            if (config.hasTextureTransforms) {
                shader += "metallicRoughnessUV = (vec3(metallicRoughnessUV, 1.0) * materialParams.metallicRoughnessUvMatrix).xy;\n";
            }
            shader += R"SHADER(
                vec4 roughness = texture(materialParams_metallicRoughnessMap, metallicRoughnessUV);
                material.roughness *= roughness.g;
                material.metallic *= roughness.b;
            )SHADER";
        }
        if (config.hasOcclusionTexture) {
            shader += "float2 aoUV = getUV" + aoUV + "();\n";
            if (config.hasTextureTransforms) {
                shader += "aoUV = (vec3(aoUV, 1.0) * materialParams.occlusionUvMatrix).xy;\n";
            }
            shader += R"SHADER(
                material.ambientOcclusion = texture(materialParams_occlusionMap, aoUV).r *
                        materialParams.aoStrength;
            )SHADER";
        }
        if (config.hasEmissiveTexture) {
            shader += "float2 emissiveUV = getUV" + emissiveUV + "();\n";
            if (config.hasTextureTransforms) {
                shader += "aoUV = (vec3(emissiveUV, 1.0) * materialParams.emissiveUvMatrix).xy;\n";
            }
            shader += R"SHADER(
                material.emissive.rgb *= texture(materialParams_emissiveMap, emissiveUV).rgb;
                material.emissive.a = 3.0;
            )SHADER";
        }
    }

    shader += "}\n";
    return shader;
}

// Filament supports up to 2 UV sets. glTF has arbitrary texcoord set indices, but it allows
// implementations to support only 2 simultaneous sets. Here we build a mapping table with 1-based
// indices where 0 means unused. Note that the order in which we drop textures can affect the look
// of certain assets. This "order of degradation" is stipulated by the glTF 2.0 specification.
static void constrainMaterial(MaterialKey* key, UvMap* uvmap) {
    const int MAX_INDEX = 2;
    UvMap retval {};
    int index = 1;
    if (key->hasBaseColorTexture) {
        retval[key->baseColorUV] = (UvSet) index++;
    }
    if (key->hasMetallicRoughnessTexture && retval[key->metallicRoughnessUV] == UNUSED) {
        retval[key->metallicRoughnessUV] = (UvSet) index++;
    }
    if (key->hasNormalTexture && retval[key->normalUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasNormalTexture = false;
        } else {
            retval[key->normalUV] = (UvSet) index++;
        }
    }
    if (key->hasOcclusionTexture && retval[key->aoUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasOcclusionTexture = false;
        } else {
            retval[key->aoUV] = (UvSet) index++;
        }
    }
    if (key->hasEmissiveTexture && retval[key->emissiveUV] == UNUSED) {
        if (index > MAX_INDEX) {
            key->hasEmissiveTexture = false;
        } else {
            retval[key->emissiveUV] = (UvSet) index++;
        }
    }
    *uvmap = retval;
}

static Material* createMaterial(Engine* engine, const MaterialKey& config, const UvMap& uvmap,
        const char* name) {
    using CullingMode = MaterialBuilder::CullingMode;
    std::string shader = shaderFromKey(config, uvmap);
    MaterialBuilder builder = MaterialBuilder()
            .name(name)
            .flipUV(true)
            .material(shader.c_str())
            .culling(config.doubleSided ? CullingMode::NONE : CullingMode::BACK)
            .doubleSided(config.doubleSided);

    auto uvset = (uint8_t*) &uvmap.front();
    static_assert(std::tuple_size<UvMap>::value == 8, "Badly sized uvset.");
    int numTextures = std::max({
        uvset[0], uvset[1], uvset[2], uvset[3],
        uvset[4], uvset[5], uvset[6], uvset[7],
    });
    if (numTextures > 0) {
        builder.require(VertexAttribute::UV0);
    }
    if (numTextures > 1) {
        builder.require(VertexAttribute::UV1);
    }

    // BASE COLOR
    builder.parameter(MaterialBuilder::UniformType::FLOAT4, "baseColorFactor");
    if (config.hasBaseColorTexture) {
        builder.parameter(MaterialBuilder::SamplerType::SAMPLER_2D, "baseColorMap");
        if (config.hasTextureTransforms) {
            builder.parameter(MaterialBuilder::UniformType::MAT3, "baseColorUvMatrix");
        }
    }
    if (config.hasVertexColors) {
        builder.require(VertexAttribute::COLOR);
    }

    // METALLIC-ROUGHNESS
    builder.parameter(MaterialBuilder::UniformType::FLOAT, "metallicFactor");
    builder.parameter(MaterialBuilder::UniformType::FLOAT, "roughnessFactor");
    if (config.hasMetallicRoughnessTexture) {
        builder.parameter(MaterialBuilder::SamplerType::SAMPLER_2D, "metallicRoughnessMap");
        if (config.hasTextureTransforms) {
            builder.parameter(MaterialBuilder::UniformType::MAT3, "metallicRoughnessUvMatrix");
        }
    }

    // NORMAL MAP
    // In the glTF spec normalScale is in normalTextureInfo; in cgltf it is part of texture_view.
    builder.parameter(MaterialBuilder::UniformType::FLOAT, "normalScale");
    if (config.hasNormalTexture) {
        builder.parameter(MaterialBuilder::SamplerType::SAMPLER_2D, "normalMap");
        if (config.hasTextureTransforms) {
            builder.parameter(MaterialBuilder::UniformType::MAT3, "normalUvMatrix");
        }
    }

    // AMBIENT OCCLUSION
    // In the glTF spec aoStrength is in occlusionTextureInfo; in cgltf it is part of texture_view.
    builder.parameter(MaterialBuilder::UniformType::FLOAT, "aoStrength");
    if (config.hasOcclusionTexture) {
        builder.parameter(MaterialBuilder::SamplerType::SAMPLER_2D, "occlusionMap");
        if (config.hasTextureTransforms) {
            builder.parameter(MaterialBuilder::UniformType::MAT3, "occlusionUvMatrix");
        }
    }

    // EMISSIVE
    builder.parameter(MaterialBuilder::UniformType::FLOAT3, "emissiveFactor");
    if (config.hasEmissiveTexture) {
        builder.parameter(MaterialBuilder::SamplerType::SAMPLER_2D, "emissiveMap");
        if (config.hasTextureTransforms) {
            builder.parameter(MaterialBuilder::UniformType::MAT3, "emissiveUvMatrix");
        }
    }

    switch(config.alphaMode) {
        case AlphaMode::MASKED:
            builder.blending(MaterialBuilder::BlendingMode::MASKED);
            builder.maskThreshold(config.alphaMaskThreshold);
            break;
        case AlphaMode::TRANSPARENT:
            builder.blending(MaterialBuilder::BlendingMode::TRANSPARENT);
            break;
        default:
            builder.blending(MaterialBuilder::BlendingMode::OPAQUE);
    }

    builder.shading(config.unlit ? Shading::UNLIT : Shading::LIT);

    Package pkg = builder.build();
    return Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
}

Material* MaterialGenerator::getOrCreateMaterial(MaterialKey* config, UvMap* uvmap,
        const char* label) {
    constrainMaterial(config, uvmap);
    auto iter = mCache.find(*config);
    if (iter == mCache.end()) {
        Material* mat = createMaterial(mEngine, *config, *uvmap, label);
        mCache.emplace(std::make_pair(*config, mat));
        mMaterials.push_back(mat);
        return mat;
    }
    return iter->second;
}

} // namespace details
} // namespace gltfio
