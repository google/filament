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

#include <gltfio/MaterialProvider.h>

#include <filamat/MaterialBuilder.h>

#include <utils/Log.h>
#include <utils/Hash.h>

#include <tsl/robin_map.h>

#include <string>

using namespace filamat;
using namespace filament;
using namespace gltfio;
using namespace utils;

namespace {

class MaterialGenerator : public MaterialProvider {
public:
    explicit MaterialGenerator(filament::Engine* engine);
    ~MaterialGenerator() override;

    MaterialSource getSource() const noexcept override { return GENERATE_SHADERS; }

    filament::MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
            const char* label) override;

    size_t getMaterialsCount() const noexcept override;
    const filament::Material* const* getMaterials() const noexcept override;
    void destroyMaterials() override;

    using HashFn = utils::hash::MurmurHashFn<MaterialKey>;
    tsl::robin_map<MaterialKey, filament::Material*, HashFn> mCache;
    std::vector<filament::Material*> mMaterials;
    filament::Engine* mEngine;
};

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

std::string shaderFromKey(const MaterialKey& config) {
    std::string shader = "void material(inout MaterialInputs material) {\n";

    if (config.hasNormalTexture && !config.unlit) {
        shader += "float2 normalUV = ${normal};\n";
        if (config.hasTextureTransforms) {
            shader += "normalUV = (vec3(normalUV, 1.0) * materialParams.normalUvMatrix).xy;\n";
        }
        shader += R"SHADER(
            material.normal = texture(materialParams_normalMap, normalUV).xyz * 2.0 - 1.0;
            material.normal.xy *= materialParams.normalScale;
        )SHADER";
    }

    if (config.enableDiagnostics && !config.unlit) {
        shader += R"SHADER(
            if (materialParams.enableDiagnostics) {
                material.normal = vec3(0, 0, 1);
            }
        )SHADER";
    }

    shader += R"SHADER(
        prepareMaterial(material);
        material.baseColor = materialParams.baseColorFactor;
    )SHADER";

    if (config.hasBaseColorTexture) {
        shader += "float2 baseColorUV = ${color};\n";
        if (config.hasTextureTransforms) {
            shader += "baseColorUV = (vec3(baseColorUV, 1.0) * "
                    "materialParams.baseColorUvMatrix).xy;\n";
        }
        shader += R"SHADER(
            material.baseColor *= texture(materialParams_baseColorMap, baseColorUV);
        )SHADER";
    }

    if (config.enableDiagnostics) {
        shader += R"SHADER(
           #if defined(HAS_ATTRIBUTE_TANGENTS)
            if (materialParams.enableDiagnostics) {
                material.baseColor.rgb = vertex_worldNormal * 0.5 + 0.5;
            }
          #endif
        )SHADER";
    }

    if (config.alphaMode == AlphaMode::BLEND) {
        shader += R"SHADER(
            material.baseColor.rgb *= material.baseColor.a;
        )SHADER";
    }

    if (config.hasVertexColors) {
        shader += "material.baseColor *= getColor();\n";
    }

    if (!config.unlit) {
        if (config.useSpecularGlossiness) {
            shader += R"SHADER(
                material.glossiness = materialParams.glossinessFactor;
                material.specularColor = materialParams.specularFactor;
                material.emissive.rgb = materialParams.emissiveFactor.rgb;
            )SHADER";
        } else {
            shader += R"SHADER(
                material.roughness = materialParams.roughnessFactor;
                material.metallic = materialParams.metallicFactor;
                material.emissive.rgb = materialParams.emissiveFactor.rgb;
            )SHADER";
        }
        if (config.hasMetallicRoughnessTexture) {
            shader += "float2 metallicRoughnessUV = ${metallic};\n";
            if (config.hasTextureTransforms) {
                shader += "metallicRoughnessUV = (vec3(metallicRoughnessUV, 1.0) * "
                        "materialParams.metallicRoughnessUvMatrix).xy;\n";
            }
            if (config.useSpecularGlossiness) {
                shader += R"SHADER(
                    vec4 sg = texture(materialParams_metallicRoughnessMap, metallicRoughnessUV);
                    material.specularColor *= sg.rgb;
                    material.glossiness *= sg.a;
                )SHADER";
            } else {
                shader += R"SHADER(
                    vec4 mr = texture(materialParams_metallicRoughnessMap, metallicRoughnessUV);
                    material.roughness *= mr.g;
                    material.metallic *= mr.b;
                )SHADER";
            }
        }
        if (config.hasOcclusionTexture) {
            shader += "float2 aoUV = ${ao};\n";
            if (config.hasTextureTransforms) {
                shader += "aoUV = (vec3(aoUV, 1.0) * materialParams.occlusionUvMatrix).xy;\n";
            }
            shader += R"SHADER(
                material.ambientOcclusion = texture(materialParams_occlusionMap, aoUV).r *
                        materialParams.aoStrength;
            )SHADER";
        }
        if (config.hasEmissiveTexture) {
            shader += "float2 emissiveUV = ${emissive};\n";
            if (config.hasTextureTransforms) {
                shader += "emissiveUV = (vec3(emissiveUV, 1.0) * "
                        "materialParams.emissiveUvMatrix).xy;\n";
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

Material* createMaterial(Engine* engine, const MaterialKey& config, const UvMap& uvmap,
        const char* name) {
    std::string shader = shaderFromKey(config);
    gltfio::details::processShaderString(&shader, uvmap, config);
    MaterialBuilder builder = MaterialBuilder()
            .name(name)
            .flipUV(false)
            .material(shader.c_str())
            .doubleSided(config.doubleSided);

    switch (engine->getBackend()) {
        case Engine::Backend::OPENGL:
            builder.targetApi(MaterialBuilder::TargetApi::OPENGL);
            break;
        case Engine::Backend::VULKAN:
            builder.targetApi(MaterialBuilder::TargetApi::VULKAN);
            break;
        case Engine::Backend::METAL:
            builder.targetApi(MaterialBuilder::TargetApi::METAL);
            break;
        default:
            slog.e << "Unresolved backend, unable to use filamat." << io::endl;
            return nullptr;
    }

#ifndef NDEBUG
    builder.optimization(MaterialBuilder::Optimization::NONE);
#endif

    static_assert(std::tuple_size<UvMap>::value == 8, "Badly sized uvset.");
    int numUvSets = getNumUvSets(uvmap);
    if (numUvSets > 0) {
        builder.require(VertexAttribute::UV0);
    }
    if (numUvSets > 1) {
        builder.require(VertexAttribute::UV1);
    }

    if (config.enableDiagnostics) {
        builder.parameter(MaterialBuilder::UniformType::BOOL, "enableDiagnostics");
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

    // SPECULAR-GLOSSINESS
    if (config.useSpecularGlossiness) {
        builder.parameter(MaterialBuilder::UniformType::FLOAT, "glossinessFactor");
        builder.parameter(MaterialBuilder::UniformType::FLOAT3, "specularFactor");
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

    switch (config.alphaMode) {
        case AlphaMode::OPAQUE:
            builder.blending(MaterialBuilder::BlendingMode::OPAQUE);
            break;
        case AlphaMode::MASK:
            builder.blending(MaterialBuilder::BlendingMode::MASKED);
            break;
        case AlphaMode::BLEND:
            builder.blending(MaterialBuilder::BlendingMode::FADE);
            builder.depthWrite(true);
            break;
        default:
            // Ignore
            break;
    }

    if (config.unlit) {
        builder.shading(Shading::UNLIT);
    } else if (config.useSpecularGlossiness) {
        builder.shading(Shading::SPECULAR_GLOSSINESS);
    } else {
        builder.shading(Shading::LIT);
    }

    Package pkg = builder.build();
    return Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
}

MaterialInstance* MaterialGenerator::createMaterialInstance(MaterialKey* config, UvMap* uvmap,
        const char* label) {
    gltfio::details::constrainMaterial(config, uvmap);
    auto iter = mCache.find(*config);
    if (iter == mCache.end()) {
        Material* mat = createMaterial(mEngine, *config, *uvmap, label);
        mCache.emplace(std::make_pair(*config, mat));
        mMaterials.push_back(mat);
        return mat->createInstance();
    }
    return iter->second->createInstance();
}

} // anonymous namespace

namespace gltfio {

MaterialProvider* createMaterialGenerator(filament::Engine* engine) {
    return new MaterialGenerator(engine);
}

} // namespace gltfio
