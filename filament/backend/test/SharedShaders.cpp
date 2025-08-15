/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "SharedShaders.h"

#include "Shader.h"
#include "absl/strings/str_format.h"
#include "gtest/gtest.h"

namespace test {

using namespace filament::backend;

namespace {

// A shader stored in pieces so that uniform declarations can be injected.
struct ShaderText {
    std::string mPrefix;
    std::string mBody;

    std::string withUniform(const std::string& uniformText) const {
        return absl::StrFormat("%s\n%s\n%s", mPrefix.c_str(), uniformText.c_str(), mBody.c_str());
    }
};

std::optional<ShaderText> GetGlslVertexShader(VertexShaderType type) {
    switch (type) {
        case VertexShaderType::Noop: {
            return ShaderText{
                    R"(
#version 450 core
layout(location = 0) in vec4 mesh_position;
)", R"(
void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL, Metal and WebGPU, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})" };
        }
        case VertexShaderType::Simple: {
            return ShaderText{
                    R"(
#version 450 core
layout(location = 0) in vec4 mesh_position;
)", R"(
void main() {
    gl_Position = vec4(
            mesh_position.xy * (params.scaleMinusOne.xy + 1.0) + params.offset.xy,
            params.scaleMinusOne.z + 1.0,
            1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL, Metal and WebGPU, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})" };
        }
        case VertexShaderType::Textured: {
            return ShaderText{
                    R"(
#version 450 core
layout(location = 0) in vec4 mesh_position;
layout(location = 0) out vec2 uv;
)", R"(
void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
    uv = (mesh_position.xy * 0.5 + 0.5);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL, Metal and WebGPU, clip space is Y-up.
    gl_Position.y = -gl_Position.y;
#endif
})" };
        }
        default:
            return std::nullopt;
    }
}

std::optional<ShaderText> GetGlslFragmentShader(FragmentShaderType type) {
    switch (type) {
        case FragmentShaderType::White: {
            return ShaderText{
                    R"(
#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
)", R"(
void main() {
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
})" };
        }
        case FragmentShaderType::SolidColored: {
            return ShaderText{
                    R"(
#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
)", R"(
void main() {
    fragColor = params.color;
})" };
        }
        case FragmentShaderType::Textured: {
            return ShaderText{
                    R"(
#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 uv;
)", R"(
void main() {
    fragColor = texture(test_tex, uv);
})" };
        }
        default:
            return std::nullopt;
    }
}

std::optional<std::string> GetGlslUniform(ShaderUniformType type) {
    switch (type) {
        case ShaderUniformType::None: {
            return "";
        }
        case ShaderUniformType::Simple: {
            return R"(
layout(binding = 0, set = 0) uniform Params {
    highp vec4 color;
    // Use scaleMinusOne instead of scale so that a 0 initialized value is a good default
    highp vec4 scaleMinusOne;
    highp vec4 offset;
} params;
)";
        }
        case ShaderUniformType::SimpleWithPadding: {
            return R"(
layout(binding = 0, set = 0) uniform Params {
    highp vec4 padding[4];  // offset of 64 bytes

    highp vec4 color;
    // Use scaleMinusOne instead of scale so that a 0 initialized value is a good default
    highp vec4 scaleMinusOne;
    highp vec4 offset;
} params;
)";
        }
        case ShaderUniformType::Sampler2D: {
            return R"(
layout(location = 0, set = 0) uniform sampler2D test_tex;
)";
        }
        case ShaderUniformType::ISampler2D: {
            return R"(
layout(location = 0, set = 0) uniform isampler2D test_tex;
)";
        }
        case ShaderUniformType::USampler2D: {
            return R"(
layout(location = 0, set = 0) uniform usampler2D test_tex;
)";
        }
        default:
            return std::nullopt;
    }
}

std::vector<UniformConfig> GetUniformConfig(ShaderUniformType type) {
    switch (type) {
        case ShaderUniformType::None: {
            return {};
        }
        case ShaderUniformType::Simple: {
            return {{ "Params" }};
        }
        case ShaderUniformType::SimpleWithPadding: {
            return {{ "Params" }};
        }
        case ShaderUniformType::Sampler2D: {
            filament::SamplerInterfaceBlock::SamplerInfo samplerInfo{
                    "backend_test", "test_tex", 0,
                    SamplerType::SAMPLER_2D, SamplerFormat::FLOAT, Precision::HIGH, false };
            return {{
                            "test_tex", DescriptorType::SAMPLER_2D_FLOAT, samplerInfo
                    }};
        }
        case ShaderUniformType::ISampler2D: {
            filament::SamplerInterfaceBlock::SamplerInfo samplerInfo{
                    "backend_test", "test_tex", 0,
                    SamplerType::SAMPLER_2D, SamplerFormat::INT, Precision::HIGH, false };
            return {{
                            "test_tex", DescriptorType::SAMPLER_2D_INT, samplerInfo
                    }};
        }
        case ShaderUniformType::USampler2D: {
            filament::SamplerInterfaceBlock::SamplerInfo samplerInfo{
                    "backend_test", "test_tex", 0,
                    SamplerType::SAMPLER_2D, SamplerFormat::UINT, Precision::HIGH, false };
            return {{
                            "test_tex", DescriptorType::SAMPLER_2D_INT, samplerInfo
                    }};
        }
        default:
            abort();
    }
}

ShaderLanguage getShaderLanguage(const Backend& backend) {
    switch (backend) {
        case Backend::METAL:
            return ShaderLanguage::MSL;
        case Backend::WEBGPU:
            return ShaderLanguage::WGSL;
        case Backend::VULKAN:
        case Backend::NOOP:
        case Backend::OPENGL:
        default: {
            return ShaderLanguage::GLSL;
        }
    }
}

} // namespace

Shader SharedShaders::makeShader(filament::backend::DriverApi& api, Cleanup& cleanup,
        ShaderRequest request) {
    std::optional<ShaderText> vertex;
    std::optional<ShaderText> fragment;
    std::optional<std::string> uniform;
    if (getShaderLanguage(BackendTest::sBackend) != ShaderLanguage::GLSL) {
        // TODO(b/422803382): If any shaders need backend/shader language specific shaders rather
        //  than transpiled versions of the GLSL shader, check here and create a shader with that
        //  config instead
    }
    vertex = GetGlslVertexShader(request.mVertexType);
    fragment = GetGlslFragmentShader(request.mFragmentType);
    uniform = GetGlslUniform(request.mUniformType);
    if (vertex.has_value() && fragment.has_value() && uniform.has_value()) {
        return Shader(api, cleanup,
                ShaderConfig{ .vertexLanguage = filament::backend::ShaderLanguage::ESSL3,
                    .vertexShader = vertex->withUniform(*uniform),
                    .fragmentLanguage = filament::backend::ShaderLanguage::ESSL3,
                    .fragmentShader = fragment->withUniform(*uniform),
                    .uniforms = GetUniformConfig(request.mUniformType) });
    }
    abort();
}

std::string SharedShaders::getVertexShaderText(VertexShaderType vertex, ShaderUniformType uniform) {
    std::optional<ShaderText> vertexText = GetGlslVertexShader(vertex);
    std::optional<std::string> uniformText = GetGlslUniform(uniform);
    if (!vertexText.has_value() || !uniformText.has_value()) {
        abort();
    }
    return vertexText->withUniform(*uniformText);
}

std::string SharedShaders::getFragmentShaderText(FragmentShaderType fragment,
        ShaderUniformType uniform) {
    std::optional<ShaderText> fragmentText = GetGlslFragmentShader(fragment);
    std::optional<std::string> uniformText = GetGlslUniform(uniform);
    if (!fragmentText.has_value() || !uniformText.has_value()) {
        abort();
    }
    return fragmentText->withUniform(*uniformText);
}

} // namespace test
