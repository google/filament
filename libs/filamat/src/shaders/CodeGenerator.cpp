/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "CodeGenerator.h"

#include "generated/shaders.h"

#include <cctype>
#include <iomanip>

namespace filamat {

// From driverEnum namespace
using namespace filament;
using namespace backend;
using namespace utils;

io::sstream& CodeGenerator::generateSeparator(io::sstream& out) const {
    out << '\n';
    return out;
}

io::sstream& CodeGenerator::generateProlog(io::sstream& out, ShaderType type,
        bool hasExternalSamplers) const {
    assert(mShaderModel != ShaderModel::UNKNOWN);
    switch (mShaderModel) {
        case ShaderModel::UNKNOWN:
            break;
        case ShaderModel::GL_ES_30:
            // Vulkan requires version 310 or higher
            if (mTargetLanguage == TargetLanguage::SPIRV) {
                // Vulkan requires layout locations on ins and outs, which were not supported
                // in the OpenGL 4.1 GLSL profile.
                out << "#version 310 es\n\n";
            } else {
                out << "#version 300 es\n\n";
            }
            if (hasExternalSamplers) {
                out << "#extension GL_OES_EGL_image_external_essl3 : require\n\n";
            }
            out << "#define TARGET_MOBILE\n";
            break;
        case ShaderModel::GL_CORE_41:
            if (mTargetLanguage == TargetLanguage::SPIRV) {
                // Vulkan requires binding specifiers on uniforms and samplers, which were not
                // supported in the OpenGL 4.1 GLSL profile.
                out << "#version 450 core\n\n";
            } else {
                out << "#version 410 core\n\n";
            }
            break;
    }

    if (mTargetApi == TargetApi::VULKAN) {
        out << "#define TARGET_VULKAN_ENVIRONMENT\n";
    }
    if (mTargetApi == TargetApi::METAL) {
        out << "#define TARGET_METAL_ENVIRONMENT\n";
    }
    if (mTargetLanguage == TargetLanguage::SPIRV) {
        out << "#define TARGET_LANGUAGE_SPIRV\n";
    }

    Precision defaultPrecision = getDefaultPrecision(type);
    const char* precision = getPrecisionQualifier(defaultPrecision, Precision::DEFAULT);
    out << "precision " << precision << " float;\n";
    out << "precision " << precision << " int;\n";

    // The version of the Metal Shading Language (MSL) we use does not have the invariant qualifier.
    // New versions of MSL (> 2.1) have it, but we want to support older devices.
    if (type == ShaderType::VERTEX && mTargetApi != TargetApi::METAL) {
        out << "\n";
        out << "invariant gl_Position;\n";
    }

    out << SHADERS_COMMON_TYPES_FS_DATA;

    out << "\n";
    return out;
}

Precision CodeGenerator::getDefaultPrecision(ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        return Precision::HIGH;
    } else if (type == ShaderType::FRAGMENT) {
        if (mShaderModel < ShaderModel::GL_CORE_41) {
            return Precision::MEDIUM;
        } else {
            return Precision::HIGH;
        }
    }
    return Precision::HIGH;
}

Precision CodeGenerator::getDefaultUniformPrecision() const {
    if (mShaderModel < ShaderModel::GL_CORE_41) {
        return Precision::MEDIUM;
    } else {
        return Precision::HIGH;
    }
}

io::sstream& CodeGenerator::generateEpilog(io::sstream& out) const {
    out << "\n"; // For line compression all shaders finish with a newline character.
    return out;
}

io::sstream& CodeGenerator::generateShaderMain(io::sstream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        out << SHADERS_SHADOWING_VS_DATA;
        out << SHADERS_MAIN_VS_DATA;
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_MAIN_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generatePostProcessMainOld(io::sstream& out,
        ShaderType type, filament::PostProcessStage variant) const {
    if (type == ShaderType::VERTEX) {
        out << SHADERS_POST_PROCESS_OLD_VS_DATA;
    } else if (type == ShaderType::FRAGMENT) {
        switch (variant) {
            case PostProcessStage::TONE_MAPPING_OPAQUE:
            case PostProcessStage::TONE_MAPPING_TRANSLUCENT:
                out << SHADERS_TONE_MAPPING_FS_DATA;
                out << SHADERS_CONVERSION_FUNCTIONS_FS_DATA;
                out << SHADERS_DITHERING_FS_DATA;
                break;
            case PostProcessStage::ANTI_ALIASING_OPAQUE:
            case PostProcessStage::ANTI_ALIASING_TRANSLUCENT:
                out << SHADERS_FXAA_FS_DATA;
                break;
        }
        out << SHADERS_POST_PROCESS_OLD_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generatePostProcessMain(io::sstream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        out << SHADERS_POST_PROCESS_VS_DATA;
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_POST_PROCESS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateVariable(io::sstream& out, ShaderType type,
        const CString& name, size_t index) const {

    if (!name.empty()) {
        if (type == ShaderType::VERTEX) {
            out << "\n#define VARIABLE_CUSTOM" << index << " " << name.c_str() << "\n";
            out << "\n#define VARIABLE_CUSTOM_AT" << index << " variable_" << name.c_str() << "\n";
            out << "LAYOUT_LOCATION(" << index << ") out vec4 variable_" << name.c_str() << ";\n";
        } else if (type == ShaderType::FRAGMENT) {
            out << "\nLAYOUT_LOCATION(" << index << ") in highp vec4 variable_" << name.c_str() << ";\n";
        }
    }
    return out;
}

io::sstream& CodeGenerator::generateShaderInputs(io::sstream& out, ShaderType type,
        const AttributeBitset& attributes, Interpolation interpolation) const {

    const char* shading = getInterpolationQualifier(interpolation);
    out << "#define SHADING_INTERPOLATION " << shading << "\n";

    bool hasTangents = attributes.test(VertexAttribute::TANGENTS);
    generateDefine(out, "HAS_ATTRIBUTE_TANGENTS", hasTangents);

    bool hasColor = attributes.test(VertexAttribute::COLOR);
    generateDefine(out, "HAS_ATTRIBUTE_COLOR", hasColor);

    bool hasUV0 = attributes.test(VertexAttribute::UV0);
    generateDefine(out, "HAS_ATTRIBUTE_UV0", hasUV0);

    bool hasUV1 = attributes.test(VertexAttribute::UV1);
    generateDefine(out, "HAS_ATTRIBUTE_UV1", hasUV1);

    bool hasBoneIndices = attributes.test(VertexAttribute::BONE_INDICES);
    generateDefine(out, "HAS_ATTRIBUTE_BONE_INDICES", hasBoneIndices);

    bool hasBoneWeights = attributes.test(VertexAttribute::BONE_WEIGHTS);
    generateDefine(out, "HAS_ATTRIBUTE_BONE_WEIGHTS", hasBoneWeights);

    for (int i = 0; i < MAX_CUSTOM_ATTRIBUTES; i++) {
        bool hasCustom = attributes.test(VertexAttribute::CUSTOM0 + i);
        if (hasCustom) {
            generateIndexedDefine(out, "HAS_ATTRIBUTE_CUSTOM", i, 1);
        }
    }

    if (type == ShaderType::VERTEX) {
        out << "\n";
        generateDefine(out, "LOCATION_POSITION", uint32_t(VertexAttribute::POSITION));
        if (hasTangents) {
            generateDefine(out, "LOCATION_TANGENTS", uint32_t(VertexAttribute::TANGENTS));
        }
        if (hasUV0) {
            generateDefine(out, "LOCATION_UV0", uint32_t(VertexAttribute::UV0));
        }
        if (hasUV1) {
            generateDefine(out, "LOCATION_UV1", uint32_t(VertexAttribute::UV1));
        }
        if (hasColor) {
            generateDefine(out, "LOCATION_COLOR", uint32_t(VertexAttribute::COLOR));
        }
        if (hasBoneIndices) {
            generateDefine(out, "LOCATION_BONE_INDICES", uint32_t(VertexAttribute::BONE_INDICES));
        }
        if (hasBoneWeights) {
            generateDefine(out, "LOCATION_BONE_WEIGHTS", uint32_t(VertexAttribute::BONE_WEIGHTS));
        }

        for (int i = 0; i < MAX_CUSTOM_ATTRIBUTES; i++) {
            if (attributes.test(VertexAttribute::CUSTOM0 + i)) {
                generateIndexedDefine(out, "LOCATION_CUSTOM", i,
                        uint32_t(VertexAttribute::CUSTOM0) + i);
            }
        }

        out << SHADERS_INPUTS_VS_DATA;
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_INPUTS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateDepthShaderMain(io::sstream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        out << SHADERS_DEPTH_MAIN_VS_DATA;
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_DEPTH_MAIN_FS_DATA;
    }
    return out;
}

const char* CodeGenerator::getUniformPrecisionQualifier(UniformType type, Precision precision,
        Precision uniformPrecision, Precision defaultPrecision) const noexcept {
    if (!hasPrecision(type)) {
        return "";
    }
    if (precision == Precision::DEFAULT) {
        precision = uniformPrecision;
    }
    return getPrecisionQualifier(precision, defaultPrecision);
}

io::sstream& CodeGenerator::generateUniforms(io::sstream& out, ShaderType shaderType,
        uint8_t binding, const UniformInterfaceBlock& uib) const {
    auto const& infos = uib.getUniformInfoList();
    if (infos.empty()) {
        return out;
    }

    const CString& blockName = uib.getName();
    std::string instanceName(uib.getName().c_str());
    instanceName.front() = char(std::tolower((unsigned char)instanceName.front()));

    Precision uniformPrecision = getDefaultUniformPrecision();
    Precision defaultPrecision = getDefaultPrecision(shaderType);

    out << "\nlayout(";
    if (mTargetLanguage == TargetLanguage::SPIRV) {
        uint32_t bindingIndex = (uint32_t) binding; // avoid char output
        out << "binding = " << bindingIndex << ", ";
    }
    out << "std140) uniform " << blockName.c_str() << " {\n";
    for (auto const& info : infos) {
        char const* const type = getUniformTypeName(info.type);
        char const* const precision = getUniformPrecisionQualifier(info.type, info.precision,
                uniformPrecision, defaultPrecision);
        out << "    " << precision;
        if (precision[0] != '\0') out << " ";
        out << type << " " << info.name.c_str();
        if (info.size > 1) {
            out << "[" << info.size << "]";
        }
        out << ";\n";
    }
    out << "} " << instanceName << ";\n";

    return out;
}

io::sstream& CodeGenerator::generateSamplers(
        io::sstream& out, uint8_t firstBinding, const SamplerInterfaceBlock& sib) const {
    auto const& infos = sib.getSamplerInfoList();
    if (infos.empty()) {
        return out;
    }

    for (auto const& info : infos) {

        CString uniformName =
                SamplerInterfaceBlock::getUniformName(
                        sib.getName().c_str(), info.name.c_str());

        auto type = info.type;
        if (type == SamplerType::SAMPLER_EXTERNAL && mShaderModel != ShaderModel::GL_ES_30) {
            // we're generating the shader for the desktop, where we assume external textures
            // are not supported, in which case we revert to texture2d
            type = SamplerType::SAMPLER_2D;
        }
        char const* const typeName = getSamplerTypeName(type, info.format, info.multisample);
        char const* const precision = getPrecisionQualifier(info.precision, Precision::DEFAULT);
        if (mTargetLanguage == TargetLanguage::SPIRV) {
            const uint32_t bindingIndex = (uint32_t) firstBinding + info.offset;
            out << "layout(binding = " << bindingIndex;

            // For Vulkan, we place uniforms in set 0 (the default set) and samplers in set 1. This
            // allows the sampler bindings to live in a separate "namespace" that starts at zero.
            // Note that the set specifier is not covered by the desktop GLSL spec, including
            // recent versions. It is only documented in the GL_KHR_vulkan_glsl extension.
            if (mTargetApi == TargetApi::VULKAN) {
                out << ", set = 1";
            }

            out << ") ";
        }
        out << "uniform " << precision << " " << typeName << " " << uniformName.c_str();
        out << ";\n";
    }
    out << "\n";

    return out;
}

void CodeGenerator::fixupExternalSamplers(
        std::string& shader, SamplerInterfaceBlock const& sib) noexcept {
    auto const& infos = sib.getSamplerInfoList();
    if (infos.empty()) {
        return;
    }

    bool hasExternalSampler = false;

    // Replace sampler2D declarations by samplerExternal declarations as they may have
    // been swapped during a previous optimization step
    for (auto const& info : infos) {
        if (info.type == SamplerType::SAMPLER_EXTERNAL) {

            CString uniformName =
                    SamplerInterfaceBlock::getUniformName(
                            sib.getName().c_str(), info.name.c_str());

            auto name = std::string("sampler2D ") + uniformName.c_str();
            size_t index = shader.find(name);

            if (index != std::string::npos) {
                hasExternalSampler = true;
                auto newName =
                        std::string("samplerExternalOES ") + uniformName.c_str();
                shader.replace(index, name.size(), newName);
            }
        }
    }

    // This method should only be called on shaders that have external samplers but since
    // they may have been removed by previous optimization steps, we check again here
    if (hasExternalSampler) {
        // Find the #version line so we can insert the #extension directive
        size_t index = shader.find("#version");
        index += 8;

        // Find the end of the line and skip the line return
        while (shader[index] != '\n') index++;
        index++;

        shader.insert(index, "#extension GL_OES_EGL_image_external_essl3 : require\n");
    }
}


io::sstream& CodeGenerator::generateDefine(io::sstream& out, const char* name, bool value) const {
    if (value) {
        out << "#define " << name << "\n";
    }
    return out;
}

io::sstream& CodeGenerator::generateDefine(io::sstream& out, const char* name, float value) const {
    char buffer[32];
    snprintf(buffer, 32, "%.1f", value);
    out << "#define " << name << " " << buffer << "\n";
    return out;
}

io::sstream& CodeGenerator::generateDefine(io::sstream& out, const char* name, uint32_t value) const {
    out << "#define " << name << " " << value << "\n";
    return out;
}

io::sstream& CodeGenerator::generateDefine(io::sstream& out, const char* name, const char* string) const {
    out << "#define " << name << " " << string << "\n";
    return out;
}

io::sstream& CodeGenerator::generateFunction(io::sstream& out, const char* returnType,
        const char* name, const char* body) const {
    out << "\n" << returnType << " " << name << "()";
    out << " {\n" << body;
    out << "\n}\n";
    return out;
}

io::sstream& CodeGenerator::generateIndexedDefine(io::sstream& out, const char* name,
        uint32_t index, uint32_t value) const {
    out << "#define " << name << index << " " << value << "\n";
    return out;
}

io::sstream& CodeGenerator::generateMaterialProperty(io::sstream& out,
        MaterialBuilder::Property property, bool isSet) const {
    if (isSet) {
        out << "#define " << "MATERIAL_HAS_" << getConstantName(property) << "\n";
    }
    return out;
}

io::sstream& CodeGenerator::generateCommon(io::sstream& out, ShaderType type) const {
    out << SHADERS_COMMON_MATH_FS_DATA;
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_COMMON_SHADING_FS_DATA;
        out << SHADERS_COMMON_GRAPHICS_FS_DATA;
        out << SHADERS_COMMON_MATERIAL_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateCommonMaterial(io::sstream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        out << SHADERS_MATERIAL_INPUTS_VS_DATA;
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_MATERIAL_INPUTS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateCommonPostProcess(io::sstream& out, ShaderType type) const {
    if (type == ShaderType::FRAGMENT) {
        out << SHADERS_COMMON_POST_PROCESS_FS_DATA;
    }
    return out;
}

utils::io::sstream& CodeGenerator::generateCommonGetters(utils::io::sstream& out) const {
    out << SHADERS_COMMON_GETTERS_FS_DATA;
    return out;
}

io::sstream& CodeGenerator::generateGetters(io::sstream& out, ShaderType type) const {
    out << SHADERS_COMMON_GETTERS_FS_DATA;
    if (type == ShaderType::VERTEX) {
        out << SHADERS_GETTERS_VS_DATA;
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_GETTERS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateParameters(io::sstream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_SHADING_PARAMETERS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateShaderLit(io::sstream& out, ShaderType type,
        filament::Variant variant, filament::Shading shading) const {
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        out << SHADERS_COMMON_LIGHTING_FS_DATA;
        if (variant.hasShadowReceiver()) {
            out << SHADERS_SHADOWING_FS_DATA;
        }

        out << SHADERS_BRDF_FS_DATA;
        switch (shading) {
            case Shading::UNLIT:
                assert("Lit shader generated with unlit shading model");
                break;
            case Shading::SPECULAR_GLOSSINESS:
            case Shading::LIT:
                out << SHADERS_SHADING_MODEL_STANDARD_FS_DATA;
                break;
            case Shading::SUBSURFACE:
                out << SHADERS_SHADING_MODEL_SUBSURFACE_FS_DATA;
                break;
            case Shading::CLOTH:
                out << SHADERS_SHADING_MODEL_CLOTH_FS_DATA;
                break;
        }

        if (shading != Shading::UNLIT) {
            out << SHADERS_AMBIENT_OCCLUSION_FS_DATA;
            out << SHADERS_LIGHT_INDIRECT_FS_DATA;
        }
        if (variant.hasDirectionalLighting()) {
            out << SHADERS_LIGHT_DIRECTIONAL_FS_DATA;
        }
        if (variant.hasDynamicLighting()) {
            out << SHADERS_LIGHT_PUNCTUAL_FS_DATA;
        }

        out << SHADERS_SHADING_LIT_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateShaderUnlit(io::sstream& out, ShaderType type,
        filament::Variant variant, bool hasShadowMultiplier) const {
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        if (hasShadowMultiplier) {
            if (variant.hasShadowReceiver()) {
                out << SHADERS_SHADOWING_FS_DATA;
            }
        }
        out << SHADERS_SHADING_UNLIT_FS_DATA;
    }
    return out;
}

/* static */
char const* CodeGenerator::getConstantName(MaterialBuilder::Property property) noexcept {
    using Property = MaterialBuilder::Property;
    switch (property) {
        case Property::BASE_COLOR:           return "BASE_COLOR";
        case Property::ROUGHNESS:            return "ROUGHNESS";
        case Property::METALLIC:             return "METALLIC";
        case Property::REFLECTANCE:          return "REFLECTANCE";
        case Property::AMBIENT_OCCLUSION:    return "AMBIENT_OCCLUSION";
        case Property::CLEAR_COAT:           return "CLEAR_COAT";
        case Property::CLEAR_COAT_ROUGHNESS: return "CLEAR_COAT_ROUGHNESS";
        case Property::CLEAR_COAT_NORMAL:    return "CLEAR_COAT_NORMAL";
        case Property::ANISOTROPY:           return "ANISOTROPY";
        case Property::ANISOTROPY_DIRECTION: return "ANISOTROPY_DIRECTION";
        case Property::THICKNESS:            return "THICKNESS";
        case Property::SUBSURFACE_POWER:     return "SUBSURFACE_POWER";
        case Property::SUBSURFACE_COLOR:     return "SUBSURFACE_COLOR";
        case Property::SHEEN_COLOR:          return "SHEEN_COLOR";
        case Property::GLOSSINESS:           return "GLOSSINESS";
        case Property::SPECULAR_COLOR:       return "SPECULAR_COLOR";
        case Property::EMISSIVE:             return "EMISSIVE";
        case Property::NORMAL:               return "NORMAL";
        case Property::POST_LIGHTING_COLOR:  return "POST_LIGHTING_COLOR";
    }
}

char const* CodeGenerator::getUniformTypeName(UniformInterfaceBlock::Type type) noexcept {
    using Type = UniformInterfaceBlock::Type;
    switch (type) {
        case Type::BOOL:   return "bool";
        case Type::BOOL2:  return "bvec2";
        case Type::BOOL3:  return "bvec3";
        case Type::BOOL4:  return "bvec4";
        case Type::FLOAT:  return "float";
        case Type::FLOAT2: return "vec2";
        case Type::FLOAT3: return "vec3";
        case Type::FLOAT4: return "vec4";
        case Type::INT:    return "int";
        case Type::INT2:   return "ivec2";
        case Type::INT3:   return "ivec3";
        case Type::INT4:   return "ivec4";
        case Type::UINT:   return "uint";
        case Type::UINT2:  return "uvec2";
        case Type::UINT3:  return "uvec3";
        case Type::UINT4:  return "uvec4";
        case Type::MAT3:   return "mat3";
        case Type::MAT4:   return "mat4";
    }
}

char const* CodeGenerator::getSamplerTypeName(SamplerType type, SamplerFormat format,
        bool multisample) const noexcept {
    switch (type) {
        case SamplerType::SAMPLER_2D:
            if (!multisample) {
                switch (format) {
                    case SamplerFormat::INT:    return "isampler2D";
                    case SamplerFormat::UINT:   return "usampler2D";
                    case SamplerFormat::FLOAT:  return "sampler2D";
                    case SamplerFormat::SHADOW: return "sampler2DShadow";
                }
            } else {
                assert(format != SamplerFormat::SHADOW);
                switch (format) {
                    case SamplerFormat::INT:    return "ms_isampler2D";
                    case SamplerFormat::UINT:   return "ms_usampler2D";
                    case SamplerFormat::FLOAT:  return "ms_sampler2D";
                    case SamplerFormat::SHADOW: return "sampler2DShadow";   // should not happen
                }
            }
        case SamplerType::SAMPLER_CUBEMAP:
            assert(!multisample);
            switch (format) {
                case SamplerFormat::INT:    return "isamplerCube";
                case SamplerFormat::UINT:   return "usamplerCube";
                case SamplerFormat::FLOAT:  return "samplerCube";
                case SamplerFormat::SHADOW: return "samplerCubeShadow";
            }
        case SamplerType::SAMPLER_EXTERNAL:
            assert(!multisample);
            assert(format != SamplerFormat::SHADOW);
            // Vulkan doesn't have external textures in the sense as GL. Vulkan external textures
            // are created via VK_ANDROID_external_memory_android_hardware_buffer, but they are
            // backed by VkImage just like a normal texture, and sampled from normally.
            return (mTargetLanguage == TargetLanguage::SPIRV) ? "sampler2D" : "samplerExternalOES";
    }
}

char const* CodeGenerator::getInterpolationQualifier(Interpolation interpolation) noexcept {
    switch (interpolation) {
        case Interpolation::SMOOTH: return "";
        case Interpolation::FLAT:   return "flat ";
    }
}

/* static */
char const* CodeGenerator::getPrecisionQualifier(Precision precision,
        Precision defaultPrecision) noexcept {
    if (precision == defaultPrecision) {
        return "";
    }

    switch (precision) {
        case Precision::LOW:     return "lowp";
        case Precision::MEDIUM:  return "mediump";
        case Precision::HIGH:    return "highp";
        case Precision::DEFAULT: return "ERROR";
    }
}

/* static */
bool CodeGenerator::hasPrecision(UniformInterfaceBlock::Type type) noexcept {
    switch (type) {
        case UniformType::BOOL:
        case UniformType::BOOL2:
        case UniformType::BOOL3:
        case UniformType::BOOL4:
            return false;
        default:
            return true;
    }
}

} // namespace filamat
