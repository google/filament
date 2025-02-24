//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompiledShaderState.h:
//   Defines a struct containing any data that is needed to build
//   a CompiledShaderState from a TCompiler.
//

#ifndef COMMON_COMPILEDSHADERSTATE_H_
#define COMMON_COMPILEDSHADERSTATE_H_

#include "common/BinaryStream.h"
#include "common/Optional.h"
#include "common/PackedEnums.h"

#include <GLSLANG/ShaderLang.h>
#include <GLSLANG/ShaderVars.h>

#include <memory>
#include <string>

namespace sh
{
struct BlockMemberInfo;

using CompilerMetadataFlags = angle::PackedEnumBitSet<sh::MetadataFlags, uint32_t>;
}  // namespace sh

namespace gl
{

// @todo this type is also defined in compiler/Compiler.h and libANGLE/renderer_utils.h. Move this
// to a single common definition?
using SpecConstUsageBits = angle::PackedEnumBitSet<sh::vk::SpecConstUsage, uint32_t>;

// Helper functions for serializing shader variables
void WriteShaderVar(gl::BinaryOutputStream *stream, const sh::ShaderVariable &var);
void LoadShaderVar(gl::BinaryInputStream *stream, sh::ShaderVariable *var);

void WriteShInterfaceBlock(gl::BinaryOutputStream *stream, const sh::InterfaceBlock &block);
void LoadShInterfaceBlock(gl::BinaryInputStream *stream, sh::InterfaceBlock *block);

bool CompareShaderVar(const sh::ShaderVariable &x, const sh::ShaderVariable &y);

struct CompiledShaderState
{
    CompiledShaderState(gl::ShaderType shaderType);
    ~CompiledShaderState();

    void buildCompiledShaderState(const ShHandle compilerHandle, const bool isBinaryOutput);

    void serialize(gl::BinaryOutputStream &stream) const;
    void deserialize(gl::BinaryInputStream &stream);

    bool hasValidGeometryShaderInputPrimitiveType() const
    {
        return metadataFlags[sh::MetadataFlags::HasValidGeometryShaderInputPrimitiveType];
    }
    bool hasValidGeometryShaderOutputPrimitiveType() const
    {
        return metadataFlags[sh::MetadataFlags::HasValidGeometryShaderOutputPrimitiveType];
    }
    bool hasValidGeometryShaderMaxVertices() const
    {
        return metadataFlags[sh::MetadataFlags::HasValidGeometryShaderMaxVertices];
    }

    const gl::ShaderType shaderType;

    int shaderVersion;
    std::string translatedSource;
    sh::BinaryBlob compiledBinary;
    sh::WorkGroupSize localSize;

    std::vector<sh::ShaderVariable> inputVaryings;
    std::vector<sh::ShaderVariable> outputVaryings;
    std::vector<sh::ShaderVariable> uniforms;
    std::vector<sh::InterfaceBlock> uniformBlocks;
    std::vector<sh::InterfaceBlock> shaderStorageBlocks;
    std::vector<sh::ShaderVariable> allAttributes;
    std::vector<sh::ShaderVariable> activeAttributes;
    std::vector<sh::ShaderVariable> activeOutputVariables;

    sh::CompilerMetadataFlags metadataFlags;
    gl::BlendEquationBitSet advancedBlendEquations;
    SpecConstUsageBits specConstUsageBits;

    // GL_OVR_multiview / GL_OVR_multiview2
    int numViews;

    // Geometry Shader
    gl::PrimitiveMode geometryShaderInputPrimitiveType;
    gl::PrimitiveMode geometryShaderOutputPrimitiveType;
    GLint geometryShaderMaxVertices;
    int geometryShaderInvocations;

    // Tessellation Shader
    int tessControlShaderVertices;
    GLenum tessGenMode;
    GLenum tessGenSpacing;
    GLenum tessGenVertexOrder;
    GLenum tessGenPointMode;

    // ANGLE_shader_pixel_local_storage: A mapping from binding index to the PLS uniform format at
    // that index.
    std::vector<ShPixelLocalStorageFormat> pixelLocalStorageFormats;
};

using SharedCompiledShaderState = std::shared_ptr<CompiledShaderState>;
}  // namespace gl

#endif  // COMMON_COMPILEDSHADERSTATE_H_
