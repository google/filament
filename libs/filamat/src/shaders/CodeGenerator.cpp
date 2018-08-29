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

#include "Shaders.h"

#include <cctype>
#include <iomanip>

namespace filamat {

// From driverEnum namespace
using namespace filament;
using namespace driver;
using namespace utils;

std::ostream& CodeGenerator::generateSeparator(std::ostream& out) const {
    out << '\n';
    return out;
}

std::ostream& CodeGenerator::generateProlog(std::ostream& out, ShaderType type,
        bool hasExternalSamplers) const {
    assert(mShaderModel != ShaderModel::UNKNOWN);
    switch (mShaderModel) {
        case ShaderModel::UNKNOWN:
            break;
        case ShaderModel::GL_ES_30:
            // Vulkan requires version 310 or higher
            if (mCodeGenTargetApi == TargetApi::VULKAN) {
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
            if (mCodeGenTargetApi == TargetApi::VULKAN) {
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
    if (mCodeGenTargetApi == TargetApi::VULKAN) {
        out << "#define CODEGEN_TARGET_VULKAN_ENVIRONMENT\n";
    }

    Precision defaultPrecision = getDefaultPrecision(type);
    const char* precision = getPrecisionQualifier(defaultPrecision, Precision::DEFAULT);
    out << "precision " << precision << " float;\n";
    out << "precision " << precision << " int;\n";

    if (type == ShaderType::VERTEX) {
        out << "\n";
        out << "invariant gl_Position;\n";
    }

    out << filament::shaders::common_types_fs;

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

std::ostream& CodeGenerator::generateEpilog(std::ostream& out) const {
    out << "\n"; // For line compression all shaders finish with a newline character.
    return out;
}

std::ostream& CodeGenerator::generateShaderMain(std::ostream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        out << filament::shaders::shadowing_vs;
        out << filament::shaders::main_vs;
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::main_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generatePostProcessMain(std::ostream& out,
        ShaderType type, filament::PostProcessStage variant) const {
    if (type == ShaderType::VERTEX) {
        out << filament::shaders::post_process_vs;
    } else if (type == ShaderType::FRAGMENT) {
        switch (variant) {
            case PostProcessStage::TONE_MAPPING_OPAQUE:
            case PostProcessStage::TONE_MAPPING_TRANSLUCENT:
                out << filament::shaders::tone_mapping_fs;
                out << filament::shaders::conversion_functions_fs;
                out << filament::shaders::dithering_fs;
                break;
            case PostProcessStage::ANTI_ALIASING_OPAQUE:
            case PostProcessStage::ANTI_ALIASING_TRANSLUCENT:
                out << filament::shaders::fxaa_fs;
                break;
        }
        out << filament::shaders::post_process_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generateVariable(std::ostream& out, ShaderType type,
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

std::ostream& CodeGenerator::generateVariables(std::ostream& out, ShaderType type,
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

        out << filament::shaders::variables_vs;
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::variables_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generateDepthShaderMain(std::ostream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        out << filament::shaders::depth_main_vs;
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::depth_main_fs;
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

std::ostream& CodeGenerator::generateUniforms(std::ostream& out, ShaderType shaderType,
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
    if (mCodeGenTargetApi == TargetApi::VULKAN) {
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

std::ostream& CodeGenerator::generateSamplers(
        std::ostream& out, uint8_t firstBinding, const SamplerInterfaceBlock& sib) const {
    auto const& infos = sib.getSamplerInfoList();
    if (infos.empty()) {
        return out;
    }

    const CString& blockName = sib.getName();
    std::string instanceName(blockName.c_str());
    instanceName.front() = char(std::tolower((unsigned char)instanceName.front()));

    for (auto const& info : infos) {
        auto type = info.type;
        if (info.type == SamplerType::SAMPLER_EXTERNAL && mShaderModel != ShaderModel::GL_ES_30) {
            // we're generating the shader for the desktop, where we assume external textures
            // are not supported, in which case we revert to texture2d
            type = SamplerType::SAMPLER_2D;
        }
        char const* const typeName = getSamplerTypeName(type, info.format, info.multisample);
        char const* const precision = getPrecisionQualifier(info.precision, Precision::DEFAULT);
        if (mCodeGenTargetApi == TargetApi::VULKAN) {
            const uint32_t bindingIndex = (uint32_t) firstBinding + info.offset;
            out << "layout(binding = " << bindingIndex << ") ";
        }
        out << "uniform " << precision << " " << typeName << " " <<
                instanceName << "_" << info.name.c_str();
        out << ";\n";
    }
    out << "\n";

    return out;
}

std::ostream& CodeGenerator::generateDefine(std::ostream& out, const char* name, bool value) const {
    if (value) {
        out << "#define " << name << "\n";
    }
    return out;
}

std::ostream& CodeGenerator::generateDefine(std::ostream& out, const char* name, float value) const {
    out << "#define " << name << " " << std::fixed << std::setprecision(1) << value << "\n";
    return out;
}

std::ostream& CodeGenerator::generateDefine(std::ostream& out, const char* name, uint32_t value) const {
    out << "#define " << name << " " << value << "\n";
    return out;
}

std::ostream& CodeGenerator::generateDefine(std::ostream& out, const char* name, const char* string) const {
    out << "#define " << name << " " << string << "\n";
    return out;
}

std::ostream& CodeGenerator::generateFunction(std::ostream& out, const char* returnType,
        const char* name, const char* body) const {
    out << "\n" << returnType << " " << name << "()";
    out << " {\n" << body;
    out << "\n}\n";
    return out;
}

std::ostream& CodeGenerator::generateMaterialProperty(std::ostream& out,
        Property property, bool isSet) const {
    if (isSet) {
        out << "#define " << "MATERIAL_HAS_" << getConstantName(property) << "\n";
    }
    return out;
}

std::ostream& CodeGenerator::generateCommon(std::ostream& out, ShaderType type) const {
    out << filament::shaders::common_math_fs;
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::common_graphics_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generateCommonMaterial(std::ostream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
        out << filament::shaders::common_material_vs;
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::common_material_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generateGetters(std::ostream& out, ShaderType type) const {
    out << filament::shaders::common_getters_fs;
    if (type == ShaderType::VERTEX) {
        out << filament::shaders::getters_vs;
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::getters_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generateParameters(std::ostream& out, ShaderType type) const {
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::shading_parameters_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generateShaderLit(std::ostream& out, ShaderType type,
        filament::Variant variant, filament::Shading shading) const {
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        out << filament::shaders::common_lighting_fs;
        if (variant.hasShadowReceiver()) {
            out << filament::shaders::shadowing_fs;
        }

        out << filament::shaders::brdf_fs;
        switch (shading) {
            case Shading::UNLIT:
                assert("Lit shader generated with unlit shading model");
                break;
            case Shading::LIT:
                out << filament::shaders::shading_model_standard_fs;
                break;
            case Shading::SUBSURFACE:
                out << filament::shaders::shading_model_subsurface_fs;
                break;
            case Shading::CLOTH:
                out << filament::shaders::shading_model_cloth_fs;
                break;
        }

        if (shading != Shading::UNLIT) {
            out << filament::shaders::light_indirect_fs;
        }
        if (variant.hasDirectionalLighting()) {
            out << filament::shaders::light_directional_fs;
        }
        if (variant.hasDynamicLighting()) {
            out << filament::shaders::light_punctual_fs;
        }

        out << filament::shaders::shading_lit_fs;
    }
    return out;
}

std::ostream& CodeGenerator::generateShaderUnlit(std::ostream& out, ShaderType type,
        filament::Variant variant, bool hasShadowMultiplier) const {
    if (type == ShaderType::VERTEX) {
    } else if (type == ShaderType::FRAGMENT) {
        if (hasShadowMultiplier) {
            if (variant.hasShadowReceiver()) {
                out << filament::shaders::shadowing_fs;
            }
        }
        out << filament::shaders::shading_unlit_fs;
    }
    return out;
}

/* static */
char const* CodeGenerator::getConstantName(Property property) noexcept {
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
        case Property::EMISSIVE:             return "EMISSIVE";
        case Property::NORMAL:               return "NORMAL";
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
            return (mCodeGenTargetApi == TargetApi::VULKAN) ? "sampler2D" : "samplerExternalOES";
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
