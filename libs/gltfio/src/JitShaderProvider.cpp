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

#include <filament/MaterialEnums.h>
#include <filamat/MaterialBuilder.h>

#include <utils/Hash.h>

#include <tsl/robin_map.h>

#include <string>
#include <unordered_map>

using namespace filamat;
using namespace filament;
using namespace filament::gltfio;
using namespace utils;

namespace {

class JitShaderProvider : public MaterialProvider {
public:
    explicit JitShaderProvider(Engine* engine, bool optimizeShaders,
            utils::FixedCapacityVector<char const*> const& variantFilters);
    ~JitShaderProvider() override;

    MaterialInstance* createMaterialInstance(MaterialKey* config, UvMap* uvmap,
            const char* label, const char* extras) override;

    Material* getMaterial(MaterialKey* config, UvMap* uvmap, const char* label) override;

    size_t getMaterialsCount() const noexcept override;
    const Material* const* getMaterials() const noexcept override;
    void destroyMaterials() override;

    bool needsDummyData(VertexAttribute attrib) const noexcept override {
        return false;
    }

private:
    using HashFn = hash::MurmurHashFn<MaterialKey>;
    tsl::robin_map<MaterialKey, Material*, HashFn> mCache;
    std::vector<Material*> mMaterials;
    Engine* const mEngine;
    const bool mOptimizeShaders;
    filament::UserVariantFilterMask mVariantFilter{};
};

JitShaderProvider::JitShaderProvider(Engine* engine, bool optimizeShaders,
        utils::FixedCapacityVector<char const*> const& variantFilters)
    : mEngine(engine),
      mOptimizeShaders(optimizeShaders) {

    // Note that this is the same as the list in tools/matc/src/ParametersProcessor.cpp
    static const std::unordered_map<std::string, filament::UserVariantFilterBit> strToEnum  = [] {
        std::unordered_map<std::string, filament::UserVariantFilterBit> strToEnum;
        strToEnum["directionalLighting"]    = filament::UserVariantFilterBit::DIRECTIONAL_LIGHTING;
        strToEnum["dynamicLighting"]        = filament::UserVariantFilterBit::DYNAMIC_LIGHTING;
        strToEnum["shadowReceiver"]         = filament::UserVariantFilterBit::SHADOW_RECEIVER;
        strToEnum["skinning"]               = filament::UserVariantFilterBit::SKINNING;
        strToEnum["vsm"]                    = filament::UserVariantFilterBit::VSM;
        strToEnum["fog"]                    = filament::UserVariantFilterBit::FOG;
        strToEnum["ssr"]                    = filament::UserVariantFilterBit::SSR;
        strToEnum["stereo"]                 = filament::UserVariantFilterBit::STE;
        return strToEnum;
    }();

    for (auto& filterStr : variantFilters) {
        mVariantFilter |= (uint32_t)strToEnum.at(filterStr);
    }
    MaterialBuilder::init();
}

JitShaderProvider::~JitShaderProvider() {
    MaterialBuilder::shutdown();
}

size_t JitShaderProvider::getMaterialsCount() const noexcept {
    return mMaterials.size();
}

const Material* const* JitShaderProvider::getMaterials() const noexcept {
    return mMaterials.data();
}

void JitShaderProvider::destroyMaterials() {
    for (auto& iter : mCache) {
        mEngine->destroy(iter.second);
    }
    mMaterials.clear();
    mCache.clear();
}

std::string shaderFromKey(const MaterialKey& config) {
    std::string shader = "void material(inout MaterialInputs material) {\n";

    if (config.hasNormalTexture && !config.unlit) {
        shader += "highp float2 normalUV = ${normal};\n";
        if (config.hasTextureTransforms) {
            shader += "normalUV = (vec3(normalUV, 1.0) * materialParams.normalUvMatrix).xy;\n";
        }
        shader += R"SHADER(
            material.normal = texture(materialParams_normalMap, normalUV).xyz * 2.0 - 1.0;
            material.normal.xy *= materialParams.normalScale;
        )SHADER";
    }

    if (config.hasClearCoat && config.hasClearCoatNormalTexture && !config.unlit) {
        shader += "highp float2 clearCoatNormalUV = ${clearCoatNormal};\n";
        if (config.hasTextureTransforms) {
            shader += "clearCoatNormalUV = (vec3(clearCoatNormalUV, 1.0) * "
                    "materialParams.clearCoatNormalUvMatrix).xy;\n";
        }
        shader += R"SHADER(
            material.clearCoatNormal =
                texture(materialParams_clearCoatNormalMap, clearCoatNormalUV).xyz * 2.0 - 1.0;
            material.clearCoatNormal.xy *= materialParams.clearCoatNormalScale;
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
        shader += "highp float2 baseColorUV = ${color};\n";
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
                material.emissive = vec4(materialParams.emissiveStrength *
                    materialParams.emissiveFactor.rgb, 0.0);
            )SHADER";
        } else {
            shader += R"SHADER(
                material.roughness = materialParams.roughnessFactor;
                material.metallic = materialParams.metallicFactor;
                material.emissive = vec4(materialParams.emissiveStrength *
                    materialParams.emissiveFactor.rgb, 0.0);
            )SHADER";
        }
        if (config.hasMetallicRoughnessTexture) {
            shader += "highp float2 metallicRoughnessUV = ${metallic};\n";
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
            shader += "highp float2 aoUV = ${ao};\n";
            if (config.hasTextureTransforms) {
                shader += "aoUV = (vec3(aoUV, 1.0) * materialParams.occlusionUvMatrix).xy;\n";
            }
            shader += R"SHADER(
                float occlusion = texture(materialParams_occlusionMap, aoUV).r;
                material.ambientOcclusion = 1.0 + materialParams.aoStrength * (occlusion - 1.0);
            )SHADER";
        }
        if (config.hasEmissiveTexture) {
            shader += "highp float2 emissiveUV = ${emissive};\n";
            if (config.hasTextureTransforms) {
                shader += "emissiveUV = (vec3(emissiveUV, 1.0) * "
                        "materialParams.emissiveUvMatrix).xy;\n";
            }
            shader += R"SHADER(
                material.emissive.rgb *= texture(materialParams_emissiveMap, emissiveUV).rgb;
            )SHADER";
        }
        if (config.hasTransmission) {
            shader += R"SHADER(
                material.transmission = materialParams.transmissionFactor;
            )SHADER";
            if (config.hasTransmissionTexture) {
                shader += "highp float2 transmissionUV = ${transmission};\n";
                if (config.hasTextureTransforms) {
                    shader += "transmissionUV = (vec3(transmissionUV, 1.0) * "
                            "materialParams.transmissionUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.transmission *= texture(materialParams_transmissionMap, transmissionUV).r;
                )SHADER";
            }
        }
        if (config.hasClearCoat) {
            shader += R"SHADER(
                material.clearCoat = materialParams.clearCoatFactor;
                material.clearCoatRoughness = materialParams.clearCoatRoughnessFactor;
            )SHADER";

            if (config.hasClearCoatTexture) {
                shader += "highp float2 clearCoatUV = ${clearCoat};\n";
                if (config.hasTextureTransforms) {
                    shader += "clearCoatUV = (vec3(clearCoatUV, 1.0) * "
                            "materialParams.clearCoatUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.clearCoat *= texture(materialParams_clearCoatMap, clearCoatUV).r;
                )SHADER";
            }

            if (config.hasClearCoatRoughnessTexture) {
                shader += "highp float2 clearCoatRoughnessUV = ${clearCoatRoughness};\n";
                if (config.hasTextureTransforms) {
                    shader += "clearCoatRoughnessUV = (vec3(clearCoatRoughnessUV, 1.0) * "
                              "materialParams.clearCoatRoughnessUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.clearCoatRoughness *= texture(materialParams_clearCoatRoughnessMap, clearCoatRoughnessUV).g;
                )SHADER";
            }
        }

        if (config.hasSheen) {
            shader += R"SHADER(
                material.sheenColor = materialParams.sheenColorFactor;
                material.sheenRoughness = materialParams.sheenRoughnessFactor;
            )SHADER";

            if (config.hasSheenColorTexture) {
                shader += "highp float2 sheenColorUV = ${sheenColor};\n";
                if (config.hasTextureTransforms) {
                    shader += "sheenColorUV = (vec3(sheenColorUV, 1.0) * "
                              "materialParams.sheenColorUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.sheenColor *= texture(materialParams_sheenColorMap, sheenColorUV).rgb;
                )SHADER";
            }

            if (config.hasSheenRoughnessTexture) {
                shader += "highp float2 sheenRoughnessUV = ${sheenRoughness};\n";
                if (config.hasTextureTransforms) {
                    shader += "sheenRoughnessUV = (vec3(sheenRoughnessUV, 1.0) * "
                              "materialParams.sheenRoughnessUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.sheenRoughness *= texture(materialParams_sheenRoughnessMap, sheenRoughnessUV).a;
                )SHADER";
            }
        }

        if (config.hasVolume) {
            shader += R"SHADER(
                material.absorption = materialParams.volumeAbsorption;

                // TODO: Provided by Filament, but this should really be provided/computed by gltfio
                // TODO: This scale is per renderable and should include the scale of the mesh node
                float scale = getObjectUserData();
                material.thickness = materialParams.volumeThicknessFactor * scale;
            )SHADER";

            if (config.hasVolumeThicknessTexture) {
                shader += "highp float2 volumeThicknessUV = ${volumeThickness};\n";
                if (config.hasTextureTransforms) {
                    shader += "volumeThicknessUV = (vec3(volumeThicknessUV, 1.0) * "
                              "materialParams.volumeThicknessUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.thickness *= texture(materialParams_volumeThicknessMap, volumeThicknessUV).g;
                )SHADER";
            }
        }

        if (config.hasIOR) {
            shader += R"SHADER(
                material.ior = materialParams.ior;
            )SHADER";
        }

        if (config.hasSpecular) {
            shader += R"SHADER(
                material.specularFactor = materialParams.specularStrength;
                material.specularColorFactor = materialParams.specularColorFactor;
            )SHADER";

            if (config.hasSpecularTexture) {
                shader += "highp float2 specularUV = ${specular};\n";
                if (config.hasTextureTransforms) {
                    shader += "specularUV = (vec3(specularUV, 1.0) * "
                              "materialParams.specularUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.specularFactor *= texture(materialParams_specularMap, specularUV).a;
                )SHADER";
            }

            if (config.hasSpecularColorTexture) {
                shader += "highp float2 specularColorUV = ${specularColor};\n";
                if (config.hasTextureTransforms) {
                    shader += "specularColorUV = (vec3(specularColorUV, 1.0) * "
                              "materialParams.specularColorUvMatrix).xy;\n";
                }
                shader += R"SHADER(
                    material.specularColorFactor *= texture(materialParams_specularColorMap, specularColorUV).rgb;
                )SHADER";
            }
        }
    }

    shader += "}\n";
    return shader;
}

Material* createMaterial(Engine* engine, const MaterialKey& config, const UvMap& uvmap,
        const char* name, bool optimizeShaders, filament::UserVariantFilterMask variantFilter) {
    std::string shader = shaderFromKey(config);
    processShaderString(&shader, uvmap, config);
    MaterialBuilder builder;
    builder.name(name)
            .flipUV(false)
            .specularAmbientOcclusion(MaterialBuilder::SpecularAmbientOcclusion::SIMPLE)
            .specularAntiAliasing(true)
            .clearCoatIorChange(false)
            .material(shader.c_str())
            .doubleSided(config.doubleSided)
            .transparencyMode(config.doubleSided
                                      ? MaterialBuilder::TransparencyMode::TWO_PASSES_TWO_SIDES
                                      : MaterialBuilder::TransparencyMode::DEFAULT)
            .reflectionMode(MaterialBuilder::ReflectionMode::SCREEN_SPACE)
            .targetApi(filamat::targetApiFromBackend(engine->getBackend()))
            .stereoscopicType(engine->getConfig().stereoscopicType)
            .stereoscopicEyeCount(engine->getConfig().stereoscopicEyeCount)
            .variantFilter(variantFilter);

    if (!optimizeShaders) {
        builder.optimization(MaterialBuilder::Optimization::NONE);
        builder.generateDebugInfo(true);
    }

    static_assert(std::tuple_size<UvMap>::value == 8, "Badly sized uvset.");
    int numUvSets = getNumUvSets(uvmap);
    if (numUvSets > 0) {
        builder.require(VertexAttribute::UV0);
    }
    if (numUvSets > 1) {
        builder.require(VertexAttribute::UV1);
    }

    if (config.enableDiagnostics) {
        builder.parameter("enableDiagnostics", MaterialBuilder::UniformType::BOOL);
    }

    // BASE COLOR
    builder.parameter("baseColorFactor", MaterialBuilder::UniformType::FLOAT4);
    if (config.hasBaseColorTexture) {
        builder.parameter("baseColorMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        if (config.hasTextureTransforms) {
            builder.parameter("baseColorUvMatrix", MaterialBuilder::UniformType::MAT3,
                    MaterialBuilder::ParameterPrecision::HIGH);
        }
    }
    if (config.hasVertexColors) {
        builder.require(VertexAttribute::COLOR);
    }

    // METALLIC-ROUGHNESS
    builder.parameter("metallicFactor", MaterialBuilder::UniformType::FLOAT);
    builder.parameter("roughnessFactor", MaterialBuilder::UniformType::FLOAT);
    if (config.hasMetallicRoughnessTexture) {
        builder.parameter("metallicRoughnessMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        if (config.hasTextureTransforms) {
            builder.parameter("metallicRoughnessUvMatrix", MaterialBuilder::UniformType::MAT3,
                    MaterialBuilder::ParameterPrecision::HIGH);
        }
    }

    // SPECULAR-GLOSSINESS
    if (config.useSpecularGlossiness) {
        builder.parameter("glossinessFactor", MaterialBuilder::UniformType::FLOAT);
        builder.parameter("specularFactor", MaterialBuilder::UniformType::FLOAT3);
    }

    // NORMAL MAP
    // In the glTF spec normalScale is in normalTextureInfo; in cgltf it is part of texture_view.
    builder.parameter("normalScale", MaterialBuilder::UniformType::FLOAT);
    if (config.hasNormalTexture) {
        builder.parameter("normalMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        if (config.hasTextureTransforms) {
            builder.parameter("normalUvMatrix", MaterialBuilder::UniformType::MAT3,
                    MaterialBuilder::ParameterPrecision::HIGH);
        }
    }

    // AMBIENT OCCLUSION
    // In the glTF spec aoStrength is in occlusionTextureInfo; in cgltf it is part of texture_view.
    builder.parameter("aoStrength", MaterialBuilder::UniformType::FLOAT);
    if (config.hasOcclusionTexture) {
        builder.parameter("occlusionMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        if (config.hasTextureTransforms) {
            builder.parameter("occlusionUvMatrix", MaterialBuilder::UniformType::MAT3,
                    MaterialBuilder::ParameterPrecision::HIGH);
        }
    }

    // EMISSIVE
    builder.parameter("emissiveFactor", MaterialBuilder::UniformType::FLOAT3);
    builder.parameter("emissiveStrength", MaterialBuilder::UniformType::FLOAT);
    if (config.hasEmissiveTexture) {
        builder.parameter("emissiveMap", MaterialBuilder::SamplerType::SAMPLER_2D);
        if (config.hasTextureTransforms) {
            builder.parameter("emissiveUvMatrix", MaterialBuilder::UniformType::MAT3,
                    MaterialBuilder::ParameterPrecision::HIGH);
        }
    }

    // CLEAR COAT
    if (config.hasClearCoat) {
        builder.parameter("clearCoatFactor", MaterialBuilder::UniformType::FLOAT);
        builder.parameter("clearCoatRoughnessFactor", MaterialBuilder::UniformType::FLOAT);
        if (config.hasClearCoatTexture) {
            builder.parameter("clearCoatMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("clearCoatUvMatrix", MaterialBuilder::UniformType::MAT3,
                        MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
        if (config.hasClearCoatRoughnessTexture) {
            builder.parameter("clearCoatRoughnessMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("clearCoatRoughnessUvMatrix", MaterialBuilder::UniformType::MAT3,
                        MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
        if (config.hasClearCoatNormalTexture) {
            builder.parameter("clearCoatNormalScale", MaterialBuilder::UniformType::FLOAT);
            builder.parameter("clearCoatNormalMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("clearCoatNormalUvMatrix", MaterialBuilder::UniformType::MAT3,
                        MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
    }

    // SHEEN
    if (config.hasSheen) {
        builder.parameter("sheenColorFactor", MaterialBuilder::UniformType::FLOAT3);
        builder.parameter("sheenRoughnessFactor", MaterialBuilder::UniformType::FLOAT);
        if (config.hasSheenColorTexture) {
            builder.parameter("sheenColorMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("sheenColorUvMatrix", MaterialBuilder::UniformType::MAT3,
                        MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
        if (config.hasSheenRoughnessTexture) {
            builder.parameter("sheenRoughnessMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("sheenRoughnessUvMatrix", MaterialBuilder::UniformType::MAT3,
                        MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
    }

    // SPECULAR
    if (config.hasSpecular) {
        builder.parameter("specularStrength", MaterialBuilder::UniformType::FLOAT);
        builder.parameter("specularColorFactor", MaterialBuilder::UniformType::FLOAT3);
        if (config.hasSpecularTexture) {
            builder.parameter("specularMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("specularUvMatrix", MaterialBuilder::UniformType::MAT3,
                                  MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
        if (config.hasSpecularColorTexture) {
            builder.parameter("specularColorMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("specularColorUvMatrix", MaterialBuilder::UniformType::MAT3,
                                  MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
    }

    // BLENDING
    switch (config.alphaMode) {
        case AlphaMode::OPAQUE:
            builder.blending(MaterialBuilder::BlendingMode::OPAQUE);
            break;
        case AlphaMode::MASK:
            builder.blending(MaterialBuilder::BlendingMode::MASKED);
            break;
        case AlphaMode::BLEND:
            builder.blending(MaterialBuilder::BlendingMode::FADE);
            break;
        default:
            // Ignore
            break;
    }

    // TRANSMISSION
    if (config.hasTransmission) {
        // According to KHR_materials_transmission, the minimum expectation for a compliant renderer
        // is to at least render any opaque objects that lie behind transmitting objects.
        builder.refractionMode(RefractionMode::SCREEN_SPACE);

        // Thin refraction probably makes the most sense, given the language of the transmission
        // spec and its lack of an IOR parameter. This means that we would do a good job rendering a
        // window pane, but a poor job of rendering a glass full of liquid.
        builder.refractionType(RefractionType::THIN);

        builder.parameter("transmissionFactor", MaterialBuilder::UniformType::FLOAT);
        if (config.hasTransmissionTexture) {
            builder.parameter("transmissionMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("transmissionUvMatrix", MaterialBuilder::UniformType::MAT3,
                        MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
    }

    // VOLUME
    if (config.hasVolume) {
        builder.refractionMode(RefractionMode::SCREEN_SPACE);

        // Override thin transmission if both extensions are used
        builder.refractionType(RefractionType::SOLID);

        builder.parameter("volumeAbsorption", MaterialBuilder::UniformType::FLOAT3);
        builder.parameter("volumeThicknessFactor", MaterialBuilder::UniformType::FLOAT);

        if (config.hasVolumeThicknessTexture) {
            builder.parameter("volumeThicknessMap", MaterialBuilder::SamplerType::SAMPLER_2D);
            if (config.hasTextureTransforms) {
                builder.parameter("volumeThicknessUvMatrix", MaterialBuilder::UniformType::MAT3,
                        MaterialBuilder::ParameterPrecision::HIGH);
            }
        }
    }

    // IOR
    if (config.hasIOR) {
        builder.parameter("ior", MaterialBuilder::UniformType::FLOAT);
    }

    if (config.unlit) {
        builder.shading(Shading::UNLIT);
    } else if (config.useSpecularGlossiness) {
        builder.shading(Shading::SPECULAR_GLOSSINESS);
    } else {
        builder.shading(Shading::LIT);
    }

    Package pkg = builder.build(engine->getJobSystem());
    return Material::Builder().package(pkg.getData(), pkg.getSize()).build(*engine);
}

Material* JitShaderProvider::getMaterial(MaterialKey* config, UvMap* uvmap, const char* label) {
    constrainMaterial(config, uvmap);
    auto iter = mCache.find(*config);
    if (iter == mCache.end()) {
        bool optimizeShaders = mOptimizeShaders;
#ifndef NDEBUG
        optimizeShaders = false;
#endif

        Material* mat =
                createMaterial(mEngine, *config, *uvmap, label, optimizeShaders, mVariantFilter);
        mCache.emplace(std::make_pair(*config, mat));
        mMaterials.push_back(mat);
        return mat;
    }
    return iter.value();
}

MaterialInstance* JitShaderProvider::createMaterialInstance(MaterialKey* config, UvMap* uvmap,
        const char* label, const char* extras) {
    return getMaterial(config, uvmap, label)->createInstance(label);
}

} // anonymous namespace

namespace filament::gltfio {

MaterialProvider* createJitShaderProvider(Engine* engine, bool optimizeShaders,
        utils::FixedCapacityVector<char const*> const& variantFilters) {
    return new JitShaderProvider(engine, optimizeShaders, variantFilters);
}

} // namespace filament::gltfio
