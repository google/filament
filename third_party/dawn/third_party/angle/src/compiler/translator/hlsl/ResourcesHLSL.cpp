//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourcesHLSL.cpp:
//   Methods for GLSL to HLSL translation for uniforms and interface blocks.
//

#include "compiler/translator/hlsl/ResourcesHLSL.h"

#include "common/utilities.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/hlsl/AtomicCounterFunctionHLSL.h"
#include "compiler/translator/hlsl/StructureHLSL.h"
#include "compiler/translator/hlsl/UtilsHLSL.h"
#include "compiler/translator/hlsl/blocklayoutHLSL.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

constexpr const ImmutableString kAngleDecorString("angle_");

static const char *UniformRegisterPrefix(const TType &type)
{
    if (IsSampler(type.getBasicType()))
    {
        return "s";
    }
    else
    {
        return "c";
    }
}

static TString InterfaceBlockFieldTypeString(const TField &field,
                                             TLayoutBlockStorage blockStorage,
                                             bool usedStructuredbuffer)
{
    const TType &fieldType                   = *field.type();
    const TLayoutMatrixPacking matrixPacking = fieldType.getLayoutQualifier().matrixPacking;
    ASSERT(matrixPacking != EmpUnspecified);
    const TStructure *structure = fieldType.getStruct();

    if (fieldType.isMatrix())
    {
        // Use HLSL row-major packing for GLSL column-major matrices
        const TString &matrixPackString =
            (matrixPacking == EmpRowMajor ? "column_major" : "row_major");
        return matrixPackString + " " + TypeString(fieldType);
    }
    else if (structure)
    {
        // If uniform block's layout is std140 and translating it to StructuredBuffer,
        // should pack structure in the end, in order to fit API buffer.
        bool forcePackingEnd = usedStructuredbuffer && (blockStorage == EbsStd140);
        // Use HLSL row-major packing for GLSL column-major matrices
        return QualifiedStructNameString(*structure, matrixPacking == EmpColumnMajor,
                                         blockStorage == EbsStd140, forcePackingEnd);
    }
    else
    {
        return TypeString(fieldType);
    }
}

static TString InterfaceBlockStructName(const TInterfaceBlock &interfaceBlock)
{
    return DecoratePrivate(interfaceBlock.name()) + "_type";
}

void OutputUniformIndexArrayInitializer(TInfoSinkBase &out,
                                        const TType &type,
                                        unsigned int startIndex)
{
    out << "{";
    TType elementType(type);
    elementType.toArrayElementType();
    for (unsigned int i = 0u; i < type.getOutermostArraySize(); ++i)
    {
        if (i > 0u)
        {
            out << ", ";
        }
        if (elementType.isArray())
        {
            OutputUniformIndexArrayInitializer(out, elementType,
                                               startIndex + i * elementType.getArraySizeProduct());
        }
        else
        {
            out << (startIndex + i);
        }
    }
    out << "}";
}

static TString InterfaceBlockScalarVectorFieldPaddingString(const TType &type)
{
    switch (type.getBasicType())
    {
        case EbtFloat:
            switch (type.getNominalSize())
            {
                case 1:
                    return "float3 padding;";
                case 2:
                    return "float2 padding;";
                case 3:
                    return "float padding;";
                default:
                    break;
            }
            break;
        case EbtInt:
            switch (type.getNominalSize())
            {
                case 1:
                    return "int3 padding;";
                case 2:
                    return "int2 padding;";
                case 3:
                    return "int padding";
                default:
                    break;
            }
            break;
        case EbtUInt:
            switch (type.getNominalSize())
            {
                case 1:
                    return "uint3 padding;";
                case 2:
                    return "uint2 padding;";
                case 3:
                    return "uint padding;";
                default:
                    break;
            }
            break;
        case EbtBool:
            switch (type.getNominalSize())
            {
                case 1:
                    return "bool3 padding;";
                case 2:
                    return "bool2 padding;";
                case 3:
                    return "bool padding;";
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return "";
}

static bool IsAnyRasterOrdered(const TVector<const TVariable *> &imageVars)
{
    for (const TVariable *imageVar : imageVars)
    {
        if (imageVar->getType().getLayoutQualifier().rasterOrdered)
        {
            return true;
        }
    }
    return false;
}

}  // anonymous namespace

ResourcesHLSL::ResourcesHLSL(StructureHLSL *structureHLSL,
                             ShShaderOutput outputType,
                             const std::vector<ShaderVariable> &uniforms,
                             unsigned int firstUniformRegister)
    : mUniformRegister(firstUniformRegister),
      mUniformBlockRegister(0),
      mSRVRegister(0),
      mUAVRegister(0),
      mSamplerCount(0),
      mStructureHLSL(structureHLSL),
      mOutputType(outputType),
      mUniforms(uniforms)
{}

void ResourcesHLSL::reserveUniformRegisters(unsigned int registerCount)
{
    mUniformRegister = registerCount;
}

void ResourcesHLSL::reserveUniformBlockRegisters(unsigned int registerCount)
{
    mUniformBlockRegister = registerCount;
}

const ShaderVariable *ResourcesHLSL::findUniformByName(const ImmutableString &name) const
{
    for (size_t uniformIndex = 0; uniformIndex < mUniforms.size(); ++uniformIndex)
    {
        if (name == mUniforms[uniformIndex].name)
        {
            return &mUniforms[uniformIndex];
        }
    }

    return nullptr;
}

unsigned int ResourcesHLSL::assignUniformRegister(const TType &type,
                                                  const ImmutableString &name,
                                                  unsigned int *outRegisterCount)
{
    unsigned int registerIndex;
    const ShaderVariable *uniform = findUniformByName(name);
    ASSERT(uniform);

    if (IsSampler(type.getBasicType()) ||
        (IsImage(type.getBasicType()) && type.getMemoryQualifier().readonly))
    {
        registerIndex = mSRVRegister;
    }
    else if (IsImage(type.getBasicType()))
    {
        registerIndex = mUAVRegister;
    }
    else
    {
        registerIndex = mUniformRegister;
    }

    if (uniform->name == "angle_DrawID" && uniform->mappedName == "angle_DrawID")
    {
        mUniformRegisterMap["gl_DrawID"] = registerIndex;
    }
    else
    {
        mUniformRegisterMap[uniform->name] = registerIndex;
    }

    if (uniform->name == "angle_BaseVertex" && uniform->mappedName == "angle_BaseVertex")
    {
        mUniformRegisterMap["gl_BaseVertex"] = registerIndex;
    }
    else
    {
        mUniformRegisterMap[uniform->name] = registerIndex;
    }

    if (uniform->name == "angle_BaseInstance" && uniform->mappedName == "angle_BaseInstance")
    {
        mUniformRegisterMap["gl_BaseInstance"] = registerIndex;
    }
    else
    {
        mUniformRegisterMap[uniform->name] = registerIndex;
    }

    unsigned int registerCount = HLSLVariableRegisterCount(*uniform, mOutputType);

    if (IsSampler(type.getBasicType()) ||
        (IsImage(type.getBasicType()) && type.getMemoryQualifier().readonly))
    {
        mSRVRegister += registerCount;
    }
    else if (IsImage(type.getBasicType()))
    {
        mUAVRegister += registerCount;
    }
    else
    {
        mUniformRegister += registerCount;
    }
    if (outRegisterCount)
    {
        *outRegisterCount = registerCount;
    }
    return registerIndex;
}

unsigned int ResourcesHLSL::assignSamplerInStructUniformRegister(const TType &type,
                                                                 const TString &name,
                                                                 unsigned int *outRegisterCount)
{
    // Sampler that is a field of a uniform structure.
    ASSERT(IsSampler(type.getBasicType()));
    unsigned int registerIndex                     = mSRVRegister;
    mUniformRegisterMap[std::string(name.c_str())] = registerIndex;
    unsigned int registerCount = type.isArray() ? type.getArraySizeProduct() : 1u;
    mSRVRegister += registerCount;
    if (outRegisterCount)
    {
        *outRegisterCount = registerCount;
    }
    return registerIndex;
}

void ResourcesHLSL::outputHLSLSamplerUniformGroup(
    TInfoSinkBase &out,
    const HLSLTextureGroup textureGroup,
    const TVector<const TVariable *> &group,
    const TMap<const TVariable *, TString> &samplerInStructSymbolsToAPINames,
    unsigned int *groupTextureRegisterIndex)
{
    if (group.empty())
    {
        return;
    }
    unsigned int groupRegisterCount = 0;
    for (const TVariable *uniform : group)
    {
        const TType &type           = uniform->getType();
        const ImmutableString &name = uniform->name();
        unsigned int registerCount;

        // The uniform might be just a regular sampler or one extracted from a struct.
        unsigned int samplerArrayIndex      = 0u;
        const ShaderVariable *uniformByName = findUniformByName(name);
        if (uniformByName)
        {
            samplerArrayIndex = assignUniformRegister(type, name, &registerCount);
        }
        else
        {
            ASSERT(samplerInStructSymbolsToAPINames.find(uniform) !=
                   samplerInStructSymbolsToAPINames.end());
            samplerArrayIndex = assignSamplerInStructUniformRegister(
                type, samplerInStructSymbolsToAPINames.at(uniform), &registerCount);
        }
        groupRegisterCount += registerCount;

        if (type.isArray())
        {
            out << "static const uint " << DecorateVariableIfNeeded(*uniform) << ArrayString(type)
                << " = ";
            OutputUniformIndexArrayInitializer(out, type, samplerArrayIndex);
            out << ";\n";
        }
        else
        {
            out << "static const uint " << DecorateVariableIfNeeded(*uniform) << " = "
                << samplerArrayIndex << ";\n";
        }
    }
    TString suffix = TextureGroupSuffix(textureGroup);
    // Since HLSL_TEXTURE_2D is the first group, it has a fixed offset of zero.
    if (textureGroup != HLSL_TEXTURE_2D)
    {
        out << "static const uint textureIndexOffset" << suffix << " = "
            << (*groupTextureRegisterIndex) << ";\n";
        out << "static const uint samplerIndexOffset" << suffix << " = "
            << (*groupTextureRegisterIndex) << ";\n";
    }
    out << "uniform " << TextureString(textureGroup) << " textures" << suffix << "["
        << groupRegisterCount << "]" << " : register(t" << (*groupTextureRegisterIndex) << ");\n";
    out << "uniform " << SamplerString(textureGroup) << " samplers" << suffix << "["
        << groupRegisterCount << "]" << " : register(s" << (*groupTextureRegisterIndex) << ");\n";
    *groupTextureRegisterIndex += groupRegisterCount;
}

void ResourcesHLSL::outputHLSLImageUniformIndices(TInfoSinkBase &out,
                                                  const TVector<const TVariable *> &group,
                                                  unsigned int imageArrayIndex,
                                                  unsigned int *groupRegisterCount)
{
    for (const TVariable *uniform : group)
    {
        const TType &type           = uniform->getType();
        const ImmutableString &name = uniform->name();
        unsigned int registerCount  = 0;

        assignUniformRegister(type, name, &registerCount);
        *groupRegisterCount += registerCount;

        if (type.isArray())
        {
            out << "static const uint " << DecorateVariableIfNeeded(*uniform) << ArrayString(type)
                << " = ";
            OutputUniformIndexArrayInitializer(out, type, imageArrayIndex);
            out << ";\n";
        }
        else
        {
            out << "static const uint " << DecorateVariableIfNeeded(*uniform) << " = "
                << imageArrayIndex << ";\n";
        }

        imageArrayIndex += registerCount;
    }
}

void ResourcesHLSL::outputHLSLReadonlyImageUniformGroup(TInfoSinkBase &out,
                                                        const HLSLTextureGroup textureGroup,
                                                        const TVector<const TVariable *> &group,
                                                        unsigned int *groupTextureRegisterIndex)
{
    if (group.empty())
    {
        return;
    }

    unsigned int groupRegisterCount = 0;
    outputHLSLImageUniformIndices(out, group, *groupTextureRegisterIndex, &groupRegisterCount);

    TString suffix = TextureGroupSuffix(textureGroup);
    out << "static const uint readonlyImageIndexOffset" << suffix << " = "
        << (*groupTextureRegisterIndex) << ";\n";
    out << "uniform " << TextureString(textureGroup) << " readonlyImages" << suffix << "["
        << groupRegisterCount << "]" << " : register(t" << (*groupTextureRegisterIndex) << ");\n";
    *groupTextureRegisterIndex += groupRegisterCount;
}

void ResourcesHLSL::outputHLSLImageUniformGroup(TInfoSinkBase &out,
                                                const HLSLRWTextureGroup textureGroup,
                                                const TVector<const TVariable *> &group,
                                                unsigned int *groupTextureRegisterIndex)
{
    if (group.empty())
    {
        return;
    }

    // ROVs should all be written out in DynamicImage2DHLSL.cpp.
    ASSERT(!IsAnyRasterOrdered(group));

    unsigned int groupRegisterCount = 0;
    outputHLSLImageUniformIndices(out, group, *groupTextureRegisterIndex, &groupRegisterCount);

    TString suffix = RWTextureGroupSuffix(textureGroup);
    out << "static const uint imageIndexOffset" << suffix << " = " << (*groupTextureRegisterIndex)
        << ";\n";
    out << "uniform " << RWTextureString(textureGroup) << " images" << suffix << "["
        << groupRegisterCount << "]" << " : register(u" << (*groupTextureRegisterIndex) << ");\n";
    *groupTextureRegisterIndex += groupRegisterCount;
}

void ResourcesHLSL::outputUniform(TInfoSinkBase &out,
                                  const TType &type,
                                  const TVariable &variable,
                                  const unsigned int registerIndex)
{
    const TStructure *structure = type.getStruct();
    // If this is a nameless struct, we need to use its full definition, rather than its (empty)
    // name.
    // TypeString() will invoke defineNameless in this case; qualifier prefixes are unnecessary for
    // nameless structs in ES, as nameless structs cannot be used anywhere that layout qualifiers
    // are permitted.
    const TString &typeName = ((structure && structure->symbolType() != SymbolType::Empty)
                                   ? QualifiedStructNameString(*structure, false, false, false)
                                   : TypeString(type));

    const TString &registerString =
        TString("register(") + UniformRegisterPrefix(type) + str(registerIndex) + ")";

    out << "uniform " << typeName << " ";

    out << DecorateVariableIfNeeded(variable);

    out << ArrayString(type) << " : " << registerString << ";\n";
}

void ResourcesHLSL::outputAtomicCounterBuffer(TInfoSinkBase &out,
                                              const int binding,
                                              const unsigned int registerIndex)
{
    // Atomic counter memory access is not incoherent
    out << "uniform globallycoherent RWByteAddressBuffer "
        << getAtomicCounterNameForBinding(binding) << " : register(u" << registerIndex << ");\n";
}

void ResourcesHLSL::uniformsHeader(TInfoSinkBase &out,
                                   ShShaderOutput outputType,
                                   const ReferencedVariables &referencedUniforms,
                                   TSymbolTable *symbolTable)
{
    if (!referencedUniforms.empty())
    {
        out << "// Uniforms\n\n";
    }
    // In the case of HLSL 4, sampler uniforms need to be grouped by type before the code is
    // written. They are grouped based on the combination of the HLSL texture type and
    // HLSL sampler type, enumerated in HLSLTextureSamplerGroup.
    TVector<TVector<const TVariable *>> groupedSamplerUniforms(HLSL_TEXTURE_MAX + 1);
    TMap<const TVariable *, TString> samplerInStructSymbolsToAPINames;
    TVector<TVector<const TVariable *>> groupedReadonlyImageUniforms(HLSL_TEXTURE_MAX + 1);
    TVector<TVector<const TVariable *>> groupedImageUniforms(HLSL_RWTEXTURE_MAX + 1);

    TUnorderedMap<int, unsigned int> assignedAtomicCounterBindings;
    unsigned int reservedReadonlyImageRegisterCount = 0, reservedImageRegisterCount = 0;
    for (auto &uniformIt : referencedUniforms)
    {
        // Output regular uniforms. Group sampler uniforms by type.
        const TVariable &variable = *uniformIt.second;
        const TType &type         = variable.getType();

        if (outputType == SH_HLSL_4_1_OUTPUT && IsSampler(type.getBasicType()))
        {
            HLSLTextureGroup group = TextureGroup(type.getBasicType());
            groupedSamplerUniforms[group].push_back(&variable);
        }
        else if (outputType == SH_HLSL_4_1_OUTPUT && IsImage(type.getBasicType()))
        {
            if (IsImage2D(type.getBasicType()))
            {
                const ShaderVariable *uniform = findUniformByName(variable.name());
                if (type.getMemoryQualifier().readonly)
                {
                    reservedReadonlyImageRegisterCount +=
                        HLSLVariableRegisterCount(*uniform, mOutputType);
                }
                else
                {
                    reservedImageRegisterCount += HLSLVariableRegisterCount(*uniform, mOutputType);
                }
                continue;
            }
            if (type.getMemoryQualifier().readonly)
            {
                HLSLTextureGroup group = TextureGroup(
                    type.getBasicType(), type.getLayoutQualifier().imageInternalFormat);
                groupedReadonlyImageUniforms[group].push_back(&variable);
            }
            else
            {
                HLSLRWTextureGroup group = RWTextureGroup(
                    type.getBasicType(), type.getLayoutQualifier().imageInternalFormat);
                groupedImageUniforms[group].push_back(&variable);
            }
        }
        else if (outputType == SH_HLSL_4_1_OUTPUT && IsAtomicCounter(type.getBasicType()))
        {
            TLayoutQualifier layout = type.getLayoutQualifier();
            int binding             = layout.binding;
            unsigned int registerIndex;
            if (assignedAtomicCounterBindings.find(binding) == assignedAtomicCounterBindings.end())
            {
                registerIndex                          = mUAVRegister++;
                assignedAtomicCounterBindings[binding] = registerIndex;
                outputAtomicCounterBuffer(out, binding, registerIndex);
            }
            else
            {
                registerIndex = assignedAtomicCounterBindings[binding];
            }
            const ShaderVariable *uniform      = findUniformByName(variable.name());
            mUniformRegisterMap[uniform->name] = registerIndex;
        }
        else
        {
            if (type.isStructureContainingSamplers())
            {
                TVector<const TVariable *> samplerSymbols;
                TMap<const TVariable *, TString> symbolsToAPINames;
                ImmutableStringBuilder namePrefix(kAngleDecorString.length() +
                                                  variable.name().length());
                namePrefix << kAngleDecorString;
                namePrefix << variable.name();
                type.createSamplerSymbols(namePrefix, TString(variable.name().data()),
                                          &samplerSymbols, &symbolsToAPINames, symbolTable);
                for (const TVariable *sampler : samplerSymbols)
                {
                    const TType &samplerType = sampler->getType();

                    if (outputType == SH_HLSL_4_1_OUTPUT)
                    {
                        HLSLTextureGroup group = TextureGroup(samplerType.getBasicType());
                        groupedSamplerUniforms[group].push_back(sampler);
                        samplerInStructSymbolsToAPINames[sampler] = symbolsToAPINames[sampler];
                    }
                    else
                    {
                        ASSERT(outputType == SH_HLSL_3_0_OUTPUT);
                        unsigned int registerIndex = assignSamplerInStructUniformRegister(
                            samplerType, symbolsToAPINames[sampler], nullptr);
                        outputUniform(out, samplerType, *sampler, registerIndex);
                    }
                }
            }
            unsigned int registerIndex = assignUniformRegister(type, variable.name(), nullptr);
            outputUniform(out, type, variable, registerIndex);
        }
    }

    if (outputType == SH_HLSL_4_1_OUTPUT)
    {
        unsigned int groupTextureRegisterIndex = 0;
        // Atomic counters and RW texture share the same resources. Therefore, RW texture need to
        // start counting after the last atomic counter.
        unsigned int groupRWTextureRegisterIndex = mUAVRegister;
        // TEXTURE_2D is special, index offset is assumed to be 0 and omitted in that case.
        ASSERT(HLSL_TEXTURE_MIN == HLSL_TEXTURE_2D);
        for (int groupId = HLSL_TEXTURE_MIN; groupId < HLSL_TEXTURE_MAX; ++groupId)
        {
            outputHLSLSamplerUniformGroup(
                out, HLSLTextureGroup(groupId), groupedSamplerUniforms[groupId],
                samplerInStructSymbolsToAPINames, &groupTextureRegisterIndex);
        }
        mSamplerCount = groupTextureRegisterIndex;

        // Reserve t type register for readonly image2D variables.
        mReadonlyImage2DRegisterIndex = mSRVRegister;
        groupTextureRegisterIndex += reservedReadonlyImageRegisterCount;
        mSRVRegister += reservedReadonlyImageRegisterCount;

        for (int groupId = HLSL_TEXTURE_MIN; groupId < HLSL_TEXTURE_MAX; ++groupId)
        {
            outputHLSLReadonlyImageUniformGroup(out, HLSLTextureGroup(groupId),
                                                groupedReadonlyImageUniforms[groupId],
                                                &groupTextureRegisterIndex);
        }
        mReadonlyImageCount = groupTextureRegisterIndex - mReadonlyImage2DRegisterIndex;
        if (mReadonlyImageCount)
        {
            out << "static const uint readonlyImageIndexStart = " << mReadonlyImage2DRegisterIndex
                << ";\n";
        }

        // Reserve u type register for writable image2D variables.
        mImage2DRegisterIndex = mUAVRegister;
        groupRWTextureRegisterIndex += reservedImageRegisterCount;
        mUAVRegister += reservedImageRegisterCount;

        for (int groupId = HLSL_RWTEXTURE_MIN; groupId < HLSL_RWTEXTURE_MAX; ++groupId)
        {
            outputHLSLImageUniformGroup(out, HLSLRWTextureGroup(groupId),
                                        groupedImageUniforms[groupId],
                                        &groupRWTextureRegisterIndex);
        }
        mImageCount = groupRWTextureRegisterIndex - mImage2DRegisterIndex;
        if (mImageCount)
        {
            out << "static const uint imageIndexStart = " << mImage2DRegisterIndex << ";\n";
        }
    }
}

void ResourcesHLSL::samplerMetadataUniforms(TInfoSinkBase &out, unsigned int regIndex)
{
    // If mSamplerCount is 0 the shader doesn't use any textures for samplers.
    if (mSamplerCount > 0)
    {
        out << "    struct SamplerMetadata\n"
               "    {\n"
               "        int baseLevel;\n"
               "        int wrapModes;\n"
               "        int2 padding;\n"
               "        int4 intBorderColor;\n"
               "    };\n"
               "    SamplerMetadata samplerMetadata["
            << mSamplerCount << "] : packoffset(c" << regIndex << ");\n";
    }
}

void ResourcesHLSL::imageMetadataUniforms(TInfoSinkBase &out, unsigned int regIndex)
{
    if (mReadonlyImageCount > 0 || mImageCount > 0)
    {
        out << "    struct ImageMetadata\n"
               "    {\n"
               "        int layer;\n"
               "        uint level;\n"
               "        int2 padding;\n"
               "    };\n";

        if (mReadonlyImageCount > 0)
        {
            out << "    ImageMetadata readonlyImageMetadata[" << mReadonlyImageCount
                << "] : packoffset(c" << regIndex << ");\n";
        }

        if (mImageCount > 0)
        {
            out << "    ImageMetadata imageMetadata[" << mImageCount << "] : packoffset(c"
                << regIndex + mReadonlyImageCount << ");\n";
        }
    }
}

TString ResourcesHLSL::uniformBlocksHeader(
    const ReferencedInterfaceBlocks &referencedInterfaceBlocks,
    const std::map<int, const TInterfaceBlock *> &uniformBlockOptimizedMap)
{
    TString interfaceBlocks;

    for (const auto &blockReference : referencedInterfaceBlocks)
    {
        const TInterfaceBlock &interfaceBlock = *blockReference.second->block;
        const TVariable *instanceVariable     = blockReference.second->instanceVariable;
        if (instanceVariable != nullptr)
        {
            interfaceBlocks += uniformBlockStructString(interfaceBlock);
        }

        // In order to avoid compile performance issue, translate uniform block to structured
        // buffer. anglebug.com/40096608.
        if (uniformBlockOptimizedMap.count(interfaceBlock.uniqueId().get()) != 0)
        {
            unsigned int structuredBufferRegister = mSRVRegister;
            if (instanceVariable != nullptr && instanceVariable->getType().isArray())
            {
                unsigned int instanceArraySize =
                    instanceVariable->getType().getOutermostArraySize();
                for (unsigned int arrayIndex = 0; arrayIndex < instanceArraySize; arrayIndex++)
                {
                    interfaceBlocks += uniformBlockWithOneLargeArrayMemberString(
                        interfaceBlock, instanceVariable, structuredBufferRegister + arrayIndex,
                        arrayIndex);
                }
                mSRVRegister += instanceArraySize;
            }
            else
            {
                interfaceBlocks += uniformBlockWithOneLargeArrayMemberString(
                    interfaceBlock, instanceVariable, structuredBufferRegister, GL_INVALID_INDEX);
                mSRVRegister += 1u;
            }
            mUniformBlockRegisterMap[interfaceBlock.name().data()] = structuredBufferRegister;
            mUniformBlockUseStructuredBufferMap[interfaceBlock.name().data()] = true;
            continue;
        }

        unsigned int activeRegister                            = mUniformBlockRegister;
        mUniformBlockRegisterMap[interfaceBlock.name().data()] = activeRegister;

        if (instanceVariable != nullptr && instanceVariable->getType().isArray())
        {
            unsigned int instanceArraySize = instanceVariable->getType().getOutermostArraySize();
            for (unsigned int arrayIndex = 0; arrayIndex < instanceArraySize; arrayIndex++)
            {
                interfaceBlocks += uniformBlockString(interfaceBlock, instanceVariable,
                                                      activeRegister + arrayIndex, arrayIndex);
            }
            mUniformBlockRegister += instanceArraySize;
        }
        else
        {
            interfaceBlocks += uniformBlockString(interfaceBlock, instanceVariable, activeRegister,
                                                  GL_INVALID_INDEX);
            mUniformBlockRegister += 1u;
        }
    }

    return (interfaceBlocks.empty() ? "" : ("// Uniform Blocks\n\n" + interfaceBlocks));
}

void ResourcesHLSL::allocateShaderStorageBlockRegisters(
    const ReferencedInterfaceBlocks &referencedInterfaceBlocks)
{
    for (const auto &interfaceBlockReference : referencedInterfaceBlocks)
    {
        const TInterfaceBlock &interfaceBlock = *interfaceBlockReference.second->block;
        const TVariable *instanceVariable     = interfaceBlockReference.second->instanceVariable;

        mShaderStorageBlockRegisterMap[interfaceBlock.name().data()] = mUAVRegister;

        if (instanceVariable != nullptr && instanceVariable->getType().isArray())
        {
            mUAVRegister += instanceVariable->getType().getOutermostArraySize();
        }
        else
        {
            mUAVRegister += 1u;
        }
    }
}

TString ResourcesHLSL::shaderStorageBlocksHeader(
    const ReferencedInterfaceBlocks &referencedInterfaceBlocks)
{
    TString interfaceBlocks;

    for (const auto &interfaceBlockReference : referencedInterfaceBlocks)
    {
        const TInterfaceBlock &interfaceBlock = *interfaceBlockReference.second->block;
        const TVariable *instanceVariable     = interfaceBlockReference.second->instanceVariable;

        unsigned int activeRegister = mShaderStorageBlockRegisterMap[interfaceBlock.name().data()];

        if (instanceVariable != nullptr && instanceVariable->getType().isArray())
        {
            unsigned int instanceArraySize = instanceVariable->getType().getOutermostArraySize();
            for (unsigned int arrayIndex = 0; arrayIndex < instanceArraySize; arrayIndex++)
            {
                interfaceBlocks += shaderStorageBlockString(
                    interfaceBlock, instanceVariable, activeRegister + arrayIndex, arrayIndex);
            }
        }
        else
        {
            interfaceBlocks += shaderStorageBlockString(interfaceBlock, instanceVariable,
                                                        activeRegister, GL_INVALID_INDEX);
        }
    }

    return interfaceBlocks;
}

TString ResourcesHLSL::uniformBlockString(const TInterfaceBlock &interfaceBlock,
                                          const TVariable *instanceVariable,
                                          unsigned int registerIndex,
                                          unsigned int arrayIndex)
{
    const TString &arrayIndexString = (arrayIndex != GL_INVALID_INDEX ? str(arrayIndex) : "");
    const TString &blockName        = TString(interfaceBlock.name().data()) + arrayIndexString;
    TString hlsl;

    hlsl += "cbuffer " + blockName + " : register(b" + str(registerIndex) +
            ")\n"
            "{\n";

    if (instanceVariable != nullptr)
    {
        hlsl += "    " + InterfaceBlockStructName(interfaceBlock) + " " +
                InterfaceBlockInstanceString(instanceVariable->name(), arrayIndex) + ";\n";
    }
    else
    {
        const TLayoutBlockStorage blockStorage = interfaceBlock.blockStorage();
        hlsl += uniformBlockMembersString(interfaceBlock, blockStorage);
    }

    hlsl += "};\n\n";

    return hlsl;
}

TString ResourcesHLSL::uniformBlockWithOneLargeArrayMemberString(
    const TInterfaceBlock &interfaceBlock,
    const TVariable *instanceVariable,
    unsigned int registerIndex,
    unsigned int arrayIndex)
{
    TString hlsl, typeString;

    const TField &field                    = *interfaceBlock.fields()[0];
    const TLayoutBlockStorage blockStorage = interfaceBlock.blockStorage();
    typeString             = InterfaceBlockFieldTypeString(field, blockStorage, true);
    const TType &fieldType = *field.type();
    if (fieldType.isMatrix())
    {
        if (arrayIndex == GL_INVALID_INDEX || arrayIndex == 0)
        {
            hlsl += "struct pack" + Decorate(interfaceBlock.name()) + " { " + typeString + " " +
                    Decorate(field.name()) + "; };\n";
        }
        typeString = "pack" + Decorate(interfaceBlock.name());
    }
    else if (fieldType.isVectorArray() || fieldType.isScalarArray())
    {
        // If the member is an array of scalars or vectors, std140 rules require the base array
        // stride are rounded up to the base alignment of a vec4.
        if (arrayIndex == GL_INVALID_INDEX || arrayIndex == 0)
        {
            hlsl += "struct pack" + Decorate(interfaceBlock.name()) + " { " + typeString + " " +
                    Decorate(field.name()) + ";\n";
            hlsl += InterfaceBlockScalarVectorFieldPaddingString(fieldType) + " };\n";
        }
        typeString = "pack" + Decorate(interfaceBlock.name());
    }

    if (instanceVariable != nullptr)
    {

        hlsl += "StructuredBuffer <" + typeString + "> " +
                InterfaceBlockInstanceString(instanceVariable->name(), arrayIndex) + "_" +
                Decorate(field.name()) + +" : register(t" + str(registerIndex) + ");\n";
    }
    else
    {
        hlsl += "StructuredBuffer <" + typeString + "> " + Decorate(field.name()) +
                " : register(t" + str(registerIndex) + ");\n";
    }

    return hlsl;
}

TString ResourcesHLSL::shaderStorageBlockString(const TInterfaceBlock &interfaceBlock,
                                                const TVariable *instanceVariable,
                                                unsigned int registerIndex,
                                                unsigned int arrayIndex)
{
    TString hlsl;
    if (instanceVariable != nullptr)
    {
        hlsl += "RWByteAddressBuffer " +
                InterfaceBlockInstanceString(instanceVariable->name(), arrayIndex) +
                ": register(u" + str(registerIndex) + ");\n";
    }
    else
    {
        hlsl += "RWByteAddressBuffer " + Decorate(interfaceBlock.name()) + ": register(u" +
                str(registerIndex) + ");\n";
    }
    return hlsl;
}

TString ResourcesHLSL::InterfaceBlockInstanceString(const ImmutableString &instanceName,
                                                    unsigned int arrayIndex)
{
    if (arrayIndex != GL_INVALID_INDEX)
    {
        return DecoratePrivate(instanceName) + "_" + str(arrayIndex);
    }
    else
    {
        return Decorate(instanceName);
    }
}

TString ResourcesHLSL::uniformBlockMembersString(const TInterfaceBlock &interfaceBlock,
                                                 TLayoutBlockStorage blockStorage)
{
    TString hlsl;

    Std140PaddingHelper padHelper = mStructureHLSL->getPaddingHelper();

    const unsigned int fieldCount = static_cast<unsigned int>(interfaceBlock.fields().size());
    for (unsigned int typeIndex = 0; typeIndex < fieldCount; typeIndex++)
    {
        const TField &field    = *interfaceBlock.fields()[typeIndex];
        const TType &fieldType = *field.type();

        if (blockStorage == EbsStd140)
        {
            // 2 and 3 component vector types in some cases need pre-padding
            hlsl += padHelper.prePaddingString(fieldType, false);
        }

        hlsl += "    " + InterfaceBlockFieldTypeString(field, blockStorage, false) + " " +
                Decorate(field.name()) + ArrayString(fieldType).data() + ";\n";

        // must pad out after matrices and arrays, where HLSL usually allows itself room to pack
        // stuff
        if (blockStorage == EbsStd140)
        {
            const bool useHLSLRowMajorPacking =
                (fieldType.getLayoutQualifier().matrixPacking == EmpColumnMajor);
            hlsl += padHelper.postPaddingString(fieldType, useHLSLRowMajorPacking,
                                                typeIndex == fieldCount - 1, false);
        }
    }

    return hlsl;
}

TString ResourcesHLSL::uniformBlockStructString(const TInterfaceBlock &interfaceBlock)
{
    const TLayoutBlockStorage blockStorage = interfaceBlock.blockStorage();

    return "struct " + InterfaceBlockStructName(interfaceBlock) +
           "\n"
           "{\n" +
           uniformBlockMembersString(interfaceBlock, blockStorage) + "};\n\n";
}
}  // namespace sh
