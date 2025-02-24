//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompiledShaderState.cpp:
//   Implements CompiledShaderState, and helper functions for serializing and deserializing
//   shader variables.
//

#include "common/CompiledShaderState.h"

#include "common/BinaryStream.h"
#include "common/utilities.h"

namespace gl
{
namespace
{
template <typename VarT>
std::vector<VarT> GetActiveShaderVariables(const std::vector<VarT> *variableList)
{
    ASSERT(variableList);
    std::vector<VarT> result;
    for (size_t varIndex = 0; varIndex < variableList->size(); varIndex++)
    {
        const VarT &var = variableList->at(varIndex);
        if (var.active)
        {
            result.push_back(var);
        }
    }
    return result;
}

template <typename VarT>
const std::vector<VarT> &GetShaderVariables(const std::vector<VarT> *variableList)
{
    ASSERT(variableList);
    return *variableList;
}
}  // namespace

// true if varying x has a higher priority in packing than y
bool CompareShaderVar(const sh::ShaderVariable &x, const sh::ShaderVariable &y)
{
    if (x.type == y.type)
    {
        return x.getArraySizeProduct() > y.getArraySizeProduct();
    }

    // Special case for handling structs: we sort these to the end of the list
    if (x.type == GL_NONE)
    {
        return false;
    }

    if (y.type == GL_NONE)
    {
        return true;
    }

    return gl::VariableSortOrder(x.type) < gl::VariableSortOrder(y.type);
}

void WriteShaderVar(gl::BinaryOutputStream *stream, const sh::ShaderVariable &var)
{
    stream->writeInt(var.type);
    stream->writeInt(var.precision);
    stream->writeString(var.name);
    stream->writeString(var.mappedName);
    stream->writeVector(var.arraySizes);
    stream->writeBool(var.staticUse);
    stream->writeBool(var.active);
    stream->writeInt<size_t>(var.fields.size());
    for (const sh::ShaderVariable &shaderVariable : var.fields)
    {
        WriteShaderVar(stream, shaderVariable);
    }
    stream->writeString(var.structOrBlockName);
    stream->writeString(var.mappedStructOrBlockName);
    stream->writeBool(var.isRowMajorLayout);
    stream->writeInt(var.location);
    stream->writeBool(var.hasImplicitLocation);
    stream->writeInt(var.binding);
    stream->writeInt(var.imageUnitFormat);
    stream->writeInt(var.offset);
    stream->writeBool(var.rasterOrdered);
    stream->writeBool(var.readonly);
    stream->writeBool(var.writeonly);
    stream->writeBool(var.isFragmentInOut);
    stream->writeInt(var.index);
    stream->writeBool(var.yuv);
    stream->writeEnum(var.interpolation);
    stream->writeBool(var.isInvariant);
    stream->writeBool(var.isShaderIOBlock);
    stream->writeBool(var.isPatch);
    stream->writeBool(var.texelFetchStaticUse);
    stream->writeInt(var.getFlattenedOffsetInParentArrays());
    stream->writeInt(var.id);
}

void LoadShaderVar(gl::BinaryInputStream *stream, sh::ShaderVariable *var)
{
    var->type      = stream->readInt<GLenum>();
    var->precision = stream->readInt<GLenum>();
    stream->readString(&var->name);
    stream->readString(&var->mappedName);
    stream->readVector(&var->arraySizes);
    var->staticUse      = stream->readBool();
    var->active         = stream->readBool();
    size_t elementCount = stream->readInt<size_t>();
    var->fields.resize(elementCount);
    for (sh::ShaderVariable &variable : var->fields)
    {
        LoadShaderVar(stream, &variable);
    }
    stream->readString(&var->structOrBlockName);
    stream->readString(&var->mappedStructOrBlockName);
    var->isRowMajorLayout    = stream->readBool();
    var->location            = stream->readInt<int>();
    var->hasImplicitLocation = stream->readBool();
    var->binding             = stream->readInt<int>();
    var->imageUnitFormat     = stream->readInt<GLenum>();
    var->offset              = stream->readInt<int>();
    var->rasterOrdered       = stream->readBool();
    var->readonly            = stream->readBool();
    var->writeonly           = stream->readBool();
    var->isFragmentInOut     = stream->readBool();
    var->index               = stream->readInt<int>();
    var->yuv                 = stream->readBool();
    var->interpolation       = stream->readEnum<sh::InterpolationType>();
    var->isInvariant         = stream->readBool();
    var->isShaderIOBlock     = stream->readBool();
    var->isPatch             = stream->readBool();
    var->texelFetchStaticUse = stream->readBool();
    var->setParentArrayIndex(stream->readInt<int>());
    var->id = stream->readInt<uint32_t>();
}

void WriteShInterfaceBlock(gl::BinaryOutputStream *stream, const sh::InterfaceBlock &block)
{
    stream->writeString(block.name);
    stream->writeString(block.mappedName);
    stream->writeString(block.instanceName);
    stream->writeInt(block.arraySize);
    stream->writeEnum(block.layout);
    stream->writeBool(block.isRowMajorLayout);
    stream->writeInt(block.binding);
    stream->writeBool(block.staticUse);
    stream->writeBool(block.active);
    stream->writeEnum(block.blockType);
    stream->writeInt(block.id);

    stream->writeInt<size_t>(block.fields.size());
    for (const sh::ShaderVariable &shaderVariable : block.fields)
    {
        WriteShaderVar(stream, shaderVariable);
    }
}

void LoadShInterfaceBlock(gl::BinaryInputStream *stream, sh::InterfaceBlock *block)
{
    block->name             = stream->readString();
    block->mappedName       = stream->readString();
    block->instanceName     = stream->readString();
    block->arraySize        = stream->readInt<unsigned int>();
    block->layout           = stream->readEnum<sh::BlockLayoutType>();
    block->isRowMajorLayout = stream->readBool();
    block->binding          = stream->readInt<int>();
    block->staticUse        = stream->readBool();
    block->active           = stream->readBool();
    block->blockType        = stream->readEnum<sh::BlockType>();
    block->id               = stream->readInt<uint32_t>();

    block->fields.resize(stream->readInt<size_t>());
    for (sh::ShaderVariable &variable : block->fields)
    {
        LoadShaderVar(stream, &variable);
    }
}

CompiledShaderState::CompiledShaderState(gl::ShaderType type)
    : shaderType(type),
      shaderVersion(100),
      numViews(-1),
      geometryShaderInputPrimitiveType(gl::PrimitiveMode::Triangles),
      geometryShaderOutputPrimitiveType(gl::PrimitiveMode::Triangles),
      geometryShaderMaxVertices(0),
      geometryShaderInvocations(1),
      tessControlShaderVertices(0),
      tessGenMode(0),
      tessGenSpacing(0),
      tessGenVertexOrder(0),
      tessGenPointMode(0)
{
    localSize.fill(-1);
}

CompiledShaderState::~CompiledShaderState() {}

void CompiledShaderState::buildCompiledShaderState(const ShHandle compilerHandle,
                                                   const bool isBinaryOutput)
{
    if (isBinaryOutput)
    {
        compiledBinary = sh::GetObjectBinaryBlob(compilerHandle);
    }
    else
    {
        translatedSource = sh::GetObjectCode(compilerHandle);
    }

    // Gather the shader information
    shaderVersion = sh::GetShaderVersion(compilerHandle);

    uniforms            = GetShaderVariables(sh::GetUniforms(compilerHandle));
    uniformBlocks       = GetShaderVariables(sh::GetUniformBlocks(compilerHandle));
    shaderStorageBlocks = GetShaderVariables(sh::GetShaderStorageBlocks(compilerHandle));
    metadataFlags       = sh::CompilerMetadataFlags(sh::GetMetadataFlags(compilerHandle));
    specConstUsageBits  = SpecConstUsageBits(sh::GetShaderSpecConstUsageBits(compilerHandle));

    switch (shaderType)
    {
        case gl::ShaderType::Compute:
        {
            allAttributes    = GetShaderVariables(sh::GetAttributes(compilerHandle));
            activeAttributes = GetActiveShaderVariables(&allAttributes);
            localSize        = sh::GetComputeShaderLocalGroupSize(compilerHandle);
            break;
        }
        case gl::ShaderType::Vertex:
        {
            outputVaryings   = GetShaderVariables(sh::GetOutputVaryings(compilerHandle));
            allAttributes    = GetShaderVariables(sh::GetAttributes(compilerHandle));
            activeAttributes = GetActiveShaderVariables(&allAttributes);
            numViews         = sh::GetVertexShaderNumViews(compilerHandle);
            break;
        }
        case gl::ShaderType::Fragment:
        {
            allAttributes    = GetShaderVariables(sh::GetAttributes(compilerHandle));
            activeAttributes = GetActiveShaderVariables(&allAttributes);
            inputVaryings    = GetShaderVariables(sh::GetInputVaryings(compilerHandle));
            // TODO(jmadill): Figure out why we only sort in the FS, and if we need to.
            std::sort(inputVaryings.begin(), inputVaryings.end(), CompareShaderVar);
            activeOutputVariables =
                GetActiveShaderVariables(sh::GetOutputVariables(compilerHandle));
            advancedBlendEquations =
                gl::BlendEquationBitSet(sh::GetAdvancedBlendEquations(compilerHandle));
            pixelLocalStorageFormats = *sh::GetPixelLocalStorageFormats(compilerHandle);
            break;
        }
        case gl::ShaderType::Geometry:
        {
            inputVaryings  = GetShaderVariables(sh::GetInputVaryings(compilerHandle));
            outputVaryings = GetShaderVariables(sh::GetOutputVaryings(compilerHandle));

            if (metadataFlags[sh::MetadataFlags::HasValidGeometryShaderInputPrimitiveType])
            {
                geometryShaderInputPrimitiveType = gl::FromGLenum<gl::PrimitiveMode>(
                    sh::GetGeometryShaderInputPrimitiveType(compilerHandle));
            }
            if (metadataFlags[sh::MetadataFlags::HasValidGeometryShaderOutputPrimitiveType])
            {
                geometryShaderOutputPrimitiveType = gl::FromGLenum<gl::PrimitiveMode>(
                    sh::GetGeometryShaderOutputPrimitiveType(compilerHandle));
            }
            if (metadataFlags[sh::MetadataFlags::HasValidGeometryShaderMaxVertices])
            {
                geometryShaderMaxVertices = sh::GetGeometryShaderMaxVertices(compilerHandle);
            }
            geometryShaderInvocations = sh::GetGeometryShaderInvocations(compilerHandle);
            break;
        }
        case gl::ShaderType::TessControl:
        {
            inputVaryings             = GetShaderVariables(sh::GetInputVaryings(compilerHandle));
            outputVaryings            = GetShaderVariables(sh::GetOutputVaryings(compilerHandle));
            tessControlShaderVertices = sh::GetTessControlShaderVertices(compilerHandle);
            break;
        }
        case gl::ShaderType::TessEvaluation:
        {
            inputVaryings  = GetShaderVariables(sh::GetInputVaryings(compilerHandle));
            outputVaryings = GetShaderVariables(sh::GetOutputVaryings(compilerHandle));
            if (metadataFlags[sh::MetadataFlags::HasValidTessGenMode])
            {
                tessGenMode = sh::GetTessGenMode(compilerHandle);
            }
            if (metadataFlags[sh::MetadataFlags::HasValidTessGenSpacing])
            {
                tessGenSpacing = sh::GetTessGenSpacing(compilerHandle);
            }
            if (metadataFlags[sh::MetadataFlags::HasValidTessGenVertexOrder])
            {
                tessGenVertexOrder = sh::GetTessGenVertexOrder(compilerHandle);
            }
            if (metadataFlags[sh::MetadataFlags::HasValidTessGenPointMode])
            {
                tessGenPointMode = sh::GetTessGenPointMode(compilerHandle);
            }
            break;
        }

        default:
            UNREACHABLE();
    }
}

void CompiledShaderState::serialize(gl::BinaryOutputStream &stream) const
{
    stream.writeInt(shaderVersion);

    stream.writeInt(uniforms.size());
    for (const sh::ShaderVariable &shaderVariable : uniforms)
    {
        WriteShaderVar(&stream, shaderVariable);
    }

    stream.writeInt(uniformBlocks.size());
    for (const sh::InterfaceBlock &interfaceBlock : uniformBlocks)
    {
        WriteShInterfaceBlock(&stream, interfaceBlock);
    }

    stream.writeInt(shaderStorageBlocks.size());
    for (const sh::InterfaceBlock &interfaceBlock : shaderStorageBlocks)
    {
        WriteShInterfaceBlock(&stream, interfaceBlock);
    }

    stream.writeInt(metadataFlags.bits());
    stream.writeInt(specConstUsageBits.bits());

    switch (shaderType)
    {
        case gl::ShaderType::Compute:
        {
            stream.writeInt(allAttributes.size());
            for (const sh::ShaderVariable &shaderVariable : allAttributes)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(activeAttributes.size());
            for (const sh::ShaderVariable &shaderVariable : activeAttributes)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(localSize[0]);
            stream.writeInt(localSize[1]);
            stream.writeInt(localSize[2]);
            break;
        }

        case gl::ShaderType::Vertex:
        {
            stream.writeInt(outputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(allAttributes.size());
            for (const sh::ShaderVariable &shaderVariable : allAttributes)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(activeAttributes.size());
            for (const sh::ShaderVariable &shaderVariable : activeAttributes)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(numViews);
            break;
        }
        case gl::ShaderType::Fragment:
        {
            stream.writeInt(inputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(activeOutputVariables.size());
            for (const sh::ShaderVariable &shaderVariable : activeOutputVariables)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(advancedBlendEquations.bits());
            stream.writeInt<size_t>(pixelLocalStorageFormats.size());
            stream.writeBytes(reinterpret_cast<const uint8_t *>(pixelLocalStorageFormats.data()),
                              pixelLocalStorageFormats.size());
            break;
        }
        case gl::ShaderType::Geometry:
        {
            stream.writeInt(inputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(outputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }

            {
                unsigned char value = static_cast<unsigned char>(geometryShaderInputPrimitiveType);
                stream.writeBytes(&value, 1);
            }
            {
                unsigned char value = static_cast<unsigned char>(geometryShaderOutputPrimitiveType);
                stream.writeBytes(&value, 1);
            }
            {
                int value = static_cast<int>(geometryShaderMaxVertices);
                stream.writeInt(value);
            }

            stream.writeInt(geometryShaderInvocations);
            break;
        }
        case gl::ShaderType::TessControl:
        {
            stream.writeInt(inputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(outputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(tessControlShaderVertices);
            break;
        }
        case gl::ShaderType::TessEvaluation:
        {
            unsigned int value;

            stream.writeInt(inputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }
            stream.writeInt(outputVaryings.size());
            for (const sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                WriteShaderVar(&stream, shaderVariable);
            }

            value = (unsigned int)(tessGenMode);
            stream.writeInt(value);

            value = (unsigned int)tessGenSpacing;
            stream.writeInt(value);

            value = (unsigned int)tessGenVertexOrder;
            stream.writeInt(value);

            value = (unsigned int)tessGenPointMode;
            stream.writeInt(value);
            break;
        }
        default:
            UNREACHABLE();
    }

    stream.writeString(translatedSource);
    stream.writeVector(compiledBinary);
}

void CompiledShaderState::deserialize(gl::BinaryInputStream &stream)
{
    stream.readInt(&shaderVersion);

    size_t size;
    size = stream.readInt<size_t>();
    uniforms.resize(size);
    for (sh::ShaderVariable &shaderVariable : uniforms)
    {
        LoadShaderVar(&stream, &shaderVariable);
    }

    size = stream.readInt<size_t>();
    uniformBlocks.resize(size);
    for (sh::InterfaceBlock &interfaceBlock : uniformBlocks)
    {
        LoadShInterfaceBlock(&stream, &interfaceBlock);
    }

    size = stream.readInt<size_t>();
    shaderStorageBlocks.resize(size);
    for (sh::InterfaceBlock &interfaceBlock : shaderStorageBlocks)
    {
        LoadShInterfaceBlock(&stream, &interfaceBlock);
    }

    metadataFlags      = sh::CompilerMetadataFlags(stream.readInt<uint32_t>());
    specConstUsageBits = SpecConstUsageBits(stream.readInt<uint32_t>());

    switch (shaderType)
    {
        case gl::ShaderType::Compute:
        {
            size = stream.readInt<size_t>();
            allAttributes.resize(size);
            for (sh::ShaderVariable &shaderVariable : allAttributes)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            size = stream.readInt<size_t>();
            activeAttributes.resize(size);
            for (sh::ShaderVariable &shaderVariable : activeAttributes)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            stream.readInt(&localSize[0]);
            stream.readInt(&localSize[1]);
            stream.readInt(&localSize[2]);
            break;
        }
        case gl::ShaderType::Vertex:
        {
            size = stream.readInt<size_t>();
            outputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            size = stream.readInt<size_t>();
            allAttributes.resize(size);
            for (sh::ShaderVariable &shaderVariable : allAttributes)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            size = stream.readInt<size_t>();
            activeAttributes.resize(size);
            for (sh::ShaderVariable &shaderVariable : activeAttributes)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            stream.readInt(&numViews);
            break;
        }
        case gl::ShaderType::Fragment:
        {
            size = stream.readInt<size_t>();
            inputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            size = stream.readInt<size_t>();
            activeOutputVariables.resize(size);
            for (sh::ShaderVariable &shaderVariable : activeOutputVariables)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            int advancedBlendEquationBits;
            stream.readInt(&advancedBlendEquationBits);
            advancedBlendEquations = gl::BlendEquationBitSet(advancedBlendEquationBits);
            pixelLocalStorageFormats.resize(stream.readInt<size_t>());
            stream.readBytes(reinterpret_cast<uint8_t *>(pixelLocalStorageFormats.data()),
                             pixelLocalStorageFormats.size());
            break;
        }
        case gl::ShaderType::Geometry:
        {
            size = stream.readInt<size_t>();
            inputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            size = stream.readInt<size_t>();
            outputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }

            {
                unsigned char value;
                stream.readBytes(&value, 1);
                geometryShaderInputPrimitiveType = static_cast<gl::PrimitiveMode>(value);
            }

            {
                unsigned char value;
                stream.readBytes(&value, 1);
                geometryShaderOutputPrimitiveType = static_cast<gl::PrimitiveMode>(value);
            }

            {
                int value;
                stream.readInt(&value);
                geometryShaderMaxVertices = static_cast<GLint>(value);
            }

            stream.readInt(&geometryShaderInvocations);
            break;
        }
        case gl::ShaderType::TessControl:
        {
            size = stream.readInt<size_t>();
            inputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            size = stream.readInt<size_t>();
            outputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            stream.readInt(&tessControlShaderVertices);
            break;
        }
        case gl::ShaderType::TessEvaluation:
        {
            unsigned int value;

            size = stream.readInt<size_t>();
            inputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : inputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }
            size = stream.readInt<size_t>();
            outputVaryings.resize(size);
            for (sh::ShaderVariable &shaderVariable : outputVaryings)
            {
                LoadShaderVar(&stream, &shaderVariable);
            }

            stream.readInt(&value);
            tessGenMode = (GLenum)value;

            stream.readInt(&value);
            tessGenSpacing = (GLenum)value;

            stream.readInt(&value);
            tessGenVertexOrder = (GLenum)value;

            stream.readInt(&value);
            tessGenPointMode = (GLenum)value;
            break;
        }
        default:
            UNREACHABLE();
    }

    stream.readString(&translatedSource);
    stream.readVector(&compiledBinary);
}
}  // namespace gl
