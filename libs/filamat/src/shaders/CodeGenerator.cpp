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

#include "MaterialInfo.h"

#include "generated/shaders.h"

#include <utils/sstream.h>

#include <cctype>
#include <iomanip>

#include <assert.h>

namespace filamat {

// From driverEnum namespace
using namespace filament;
using namespace backend;
using namespace utils;

io::sstream& CodeGenerator::generateSeparator(io::sstream& out) {
    out << '\n';
    return out;
}

utils::io::sstream& CodeGenerator::generateProlog(utils::io::sstream& out, ShaderStage stage,
        MaterialInfo const& material) const {
    switch (mShaderModel) {
        case ShaderModel::MOBILE:
            // Vulkan requires version 310 or higher
            if (mTargetLanguage == TargetLanguage::SPIRV ||
                material.featureLevel >= FeatureLevel::FEATURE_LEVEL_2) {
                // Vulkan requires layout locations on ins and outs, which were not supported
                // in the OpenGL 4.1 GLSL profile.
                out << "#version 310 es\n\n";
            } else {
                out << "#version 300 es\n\n";
            }
            if (material.hasExternalSamplers) {
                out << "#extension GL_OES_EGL_image_external_essl3 : require\n\n";
            }
            out << "#define TARGET_MOBILE\n";
            break;
        case ShaderModel::DESKTOP:
            if (mTargetLanguage == TargetLanguage::SPIRV ||
                material.featureLevel >= FeatureLevel::FEATURE_LEVEL_2) {
                // Vulkan requires binding specifiers on uniforms and samplers, which were not
                // supported in the OpenGL 4.1 GLSL profile.
                out << "#version 450 core\n\n";
            } else {
                out << "#version 410 core\n\n";
                out << "#extension GL_ARB_shading_language_packing : enable\n\n";
            }
            break;
    }

    // This allows our includer system to use the #line directive to denote the source file for
    // #included code. This way, glslang reports errors more accurately.
    out << "#extension GL_GOOGLE_cpp_style_line_directive : enable\n\n";

    if (stage == ShaderStage::COMPUTE) {
        out << "layout(local_size_x = " << material.groupSize.x
            << ", local_size_y = " <<  material.groupSize.y
            << ", local_size_z = " <<  material.groupSize.z
            << ") in;\n\n";
    }

    if (mTargetApi == TargetApi::VULKAN) {
        out << "#define TARGET_VULKAN_ENVIRONMENT\n";
    }
    if (mTargetApi == TargetApi::METAL) {
        out << "#define TARGET_METAL_ENVIRONMENT\n";
    }
    if (mTargetApi == TargetApi::OPENGL && mShaderModel == ShaderModel::MOBILE) {
        out << "#define TARGET_GLES_ENVIRONMENT\n";
    }
    if (mTargetApi == TargetApi::OPENGL && mShaderModel == ShaderModel::DESKTOP) {
        out << "#define TARGET_GL_ENVIRONMENT\n";
    }

    out << '\n';
    if (mTargetLanguage == TargetLanguage::SPIRV) {
        out << "#define FILAMENT_VULKAN_SEMANTICS\n";
    }
    if (mTargetLanguage == TargetLanguage::GLSL) {
        out << "#define FILAMENT_OPENGL_SEMANTICS\n";
    }

    out << '\n';
    if (mTargetApi == TargetApi::VULKAN ||
        mTargetApi == TargetApi::METAL ||
        (mTargetApi == TargetApi::OPENGL && mShaderModel == ShaderModel::DESKTOP) ||
        material.featureLevel >= FeatureLevel::FEATURE_LEVEL_2) {
        out << "#define FILAMENT_HAS_FEATURE_TEXTURE_GATHER\n";
    }

    Precision defaultPrecision = getDefaultPrecision(stage);
    const char* precision = getPrecisionQualifier(defaultPrecision, Precision::DEFAULT);
    out << "precision " << precision << " float;\n";
    out << "precision " << precision << " int;\n";
    if (mShaderModel == ShaderModel::MOBILE) {
        out << "precision lowp sampler2DArray;\n";
        out << "precision lowp sampler3D;\n";
    }

    // specification constants
    out << '\n';
    generateSpecificationConstant(out, "BACKEND_FEATURE_LEVEL", 0, 1);
    generateSpecificationConstant(out, "CONFIG_MAX_INSTANCES", 1, (int)CONFIG_MAX_INSTANCES);

    out << '\n';
    out << SHADERS_COMMON_DEFINES_GLSL_DATA;

    if (stage != ShaderStage::COMPUTE) {
        out << '\n';
        out << SHADERS_COMMON_TYPES_GLSL_DATA;
    }

    out << "\n";
    return out;
}

Precision CodeGenerator::getDefaultPrecision(ShaderStage stage) const {
    switch (stage) {
        case ShaderStage::VERTEX:
            return Precision::HIGH;
            break;
        case ShaderStage::FRAGMENT:
            if (mShaderModel < ShaderModel::DESKTOP) {
                return Precision::MEDIUM;
            } else {
                return Precision::HIGH;
            }
            break;
        case ShaderStage::COMPUTE:
            return Precision::HIGH;
            break;
    }
}

Precision CodeGenerator::getDefaultUniformPrecision() const {
    if (mShaderModel < ShaderModel::DESKTOP) {
        return Precision::MEDIUM;
    } else {
        return Precision::HIGH;
    }
}

io::sstream& CodeGenerator::generateEpilog(io::sstream& out) {
    out << "\n"; // For line compression all shaders finish with a newline character.
    return out;
}

io::sstream& CodeGenerator::generateShaderMain(io::sstream& out, ShaderStage stage) {
    switch (stage) {
        case ShaderStage::VERTEX:
            out << SHADERS_MAIN_VS_DATA;
            break;
        case ShaderStage::FRAGMENT:
            out << SHADERS_MAIN_FS_DATA;
            break;
        case ShaderStage::COMPUTE:
            out << SHADERS_MAIN_CS_DATA;
            break;
    }
    return out;
}

io::sstream& CodeGenerator::generatePostProcessMain(io::sstream& out, ShaderStage type) {
    if (type == ShaderStage::VERTEX) {
        out << SHADERS_POST_PROCESS_VS_DATA;
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_POST_PROCESS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateVariable(io::sstream& out, ShaderStage type,
        const CString& name, size_t index) {

    if (!name.empty()) {
        if (type == ShaderStage::VERTEX) {
            out << "\n#define VARIABLE_CUSTOM" << index << " " << name.c_str() << "\n";
            out << "\n#define VARIABLE_CUSTOM_AT" << index << " variable_" << name.c_str() << "\n";
            out << "LAYOUT_LOCATION(" << index << ") out vec4 variable_" << name.c_str() << ";\n";
        } else if (type == ShaderStage::FRAGMENT) {
            out << "\nLAYOUT_LOCATION(" << index << ") in highp vec4 variable_" << name.c_str() << ";\n";
        }
    }
    return out;
}

io::sstream& CodeGenerator::generateShaderInputs(io::sstream& out, ShaderStage type,
        const AttributeBitset& attributes, Interpolation interpolation) {

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

    if (type == ShaderStage::VERTEX) {
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

        out << SHADERS_ATTRIBUTES_VS_DATA;

        generateDefine(out, "VARYING", "out");
    } else if (type == ShaderStage::FRAGMENT) {
        generateDefine(out, "VARYING", "in");
    }
    out << SHADERS_VARYINGS_GLSL_DATA;
    return out;
}

io::sstream& CodeGenerator::generateOutput(io::sstream& out, ShaderStage type,
        const CString& name, size_t index,
        MaterialBuilder::VariableQualifier qualifier,
        MaterialBuilder::OutputType outputType) const {
    if (name.empty() || type == ShaderStage::VERTEX) {
        return out;
    }

    // TODO: add and support additional variable qualifiers
    (void) qualifier;
    assert(qualifier == MaterialBuilder::VariableQualifier::OUT);

    // The material output type is the type the shader writes to from the material.
    const MaterialBuilder::OutputType materialOutputType = outputType;

    const char* swizzleString = "";

    // Metal doesn't support some 3-component texture formats, so the backend uses 4-component
    // formats behind the scenes. It's an error to output fewer components than the attachment
    // needs, so we always output a float4 instead of a float3. It's never an error to output extra
    // components.
    if (mTargetApi == TargetApi::METAL) {
        if (outputType == MaterialBuilder::OutputType::FLOAT3) {
            outputType = MaterialBuilder::OutputType::FLOAT4;
            swizzleString = ".rgb";
        }
    }

    const char* materialTypeString = getOutputTypeName(materialOutputType);
    const char* typeString = getOutputTypeName(outputType);

    out << "\n#define FRAG_OUTPUT" << index << " " << name.c_str() << "\n";
    out << "\n#define FRAG_OUTPUT_AT" << index << " output_" << name.c_str() << "\n";
    out << "\n#define FRAG_OUTPUT_MATERIAL_TYPE" << index << " " << materialTypeString << "\n";
    out << "\n#define FRAG_OUTPUT_TYPE" << index << " " << typeString << "\n";
    out << "\n#define FRAG_OUTPUT_SWIZZLE" << index << " " << swizzleString << "\n";
    out << "layout(location=" << index << ") out " << typeString <<
        " output_" << name.c_str() << ";\n";

    return out;
}


io::sstream& CodeGenerator::generateDepthShaderMain(io::sstream& out, ShaderStage type) {
    assert(type != ShaderStage::VERTEX);
    if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_DEPTH_MAIN_FS_DATA;
    }
    return out;
}

const char* CodeGenerator::getUniformPrecisionQualifier(UniformType type, Precision precision,
        Precision uniformPrecision, Precision defaultPrecision) noexcept {
    if (!hasPrecision(type)) {
        return "";
    }
    if (precision == Precision::DEFAULT) {
        precision = uniformPrecision;
    }
    return getPrecisionQualifier(precision, defaultPrecision);
}

utils::io::sstream& CodeGenerator::generateBuffers(utils::io::sstream& out,
        MaterialInfo::BufferContainer const& buffers) const {
    uint32_t binding = 0;
    for (auto const* buffer : buffers) {
        generateBufferInterfaceBlock(out, ShaderStage::COMPUTE, binding, *buffer);
        binding++;
    }
    return out;
}

io::sstream& CodeGenerator::generateUniforms(io::sstream& out, ShaderStage stage,
        UniformBindingPoints binding, const BufferInterfaceBlock& uib) const {
    return generateBufferInterfaceBlock(out, stage, +binding, uib);
}

io::sstream& CodeGenerator::generateBufferInterfaceBlock(io::sstream& out, ShaderStage stage,
        uint32_t binding, const BufferInterfaceBlock& uib) const {
    auto const& infos = uib.getFieldInfoList();
    if (infos.empty()) {
        return out;
    }

    std::string blockName{ uib.getName() };
    std::string instanceName{ uib.getName() };
    blockName.front() = char(std::toupper((unsigned char)blockName.front()));
    instanceName.front() = char(std::tolower((unsigned char)instanceName.front()));

    Precision uniformPrecision = getDefaultUniformPrecision();
    Precision defaultPrecision = getDefaultPrecision(stage);

    out << "\nlayout(";
    if (mTargetLanguage == TargetLanguage::SPIRV ||
        mFeatureLevel >= FeatureLevel::FEATURE_LEVEL_2) {
        switch (mTargetApi) {
            case TargetApi::METAL:
                out << "binding = " << METAL_UNIFORM_BUFFER_BINDING_START + binding << ", ";
                break;

            case TargetApi::OPENGL:
                // GLSL 4.5 / ESSL 3.1 require the 'binding' layout qualifier
            case TargetApi::VULKAN:
                out << "binding = " << binding << ", ";
                break;

            case TargetApi::ALL:
                // nonsensical, shouldn't happen.
                break;
        }
    }
    switch (uib.getAlignment()) {
        case BufferInterfaceBlock::Alignment::std140:
            out << "std140";
            break;
        case BufferInterfaceBlock::Alignment::std430:
            out << "std430";
            break;
    }

    out << ") ";

    switch (uib.getTarget()) {
        case BufferInterfaceBlock::Target::UNIFORM:
            out << "uniform ";
            break;
        case BufferInterfaceBlock::Target::SSBO:
            out << "buffer ";
            break;
    }

    out << blockName << " ";

    if (uib.getTarget() == BufferInterfaceBlock::Target::SSBO) {
        uint8_t qualifiers = uib.getQualifier();
        while (qualifiers) {
            uint8_t mask = 1u << utils::ctz(unsigned(qualifiers));
            switch (BufferInterfaceBlock::Qualifier(qualifiers & mask)) {
                case BufferInterfaceBlock::Qualifier::COHERENT:  out << "coherent "; break;
                case BufferInterfaceBlock::Qualifier::WRITEONLY: out << "writeonly "; break;
                case BufferInterfaceBlock::Qualifier::READONLY:  out << "readonly "; break;
                case BufferInterfaceBlock::Qualifier::VOLATILE:  out << "volatile "; break;
                case BufferInterfaceBlock::Qualifier::RESTRICT:  out << "restrict "; break;
            }
            qualifiers &= ~mask;
        }
    }

    out << "{\n";

    for (auto const& info : infos) {
        char const* const type = getUniformTypeName(info);
        char const* const precision = getUniformPrecisionQualifier(info.type, info.precision,
                uniformPrecision, defaultPrecision);
        out << "    " << precision;
        if (precision[0] != '\0') out << " ";
        out << type << " " << info.name.c_str();
        if (info.isArray) {
            if (info.sizeName.empty()) {
                if (info.size) {
                    out << "[" << info.size << "]";
                } else {
                    out << "[]";
                }
            } else {
                out << "[" << info.sizeName.c_str() << "]";
            }
        }
        out << ";\n";
    }
    out << "} " << instanceName << ";\n";

    return out;
}

io::sstream& CodeGenerator::generateSamplers(
        io::sstream& out, SamplerBindingPoints bindingPoint, uint8_t firstBinding,
        const SamplerInterfaceBlock& sib) const {
    auto const& infos = sib.getSamplerInfoList();
    if (infos.empty()) {
        return out;
    }

    for (auto const& info : infos) {
        auto type = info.type;
        if (type == SamplerType::SAMPLER_EXTERNAL && mShaderModel != ShaderModel::MOBILE) {
            // we're generating the shader for the desktop, where we assume external textures
            // are not supported, in which case we revert to texture2d
            type = SamplerType::SAMPLER_2D;
        }
        char const* const typeName = getSamplerTypeName(type, info.format, info.multisample);
        char const* const precision = getPrecisionQualifier(info.precision, Precision::DEFAULT);
        if (mTargetLanguage == TargetLanguage::SPIRV) {
            const uint32_t bindingIndex = (uint32_t) firstBinding + info.offset;
            switch (mTargetApi) {
                // For Vulkan, we place uniforms in set 0 (the default set) and samplers in set 1. This
                // allows the sampler bindings to live in a separate "namespace" that starts at zero.
                // Note that the set specifier is not covered by the desktop GLSL spec, including
                // recent versions. It is only documented in the GL_KHR_vulkan_glsl extension.
                case TargetApi::VULKAN:
                    out << "layout(binding = " << bindingIndex << ", set = 1) ";
                    break;

                // For Metal, each sampler group gets its own descriptor set, each of which will
                // become an argument buffer. The first descriptor set is reserved for uniforms,
                // hence the +1 here.
                case TargetApi::METAL:
                    out << "layout(binding = " << (uint32_t) info.offset
                        << ", set = " << (uint32_t) bindingPoint + 1 << ") ";
                    break;

                default:
                case TargetApi::OPENGL:
                    out << "layout(binding = " << bindingIndex << ") ";
                    break;
            }
        }
        out << "uniform " << precision << " " << typeName << " " << info.uniformName.c_str();
        out << ";\n";
    }
    out << "\n";

    return out;
}

io::sstream& CodeGenerator::generateSubpass(io::sstream& out, SubpassInfo subpass) {
    if (!subpass.isValid) {
        return out;
    }

    CString subpassName =
            SamplerInterfaceBlock::generateUniformName(subpass.block.c_str(), subpass.name.c_str());

    char const* const typeName = "subpassInput";
    // In our Vulkan backend, subpass inputs always live in descriptor set 2. (ignored for GLES)
    char const* const precision = getPrecisionQualifier(subpass.precision, Precision::DEFAULT);
    out << "layout(input_attachment_index = " << (int) subpass.attachmentIndex
        << ", set = 2, binding = " << (int) subpass.binding
        << ") ";
    out << "uniform " << precision << " " << typeName << " " << subpassName.c_str();
    out << ";\n";

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
            auto name = std::string("sampler2D ") + info.uniformName.c_str();
            size_t index = shader.find(name);

            if (index != std::string::npos) {
                hasExternalSampler = true;
                auto newName =
                        std::string("samplerExternalOES ") + info.uniformName.c_str();
                shader.replace(index, name.size(), newName);
            }
        }
    }

    // This method should only be called on shaders that have external samplers but since
    // they may have been removed by previous optimization steps, we check again here
    if (hasExternalSampler) {
        // Find the #version line, so we can insert the #extension directive
        size_t index = shader.find("#version");
        index += 8;

        // Find the end of the line and skip the line return
        while (shader[index] != '\n') index++;
        index++;

        shader.insert(index, "#extension GL_OES_EGL_image_external_essl3 : require\n");
    }
}


io::sstream& CodeGenerator::generateDefine(io::sstream& out, const char* name, bool value) {
    if (value) {
        out << "#define " << name << "\n";
    }
    return out;
}

io::sstream& CodeGenerator::generateDefine(io::sstream& out, const char* name, uint32_t value) {
    out << "#define " << name << " " << value << "\n";
    return out;
}

io::sstream& CodeGenerator::generateDefine(io::sstream& out, const char* name, const char* string) {
    out << "#define " << name << " " << string << "\n";
    return out;
}

io::sstream& CodeGenerator::generateIndexedDefine(io::sstream& out, const char* name,
        uint32_t index, uint32_t value) {
    out << "#define " << name << index << " " << value << "\n";
    return out;
}

utils::io::sstream& CodeGenerator::generateSpecificationConstant(utils::io::sstream& out,
        const char* name, uint32_t id, std::variant<int, float, bool> value) const {
    static const char* types[] = { "int", "float", "bool" };
    if (mTargetLanguage == MaterialBuilderBase::TargetLanguage::SPIRV) {
        std::visit([&](auto&& arg) {
            out << "layout (constant_id = " << id << ") const "
                << types[value.index()] << " " << name << " = " << arg << ";\n\n";
        }, value);
    } else {
        std::visit([&](auto&& arg) {
            out << "#ifndef SPIRV_CROSS_CONSTANT_ID_" << id << '\n'
                << "#define SPIRV_CROSS_CONSTANT_ID_" << id << " " << arg << '\n'
                << "#endif" << '\n'
                << "const " << types[value.index()] << " " << name << " = SPIRV_CROSS_CONSTANT_ID_" << id
                << ";\n\n";
        }, value);
    }
    return out;
}


io::sstream& CodeGenerator::generateMaterialProperty(io::sstream& out,
        MaterialBuilder::Property property, bool isSet) {
    if (isSet) {
        out << "#define " << "MATERIAL_HAS_" << getConstantName(property) << "\n";
    }
    return out;
}

io::sstream& CodeGenerator::generateQualityDefine(io::sstream& out, ShaderQuality quality) const {
    out << "#define FILAMENT_QUALITY_LOW    0\n";
    out << "#define FILAMENT_QUALITY_NORMAL 1\n";
    out << "#define FILAMENT_QUALITY_HIGH   2\n";

    switch (quality) {
        case ShaderQuality::DEFAULT:
            switch (mShaderModel) {
                default:                        goto quality_normal;
                case ShaderModel::DESKTOP:   goto quality_high;
                case ShaderModel::MOBILE:     goto quality_low;
            }
        case ShaderQuality::LOW:
        quality_low:
            out << "#define FILAMENT_QUALITY FILAMENT_QUALITY_LOW\n";
            break;
        case ShaderQuality::NORMAL:
        default:
        quality_normal:
            out << "#define FILAMENT_QUALITY FILAMENT_QUALITY_NORMAL\n";
            break;
        case ShaderQuality::HIGH:
        quality_high:
            out << "#define FILAMENT_QUALITY FILAMENT_QUALITY_HIGH\n";
            break;
    }

    return out;
}

io::sstream& CodeGenerator::generateCommon(io::sstream& out, ShaderStage stage) {

    out << SHADERS_COMMON_MATH_GLSL_DATA;

    switch (stage) {
        case ShaderStage::VERTEX:
            out << SHADERS_COMMON_SHADOWING_GLSL_DATA;
            break;
        case ShaderStage::FRAGMENT:
            out << SHADERS_COMMON_SHADOWING_GLSL_DATA;
            out << SHADERS_COMMON_SHADING_FS_DATA;
            out << SHADERS_COMMON_GRAPHICS_FS_DATA;
            out << SHADERS_COMMON_MATERIAL_FS_DATA;
            break;
        case ShaderStage::COMPUTE:
            out << '\n';
            // TODO: figure out if we need some common files here
            break;
    }
    return out;
}

io::sstream& CodeGenerator::generatePostProcessCommon(io::sstream& out, ShaderStage type) {
    out << SHADERS_COMMON_MATH_GLSL_DATA;
    if (type == ShaderStage::VERTEX) {
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_COMMON_SHADING_FS_DATA;
        out << SHADERS_COMMON_GRAPHICS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateFog(io::sstream& out, ShaderStage type) {
    if (type == ShaderStage::VERTEX) {
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_FOG_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateCommonMaterial(io::sstream& out, ShaderStage type) {
    if (type == ShaderStage::VERTEX) {
        out << SHADERS_MATERIAL_INPUTS_VS_DATA;
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_MATERIAL_INPUTS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generatePostProcessInputs(io::sstream& out, ShaderStage type) {
    if (type == ShaderStage::VERTEX) {
        out << SHADERS_POST_PROCESS_INPUTS_VS_DATA;
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_POST_PROCESS_INPUTS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generatePostProcessGetters(io::sstream& out, ShaderStage type) {
    out << SHADERS_COMMON_GETTERS_GLSL_DATA;
    if (type == ShaderStage::VERTEX) {
        out << SHADERS_POST_PROCESS_GETTERS_VS_DATA;
    } else if (type == ShaderStage::FRAGMENT) {
    }
    return out;
}

io::sstream& CodeGenerator::generateGetters(io::sstream& out, ShaderStage stage) {
    out << SHADERS_COMMON_GETTERS_GLSL_DATA;
    switch (stage) {
        case ShaderStage::VERTEX:
            out << SHADERS_GETTERS_VS_DATA;
            break;
        case ShaderStage::FRAGMENT:
            out << SHADERS_GETTERS_FS_DATA;
            break;
        case ShaderStage::COMPUTE:
            out << SHADERS_GETTERS_CS_DATA;
            break;
    }
    return out;
}

io::sstream& CodeGenerator::generateParameters(io::sstream& out, ShaderStage type) {
    if (type == ShaderStage::VERTEX) {
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_SHADING_PARAMETERS_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateShaderLit(io::sstream& out, ShaderStage type,
        filament::Variant variant, Shading shading, bool customSurfaceShading) {
    if (type == ShaderStage::VERTEX) {
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_COMMON_LIGHTING_FS_DATA;
        if (filament::Variant::isShadowReceiverVariant(variant)) {
            out << SHADERS_SHADOWING_FS_DATA;
        }

        // the only reason we have this assert here is that we used to have a check,
        // which seemed unnecessary.
        assert_invariant(shading != Shading::UNLIT);

        out << SHADERS_BRDF_FS_DATA;
        switch (shading) {
            case Shading::UNLIT:
                // can't happen
                break;
            case Shading::SPECULAR_GLOSSINESS:
            case Shading::LIT:
                if (customSurfaceShading) {
                    out << SHADERS_SHADING_LIT_CUSTOM_FS_DATA;
                } else {
                    out << SHADERS_SHADING_MODEL_STANDARD_FS_DATA;
                }
                break;
            case Shading::SUBSURFACE:
                out << SHADERS_SHADING_MODEL_SUBSURFACE_FS_DATA;
                break;
            case Shading::CLOTH:
                out << SHADERS_SHADING_MODEL_CLOTH_FS_DATA;
                break;
        }

        out << SHADERS_AMBIENT_OCCLUSION_FS_DATA;
        out << SHADERS_LIGHT_INDIRECT_FS_DATA;

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

io::sstream& CodeGenerator::generateShaderUnlit(io::sstream& out, ShaderStage type,
        filament::Variant variant, bool hasShadowMultiplier) {
    if (type == ShaderStage::VERTEX) {
    } else if (type == ShaderStage::FRAGMENT) {
        if (hasShadowMultiplier) {
            if (filament::Variant::isShadowReceiverVariant(variant)) {
                out << SHADERS_SHADOWING_FS_DATA;
            }
        }
        out << SHADERS_SHADING_UNLIT_FS_DATA;
    }
    return out;
}

io::sstream& CodeGenerator::generateShaderReflections(utils::io::sstream& out, ShaderStage type,
        filament::Variant variant) {
    if (type == ShaderStage::VERTEX) {
    } else if (type == ShaderStage::FRAGMENT) {
        out << SHADERS_COMMON_LIGHTING_FS_DATA;
        out << SHADERS_LIGHT_REFLECTIONS_FS_DATA;
        out << SHADERS_SHADING_REFLECTIONS_FS_DATA;
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
        case Property::SHEEN_ROUGHNESS:      return "SHEEN_ROUGHNESS";
        case Property::GLOSSINESS:           return "GLOSSINESS";
        case Property::SPECULAR_COLOR:       return "SPECULAR_COLOR";
        case Property::EMISSIVE:             return "EMISSIVE";
        case Property::NORMAL:               return "NORMAL";
        case Property::POST_LIGHTING_COLOR:  return "POST_LIGHTING_COLOR";
        case Property::CLIP_SPACE_TRANSFORM: return "CLIP_SPACE_TRANSFORM";
        case Property::ABSORPTION:           return "ABSORPTION";
        case Property::TRANSMISSION:         return "TRANSMISSION";
        case Property::IOR:                  return "IOR";
        case Property::MICRO_THICKNESS:      return "MICRO_THICKNESS";
        case Property::BENT_NORMAL:          return "BENT_NORMAL";
    }
}

char const* CodeGenerator::getUniformTypeName(BufferInterfaceBlock::FieldInfo const& info) noexcept {
    using Type = BufferInterfaceBlock::Type;
    switch (info.type) {
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
        case Type::STRUCT: return info.structName.c_str();
    }
}

char const* CodeGenerator::getOutputTypeName(MaterialBuilder::OutputType type) noexcept {
    switch (type) {
        case MaterialBuilder::OutputType::FLOAT:  return "float";
        case MaterialBuilder::OutputType::FLOAT2: return "float2";
        case MaterialBuilder::OutputType::FLOAT3: return "float3";
        case MaterialBuilder::OutputType::FLOAT4: return "float4";
    }
}

char const* CodeGenerator::getSamplerTypeName(SamplerType type, SamplerFormat format,
        bool multisample) const noexcept {
    assert(!multisample);   // multisample samplers not yet supported.
    switch (type) {
        case SamplerType::SAMPLER_2D:
            switch (format) {
                case SamplerFormat::INT:    return "isampler2D";
                case SamplerFormat::UINT:   return "usampler2D";
                case SamplerFormat::FLOAT:  return "sampler2D";
                case SamplerFormat::SHADOW: return "sampler2DShadow";
            }
        case SamplerType::SAMPLER_3D:
            assert(format != SamplerFormat::SHADOW);
            switch (format) {
                case SamplerFormat::INT:    return "isampler3D";
                case SamplerFormat::UINT:   return "usampler3D";
                case SamplerFormat::FLOAT:  return "sampler3D";
                case SamplerFormat::SHADOW: return nullptr;
            }
        case SamplerType::SAMPLER_2D_ARRAY:
            switch (format) {
                case SamplerFormat::INT:    return "isampler2DArray";
                case SamplerFormat::UINT:   return "usampler2DArray";
                case SamplerFormat::FLOAT:  return "sampler2DArray";
                case SamplerFormat::SHADOW: return "sampler2DArrayShadow";
            }
        case SamplerType::SAMPLER_CUBEMAP:
            switch (format) {
                case SamplerFormat::INT:    return "isamplerCube";
                case SamplerFormat::UINT:   return "usamplerCube";
                case SamplerFormat::FLOAT:  return "samplerCube";
                case SamplerFormat::SHADOW: return "samplerCubeShadow";
            }
        case SamplerType::SAMPLER_EXTERNAL:
            assert(format != SamplerFormat::SHADOW);
            // Vulkan doesn't have external textures in the sense as GL. Vulkan external textures
            // are created via VK_ANDROID_external_memory_android_hardware_buffer, but they are
            // backed by VkImage just like a normal texture, and sampled from normally.
            return (mTargetLanguage == TargetLanguage::SPIRV) ? "sampler2D" : "samplerExternalOES";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            switch (format) {
                case SamplerFormat::INT:    return "isamplerCubeArray";
                case SamplerFormat::UINT:   return "usamplerCubeArray";
                case SamplerFormat::FLOAT:  return "samplerCubeArray";
                case SamplerFormat::SHADOW: return "samplerCubeArrayShadow";
            }
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
bool CodeGenerator::hasPrecision(BufferInterfaceBlock::Type type) noexcept {
    switch (type) {
        case UniformType::BOOL:
        case UniformType::BOOL2:
        case UniformType::BOOL3:
        case UniformType::BOOL4:
        case UniformType::STRUCT:
            return false;
        default:
            return true;
    }
}

} // namespace filamat
