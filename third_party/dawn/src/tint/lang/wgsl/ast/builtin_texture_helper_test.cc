// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/ast/builtin_texture_helper_test.h"

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

namespace tint::ast::test {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

StringStream& operator<<(StringStream& out, const TextureKind& kind) {
    switch (kind) {
        case TextureKind::kRegular:
            out << "regular";
            break;
        case TextureKind::kDepth:
            out << "depth";
            break;
        case TextureKind::kDepthMultisampled:
            out << "depth-multisampled";
            break;
        case TextureKind::kMultisampled:
            out << "multisampled";
            break;
        case TextureKind::kStorage:
            out << "storage";
            break;
    }
    return out;
}

StringStream& operator<<(StringStream& out, const TextureDataType& ty) {
    switch (ty) {
        case TextureDataType::kF32:
            out << "f32";
            break;
        case TextureDataType::kU32:
            out << "u32";
            break;
        case TextureDataType::kI32:
            out << "i32";
            break;
    }
    return out;
}

}  // namespace

TextureOverloadCase::TextureOverloadCase(ValidTextureOverload o,
                                         const char* desc,
                                         TextureKind tk,
                                         core::type::SamplerKind sk,
                                         core::type::TextureDimension dims,
                                         TextureDataType datatype,
                                         const char* f,
                                         std::function<Args(ProgramBuilder*)> a,
                                         bool ret_val)
    : overload(o),
      description(desc),
      texture_kind(tk),
      sampler_kind(sk),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)),
      returns_value(ret_val) {}
TextureOverloadCase::TextureOverloadCase(ValidTextureOverload o,
                                         const char* desc,
                                         TextureKind tk,
                                         core::type::TextureDimension dims,
                                         TextureDataType datatype,
                                         const char* f,
                                         std::function<Args(ProgramBuilder*)> a,
                                         bool ret_val)
    : overload(o),
      description(desc),
      texture_kind(tk),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)),
      returns_value(ret_val) {}
TextureOverloadCase::TextureOverloadCase(ValidTextureOverload o,
                                         const char* d,
                                         tint::core::Access acc,
                                         tint::core::TexelFormat fmt,
                                         core::type::TextureDimension dims,
                                         TextureDataType datatype,
                                         const char* f,
                                         std::function<Args(ProgramBuilder*)> a,
                                         bool ret_val)
    : overload(o),
      description(d),
      texture_kind(TextureKind::kStorage),
      access(acc),
      texel_format(fmt),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)),
      returns_value(ret_val) {}
TextureOverloadCase::TextureOverloadCase(const TextureOverloadCase&) = default;
TextureOverloadCase::~TextureOverloadCase() = default;

std::ostream& operator<<(std::ostream& out, const TextureOverloadCase& data) {
    StringStream str;
    str << "TextureOverloadCase " << static_cast<int>(data.overload) << "\n";
    str << data.description << "\n";
    str << "texture_kind:      " << data.texture_kind << "\n";
    str << "sampler_kind:      ";
    if (data.texture_kind != TextureKind::kStorage) {
        str << data.sampler_kind;
    } else {
        str << "<unused>";
    }
    str << "\n";
    str << "access:            " << data.access << "\n";
    str << "texel_format:      " << data.texel_format << "\n";
    str << "texture_dimension: " << data.texture_dimension << "\n";
    str << "texture_data_type: " << data.texture_data_type << "\n";

    out << str.str();
    return out;
}

Type TextureOverloadCase::BuildResultVectorComponentType(ProgramBuilder* b) const {
    switch (texture_data_type) {
        case test::TextureDataType::kF32:
            return b->ty.f32();
        case test::TextureDataType::kU32:
            return b->ty.u32();
        case test::TextureDataType::kI32:
            return b->ty.i32();
    }

    TINT_UNREACHABLE();
}

const Variable* TextureOverloadCase::BuildTextureVariable(ProgramBuilder* b) const {
    tint::Vector attrs{
        b->Group(0_u),
        b->Binding(0_a),
    };
    switch (texture_kind) {
        case test::TextureKind::kRegular:
            return b->GlobalVar(
                kTextureName,
                b->ty.sampled_texture(texture_dimension, BuildResultVectorComponentType(b)), attrs);

        case test::TextureKind::kDepth:
            return b->GlobalVar(kTextureName, b->ty.depth_texture(texture_dimension), attrs);

        case test::TextureKind::kDepthMultisampled:
            return b->GlobalVar(kTextureName, b->ty.depth_multisampled_texture(texture_dimension),
                                attrs);

        case test::TextureKind::kMultisampled:
            return b->GlobalVar(
                kTextureName,
                b->ty.multisampled_texture(texture_dimension, BuildResultVectorComponentType(b)),
                attrs);

        case test::TextureKind::kStorage: {
            auto st = b->ty.storage_texture(texture_dimension, texel_format, access);
            return b->GlobalVar(kTextureName, st, attrs);
        }
    }

    TINT_UNREACHABLE();
}

const Variable* TextureOverloadCase::BuildSamplerVariable(ProgramBuilder* b) const {
    tint::Vector attrs = {b->Group(0_a), b->Binding(1_a)};
    return b->GlobalVar(kSamplerName, b->ty.sampler(sampler_kind), attrs);
}

std::vector<TextureOverloadCase> TextureOverloadCase::ValidCases() {
    return {
        {
            ValidTextureOverload::kDimensions1d,
            "textureDimensions(t : texture_1d<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensions2d,
            "textureDimensions(t : texture_2d<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensions2dLevel,
            "textureDimensions(t     : texture_2d<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensions2dArray,
            "textureDimensions(t : texture_2d_array<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensions2dArrayLevel,
            "textureDimensions(t     : texture_2d_array<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensions3d,
            "textureDimensions(t : texture_3d<f32>) -> vec3<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensions3dLevel,
            "textureDimensions(t     : texture_3d<f32>,\n"
            "                  level : i32) -> vec3<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsCube,
            "textureDimensions(t : texture_cube<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsCubeLevel,
            "textureDimensions(t     : texture_cube<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsCubeArray,
            "textureDimensions(t : texture_cube_array<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsCubeArrayLevel,
            "textureDimensions(t     : texture_cube_array<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsMultisampled2d,
            "textureDimensions(t : texture_multisampled_2d<f32>)-> vec2<u32>",
            TextureKind::kMultisampled,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepth2d,
            "textureDimensions(t : texture_depth_2d) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepth2dLevel,
            "textureDimensions(t     : texture_depth_2d,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepth2dArray,
            "textureDimensions(t : texture_depth_2d_array) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepth2dArrayLevel,
            "textureDimensions(t     : texture_depth_2d_array,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepthCube,
            "textureDimensions(t : texture_depth_cube) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepthCubeLevel,
            "textureDimensions(t     : texture_depth_cube,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepthCubeArray,
            "textureDimensions(t : texture_depth_cube_array) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepthCubeArrayLevel,
            "textureDimensions(t     : texture_depth_cube_array,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsDepthMultisampled2d,
            "textureDimensions(t : texture_depth_multisampled_2d) -> vec2<u32>",
            TextureKind::kDepthMultisampled,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsStorageWO1d,
            "textureDimensions(t : texture_storage_1d<rgba32float>) -> u32",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsStorageWO2d,
            "textureDimensions(t : texture_storage_2d<rgba32float>) -> "
            "vec2<u32>",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsStorageWO2dArray,
            "textureDimensions(t : texture_storage_2d_array<rgba32float>) -> "
            "vec2<u32>",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kDimensionsStorageWO3d,
            "textureDimensions(t : texture_storage_3d<rgba32float>) -> "
            "vec3<u32>",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },

        {
            ValidTextureOverload::kGather2dF32,
            "textureGather(component : i32,\n"
            "              t         : texture_2d<T>,\n"
            "              s         : sampler,\n"
            "              coords    : vec2<f32>) -> vec4<T>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_i,                            // component
                                   kTextureName,                   // t
                                   kSamplerName,                   // s
                                   b->Call<vec2<f32>>(1_f, 2_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGather2dOffsetF32,
            "textureGather(component : u32,\n"
            "              t         : texture_2d<T>,\n"
            "              s         : sampler,\n"
            "              coords    : vec2<f32>,\n"
            "              offset    : vec2<i32>) -> vec4<T>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(3_i, 4_i);
                return b->ExprList(0_u,           // component
                                   kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGather2dArrayF32,
            "textureGather(component   : i32,\n"
            "              t           : texture_2d_array<T>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32) -> vec4<T>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_i,                           // component
                                   kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_i);                          // array index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGather2dArrayOffsetF32,
            "textureGather(component   : u32,\n"
            "              t           : texture_2d_array<T>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : u32,\n"
            "              offset      : vec2<i32>) -> vec4<T>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(0_u,           // component
                                   kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_u,  // array_index
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCubeF32,
            "textureGather(component : i32,\n"
            "              t         : texture_cube<T>,\n"
            "              s         : sampler,\n"
            "              coords    : vec3<f32>) -> vec4<T>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_i,                                 // component
                                   kTextureName,                        // t
                                   kSamplerName,                        // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCubeArrayF32,
            "textureGather(component   : u32,\n"
            "              t           : texture_cube_array<T>,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : u32) -> vec4<T>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_u,                                // component
                                   kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_u);                               // array_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherDepth2dF32,
            "textureGather(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                   // t
                                   kSamplerName,                   // s
                                   b->Call<vec2<f32>>(1_f, 2_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherDepth2dOffsetF32,
            "textureGather(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>,\n"
            "              offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(3_i, 4_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherDepth2dArrayF32,
            "textureGather(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : u32) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_u);                          // array_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherDepth2dArrayOffsetF32,
            "textureGather(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32,\n"
            "              offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // array_index
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherDepthCubeF32,
            "textureGather(t      : texture_depth_cube,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                        // t
                                   kSamplerName,                        // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherDepthCubeArrayF32,
            "textureGather(t           : texture_depth_cube_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : u32) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_u);                               // array_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dF32,
            "textureGatherCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_f);                          // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dOffsetF32,
            "textureGatherCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32,\n"
            "                     offset    : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_f,  // depth_ref
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dArrayF32,
            "textureGatherCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_i,                           // array_index
                                   4_f);                          // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32,
            "textureGatherCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32,\n"
            "                     offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(5_i, 6_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // array_index
                                   4_f,  // depth_ref
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCompareDepthCubeF32,
            "textureGatherCompare(t         : texture_depth_cube,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec3<f32>,\n"
            "                     depth_ref : f32) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_f);                               // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kGatherCompareDepthCubeArrayF32,
            "textureGatherCompare(t           : texture_depth_cube_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec3<f32>,\n"
            "                     array_index : u32,\n"
            "                     depth_ref   : f32) -> vec4<f32>",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_u,                                // array_index
                                   5_f);                               // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLayers2dArray,
            "textureNumLayers(t : texture_2d_array<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLayersCubeArray,
            "textureNumLayers(t : texture_cube_array<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLayersDepth2dArray,
            "textureNumLayers(t : texture_depth_2d_array) -> u32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLayersDepthCubeArray,
            "textureNumLayers(t : texture_depth_cube_array) -> u32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLayersStorageWO2dArray,
            "textureNumLayers(t : texture_storage_2d_array<rgba32float>) -> "
            "u32",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevels2d,
            "textureNumLevels(t : texture_2d<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevels2dArray,
            "textureNumLevels(t : texture_2d_array<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevels3d,
            "textureNumLevels(t : texture_3d<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevelsCube,
            "textureNumLevels(t : texture_cube<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevelsCubeArray,
            "textureNumLevels(t : texture_cube_array<f32>) -> u32",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevelsDepth2d,
            "textureNumLevels(t : texture_depth_2d) -> u32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevelsDepth2dArray,
            "textureNumLevels(t : texture_depth_2d_array) -> u32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevelsDepthCube,
            "textureNumLevels(t : texture_depth_cube) -> u32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumLevelsDepthCubeArray,
            "textureNumLevels(t : texture_depth_cube_array) -> u32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumSamplesMultisampled2d,
            "textureNumSamples(t : texture_multisampled_2d<f32>) -> u32",
            TextureKind::kMultisampled,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumSamples",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kNumSamplesDepthMultisampled2d,
            "textureNumSamples(t : texture_depth_multisampled_2d<f32>) -> u32",
            TextureKind::kMultisampled,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumSamples",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSample1dF32,
            "textureSample(t      : texture_1d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   1_f);          // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSample2dF32,
            "textureSample(t      : texture_2d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                   // t
                                   kSamplerName,                   // s
                                   b->Call<vec2<f32>>(1_f, 2_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSample2dOffsetF32,
            "textureSample(t      : texture_2d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>\n"
            "              offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(3_i, 4_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSample2dArrayF32,
            "textureSample(t           : texture_2d_array<f32>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_i);                          // array_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSample2dArrayOffsetF32,
            "textureSample(t           : texture_2d_array<f32>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : u32\n"
            "              offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_u,  // array_index
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSample3dF32,
            "textureSample(t      : texture_3d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                        // t
                                   kSamplerName,                        // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSample3dOffsetF32,
            "textureSample(t      : texture_3d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>\n"
            "              offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<f32>>(1_f, 2_f, 3_f);
                auto* offset = b->Call<vec3<i32>>(4_i, 5_i, 6_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCubeF32,
            "textureSample(t      : texture_cube<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                        // t
                                   kSamplerName,                        // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCubeArrayF32,
            "textureSample(t           : texture_cube_array<f32>,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : i32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_i);                               // array_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleDepth2dF32,
            "textureSample(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                   // t
                                   kSamplerName,                   // s
                                   b->Call<vec2<f32>>(1_f, 2_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleDepth2dOffsetF32,
            "textureSample(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>\n"
            "              offset : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(3_i, 4_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleDepth2dArrayF32,
            "textureSample(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_i);                          // array_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleDepth2dArrayOffsetF32,
            "textureSample(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32\n"
            "              offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // array_index
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleDepthCubeF32,
            "textureSample(t      : texture_depth_cube,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                        // t
                                   kSamplerName,                        // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f));  // coords
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleDepthCubeArrayF32,
            "textureSample(t           : texture_depth_cube_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : u32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_u);                               // array_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBias2dF32,
            "textureSampleBias(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>,\n"
            "                  bias   : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_f);                          // bias
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBias2dOffsetF32,
            "textureSampleBias(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>,\n"
            "                  bias   : f32,\n"
            "                  offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_f,  // bias
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBias2dArrayF32,
            "textureSampleBias(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : u32,\n"
            "                  bias        : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   4_u,                           // array_index
                                   3_f);                          // bias
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBias2dArrayOffsetF32,
            "textureSampleBias(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : i32,\n"
            "                  bias        : f32,\n"
            "                  offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(5_i, 6_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // array_index
                                   4_f,  // bias
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBias3dF32,
            "textureSampleBias(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  bias   : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_f);                               // bias
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBias3dOffsetF32,
            "textureSampleBias(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  bias   : f32,\n"
            "                  offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<f32>>(1_f, 2_f, 3_f);
                auto* offset = b->Call<vec3<i32>>(5_i, 6_i, 7_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   4_f,  // bias
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBiasCubeF32,
            "textureSampleBias(t      : texture_cube<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  bias   : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_f);                               // bias
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleBiasCubeArrayF32,
            "textureSampleBias(t           : texture_cube_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec3<f32>,\n"
            "                  array_index : i32,\n"
            "                  bias        : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   3_i,                                // array_index
                                   4_f);                               // bias
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevel2dF32,
            "textureSampleLevel(t      : texture_2d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_f);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevel2dOffsetF32,
            "textureSampleLevel(t      : texture_2d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : f32,\n"
            "                   offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_f,  // level
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevel2dArrayF32,
            "textureSampleLevel(t           : texture_2d_array<f32>,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_i,                           // array_index
                                   4_f);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevel2dArrayOffsetF32,
            "textureSampleLevel(t           : texture_2d_array<f32>,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : f32,\n"
            "                   offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(5_i, 6_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // array_index
                                   4_f,  // level
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevel3dF32,
            "textureSampleLevel(t      : texture_3d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_f);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevel3dOffsetF32,
            "textureSampleLevel(t      : texture_3d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : f32,\n"
            "                   offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<f32>>(1_f, 2_f, 3_f);
                auto* offset = b->Call<vec3<i32>>(5_i, 6_i, 7_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   4_f,  // level
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelCubeF32,
            "textureSampleLevel(t      : texture_cube<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_f);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelCubeArrayF32,
            "textureSampleLevel(t           : texture_cube_array<f32>,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec3<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : f32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_i,                                // array_index
                                   5_f);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dF32,
            "textureSampleLevel(t      : texture_depth_2d,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : u32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_u);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dOffsetF32,
            "textureSampleLevel(t      : texture_depth_2d,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : i32,\n"
            "                   offset : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // level
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dArrayF32,
            "textureSampleLevel(t           : texture_depth_2d_array,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : u32,\n"
            "                   level       : u32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_u,                           // array_index
                                   4_u);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32,
            "textureSampleLevel(t           : texture_depth_2d_array,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : u32,\n"
            "                   level       : u32,\n"
            "                   offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(5_i, 6_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_u,  // array_index
                                   4_u,  // level
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelDepthCubeF32,
            "textureSampleLevel(t      : texture_depth_cube,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : i32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_i);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleLevelDepthCubeArrayF32,
            "textureSampleLevel(t           : texture_depth_cube_array,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec3<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : i32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_i,                                // array_index
                                   5_i);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGrad2dF32,
            "textureSampleGrad(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>\n"
            "                  ddx    : vec2<f32>,\n"
            "                  ddy    : vec2<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* ddx = b->Call<vec2<f32>>(3_f, 4_f);
                auto* ddy = b->Call<vec2<f32>>(5_f, 6_f);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, ddx, ddy);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGrad2dOffsetF32,
            "textureSampleGrad(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>,\n"
            "                  ddx    : vec2<f32>,\n"
            "                  ddy    : vec2<f32>,\n"
            "                  offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* ddx = b->Call<vec2<f32>>(3_f, 4_f);
                auto* ddy = b->Call<vec2<f32>>(5_f, 6_f);
                auto* offset = b->Call<vec2<i32>>(7_i, 7_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, ddx, ddy, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGrad2dArrayF32,
            "textureSampleGrad(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : i32,\n"
            "                  ddx         : vec2<f32>,\n"
            "                  ddy         : vec2<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* ddx = b->Call<vec2<f32>>(4_f, 5_f);
                auto* ddy = b->Call<vec2<f32>>(6_f, 7_f);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // array_index
                                   ddx, ddy);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGrad2dArrayOffsetF32,
            "textureSampleGrad(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : u32,\n"
            "                  ddx         : vec2<f32>,\n"
            "                  ddy         : vec2<f32>,\n"
            "                  offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* ddx = b->Call<vec2<f32>>(4_f, 5_f);
                auto* ddy = b->Call<vec2<f32>>(6_f, 7_f);
                auto* offset = b->Call<vec2<i32>>(6_i, 7_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_u,  // array_index
                                   ddx, ddy, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGrad3dF32,
            "textureSampleGrad(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  ddx    : vec3<f32>,\n"
            "                  ddy    : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<f32>>(1_f, 2_f, 3_f);
                auto* ddx = b->Call<vec3<f32>>(4_f, 5_f, 6_f);
                auto* ddy = b->Call<vec3<f32>>(7_f, 8_f, 9_f);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, ddx, ddy);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGrad3dOffsetF32,
            "textureSampleGrad(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  ddx    : vec3<f32>,\n"
            "                  ddy    : vec3<f32>,\n"
            "                  offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<f32>>(1_f, 2_f, 3_f);
                auto* ddx = b->Call<vec3<f32>>(4_f, 5_f, 6_f);
                auto* ddy = b->Call<vec3<f32>>(7_f, 8_f, 9_f);
                auto* offset = b->Call<vec3<i32>>(0_i, 1_i, 2_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, ddx, ddy, offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGradCubeF32,
            "textureSampleGrad(t      : texture_cube<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  ddx    : vec3<f32>,\n"
            "                  ddy    : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<f32>>(1_f, 2_f, 3_f);
                auto* ddx = b->Call<vec3<f32>>(4_f, 5_f, 6_f);
                auto* ddy = b->Call<vec3<f32>>(7_f, 8_f, 9_f);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords, ddx, ddy);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleGradCubeArrayF32,
            "textureSampleGrad(t           : texture_cube_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec3<f32>,\n"
            "                  array_index : u32,\n"
            "                  ddx         : vec3<f32>,\n"
            "                  ddy         : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::SamplerKind::kSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<f32>>(1_f, 2_f, 3_f);
                auto* ddx = b->Call<vec3<f32>>(5_f, 6_f, 7_f);
                auto* ddy = b->Call<vec3<f32>>(8_f, 9_f, 10_f);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   4_u,  // array_index
                                   ddx, ddy);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dF32,
            "textureSampleCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_f);                          // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dOffsetF32,
            "textureSampleCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32,\n"
            "                     offset    : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_f,  // depth_ref
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dArrayF32,
            "textureSampleCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   4_i,                           // array_index
                                   3_f);                          // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32,
            "textureSampleCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : u32,\n"
            "                     depth_ref   : f32,\n"
            "                     offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(5_i, 6_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   4_u,  // array_index
                                   3_f,  // depth_ref
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareDepthCubeF32,
            "textureSampleCompare(t         : texture_depth_cube,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec3<f32>,\n"
            "                     depth_ref : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_f);                               // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareDepthCubeArrayF32,
            "textureSampleCompare(t           : texture_depth_cube_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec3<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_i,                                // array_index
                                   5_f);                               // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dF32,
            "textureSampleCompareLevel(t         : texture_depth_2d,\n"
            "                          s         : sampler_comparison,\n"
            "                          coords    : vec2<f32>,\n"
            "                          depth_ref : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_f);                          // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32,
            "textureSampleCompareLevel(t         : texture_depth_2d,\n"
            "                          s         : sampler_comparison,\n"
            "                          coords    : vec2<f32>,\n"
            "                          depth_ref : f32,\n"
            "                          offset    : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(4_i, 5_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_f,  // depth_ref
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32,
            "textureSampleCompareLevel(t           : texture_depth_2d_array,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec2<f32>,\n"
            "                          array_index : i32,\n"
            "                          depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->Call<vec2<f32>>(1_f, 2_f),  // coords
                                   3_i,                           // array_index
                                   4_f);                          // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32,
            "textureSampleCompareLevel(t           : texture_depth_2d_array,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec2<f32>,\n"
            "                          array_index : i32,\n"
            "                          depth_ref   : f32,\n"
            "                          offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<f32>>(1_f, 2_f);
                auto* offset = b->Call<vec2<i32>>(5_i, 6_i);
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   coords,
                                   3_i,  // array_index
                                   4_f,  // depth_ref
                                   offset);
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepthCubeF32,
            "textureSampleCompareLevel(t           : texture_depth_cube,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec3<f32>,\n"
            "                          depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_f);                               // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32,
            "textureSampleCompareLevel(t           : "
            "texture_depth_cube_array,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec3<f32>,\n"
            "                          array_index : i32,\n"
            "                          depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            core::type::SamplerKind::kComparisonSampler,
            core::type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   kSamplerName,                       // s
                                   b->Call<vec3<f32>>(1_f, 2_f, 3_f),  // coords
                                   4_i,                                // array_index
                                   5_f);                               // depth_ref
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad1dLevelF32,
            "textureLoad(t      : texture_1d<f32>,\n"
            "            coords : u32,\n"
            "            level  : u32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   1_u,           // coords
                                   3_u);          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad1dLevelU32,
            "textureLoad(t      : texture_1d<u32>,\n"
            "            coords : i32,\n"
            "            level  : i32) -> vec4<u32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k1d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   1_i,           // coords
                                   3_i);          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad1dLevelI32,
            "textureLoad(t      : texture_1d<i32>,\n"
            "            coords : i32,\n"
            "            level  : i32) -> vec4<i32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k1d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   1_i,           // coords
                                   3_i);          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad2dLevelF32,
            "textureLoad(t      : texture_2d<f32>,\n"
            "            coords : vec2<u32>,\n"
            "            level  : u32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<u32>>(1_u, 2_u),  // coords
                                   3_u);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad2dLevelU32,
            "textureLoad(t      : texture_2d<u32>,\n"
            "            coords : vec2<i32>,\n"
            "            level  : i32) -> vec4<u32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k2d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<i32>>(1_i, 2_i),  // coords
                                   3_i);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad2dLevelI32,
            "textureLoad(t      : texture_2d<i32>,\n"
            "            coords : vec2<u32>,\n"
            "            level  : u32) -> vec4<i32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k2d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<u32>>(1_u, 2_u),  // coords
                                   3_u);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad2dArrayLevelF32,
            "textureLoad(t           : texture_2d_array<f32>,\n"
            "            coords      : vec2<i32>,\n"
            "            array_index : i32,\n"
            "            level       : i32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<i32>>(1_i, 2_i),  // coords
                                   3_i,                           // array_index
                                   4_i);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad2dArrayLevelU32,
            "textureLoad(t           : texture_2d_array<u32>,\n"
            "            coords      : vec2<i32>,\n"
            "            array_index : i32,\n"
            "            level       : i32) -> vec4<u32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<i32>>(1_i, 2_i),  // coords
                                   3_i,                           // array_index
                                   4_i);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad2dArrayLevelI32,
            "textureLoad(t           : texture_2d_array<i32>,\n"
            "            coords      : vec2<u32>,\n"
            "            array_index : u32,\n"
            "            level       : u32) -> vec4<i32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<u32>>(1_u, 2_u),  // coords
                                   3_u,                           // array_index
                                   4_u);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad3dLevelF32,
            "textureLoad(t      : texture_3d<f32>,\n"
            "            coords : vec3<i32>,\n"
            "            level  : i32) -> vec4<f32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   b->Call<vec3<i32>>(1_i, 2_i, 3_i),  // coords
                                   4_i);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad3dLevelU32,
            "textureLoad(t      : texture_3d<u32>,\n"
            "            coords : vec3<i32>,\n"
            "            level  : i32) -> vec4<u32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k3d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   b->Call<vec3<i32>>(1_i, 2_i, 3_i),  // coords
                                   4_i);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoad3dLevelI32,
            "textureLoad(t      : texture_3d<i32>,\n"
            "            coords : vec3<u32>,\n"
            "            level  : u32) -> vec4<i32>",
            TextureKind::kRegular,
            core::type::TextureDimension::k3d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   b->Call<vec3<u32>>(1_u, 2_u, 3_u),  // coords
                                   4_u);                               // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoadMultisampled2dF32,
            "textureLoad(t            : texture_multisampled_2d<f32>,\n"
            "            coords       : vec2<i32>,\n"
            "            sample_index : i32) -> vec4<f32>",
            TextureKind::kMultisampled,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<i32>>(1_i, 2_i),  // coords
                                   3_i);                          // sample_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoadMultisampled2dU32,
            "textureLoad(t            : texture_multisampled_2d<u32>,\n"
            "            coords       : vec2<i32>,\n"
            "            sample_index : i32) -> vec4<u32>",
            TextureKind::kMultisampled,
            core::type::TextureDimension::k2d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<i32>>(1_i, 2_i),  // coords
                                   3_i);                          // sample_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoadMultisampled2dI32,
            "textureLoad(t            : texture_multisampled_2d<i32>,\n"
            "            coords       : vec2<u32>,\n"
            "            sample_index : u32) -> vec4<i32>",
            TextureKind::kMultisampled,
            core::type::TextureDimension::k2d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<u32>>(1_u, 2_u),  // coords
                                   3_u);                          // sample_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoadDepth2dLevelF32,
            "textureLoad(t      : texture_depth_2d,\n"
            "            coords : vec2<i32>,\n"
            "            level  : i32) -> f32",
            TextureKind::kDepth,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<i32>>(1_i, 2_i),  // coords
                                   3_i);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoadDepth2dArrayLevelF32,
            "textureLoad(t           : texture_depth_2d_array,\n"
            "            coords      : vec2<u32>,\n"
            "            array_index : u32,\n"
            "            level       : u32) -> f32",
            TextureKind::kDepth,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<u32>>(1_u, 2_u),  // coords
                                   3_u,                           // array_index
                                   4_u);                          // level
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kLoadDepthMultisampled2dF32,
            "textureLoad(t            : texture_depth_multisampled_2d,\n"
            "            coords       : vec2<u32>,\n"
            "            sample_index : u32) -> f32",
            TextureKind::kDepthMultisampled,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   b->Call<vec2<u32>>(1_u, 2_u),  // coords
                                   3_u);                          // sample_index
            },
            /* returns value */ true,
        },
        {
            ValidTextureOverload::kStoreWO1dRgba32float,
            "textureStore(t      : texture_storage_1d<rgba32float>,\n"
            "             coords : i32,\n"
            "             value  : vec4<T>)",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                             // t
                                   1_i,                                      // coords
                                   b->Call<vec4<f32>>(2_f, 3_f, 4_f, 5_f));  // value
            },
            /* returns value */ false,
        },
        {
            ValidTextureOverload::kStoreWO2dRgba32float,
            "textureStore(t      : texture_storage_2d<rgba32float>,\n"
            "             coords : vec2<i32>,\n"
            "             value  : vec4<T>)",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<i32>>(1_i, 2_i);
                auto* value = b->Call<vec4<f32>>(3_f, 4_f, 5_f, 6_f);
                return b->ExprList(kTextureName,  // t
                                   coords, value);
            },
            /* returns value */ false,
        },
        {
            ValidTextureOverload::kStoreWO2dArrayRgba32float,
            "textureStore(t           : "
            "texture_storage_2d_array<rgba32float>,\n"
            "             coords      : vec2<u32>,\n"
            "             array_index : u32,\n"
            "             value       : vec4<T>)",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec2<u32>>(1_u, 2_u);
                auto* value = b->Call<vec4<f32>>(4_f, 5_f, 6_f, 7_f);
                return b->ExprList(kTextureName,  // t
                                   coords,
                                   3_u,  // array_index
                                   value);
            },
            /* returns value */ false,
        },
        {
            ValidTextureOverload::kStoreWO3dRgba32float,
            "textureStore(t      : texture_storage_3d<rgba32float>,\n"
            "             coords : vec3<u32>,\n"
            "             value  : vec4<T>)",
            tint::core::Access::kWrite,
            tint::core::TexelFormat::kRgba32Float,
            core::type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                auto* coords = b->Call<vec3<u32>>(1_u, 2_u, 3_u);
                auto* value = b->Call<vec4<f32>>(4_f, 5_f, 6_f, 7_f);
                return b->ExprList(kTextureName,  // t
                                   coords, value);
            },
            /* returns value */ false,
        },
    };
}

bool ReturnsVoid(ValidTextureOverload texture_overload) {
    switch (texture_overload) {
        case ValidTextureOverload::kStoreWO1dRgba32float:
        case ValidTextureOverload::kStoreWO2dRgba32float:
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return true;
        default:
            return false;
    }
}

}  // namespace tint::ast::test
