/*
 * Copyright (C) 2025 The Android Open Source Project
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


/*
 * NOTE: This file is meant to be included with #include in WebGPUBlitter.cpp
 * It is separated out for clarity.
 */

namespace {
// note that the placeholders below must start and end with this prefix and suffix and should not
// otherwise be present in the template:
constexpr std::string_view PLACEHOLDER_PREFIX{ "{{" };
constexpr std::string_view PLACEHOLDER_SUFFIX{ "}}" };

#define PLACEHOLDER_DEF(name) constexpr std::string_view name = #name

PLACEHOLDER_DEF(FRAGMENT_SHADER_SNIPPET);
PLACEHOLDER_DEF(TEXTURE_SAMPLE_IMPL);
PLACEHOLDER_DEF(TEXTURE_SAMPLE_IMPL_MAIN);

// texture_multisampled_2d<f32>
// texture_2d<f32/i32/u32>
// texture_3d<f32/i32/u32>
// texture_depth_2d
// texture_depth_multisampled_2d
PLACEHOLDER_DEF(TEXTURE_TYPE);

// vec2, vec3
PLACEHOLDER_DEF(VECTOR_DIM);

// vec_<f32>, vec_<u32>, vec_<i32>, f32
PLACEHOLDER_DEF(RET_TYPE);

// @builtin(frag_depth), @location(0)
PLACEHOLDER_DEF(RET_ATTRIBUTE);

// <f32>, <u32>, <i32>
PLACEHOLDER_DEF(DST_PRIM_TYPE);
PLACEHOLDER_DEF(SRC_PRIM_TYPE);

#undef PLACEHOLDER_DEF

constexpr std::string_view SHADER_SOURCE_TEMPLATE{ R"(
    struct BlitFragmentShaderArgs {
        depthPlane:        u32,
        scale:             vec2<f32>,
        sourceOffset:      vec2<u32>,
        destinationOffset: vec2<u32>,
    };

    @group(0) @binding(0) var sourceTexture: {{TEXTURE_TYPE}};
    @group(0) @binding(1) var<uniform> fragmentShaderArgs: BlitFragmentShaderArgs;

    fn getUnnormalizedSourceTextureCoordinates(position: vec2<f32>) -> vec2<f32> {
        // These coordinates match the Vulkan vkCmdBlitImage spec:
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdBlitImage.html
        let uvOffset: vec2<f32> = position - vec2<f32>(f32(fragmentShaderArgs.destinationOffset.x),f32(fragmentShaderArgs.destinationOffset.y));
        let uvScaled: vec2<f32> = uvOffset * fragmentShaderArgs.scale;
        return uvScaled + vec2<f32>(f32(fragmentShaderArgs.sourceOffset.x), f32(fragmentShaderArgs.sourceOffset.y));
    }

    fn normalize2dSourceTextureCoordinates(
            unnormalizedSourceTextureCoordinates: vec2<f32>,
            sourceDimensions: vec2<u32>) -> vec2<f32> {
        return unnormalizedSourceTextureCoordinates /
            vec2<f32>(f32(sourceDimensions.x), f32(sourceDimensions.y));
    }

    @vertex
    fn vertexShaderMain(@builtin(vertex_index) vertexIndex: u32) -> @builtin(position) vec4<f32> {
        let fullScreenTriangleVertices = array<vec2<f32>, 3>(
            vec2<f32>(-1.0, -1.0),
            vec2<f32>( 3.0, -1.0),
            vec2<f32>(-1.0,  3.0)
        );
        return vec4<f32>(fullScreenTriangleVertices[vertexIndex].xy, 0.0, 1.0);
    }

    {{FRAGMENT_SHADER_SNIPPET}}
)"
};

constexpr std::string_view FLOAT_TEXTURE_SAMPLE_IMPL_TEMPLATE{ R"(
        return textureSample(sourceTexture, sourceSampler, coordinates);
)" };

constexpr std::string_view INT_TEXTURE_SAMPLE_IMPL_TEMPLATE { R"(
        let texelCoords = {{VECTOR_DIM}}<u32>(coordinates * {{VECTOR_DIM}}<f32>(sourceDimensions));
        return textureLoad(sourceTexture, texelCoords, 0);
)"};

constexpr std::string_view TEXTURE_SAMPLE_IMPL_MAIN_TEMPLATE { R"(
    // Assumes that "@group(0) @binding(2) var sourceSampler: sampler;" has been declared in the main program;
    fn sampleTextureImpl(sourceTexture: {{TEXTURE_TYPE}}, sourceDimensions: {{VECTOR_DIM}}<u32>,
                  coordinates: {{VECTOR_DIM}}<f32>) -> vec4{{DST_PRIM_TYPE}} {
        {{TEXTURE_SAMPLE_IMPL}}
    }
)"};

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_MSAA_INPUT_TEMPLATE{ R"(
    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> {{RET_ATTRIBUTE}} {{RET_TYPE}} {
        var color: vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 0.0);
        let numberOfSamples: u32 = textureNumSamples(sourceTexture);
        let coordinatesF: vec2<f32> = getUnnormalizedSourceTextureCoordinates(position.xy);
        let coordinates: vec2<u32> = vec2<u32>(u32(coordinatesF.x), u32(coordinatesF.y));
        for (var sampleIndex: u32 = 0; sampleIndex < numberOfSamples; sampleIndex++) {
            color += textureLoad(sourceTexture, coordinates, sampleIndex);
        }
        color /= f32(numberOfSamples);
        return color;
    }
)" };

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_3D_INPUT_TEMPLATE{ R"(
    @group(0) @binding(2) var sourceSampler: sampler;

    {{TEXTURE_SAMPLE_IMPL_MAIN}}

    fn normalize3dSourceTextureCoordinates(
            unnormalizedSourceTextureCoordinates: vec2<f32>,
            sourceDimensions: vec3<u32>) -> vec3<f32> {
        let uvNormalized: vec2<f32> = normalize2dSourceTextureCoordinates(
            unnormalizedSourceTextureCoordinates,
            sourceDimensions.xy
        );
        return vec3<f32>(
            uvNormalized,
            (f32(fragmentShaderArgs.depthPlane) + 0.5) / f32(sourceDimensions.z)
        );
    }

    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> {{RET_ATTRIBUTE}} {{RET_TYPE}} {
        let sourceDimensions: vec3<u32> = textureDimensions(sourceTexture);
        let coordinates: vec3<f32> = normalize3dSourceTextureCoordinates(
            getUnnormalizedSourceTextureCoordinates(position),
            sourceDimensions
        );
        return sampleTextureImpl(sourceTexture, sourceDimensions, coordinates);
    }
)" };

constexpr std::string_view FRAGMENT_SHADER_SNIPPET_2D_INPUT_TEMPLATE { R"(
    @group(0) @binding(2) var sourceSampler: sampler;

    {{TEXTURE_SAMPLE_IMPL_MAIN}}

    @fragment
    fn fragmentShaderMain(@builtin(position) position: vec4<f32>) -> {{RET_ATTRIBUTE}} {{RET_TYPE}} {
        let sourceDimensions: vec2<u32> = textureDimensions(sourceTexture);
        let coordinates: vec2<f32> = normalize2dSourceTextureCoordinates(
            getUnnormalizedSourceTextureCoordinates(position.xy),
            sourceDimensions
        );
        return sampleTextureImpl(sourceTexture, sourceDimensions, coordinates);
    }
)" };

} // namespace
