//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramLinkedResources.cpp: implements link-time checks for default block uniforms, and generates
// uniform locations. Populates data structures related to uniforms so that they can be stored in
// program state.

#include "libANGLE/ProgramLinkedResources.h"

#include "common/string_utils.h"
#include "common/utilities.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Context.h"
#include "libANGLE/Shader.h"
#include "libANGLE/features.h"

namespace gl
{
namespace
{
UsedUniform *FindUniform(std::vector<UsedUniform> &list, const std::string &name)
{
    for (UsedUniform &uniform : list)
    {
        if (uniform.name == name)
            return &uniform;
    }

    return nullptr;
}

template <typename VarT>
void SetActive(std::vector<VarT> *list,
               const std::string &name,
               ShaderType shaderType,
               bool active,
               uint32_t id)
{
    for (auto &variable : *list)
    {
        if (variable.name == name)
        {
            variable.setActive(shaderType, active, id);
            return;
        }
    }
}

template <typename VarT>
void SetActive(std::vector<VarT> *list,
               std::vector<std::string> *nameList,
               const std::string &name,
               ShaderType shaderType,
               bool active,
               uint32_t id)
{
    for (GLint index = 0; index < static_cast<GLint>(nameList->size()); index++)
    {
        if ((*nameList)[index] == name)
        {
            (*list)[index].setActive(shaderType, active, id);
            return;
        }
    }
}

// GLSL ES Spec 3.00.3, section 4.3.5.
LinkMismatchError LinkValidateUniforms(const sh::ShaderVariable &uniform1,
                                       const sh::ShaderVariable &uniform2,
                                       std::string *mismatchedStructFieldName)
{
#if ANGLE_PROGRAM_LINK_VALIDATE_UNIFORM_PRECISION
    const bool validatePrecisionFeature = true;
#else
    const bool validatePrecisionFeature = false;
#endif

    // Validate precision match of uniforms iff they are statically used
    bool validatePrecision = uniform1.staticUse && uniform2.staticUse && validatePrecisionFeature;
    LinkMismatchError linkError = LinkValidateProgramVariables(
        uniform1, uniform2, validatePrecision, false, false, mismatchedStructFieldName);
    if (linkError != LinkMismatchError::NO_MISMATCH)
    {
        return linkError;
    }

    // GLSL ES Spec 3.10.4, section 4.4.5.
    if (uniform1.binding != -1 && uniform2.binding != -1 && uniform1.binding != uniform2.binding)
    {
        return LinkMismatchError::BINDING_MISMATCH;
    }

    // GLSL ES Spec 3.10.4, section 9.2.1.
    if (uniform1.location != -1 && uniform2.location != -1 &&
        uniform1.location != uniform2.location)
    {
        return LinkMismatchError::LOCATION_MISMATCH;
    }
    if (uniform1.offset != uniform2.offset)
    {
        return LinkMismatchError::OFFSET_MISMATCH;
    }

    return LinkMismatchError::NO_MISMATCH;
}

GLuint GetMaximumShaderUniformVectors(ShaderType shaderType, const Caps &caps)
{
    switch (shaderType)
    {
        case ShaderType::Vertex:
            return static_cast<GLuint>(caps.maxVertexUniformVectors);
        case ShaderType::Fragment:
            return static_cast<GLuint>(caps.maxFragmentUniformVectors);

        case ShaderType::Compute:
        case ShaderType::Geometry:
        case ShaderType::TessControl:
        case ShaderType::TessEvaluation:
            return static_cast<GLuint>(caps.maxShaderUniformComponents[shaderType]) / 4;

        default:
            UNREACHABLE();
            return 0u;
    }
}

enum class UniformType : uint8_t
{
    Variable      = 0,
    Sampler       = 1,
    Image         = 2,
    AtomicCounter = 3,

    InvalidEnum = 4,
    EnumCount   = 4,
};

const char *GetUniformResourceNameString(UniformType uniformType)
{
    switch (uniformType)
    {
        case UniformType::Variable:
            return "uniform";
        case UniformType::Sampler:
            return "texture image unit";
        case UniformType::Image:
            return "image uniform";
        case UniformType::AtomicCounter:
            return "atomic counter";
        default:
            UNREACHABLE();
            return "";
    }
}

std::string GetUniformResourceLimitName(ShaderType shaderType, UniformType uniformType)
{
    // Special case: MAX_TEXTURE_IMAGE_UNITS (no "MAX_FRAGMENT_TEXTURE_IMAGE_UNITS")
    if (shaderType == ShaderType::Fragment && uniformType == UniformType::Sampler)
    {
        return "MAX_TEXTURE_IMAGE_UNITS";
    }

    std::ostringstream ostream;
    ostream << "MAX_" << GetShaderTypeString(shaderType) << "_";

    switch (uniformType)
    {
        case UniformType::Variable:
            // For vertex and fragment shaders, ES 2.0 only defines MAX_VERTEX_UNIFORM_VECTORS and
            // MAX_FRAGMENT_UNIFORM_VECTORS ([OpenGL ES 2.0] Table 6.20).
            if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment)
            {
                ostream << "UNIFORM_VECTORS";
                break;
            }
            // For compute and geometry shaders, there are no definitions on
            // "MAX_COMPUTE_UNIFORM_VECTORS" or "MAX_GEOMETRY_UNIFORM_VECTORS_EXT"
            // ([OpenGL ES 3.1] Table 20.45, [EXT_geometry_shader] Table 20.43gs).
            else
            {
                ostream << "UNIFORM_COMPONENTS";
            }
            break;
        case UniformType::Sampler:
            ostream << "TEXTURE_IMAGE_UNITS";
            break;
        case UniformType::Image:
            ostream << "IMAGE_UNIFORMS";
            break;
        case UniformType::AtomicCounter:
            ostream << "ATOMIC_COUNTERS";
            break;
        default:
            UNREACHABLE();
            return "";
    }

    if (shaderType == ShaderType::Geometry)
    {
        ostream << "_EXT";
    }

    return ostream.str();
}

void LogUniformsExceedLimit(ShaderType shaderType,
                            UniformType uniformType,
                            GLuint limit,
                            InfoLog &infoLog)
{
    infoLog << GetShaderTypeString(shaderType) << " shader "
            << GetUniformResourceNameString(uniformType) << "s count exceeds "
            << GetUniformResourceLimitName(shaderType, uniformType) << "(" << limit << ")";
}

// The purpose of this visitor is to capture the uniforms in a uniform block. Each new uniform is
// added to "uniformsOut".
class UniformBlockEncodingVisitor : public sh::VariableNameVisitor
{
  public:
    UniformBlockEncodingVisitor(const GetBlockMemberInfoFunc &getMemberInfo,
                                const std::string &namePrefix,
                                const std::string &mappedNamePrefix,
                                std::vector<LinkedUniform> *uniformsOut,
                                std::vector<std::string> *uniformNamesOut,
                                std::vector<std::string> *uniformMappedNamesOut,
                                ShaderType shaderType,
                                int blockIndex)
        : sh::VariableNameVisitor(namePrefix, mappedNamePrefix),
          mGetMemberInfo(getMemberInfo),
          mUniformsOut(uniformsOut),
          mUniformNamesOut(uniformNamesOut),
          mUniformMappedNamesOut(uniformMappedNamesOut),
          mShaderType(shaderType),
          mBlockIndex(blockIndex)
    {}

    void visitNamedVariable(const sh::ShaderVariable &variable,
                            bool isRowMajor,
                            const std::string &name,
                            const std::string &mappedName,
                            const std::vector<unsigned int> &arraySizes) override
    {
        // If getBlockMemberInfo returns false, the variable is optimized out.
        sh::BlockMemberInfo variableInfo;
        if (!mGetMemberInfo(name, mappedName, &variableInfo))
            return;

        std::string nameWithArrayIndex       = name;
        std::string mappedNameWithArrayIndex = mappedName;

        if (variable.isArray())
        {
            nameWithArrayIndex += "[0]";
            mappedNameWithArrayIndex += "[0]";
        }

        if (mBlockIndex == -1)
        {
            SetActive(mUniformsOut, mUniformNamesOut, nameWithArrayIndex, mShaderType,
                      variable.active, variable.id);
            return;
        }

        LinkedUniform newUniform(variable.type, variable.precision, variable.arraySizes, -1, -1, -1,
                                 mBlockIndex, variableInfo);
        newUniform.setActive(mShaderType, variable.active, variable.id);

        // Since block uniforms have no location, we don't need to store them in the uniform
        // locations list.
        mUniformsOut->push_back(newUniform);
        mUniformNamesOut->push_back(nameWithArrayIndex);
        mUniformMappedNamesOut->push_back(mappedNameWithArrayIndex);
    }

  private:
    const GetBlockMemberInfoFunc &mGetMemberInfo;
    std::vector<LinkedUniform> *mUniformsOut;
    std::vector<std::string> *mUniformNamesOut;
    std::vector<std::string> *mUniformMappedNamesOut;
    const ShaderType mShaderType;
    const int mBlockIndex;
};

// The purpose of this visitor is to capture the buffer variables in a shader storage block. Each
// new buffer variable is stored in "bufferVariablesOut".
class ShaderStorageBlockVisitor : public sh::BlockEncoderVisitor
{
  public:
    ShaderStorageBlockVisitor(const GetBlockMemberInfoFunc &getMemberInfo,
                              const std::string &namePrefix,
                              const std::string &mappedNamePrefix,
                              std::vector<BufferVariable> *bufferVariablesOut,
                              ShaderType shaderType,
                              int blockIndex)
        : sh::BlockEncoderVisitor(namePrefix, mappedNamePrefix, &mStubEncoder),
          mGetMemberInfo(getMemberInfo),
          mBufferVariablesOut(bufferVariablesOut),
          mShaderType(shaderType),
          mBlockIndex(blockIndex)
    {}

    void visitNamedVariable(const sh::ShaderVariable &variable,
                            bool isRowMajor,
                            const std::string &name,
                            const std::string &mappedName,
                            const std::vector<unsigned int> &arraySizes) override
    {
        if (mSkipEnabled)
            return;

        // If getBlockMemberInfo returns false, the variable is optimized out.
        sh::BlockMemberInfo variableInfo;
        if (!mGetMemberInfo(name, mappedName, &variableInfo))
            return;

        std::string nameWithArrayIndex       = name;
        std::string mappedNameWithArrayIndex = mappedName;

        if (variable.isArray())
        {
            nameWithArrayIndex += "[0]";
            mappedNameWithArrayIndex += "[0]";
        }

        if (mBlockIndex == -1)
        {
            SetActive(mBufferVariablesOut, nameWithArrayIndex, mShaderType, variable.active,
                      variable.id);
            return;
        }

        BufferVariable newBufferVariable(variable.type, variable.precision, nameWithArrayIndex,
                                         variable.arraySizes, mBlockIndex, mTopLevelArraySize,
                                         variableInfo);
        newBufferVariable.mappedName = mappedNameWithArrayIndex;
        newBufferVariable.setActive(mShaderType, variable.active, variable.id);

        mBufferVariablesOut->push_back(newBufferVariable);
    }

  private:
    const GetBlockMemberInfoFunc &mGetMemberInfo;
    std::vector<BufferVariable> *mBufferVariablesOut;
    const ShaderType mShaderType;
    const int mBlockIndex;
    sh::StubBlockEncoder mStubEncoder;
};

struct ShaderUniformCount
{
    unsigned int vectorCount        = 0;
    unsigned int samplerCount       = 0;
    unsigned int imageCount         = 0;
    unsigned int atomicCounterCount = 0;
    unsigned int fragmentInOutCount = 0;
};

ShaderUniformCount &operator+=(ShaderUniformCount &lhs, const ShaderUniformCount &rhs)
{
    lhs.vectorCount += rhs.vectorCount;
    lhs.samplerCount += rhs.samplerCount;
    lhs.imageCount += rhs.imageCount;
    lhs.atomicCounterCount += rhs.atomicCounterCount;
    lhs.fragmentInOutCount += rhs.fragmentInOutCount;
    return lhs;
}

// The purpose of this visitor is to flatten struct and array uniforms into a list of singleton
// uniforms. They are stored in separate lists by uniform type so they can be sorted in order.
// Counts for each uniform category are stored and can be queried with "getCounts".
class FlattenUniformVisitor : public sh::VariableNameVisitor
{
  public:
    FlattenUniformVisitor(ShaderType shaderType,
                          const sh::ShaderVariable &uniform,
                          std::vector<UsedUniform> *uniforms,
                          std::vector<UsedUniform> *samplerUniforms,
                          std::vector<UsedUniform> *imageUniforms,
                          std::vector<UsedUniform> *atomicCounterUniforms,
                          std::vector<UsedUniform> *inputAttachmentUniforms,
                          std::vector<UnusedUniform> *unusedUniforms)
        : sh::VariableNameVisitor("", ""),
          mShaderType(shaderType),
          mMarkActive(uniform.active),
          mMarkStaticUse(uniform.staticUse),
          mBinding(uniform.binding),
          mOffset(uniform.offset),
          mLocation(uniform.location),
          mUniforms(uniforms),
          mSamplerUniforms(samplerUniforms),
          mImageUniforms(imageUniforms),
          mAtomicCounterUniforms(atomicCounterUniforms),
          mInputAttachmentUniforms(inputAttachmentUniforms),
          mUnusedUniforms(unusedUniforms)
    {}

    void visitNamedOpaqueObject(const sh::ShaderVariable &variable,
                                const std::string &name,
                                const std::string &mappedName,
                                const std::vector<unsigned int> &arraySizes) override
    {
        visitNamedVariable(variable, false, name, mappedName, arraySizes);
    }

    void visitNamedVariable(const sh::ShaderVariable &variable,
                            bool isRowMajor,
                            const std::string &name,
                            const std::string &mappedName,
                            const std::vector<unsigned int> &arraySizes) override
    {
        bool isSampler                        = IsSamplerType(variable.type);
        bool isImage                          = IsImageType(variable.type);
        bool isAtomicCounter                  = IsAtomicCounterType(variable.type);
        bool isFragmentInOut                  = variable.isFragmentInOut;
        std::vector<UsedUniform> *uniformList = mUniforms;
        if (isSampler)
        {
            uniformList = mSamplerUniforms;
        }
        else if (isImage)
        {
            uniformList = mImageUniforms;
        }
        else if (isAtomicCounter)
        {
            uniformList = mAtomicCounterUniforms;
        }
        else if (isFragmentInOut)
        {
            uniformList = mInputAttachmentUniforms;
        }

        std::string fullNameWithArrayIndex(name);
        std::string fullMappedNameWithArrayIndex(mappedName);

        if (variable.isArray())
        {
            // We're following the GLES 3.1 November 2016 spec section 7.3.1.1 Naming Active
            // Resources and including [0] at the end of array variable names.
            fullNameWithArrayIndex += "[0]";
            fullMappedNameWithArrayIndex += "[0]";
        }

        UsedUniform *existingUniform = FindUniform(*uniformList, fullNameWithArrayIndex);
        if (existingUniform)
        {
            if (getBinding() != -1)
            {
                existingUniform->binding = getBinding();
            }
            if (getOffset() != -1)
            {
                existingUniform->offset = getOffset();
            }
            if (mLocation != -1)
            {
                existingUniform->location = mLocation;
            }
            if (mMarkActive)
            {
                existingUniform->active = true;
                existingUniform->setActive(mShaderType, true, variable.id);
            }
            if (mMarkStaticUse)
            {
                existingUniform->staticUse = true;
            }
        }
        else
        {
            UsedUniform linkedUniform(variable.type, variable.precision, fullNameWithArrayIndex,
                                      variable.arraySizes, getBinding(), getOffset(), mLocation, -1,
                                      sh::kDefaultBlockMemberInfo);
            linkedUniform.mappedName          = fullMappedNameWithArrayIndex;
            linkedUniform.active              = mMarkActive;
            linkedUniform.staticUse           = mMarkStaticUse;
            linkedUniform.outerArraySizes     = arraySizes;
            linkedUniform.texelFetchStaticUse = variable.texelFetchStaticUse;
            linkedUniform.id                  = variable.id;
            linkedUniform.imageUnitFormat     = variable.imageUnitFormat;
            linkedUniform.isFragmentInOut     = variable.isFragmentInOut;
            if (variable.hasParentArrayIndex())
            {
                linkedUniform.setParentArrayIndex(variable.parentArrayIndex());
            }

            std::vector<unsigned int> arrayDims = arraySizes;
            ASSERT(variable.arraySizes.size() == 1 || variable.arraySizes.size() == 0);
            arrayDims.push_back(variable.arraySizes.empty() ? 1 : variable.arraySizes[0]);

            size_t numDimensions = arraySizes.size();
            uint32_t arrayStride = 1;
            for (size_t dimension = numDimensions; dimension > 0;)
            {
                --dimension;
                arrayStride *= arrayDims[dimension + 1];
                linkedUniform.outerArrayOffset += arrayStride * mArrayElementStack[dimension];
            }

            if (mMarkActive)
            {
                linkedUniform.setActive(mShaderType, true, variable.id);
            }
            else
            {
                mUnusedUniforms->emplace_back(
                    linkedUniform.name, linkedUniform.isSampler(), linkedUniform.isImage(),
                    linkedUniform.isAtomicCounter(), linkedUniform.isFragmentInOut);
            }

            uniformList->push_back(linkedUniform);
        }

        unsigned int elementCount = variable.getBasicTypeElementCount();

        // Samplers and images aren't "real" uniforms, so they don't count towards register usage.
        // Likewise, don't count "real" uniforms towards opaque count.

        if (!IsOpaqueType(variable.type) && !isFragmentInOut)
        {
            mUniformCount.vectorCount += VariableRegisterCount(variable.type) * elementCount;
        }

        mUniformCount.samplerCount += (isSampler ? elementCount : 0);
        mUniformCount.imageCount += (isImage ? elementCount : 0);
        mUniformCount.atomicCounterCount += (isAtomicCounter ? elementCount : 0);
        mUniformCount.fragmentInOutCount += (isFragmentInOut ? elementCount : 0);

        if (mLocation != -1)
        {
            mLocation += elementCount;
        }
    }

    void enterStructAccess(const sh::ShaderVariable &structVar, bool isRowMajor) override
    {
        mStructStackSize++;
        sh::VariableNameVisitor::enterStructAccess(structVar, isRowMajor);
    }

    void exitStructAccess(const sh::ShaderVariable &structVar, bool isRowMajor) override
    {
        mStructStackSize--;
        sh::VariableNameVisitor::exitStructAccess(structVar, isRowMajor);
    }

    void enterArrayElement(const sh::ShaderVariable &arrayVar, unsigned int arrayElement) override
    {
        mArrayElementStack.push_back(arrayElement);
        sh::VariableNameVisitor::enterArrayElement(arrayVar, arrayElement);
    }

    void exitArrayElement(const sh::ShaderVariable &arrayVar, unsigned int arrayElement) override
    {
        mArrayElementStack.pop_back();
        sh::VariableNameVisitor::exitArrayElement(arrayVar, arrayElement);
    }

    ShaderUniformCount getCounts() const { return mUniformCount; }

  private:
    int getBinding() const { return mStructStackSize == 0 ? mBinding : -1; }
    int getOffset() const { return mStructStackSize == 0 ? mOffset : -1; }

    ShaderType mShaderType;

    // Active and StaticUse are given separately because they are tracked at struct granularity.
    bool mMarkActive;
    bool mMarkStaticUse;
    int mBinding;
    int mOffset;
    int mLocation;
    std::vector<UsedUniform> *mUniforms;
    std::vector<UsedUniform> *mSamplerUniforms;
    std::vector<UsedUniform> *mImageUniforms;
    std::vector<UsedUniform> *mAtomicCounterUniforms;
    std::vector<UsedUniform> *mInputAttachmentUniforms;
    std::vector<UnusedUniform> *mUnusedUniforms;
    std::vector<unsigned int> mArrayElementStack;
    ShaderUniformCount mUniformCount;
    unsigned int mStructStackSize = 0;
};

class InterfaceBlockInfo final : angle::NonCopyable
{
  public:
    InterfaceBlockInfo(CustomBlockLayoutEncoderFactory *customEncoderFactory)
        : mCustomEncoderFactory(customEncoderFactory)
    {}

    void getShaderBlockInfo(const std::vector<sh::InterfaceBlock> &interfaceBlocks);

    bool getBlockSize(const std::string &name, const std::string &mappedName, size_t *sizeOut);
    bool getBlockMemberInfo(const std::string &name,
                            const std::string &mappedName,
                            sh::BlockMemberInfo *infoOut);

  private:
    size_t getBlockInfo(const sh::InterfaceBlock &interfaceBlock);

    std::map<std::string, size_t> mBlockSizes;
    sh::BlockLayoutMap mBlockLayout;
    // Based on the interface block layout, the std140 or std430 encoders are used.  On some
    // platforms (currently only D3D), there could be another non-standard encoder used.
    CustomBlockLayoutEncoderFactory *mCustomEncoderFactory;
};

void InterfaceBlockInfo::getShaderBlockInfo(const std::vector<sh::InterfaceBlock> &interfaceBlocks)
{
    for (const sh::InterfaceBlock &interfaceBlock : interfaceBlocks)
    {
        if (!IsActiveInterfaceBlock(interfaceBlock))
            continue;

        if (mBlockSizes.count(interfaceBlock.name) > 0)
            continue;

        size_t dataSize                  = getBlockInfo(interfaceBlock);
        mBlockSizes[interfaceBlock.name] = dataSize;
    }
}

size_t InterfaceBlockInfo::getBlockInfo(const sh::InterfaceBlock &interfaceBlock)
{
    ASSERT(IsActiveInterfaceBlock(interfaceBlock));

    // define member uniforms
    sh::Std140BlockEncoder std140Encoder;
    sh::Std430BlockEncoder std430Encoder;
    sh::BlockLayoutEncoder *customEncoder = nullptr;
    sh::BlockLayoutEncoder *encoder       = nullptr;

    if (interfaceBlock.layout == sh::BLOCKLAYOUT_STD140)
    {
        encoder = &std140Encoder;
    }
    else if (interfaceBlock.layout == sh::BLOCKLAYOUT_STD430)
    {
        encoder = &std430Encoder;
    }
    else if (mCustomEncoderFactory)
    {
        encoder = customEncoder = mCustomEncoderFactory->makeEncoder();
    }
    else
    {
        UNREACHABLE();
        return 0;
    }

    sh::GetInterfaceBlockInfo(interfaceBlock.fields, interfaceBlock.fieldPrefix(), encoder,
                              &mBlockLayout);

    size_t offset = encoder->getCurrentOffset();

    SafeDelete(customEncoder);

    return offset;
}

bool InterfaceBlockInfo::getBlockSize(const std::string &name,
                                      const std::string &mappedName,
                                      size_t *sizeOut)
{
    size_t nameLengthWithoutArrayIndex;
    ParseArrayIndex(name, &nameLengthWithoutArrayIndex);
    std::string baseName = name.substr(0u, nameLengthWithoutArrayIndex);
    auto sizeIter        = mBlockSizes.find(baseName);
    if (sizeIter == mBlockSizes.end())
    {
        *sizeOut = 0;
        return false;
    }

    *sizeOut = sizeIter->second;
    return true;
}

bool InterfaceBlockInfo::getBlockMemberInfo(const std::string &name,
                                            const std::string &mappedName,
                                            sh::BlockMemberInfo *infoOut)
{
    auto infoIter = mBlockLayout.find(name);
    if (infoIter == mBlockLayout.end())
    {
        *infoOut = sh::kDefaultBlockMemberInfo;
        return false;
    }

    *infoOut = infoIter->second;
    return true;
}

void GetFilteredVaryings(const std::vector<sh::ShaderVariable> &varyings,
                         std::vector<const sh::ShaderVariable *> *filteredVaryingsOut)
{
    for (const sh::ShaderVariable &varying : varyings)
    {
        // Built-in varyings obey special rules
        if (varying.isBuiltIn())
        {
            continue;
        }

        filteredVaryingsOut->push_back(&varying);
    }
}

LinkMismatchError LinkValidateVaryings(const sh::ShaderVariable &outputVarying,
                                       const sh::ShaderVariable &inputVarying,
                                       int shaderVersion,
                                       ShaderType frontShaderType,
                                       ShaderType backShaderType,
                                       bool isSeparable,
                                       std::string *mismatchedStructFieldName)
{
    // [ES 3.2 spec] 7.4.1 Shader Interface Matching:
    // Tessellation control shader per-vertex output variables and blocks and tessellation control,
    // tessellation evaluation, and geometry shader per-vertex input variables and blocks are
    // required to be declared as arrays, with each element representing input or output values for
    // a single vertex of a multi-vertex primitive. For the purposes of interface matching, such
    // variables and blocks are treated as though they were not declared as arrays.
    bool treatOutputAsNonArray =
        (frontShaderType == ShaderType::TessControl && !outputVarying.isPatch);
    bool treatInputAsNonArray =
        ((backShaderType == ShaderType::TessControl ||
          backShaderType == ShaderType::TessEvaluation || backShaderType == ShaderType::Geometry) &&
         !inputVarying.isPatch);

    // Skip the validation on the array sizes between a vertex output varying and a geometry input
    // varying as it has been done before.
    bool validatePrecision      = isSeparable && (shaderVersion > 100);
    LinkMismatchError linkError = LinkValidateProgramVariables(
        outputVarying, inputVarying, validatePrecision, treatOutputAsNonArray, treatInputAsNonArray,
        mismatchedStructFieldName);
    if (linkError != LinkMismatchError::NO_MISMATCH)
    {
        return linkError;
    }

    // Explicit locations must match if the names match.
    if (outputVarying.isSameNameAtLinkTime(inputVarying) &&
        outputVarying.location != inputVarying.location)
    {
        return LinkMismatchError::LOCATION_MISMATCH;
    }

    if (!sh::InterpolationTypesMatch(outputVarying.interpolation, inputVarying.interpolation))
    {
        return LinkMismatchError::INTERPOLATION_TYPE_MISMATCH;
    }

    if (shaderVersion == 100 && outputVarying.isInvariant != inputVarying.isInvariant)
    {
        return LinkMismatchError::INVARIANCE_MISMATCH;
    }

    return LinkMismatchError::NO_MISMATCH;
}

bool DoShaderVariablesMatch(int frontShaderVersion,
                            ShaderType frontShaderType,
                            ShaderType backShaderType,
                            const sh::ShaderVariable &input,
                            const sh::ShaderVariable &output,
                            bool isSeparable,
                            gl::InfoLog &infoLog)
{
    bool namesMatch     = input.isSameNameAtLinkTime(output);
    bool locationsMatch = input.location != -1 && input.location == output.location;

    // An output block is considered to match an input block in the subsequent
    // shader if the two blocks have the same block name, and the members of the
    // block match exactly in name, type, qualification, and declaration order.
    //
    // - For the purposes of shader interface matching, the gl_PointSize
    //   member of the intrinsically declared gl_PerVertex shader interface
    //   block is ignored.
    // - Output blocks that do not match in name, but have a location and match
    //   in every other way listed above may be considered to match by some
    //   implementations, but not all - so this behaviour should not be relied
    //   upon.

    // An output variable is considered to match an input variable in the subsequent
    // shader if:
    //
    // - the two variables match in name, type, and qualification; or
    // - the two variables are declared with the same location qualifier and
    //   match in type and qualification.

    if (namesMatch || locationsMatch)
    {
        std::string mismatchedStructFieldName;
        LinkMismatchError linkError =
            LinkValidateVaryings(output, input, frontShaderVersion, frontShaderType, backShaderType,
                                 isSeparable, &mismatchedStructFieldName);
        if (linkError != LinkMismatchError::NO_MISMATCH)
        {
            LogLinkMismatch(infoLog, input.name, "varying", linkError, mismatchedStructFieldName,
                            frontShaderType, backShaderType);
            return false;
        }

        return true;
    }

    return false;
}

const char *GetInterfaceBlockTypeString(sh::BlockType blockType)
{
    switch (blockType)
    {
        case sh::BlockType::kBlockUniform:
            return "uniform block";
        case sh::BlockType::kBlockBuffer:
            return "shader storage block";
        default:
            UNREACHABLE();
            return "";
    }
}

std::string GetInterfaceBlockLimitName(ShaderType shaderType, sh::BlockType blockType)
{
    std::ostringstream stream;
    stream << "GL_MAX_" << GetShaderTypeString(shaderType) << "_";

    switch (blockType)
    {
        case sh::BlockType::kBlockUniform:
            stream << "UNIFORM_BUFFERS";
            break;
        case sh::BlockType::kBlockBuffer:
            stream << "SHADER_STORAGE_BLOCKS";
            break;
        default:
            UNREACHABLE();
            return "";
    }

    if (shaderType == ShaderType::Geometry)
    {
        stream << "_EXT";
    }

    return stream.str();
}

void LogInterfaceBlocksExceedLimit(InfoLog &infoLog,
                                   ShaderType shaderType,
                                   sh::BlockType blockType,
                                   GLuint limit)
{
    infoLog << GetShaderTypeString(shaderType) << " shader "
            << GetInterfaceBlockTypeString(blockType) << " count exceeds "
            << GetInterfaceBlockLimitName(shaderType, blockType) << " (" << limit << ")";
}

bool ValidateInterfaceBlocksCount(GLuint maxInterfaceBlocks,
                                  const std::vector<sh::InterfaceBlock> &interfaceBlocks,
                                  ShaderType shaderType,
                                  sh::BlockType blockType,
                                  GLuint *combinedInterfaceBlocksCount,
                                  InfoLog &infoLog)
{
    GLuint blockCount = 0;
    for (const sh::InterfaceBlock &block : interfaceBlocks)
    {
        if (IsActiveInterfaceBlock(block))
        {
            blockCount += std::max(block.arraySize, 1u);
            if (blockCount > maxInterfaceBlocks)
            {
                LogInterfaceBlocksExceedLimit(infoLog, shaderType, blockType, maxInterfaceBlocks);
                return false;
            }
        }
    }

    // [OpenGL ES 3.1] Chapter 7.6.2 Page 105:
    // If a uniform block is used by multiple shader stages, each such use counts separately
    // against this combined limit.
    // [OpenGL ES 3.1] Chapter 7.8 Page 111:
    // If a shader storage block in a program is referenced by multiple shaders, each such
    // reference counts separately against this combined limit.
    if (combinedInterfaceBlocksCount)
    {
        *combinedInterfaceBlocksCount += blockCount;
    }

    return true;
}
}  // anonymous namespace

// UsedUniform implementation
UsedUniform::UsedUniform() {}

UsedUniform::UsedUniform(GLenum typeIn,
                         GLenum precisionIn,
                         const std::string &nameIn,
                         const std::vector<unsigned int> &arraySizesIn,
                         const int bindingIn,
                         const int offsetIn,
                         const int locationIn,
                         const int bufferIndexIn,
                         const sh::BlockMemberInfo &blockInfoIn)
    : typeInfo(&GetUniformTypeInfo(typeIn)),
      bufferIndex(bufferIndexIn),
      blockInfo(blockInfoIn),
      outerArrayOffset(0)
{
    type       = typeIn;
    precision  = precisionIn;
    name       = nameIn;
    arraySizes = arraySizesIn;
    binding    = bindingIn;
    offset     = offsetIn;
    location   = locationIn;
    ASSERT(!isArrayOfArrays());
    ASSERT(!isArray() || !isStruct());
}

UsedUniform::UsedUniform(const UsedUniform &other)
{
    *this = other;
}

UsedUniform &UsedUniform::operator=(const UsedUniform &other)
{
    if (this != &other)
    {
        sh::ShaderVariable::operator=(other);
        activeVariable = other.activeVariable;

        typeInfo         = other.typeInfo;
        bufferIndex      = other.bufferIndex;
        blockInfo        = other.blockInfo;
        outerArraySizes  = other.outerArraySizes;
        outerArrayOffset = other.outerArrayOffset;
    }
    return *this;
}

UsedUniform::~UsedUniform() {}

// UniformLinker implementation
UniformLinker::UniformLinker(const ShaderBitSet &activeShaderStages,
                             const ShaderMap<std::vector<sh::ShaderVariable>> &shaderUniforms)
    : mActiveShaderStages(activeShaderStages), mShaderUniforms(shaderUniforms)
{}

UniformLinker::~UniformLinker() = default;

void UniformLinker::getResults(std::vector<LinkedUniform> *uniforms,
                               std::vector<std::string> *uniformNames,
                               std::vector<std::string> *uniformMappedNames,
                               std::vector<UnusedUniform> *unusedUniformsOutOrNull,
                               std::vector<VariableLocation> *uniformLocationsOutOrNull)
{
    uniforms->reserve(mUniforms.size());
    uniformNames->reserve(mUniforms.size());
    uniformMappedNames->reserve(mUniforms.size());
    for (const UsedUniform &usedUniform : mUniforms)
    {
        uniforms->emplace_back(usedUniform);
        uniformNames->emplace_back(usedUniform.name);
        uniformMappedNames->emplace_back(usedUniform.mappedName);
    }

    if (unusedUniformsOutOrNull)
    {
        unusedUniformsOutOrNull->swap(mUnusedUniforms);
    }

    if (uniformLocationsOutOrNull)
    {
        uniformLocationsOutOrNull->swap(mUniformLocations);
    }
}

bool UniformLinker::link(const Caps &caps,
                         InfoLog &infoLog,
                         const ProgramAliasedBindings &uniformLocationBindings)
{
    if (mActiveShaderStages[ShaderType::Vertex] && mActiveShaderStages[ShaderType::Fragment])
    {
        if (!validateGraphicsUniforms(infoLog))
        {
            return false;
        }
    }

    // Flatten the uniforms list (nested fields) into a simple list (no nesting).
    // Also check the maximum uniform vector and sampler counts.
    if (!flattenUniformsAndCheckCaps(caps, infoLog))
    {
        return false;
    }

    if (!checkMaxCombinedAtomicCounters(caps, infoLog))
    {
        return false;
    }

    if (!indexUniforms(infoLog, uniformLocationBindings))
    {
        return false;
    }

    return true;
}

bool UniformLinker::validateGraphicsUniforms(InfoLog &infoLog) const
{
    // Check that uniforms defined in the graphics shaders are identical
    std::map<std::string, ShaderUniform> linkedUniforms;

    for (const ShaderType shaderType : mActiveShaderStages)
    {
        if (shaderType == ShaderType::Vertex)
        {
            for (const sh::ShaderVariable &vertexUniform : mShaderUniforms[ShaderType::Vertex])
            {
                linkedUniforms[vertexUniform.name] =
                    std::make_pair(ShaderType::Vertex, &vertexUniform);
            }
        }
        else
        {
            bool isLastShader = (shaderType == ShaderType::Fragment);
            if (!validateGraphicsUniformsPerShader(shaderType, !isLastShader, &linkedUniforms,
                                                   infoLog))
            {
                return false;
            }
        }
    }

    return true;
}

bool UniformLinker::validateGraphicsUniformsPerShader(
    ShaderType shaderToLink,
    bool extendLinkedUniforms,
    std::map<std::string, ShaderUniform> *linkedUniforms,
    InfoLog &infoLog) const
{
    ASSERT(mActiveShaderStages[shaderToLink] && linkedUniforms);

    for (const sh::ShaderVariable &uniform : mShaderUniforms[shaderToLink])
    {
        const auto &entry = linkedUniforms->find(uniform.name);
        if (entry != linkedUniforms->end())
        {
            const sh::ShaderVariable &linkedUniform = *(entry->second.second);
            std::string mismatchedStructFieldName;
            LinkMismatchError linkError =
                LinkValidateUniforms(uniform, linkedUniform, &mismatchedStructFieldName);
            if (linkError != LinkMismatchError::NO_MISMATCH)
            {
                LogLinkMismatch(infoLog, uniform.name, "uniform", linkError,
                                mismatchedStructFieldName, entry->second.first, shaderToLink);
                return false;
            }
        }
        else if (extendLinkedUniforms)
        {
            (*linkedUniforms)[uniform.name] = std::make_pair(shaderToLink, &uniform);
        }
    }

    return true;
}

bool UniformLinker::indexUniforms(InfoLog &infoLog,
                                  const ProgramAliasedBindings &uniformLocationBindings)
{
    // Locations which have been allocated for an unused uniform.
    std::set<GLuint> ignoredLocations;

    int maxUniformLocation = -1;

    // Gather uniform locations that have been set either using the bindUniformLocationCHROMIUM API
    // or by using a location layout qualifier and check conflicts between them.
    if (!gatherUniformLocationsAndCheckConflicts(infoLog, uniformLocationBindings,
                                                 &ignoredLocations, &maxUniformLocation))
    {
        return false;
    }

    // Conflicts have been checked, now we can prune non-statically used uniforms. Code further down
    // the line relies on only having statically used uniforms in mUniforms.
    pruneUnusedUniforms();

    // Gather uniforms that have their location pre-set and uniforms that don't yet have a location.
    std::vector<VariableLocation> unlocatedUniforms;
    std::map<GLuint, VariableLocation> preLocatedUniforms;

    for (size_t uniformIndex = 0; uniformIndex < mUniforms.size(); uniformIndex++)
    {
        const UsedUniform &uniform = mUniforms[uniformIndex];

        if ((uniform.isBuiltIn() && !uniform.isEmulatedBuiltIn()) ||
            IsAtomicCounterType(uniform.type) || uniform.isFragmentInOut)
        {
            continue;
        }

        int preSetLocation = uniformLocationBindings.getBinding(uniform);
        int shaderLocation = uniform.location;

        if (shaderLocation != -1)
        {
            preSetLocation = shaderLocation;
        }

        unsigned int elementCount = uniform.getBasicTypeElementCount();
        for (unsigned int arrayIndex = 0; arrayIndex < elementCount; arrayIndex++)
        {
            VariableLocation location(arrayIndex, static_cast<unsigned int>(uniformIndex));

            if ((arrayIndex == 0 && preSetLocation != -1) || shaderLocation != -1)
            {
                int elementLocation                 = preSetLocation + arrayIndex;
                preLocatedUniforms[elementLocation] = location;
            }
            else
            {
                unlocatedUniforms.push_back(location);
            }
        }
    }

    // Make enough space for all uniforms, with pre-set locations or not.
    mUniformLocations.resize(
        std::max(unlocatedUniforms.size() + preLocatedUniforms.size() + ignoredLocations.size(),
                 static_cast<size_t>(maxUniformLocation + 1)));

    // Assign uniforms with pre-set locations
    for (const auto &uniform : preLocatedUniforms)
    {
        mUniformLocations[uniform.first] = uniform.second;
    }

    // Assign ignored uniforms
    for (const auto &ignoredLocation : ignoredLocations)
    {
        mUniformLocations[ignoredLocation].markIgnored();
    }

    // Automatically assign locations for the rest of the uniforms
    size_t nextUniformLocation = 0;
    for (const auto &unlocatedUniform : unlocatedUniforms)
    {
        while (mUniformLocations[nextUniformLocation].used() ||
               mUniformLocations[nextUniformLocation].ignored)
        {
            nextUniformLocation++;
        }

        ASSERT(nextUniformLocation < mUniformLocations.size());
        mUniformLocations[nextUniformLocation] = unlocatedUniform;
        nextUniformLocation++;
    }

    return true;
}

bool UniformLinker::gatherUniformLocationsAndCheckConflicts(
    InfoLog &infoLog,
    const ProgramAliasedBindings &uniformLocationBindings,
    std::set<GLuint> *ignoredLocations,
    int *maxUniformLocation)
{
    // All the locations where another uniform can't be located.
    std::set<GLuint> reservedLocations;

    for (const UsedUniform &uniform : mUniforms)
    {
        if ((uniform.isBuiltIn() && !uniform.isEmulatedBuiltIn()) || uniform.isFragmentInOut)
        {
            // The uniform of the fragment inout is not a normal uniform type. So, in the case of
            // the fragment inout, this routine should be skipped.
            continue;
        }

        int apiBoundLocation = uniformLocationBindings.getBinding(uniform);
        int shaderLocation   = uniform.location;

        if (shaderLocation != -1)
        {
            unsigned int elementCount = uniform.getBasicTypeElementCount();

            for (unsigned int arrayIndex = 0; arrayIndex < elementCount; arrayIndex++)
            {
                // GLSL ES 3.10 section 4.4.3
                int elementLocation = shaderLocation + arrayIndex;
                *maxUniformLocation = std::max(*maxUniformLocation, elementLocation);
                if (reservedLocations.find(elementLocation) != reservedLocations.end())
                {
                    infoLog << "Multiple uniforms bound to location " << elementLocation << ".";
                    return false;
                }
                reservedLocations.insert(elementLocation);
                if (!uniform.active)
                {
                    ignoredLocations->insert(elementLocation);
                }
            }
        }
        else if (apiBoundLocation != -1 && uniform.staticUse)
        {
            // Only the first location is reserved even if the uniform is an array.
            *maxUniformLocation = std::max(*maxUniformLocation, apiBoundLocation);
            if (reservedLocations.find(apiBoundLocation) != reservedLocations.end())
            {
                infoLog << "Multiple uniforms bound to location " << apiBoundLocation << ".";
                return false;
            }
            reservedLocations.insert(apiBoundLocation);
            if (!uniform.active)
            {
                ignoredLocations->insert(apiBoundLocation);
            }
        }
    }

    // Record the uniform locations that were bound using the API for uniforms that were not found
    // from the shader. Other uniforms should not be assigned to those locations.
    for (const auto &locationBinding : uniformLocationBindings)
    {
        GLuint location = locationBinding.second.location;
        if (reservedLocations.find(location) == reservedLocations.end())
        {
            ignoredLocations->insert(location);
            *maxUniformLocation = std::max(*maxUniformLocation, static_cast<int>(location));
        }
    }

    return true;
}

void UniformLinker::pruneUnusedUniforms()
{
    auto uniformIter = mUniforms.begin();
    while (uniformIter != mUniforms.end())
    {
        if (uniformIter->active)
        {
            ++uniformIter;
        }
        else
        {
            mUnusedUniforms.emplace_back(uniformIter->name, uniformIter->isSampler(),
                                         uniformIter->isImage(), uniformIter->isAtomicCounter(),
                                         uniformIter->isFragmentInOut);
            uniformIter = mUniforms.erase(uniformIter);
        }
    }
}

bool UniformLinker::flattenUniformsAndCheckCapsForShader(
    ShaderType shaderType,
    const Caps &caps,
    std::vector<UsedUniform> &samplerUniforms,
    std::vector<UsedUniform> &imageUniforms,
    std::vector<UsedUniform> &atomicCounterUniforms,
    std::vector<UsedUniform> &inputAttachmentUniforms,
    std::vector<UnusedUniform> &unusedUniforms,
    InfoLog &infoLog)
{
    ShaderUniformCount shaderUniformCount;
    for (const sh::ShaderVariable &uniform : mShaderUniforms[shaderType])
    {
        FlattenUniformVisitor flattener(shaderType, uniform, &mUniforms, &samplerUniforms,
                                        &imageUniforms, &atomicCounterUniforms,
                                        &inputAttachmentUniforms, &unusedUniforms);
        sh::TraverseShaderVariable(uniform, false, &flattener);

        if (uniform.active)
        {
            shaderUniformCount += flattener.getCounts();
        }
        else
        {
            unusedUniforms.emplace_back(uniform.name, IsSamplerType(uniform.type),
                                        IsImageType(uniform.type),
                                        IsAtomicCounterType(uniform.type), uniform.isFragmentInOut);
        }
    }

    // This code does not do fine-grained component counting.
    GLuint maxUniformVectorsCount = GetMaximumShaderUniformVectors(shaderType, caps);
    if (shaderUniformCount.vectorCount > maxUniformVectorsCount)
    {
        GLuint maxUniforms = 0u;

        // See comments in GetUniformResourceLimitName()
        if (shaderType == ShaderType::Vertex || shaderType == ShaderType::Fragment)
        {
            maxUniforms = maxUniformVectorsCount;
        }
        else
        {
            maxUniforms = maxUniformVectorsCount * 4;
        }

        LogUniformsExceedLimit(shaderType, UniformType::Variable, maxUniforms, infoLog);
        return false;
    }

    if (shaderUniformCount.samplerCount >
        static_cast<GLuint>(caps.maxShaderTextureImageUnits[shaderType]))
    {
        LogUniformsExceedLimit(shaderType, UniformType::Sampler,
                               caps.maxShaderTextureImageUnits[shaderType], infoLog);
        return false;
    }

    if (shaderUniformCount.imageCount >
        static_cast<GLuint>(caps.maxShaderImageUniforms[shaderType]))
    {
        LogUniformsExceedLimit(shaderType, UniformType::Image,
                               caps.maxShaderImageUniforms[shaderType], infoLog);
        return false;
    }

    if (shaderUniformCount.atomicCounterCount >
        static_cast<GLuint>(caps.maxShaderAtomicCounters[shaderType]))
    {
        LogUniformsExceedLimit(shaderType, UniformType::AtomicCounter,
                               caps.maxShaderAtomicCounters[shaderType], infoLog);
        return false;
    }

    return true;
}

bool UniformLinker::flattenUniformsAndCheckCaps(const Caps &caps, InfoLog &infoLog)
{
    std::vector<UsedUniform> samplerUniforms;
    std::vector<UsedUniform> imageUniforms;
    std::vector<UsedUniform> atomicCounterUniforms;
    std::vector<UsedUniform> inputAttachmentUniforms;
    std::vector<UnusedUniform> unusedUniforms;

    for (const ShaderType shaderType : mActiveShaderStages)
    {
        if (!flattenUniformsAndCheckCapsForShader(shaderType, caps, samplerUniforms, imageUniforms,
                                                  atomicCounterUniforms, inputAttachmentUniforms,
                                                  unusedUniforms, infoLog))
        {
            return false;
        }
    }

    mUniforms.insert(mUniforms.end(), samplerUniforms.begin(), samplerUniforms.end());
    mUniforms.insert(mUniforms.end(), imageUniforms.begin(), imageUniforms.end());
    mUniforms.insert(mUniforms.end(), atomicCounterUniforms.begin(), atomicCounterUniforms.end());
    mUniforms.insert(mUniforms.end(), inputAttachmentUniforms.begin(),
                     inputAttachmentUniforms.end());
    mUnusedUniforms.insert(mUnusedUniforms.end(), unusedUniforms.begin(), unusedUniforms.end());
    return true;
}

bool UniformLinker::checkMaxCombinedAtomicCounters(const Caps &caps, InfoLog &infoLog)
{
    unsigned int atomicCounterCount = 0;
    for (const auto &uniform : mUniforms)
    {
        if (IsAtomicCounterType(uniform.type) && uniform.active)
        {
            atomicCounterCount += uniform.getBasicTypeElementCount();
            if (atomicCounterCount > static_cast<GLuint>(caps.maxCombinedAtomicCounters))
            {
                infoLog << "atomic counter count exceeds MAX_COMBINED_ATOMIC_COUNTERS"
                        << caps.maxCombinedAtomicCounters << ").";
                return false;
            }
        }
    }
    return true;
}

// InterfaceBlockLinker implementation.
InterfaceBlockLinker::InterfaceBlockLinker() = default;

InterfaceBlockLinker::~InterfaceBlockLinker() = default;

void InterfaceBlockLinker::init(std::vector<InterfaceBlock> *blocksOut,
                                std::vector<std::string> *unusedInterfaceBlocksOut)
{
    mBlocksOut                = blocksOut;
    mUnusedInterfaceBlocksOut = unusedInterfaceBlocksOut;
}

void InterfaceBlockLinker::addShaderBlocks(ShaderType shaderType,
                                           const std::vector<sh::InterfaceBlock> *blocks)
{
    mShaderBlocks[shaderType] = blocks;
}

void InterfaceBlockLinker::linkBlocks(const GetBlockSizeFunc &getBlockSize,
                                      const GetBlockMemberInfoFunc &getMemberInfo) const
{
    ASSERT(mBlocksOut->empty());

    std::set<std::string> visitedList;

    for (const ShaderType shaderType : AllShaderTypes())
    {
        if (!mShaderBlocks[shaderType])
        {
            continue;
        }

        for (const sh::InterfaceBlock &block : *mShaderBlocks[shaderType])
        {
            if (!IsActiveInterfaceBlock(block))
            {
                mUnusedInterfaceBlocksOut->push_back(block.name);
                continue;
            }

            if (visitedList.count(block.name) == 0)
            {
                defineInterfaceBlock(getBlockSize, getMemberInfo, block, shaderType);
                visitedList.insert(block.name);
                continue;
            }

            if (!block.active)
            {
                mUnusedInterfaceBlocksOut->push_back(block.name);
                continue;
            }

            for (InterfaceBlock &priorBlock : *mBlocksOut)
            {
                if (block.name == priorBlock.name)
                {
                    priorBlock.setActive(shaderType, true, block.id);

                    std::unique_ptr<sh::ShaderVariableVisitor> visitor(
                        getVisitor(getMemberInfo, block.fieldPrefix(), block.fieldMappedPrefix(),
                                   shaderType, -1));

                    sh::TraverseShaderVariables(block.fields, false, visitor.get());
                }
            }
        }
    }
}

void InterfaceBlockLinker::defineInterfaceBlock(const GetBlockSizeFunc &getBlockSize,
                                                const GetBlockMemberInfoFunc &getMemberInfo,
                                                const sh::InterfaceBlock &interfaceBlock,
                                                ShaderType shaderType) const
{
    size_t blockSize = 0;
    std::vector<unsigned int> blockIndexes;

    const int blockIndex = static_cast<int>(mBlocksOut->size());
    // Track the first and last block member index to determine the range of active block members in
    // the block.
    const size_t firstBlockMemberIndex = getCurrentBlockMemberIndex();

    std::unique_ptr<sh::ShaderVariableVisitor> visitor(
        getVisitor(getMemberInfo, interfaceBlock.fieldPrefix(), interfaceBlock.fieldMappedPrefix(),
                   shaderType, blockIndex));
    sh::TraverseShaderVariables(interfaceBlock.fields, false, visitor.get());

    const size_t lastBlockMemberIndex = getCurrentBlockMemberIndex();

    for (size_t blockMemberIndex = firstBlockMemberIndex; blockMemberIndex < lastBlockMemberIndex;
         ++blockMemberIndex)
    {
        blockIndexes.push_back(static_cast<unsigned int>(blockMemberIndex));
    }

    const unsigned int firstFieldArraySize = interfaceBlock.fields[0].getArraySizeProduct();

    for (unsigned int arrayElement = 0; arrayElement < interfaceBlock.elementCount();
         ++arrayElement)
    {
        std::string blockArrayName       = interfaceBlock.name;
        std::string blockMappedArrayName = interfaceBlock.mappedName;
        if (interfaceBlock.isArray())
        {
            blockArrayName += ArrayString(arrayElement);
            blockMappedArrayName += ArrayString(arrayElement);
        }

        // Don't define this block at all if it's not active in the implementation.
        if (!getBlockSize(blockArrayName, blockMappedArrayName, &blockSize))
        {
            continue;
        }

        // ESSL 3.10 section 4.4.4 page 58:
        // Any uniform or shader storage block declared without a binding qualifier is initially
        // assigned to block binding point zero.
        const int blockBinding =
            (interfaceBlock.binding == -1 ? 0 : interfaceBlock.binding + arrayElement);
        InterfaceBlock block(interfaceBlock.name, interfaceBlock.mappedName,
                             interfaceBlock.isArray(), interfaceBlock.isReadOnly, arrayElement,
                             firstFieldArraySize, blockBinding);
        block.memberIndexes = blockIndexes;
        block.setActive(shaderType, interfaceBlock.active, interfaceBlock.id);

        // Since all block elements in an array share the same active interface blocks, they
        // will all be active once any block member is used. So, since interfaceBlock.name[0]
        // was active, here we will add every block element in the array.
        block.pod.dataSize = static_cast<unsigned int>(blockSize);
        mBlocksOut->push_back(block);
    }
}

// UniformBlockLinker implementation.
UniformBlockLinker::UniformBlockLinker() = default;

UniformBlockLinker::~UniformBlockLinker() {}

void UniformBlockLinker::init(std::vector<InterfaceBlock> *blocksOut,
                              std::vector<LinkedUniform> *uniformsOut,
                              std::vector<std::string> *uniformNamesOut,
                              std::vector<std::string> *uniformMappedNamesOut,
                              std::vector<std::string> *unusedInterfaceBlocksOut)
{
    InterfaceBlockLinker::init(blocksOut, unusedInterfaceBlocksOut);
    mUniformsOut           = uniformsOut;
    mUniformNamesOut       = uniformNamesOut;
    mUniformMappedNamesOut = uniformMappedNamesOut;
}

size_t UniformBlockLinker::getCurrentBlockMemberIndex() const
{
    return mUniformsOut->size();
}

sh::ShaderVariableVisitor *UniformBlockLinker::getVisitor(
    const GetBlockMemberInfoFunc &getMemberInfo,
    const std::string &namePrefix,
    const std::string &mappedNamePrefix,
    ShaderType shaderType,
    int blockIndex) const
{
    return new UniformBlockEncodingVisitor(getMemberInfo, namePrefix, mappedNamePrefix,
                                           mUniformsOut, mUniformNamesOut, mUniformMappedNamesOut,
                                           shaderType, blockIndex);
}

// ShaderStorageBlockLinker implementation.
ShaderStorageBlockLinker::ShaderStorageBlockLinker() = default;

ShaderStorageBlockLinker::~ShaderStorageBlockLinker() = default;

void ShaderStorageBlockLinker::init(std::vector<InterfaceBlock> *blocksOut,
                                    std::vector<BufferVariable> *bufferVariablesOut,
                                    std::vector<std::string> *unusedInterfaceBlocksOut)
{
    InterfaceBlockLinker::init(blocksOut, unusedInterfaceBlocksOut);
    mBufferVariablesOut = bufferVariablesOut;
}

size_t ShaderStorageBlockLinker::getCurrentBlockMemberIndex() const
{
    return mBufferVariablesOut->size();
}

sh::ShaderVariableVisitor *ShaderStorageBlockLinker::getVisitor(
    const GetBlockMemberInfoFunc &getMemberInfo,
    const std::string &namePrefix,
    const std::string &mappedNamePrefix,
    ShaderType shaderType,
    int blockIndex) const
{
    return new ShaderStorageBlockVisitor(getMemberInfo, namePrefix, mappedNamePrefix,
                                         mBufferVariablesOut, shaderType, blockIndex);
}

// AtomicCounterBufferLinker implementation.
AtomicCounterBufferLinker::AtomicCounterBufferLinker() = default;

AtomicCounterBufferLinker::~AtomicCounterBufferLinker() = default;

void AtomicCounterBufferLinker::init(std::vector<AtomicCounterBuffer> *atomicCounterBuffersOut)
{
    mAtomicCounterBuffersOut = atomicCounterBuffersOut;
}

void AtomicCounterBufferLinker::link(const std::map<int, unsigned int> &sizeMap) const
{
    for (auto &atomicCounterBuffer : *mAtomicCounterBuffersOut)
    {
        auto bufferSize = sizeMap.find(atomicCounterBuffer.pod.inShaderBinding);
        ASSERT(bufferSize != sizeMap.end());
        atomicCounterBuffer.pod.dataSize = bufferSize->second;
    }
}

PixelLocalStorageLinker::PixelLocalStorageLinker() = default;

PixelLocalStorageLinker::~PixelLocalStorageLinker() = default;

void PixelLocalStorageLinker::init(
    std::vector<ShPixelLocalStorageFormat> *pixelLocalStorageFormatsOut)
{
    mPixelLocalStorageFormatsOut = pixelLocalStorageFormatsOut;
}

void PixelLocalStorageLinker::link(
    const std::vector<ShPixelLocalStorageFormat> &pixelLocalStorageFormats) const
{
    *mPixelLocalStorageFormatsOut = pixelLocalStorageFormats;
}

LinkingVariables::LinkingVariables()  = default;
LinkingVariables::~LinkingVariables() = default;

void LinkingVariables::initForProgram(const ProgramState &state)
{
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        const SharedCompiledShaderState &shader = state.getAttachedShader(shaderType);
        if (shader)
        {
            outputVaryings[shaderType] = shader->outputVaryings;
            inputVaryings[shaderType]  = shader->inputVaryings;
            uniforms[shaderType]       = shader->uniforms;
            uniformBlocks[shaderType]  = shader->uniformBlocks;
            isShaderStageUsedBitset.set(shaderType);
        }
    }
}

void LinkingVariables::initForProgramPipeline(const ProgramPipelineState &state)
{
    for (ShaderType shaderType : state.getExecutable().getLinkedShaderStages())
    {
        const SharedProgramExecutable &executable = state.getShaderProgramExecutable(shaderType);
        ASSERT(executable);
        outputVaryings[shaderType] = executable->getLinkedOutputVaryings(shaderType);
        inputVaryings[shaderType]  = executable->getLinkedInputVaryings(shaderType);
        uniforms[shaderType]       = executable->getLinkedUniforms(shaderType);
        uniformBlocks[shaderType]  = executable->getLinkedUniformBlocks(shaderType);
        isShaderStageUsedBitset.set(shaderType);
    }
}

ProgramLinkedResources::ProgramLinkedResources()  = default;
ProgramLinkedResources::~ProgramLinkedResources() = default;

void ProgramLinkedResources::init(
    std::vector<InterfaceBlock> *uniformBlocksOut,
    std::vector<LinkedUniform> *uniformsOut,
    std::vector<std::string> *uniformNamesOut,
    std::vector<std::string> *uniformMappedNamesOut,
    std::vector<InterfaceBlock> *shaderStorageBlocksOut,
    std::vector<BufferVariable> *bufferVariablesOut,
    std::vector<AtomicCounterBuffer> *atomicCounterBuffersOut,
    std::vector<ShPixelLocalStorageFormat> *pixelLocalStorageFormatsOut)
{
    uniformBlockLinker.init(uniformBlocksOut, uniformsOut, uniformNamesOut, uniformMappedNamesOut,
                            &unusedInterfaceBlocks);
    shaderStorageBlockLinker.init(shaderStorageBlocksOut, bufferVariablesOut,
                                  &unusedInterfaceBlocks);
    atomicCounterBufferLinker.init(atomicCounterBuffersOut);
    pixelLocalStorageLinker.init(pixelLocalStorageFormatsOut);
}

void ProgramLinkedResourcesLinker::linkResources(const ProgramState &programState,
                                                 const ProgramLinkedResources &resources) const
{
    // Gather uniform interface block info.
    InterfaceBlockInfo uniformBlockInfo(mCustomEncoderFactory);
    for (const ShaderType shaderType : AllShaderTypes())
    {
        const SharedCompiledShaderState &shader = programState.getAttachedShader(shaderType);
        if (shader)
        {
            uniformBlockInfo.getShaderBlockInfo(shader->uniformBlocks);
        }
    }

    auto getUniformBlockSize = [&uniformBlockInfo](const std::string &name,
                                                   const std::string &mappedName, size_t *sizeOut) {
        return uniformBlockInfo.getBlockSize(name, mappedName, sizeOut);
    };

    auto getUniformBlockMemberInfo = [&uniformBlockInfo](const std::string &name,
                                                         const std::string &mappedName,
                                                         sh::BlockMemberInfo *infoOut) {
        return uniformBlockInfo.getBlockMemberInfo(name, mappedName, infoOut);
    };

    // Link uniform interface blocks.
    resources.uniformBlockLinker.linkBlocks(getUniformBlockSize, getUniformBlockMemberInfo);

    // Gather storage buffer interface block info.
    InterfaceBlockInfo shaderStorageBlockInfo(mCustomEncoderFactory);
    for (const ShaderType shaderType : AllShaderTypes())
    {
        const SharedCompiledShaderState &shader = programState.getAttachedShader(shaderType);
        if (shader)
        {
            shaderStorageBlockInfo.getShaderBlockInfo(shader->shaderStorageBlocks);
        }
    }
    auto getShaderStorageBlockSize = [&shaderStorageBlockInfo](const std::string &name,
                                                               const std::string &mappedName,
                                                               size_t *sizeOut) {
        return shaderStorageBlockInfo.getBlockSize(name, mappedName, sizeOut);
    };

    auto getShaderStorageBlockMemberInfo = [&shaderStorageBlockInfo](const std::string &name,
                                                                     const std::string &mappedName,
                                                                     sh::BlockMemberInfo *infoOut) {
        return shaderStorageBlockInfo.getBlockMemberInfo(name, mappedName, infoOut);
    };

    // Link storage buffer interface blocks.
    resources.shaderStorageBlockLinker.linkBlocks(getShaderStorageBlockSize,
                                                  getShaderStorageBlockMemberInfo);

    // Gather and link atomic counter buffer interface blocks.
    std::map<int, unsigned int> sizeMap;
    getAtomicCounterBufferSizeMap(programState.getExecutable(), sizeMap);
    resources.atomicCounterBufferLinker.link(sizeMap);

    const gl::SharedCompiledShaderState &fragmentShader =
        programState.getAttachedShader(gl::ShaderType::Fragment);
    if (fragmentShader != nullptr)
    {
        resources.pixelLocalStorageLinker.link(fragmentShader->pixelLocalStorageFormats);
    }
}

void ProgramLinkedResourcesLinker::getAtomicCounterBufferSizeMap(
    const ProgramExecutable &executable,
    std::map<int, unsigned int> &sizeMapOut) const
{
    for (unsigned int index : executable.getAtomicCounterUniformRange())
    {
        const LinkedUniform &glUniform = executable.getUniforms()[index];

        auto &bufferDataSize = sizeMapOut[glUniform.getBinding()];

        // Calculate the size of the buffer by finding the end of the last uniform with the same
        // binding. The end of the uniform is calculated by finding the initial offset of the
        // uniform and adding size of the uniform. For arrays, the size is the number of elements
        // times the element size (should always by 4 for atomic_units).
        unsigned dataOffset =
            glUniform.getOffset() + static_cast<unsigned int>(glUniform.getBasicTypeElementCount() *
                                                              glUniform.getElementSize());
        if (dataOffset > bufferDataSize)
        {
            bufferDataSize = dataOffset;
        }
    }
}

bool LinkValidateProgramGlobalNames(InfoLog &infoLog,
                                    const ProgramExecutable &executable,
                                    const LinkingVariables &linkingVariables)
{
    angle::HashMap<std::string, const sh::ShaderVariable *> uniformMap;
    using BlockAndFieldPair = std::pair<const sh::InterfaceBlock *, const sh::ShaderVariable *>;
    angle::HashMap<std::string, std::vector<BlockAndFieldPair>> uniformBlockFieldMap;

    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        if (!linkingVariables.isShaderStageUsedBitset[shaderType])
        {
            continue;
        }

        // Build a map of Uniforms
        const std::vector<sh::ShaderVariable> &uniforms = linkingVariables.uniforms[shaderType];
        for (const auto &uniform : uniforms)
        {
            uniformMap[uniform.name] = &uniform;
        }

        // Build a map of Uniform Blocks
        // This will also detect any field name conflicts between Uniform Blocks without instance
        // names
        const std::vector<sh::InterfaceBlock> &uniformBlocks =
            linkingVariables.uniformBlocks[shaderType];

        for (const auto &uniformBlock : uniformBlocks)
        {
            // Only uniform blocks without an instance name can create a conflict with their field
            // names
            if (!uniformBlock.instanceName.empty())
            {
                continue;
            }

            for (const auto &field : uniformBlock.fields)
            {
                if (!uniformBlockFieldMap.count(field.name))
                {
                    // First time we've seen this uniform block field name, so add the
                    // (Uniform Block, Field) pair immediately since there can't be a conflict yet
                    BlockAndFieldPair blockAndFieldPair(&uniformBlock, &field);
                    std::vector<BlockAndFieldPair> newUniformBlockList;
                    newUniformBlockList.push_back(blockAndFieldPair);
                    uniformBlockFieldMap[field.name] = newUniformBlockList;
                    continue;
                }

                // We've seen this name before.
                // We need to check each of the uniform blocks that contain a field with this name
                // to see if there's a conflict or not.
                std::vector<BlockAndFieldPair> prevBlockFieldPairs =
                    uniformBlockFieldMap[field.name];
                for (const auto &prevBlockFieldPair : prevBlockFieldPairs)
                {
                    const sh::InterfaceBlock *prevUniformBlock      = prevBlockFieldPair.first;
                    const sh::ShaderVariable *prevUniformBlockField = prevBlockFieldPair.second;

                    if (uniformBlock.isSameInterfaceBlockAtLinkTime(*prevUniformBlock))
                    {
                        // The same uniform block should, by definition, contain the same field name
                        continue;
                    }

                    // The uniform blocks don't match, so check if the necessary field properties
                    // also match
                    if ((field.name == prevUniformBlockField->name) &&
                        (field.type == prevUniformBlockField->type) &&
                        (field.precision == prevUniformBlockField->precision))
                    {
                        infoLog << "Name conflicts between uniform block field names: "
                                << field.name;
                        return false;
                    }
                }

                // No conflict, so record this pair
                BlockAndFieldPair blockAndFieldPair(&uniformBlock, &field);
                uniformBlockFieldMap[field.name].push_back(blockAndFieldPair);
            }
        }
    }

    // Validate no uniform names conflict with attribute names
    if (linkingVariables.isShaderStageUsedBitset[ShaderType::Vertex])
    {
        // ESSL 3.00.6 section 4.3.5:
        // If a uniform variable name is declared in one stage (e.g., a vertex shader)
        // but not in another (e.g., a fragment shader), then that name is still
        // available in the other stage for a different use.
        std::unordered_set<std::string> uniforms;
        for (const sh::ShaderVariable &uniform : linkingVariables.uniforms[ShaderType::Vertex])
        {
            uniforms.insert(uniform.name);
        }
        for (const auto &attrib : executable.getProgramInputs())
        {
            if (uniforms.count(attrib.name))
            {
                infoLog << "Name conflicts between a uniform and an attribute: " << attrib.name;
                return false;
            }
        }
    }

    // Validate no Uniform Block fields conflict with other Uniforms
    for (const auto &uniformBlockField : uniformBlockFieldMap)
    {
        const std::string &fieldName = uniformBlockField.first;
        if (uniformMap.count(fieldName))
        {
            infoLog << "Name conflicts between a uniform and a uniform block field: " << fieldName;
            return false;
        }
    }

    return true;
}

// [OpenGL ES 3.2] Chapter 7.4.1 "Shader Interface Matching"
bool LinkValidateShaderInterfaceMatching(const std::vector<sh::ShaderVariable> &outputVaryings,
                                         const std::vector<sh::ShaderVariable> &inputVaryings,
                                         ShaderType frontShaderType,
                                         ShaderType backShaderType,
                                         int frontShaderVersion,
                                         int backShaderVersion,
                                         bool isSeparable,
                                         gl::InfoLog &infoLog)
{
    ASSERT(frontShaderVersion == backShaderVersion);

    std::vector<const sh::ShaderVariable *> filteredInputVaryings;
    std::vector<const sh::ShaderVariable *> filteredOutputVaryings;

    GetFilteredVaryings(inputVaryings, &filteredInputVaryings);
    GetFilteredVaryings(outputVaryings, &filteredOutputVaryings);

    // Separable programs require the number of inputs and outputs match
    if (isSeparable && filteredInputVaryings.size() < filteredOutputVaryings.size())
    {
        infoLog << GetShaderTypeString(backShaderType)
                << " does not consume all varyings generated by "
                << GetShaderTypeString(frontShaderType);
        return false;
    }
    if (isSeparable && filteredInputVaryings.size() > filteredOutputVaryings.size())
    {
        infoLog << GetShaderTypeString(frontShaderType)
                << " does not generate all varyings consumed by "
                << GetShaderTypeString(backShaderType);
        return false;
    }

    // All inputs must match all outputs
    for (const sh::ShaderVariable *input : filteredInputVaryings)
    {
        bool match = false;
        for (const sh::ShaderVariable *output : filteredOutputVaryings)
        {
            if (DoShaderVariablesMatch(frontShaderVersion, frontShaderType, backShaderType, *input,
                                       *output, isSeparable, infoLog))
            {
                match = true;
                break;
            }
        }

        // We permit unmatched, unreferenced varyings. Note that this specifically depends on
        // whether the input is statically used - a statically used input should fail this test even
        // if it is not active. GLSL ES 3.00.6 section 4.3.10.
        if (!match && input->staticUse)
        {
            const std::string &name =
                input->isShaderIOBlock ? input->structOrBlockName : input->name;
            infoLog << GetShaderTypeString(backShaderType) << " varying " << name
                    << " does not match any " << GetShaderTypeString(frontShaderType) << " varying";
            return false;
        }
    }

    return true;
}

LinkMismatchError LinkValidateProgramVariables(const sh::ShaderVariable &variable1,
                                               const sh::ShaderVariable &variable2,
                                               bool validatePrecision,
                                               bool treatVariable1AsNonArray,
                                               bool treatVariable2AsNonArray,
                                               std::string *mismatchedStructOrBlockMemberName)
{
    if (variable1.type != variable2.type)
    {
        return LinkMismatchError::TYPE_MISMATCH;
    }

    bool variable1IsArray = variable1.isArray();
    bool variable2IsArray = variable2.isArray();
    if (treatVariable1AsNonArray)
    {
        ASSERT(variable1IsArray);
        variable1IsArray = false;
    }
    if (treatVariable2AsNonArray)
    {
        ASSERT(variable2IsArray);
        variable2IsArray = false;
    }
    // TODO(anglebug.com/42264094): Investigate interactions with arrays-of-arrays.
    if (variable1IsArray != variable2IsArray)
    {
        return LinkMismatchError::ARRAYNESS_MISMATCH;
    }
    if (!treatVariable1AsNonArray && !treatVariable2AsNonArray &&
        variable1.arraySizes != variable2.arraySizes)
    {
        return LinkMismatchError::ARRAY_SIZE_MISMATCH;
    }
    if (validatePrecision && variable1.precision != variable2.precision)
    {
        return LinkMismatchError::PRECISION_MISMATCH;
    }
    if (!variable1.isShaderIOBlock && !variable2.isShaderIOBlock &&
        variable1.structOrBlockName != variable2.structOrBlockName)
    {
        return LinkMismatchError::STRUCT_NAME_MISMATCH;
    }
    if (variable1.imageUnitFormat != variable2.imageUnitFormat)
    {
        return LinkMismatchError::FORMAT_MISMATCH;
    }

    if (variable1.fields.size() != variable2.fields.size())
    {
        return LinkMismatchError::FIELD_NUMBER_MISMATCH;
    }
    const unsigned int numMembers = static_cast<unsigned int>(variable1.fields.size());
    for (unsigned int memberIndex = 0; memberIndex < numMembers; memberIndex++)
    {
        const sh::ShaderVariable &member1 = variable1.fields[memberIndex];
        const sh::ShaderVariable &member2 = variable2.fields[memberIndex];

        if (member1.name != member2.name)
        {
            return LinkMismatchError::FIELD_NAME_MISMATCH;
        }

        if (member1.interpolation != member2.interpolation)
        {
            return LinkMismatchError::INTERPOLATION_TYPE_MISMATCH;
        }

        if (variable1.isShaderIOBlock && variable2.isShaderIOBlock)
        {
            if (member1.location != member2.location)
            {
                return LinkMismatchError::FIELD_LOCATION_MISMATCH;
            }

            if (member1.structOrBlockName != member2.structOrBlockName)
            {
                return LinkMismatchError::FIELD_STRUCT_NAME_MISMATCH;
            }
        }

        LinkMismatchError linkErrorOnField = LinkValidateProgramVariables(
            member1, member2, validatePrecision, false, false, mismatchedStructOrBlockMemberName);
        if (linkErrorOnField != LinkMismatchError::NO_MISMATCH)
        {
            AddProgramVariableParentPrefix(member1.name, mismatchedStructOrBlockMemberName);
            return linkErrorOnField;
        }
    }

    return LinkMismatchError::NO_MISMATCH;
}

void AddProgramVariableParentPrefix(const std::string &parentName, std::string *mismatchedFieldName)
{
    ASSERT(mismatchedFieldName);
    if (mismatchedFieldName->empty())
    {
        *mismatchedFieldName = parentName;
    }
    else
    {
        std::ostringstream stream;
        stream << parentName << "." << *mismatchedFieldName;
        *mismatchedFieldName = stream.str();
    }
}

bool LinkValidateBuiltInVaryingsInvariant(const std::vector<sh::ShaderVariable> &vertexVaryings,
                                          const std::vector<sh::ShaderVariable> &fragmentVaryings,
                                          int vertexShaderVersion,
                                          InfoLog &infoLog)
{
    bool glPositionIsInvariant   = false;
    bool glPointSizeIsInvariant  = false;
    bool glFragCoordIsInvariant  = false;
    bool glPointCoordIsInvariant = false;

    for (const sh::ShaderVariable &varying : vertexVaryings)
    {
        if (!varying.isBuiltIn())
        {
            continue;
        }
        if (varying.name.compare("gl_Position") == 0)
        {
            glPositionIsInvariant = varying.isInvariant;
        }
        else if (varying.name.compare("gl_PointSize") == 0)
        {
            glPointSizeIsInvariant = varying.isInvariant;
        }
    }

    for (const sh::ShaderVariable &varying : fragmentVaryings)
    {
        if (!varying.isBuiltIn())
        {
            continue;
        }
        if (varying.name.compare("gl_FragCoord") == 0)
        {
            glFragCoordIsInvariant = varying.isInvariant;
        }
        else if (varying.name.compare("gl_PointCoord") == 0)
        {
            glPointCoordIsInvariant = varying.isInvariant;
        }
    }

    // There is some ambiguity in ESSL 1.00.17 paragraph 4.6.4 interpretation,
    // for example, https://cvs.khronos.org/bugzilla/show_bug.cgi?id=13842.
    // Not requiring invariance to match is supported by:
    // dEQP, WebGL CTS, Nexus 5X GLES
    if (glFragCoordIsInvariant && !glPositionIsInvariant)
    {
        infoLog << "gl_FragCoord can only be declared invariant if and only if gl_Position is "
                   "declared invariant.";
        return false;
    }
    if (glPointCoordIsInvariant && !glPointSizeIsInvariant)
    {
        infoLog << "gl_PointCoord can only be declared invariant if and only if gl_PointSize is "
                   "declared invariant.";
        return false;
    }

    return true;
}

bool LinkValidateBuiltInVaryings(const std::vector<sh::ShaderVariable> &outputVaryings,
                                 const std::vector<sh::ShaderVariable> &inputVaryings,
                                 ShaderType outputShaderType,
                                 ShaderType inputShaderType,
                                 int outputShaderVersion,
                                 int inputShaderVersion,
                                 InfoLog &infoLog)
{
    ASSERT(outputShaderVersion == inputShaderVersion);

    // Only ESSL 1.0 has restrictions on matching input and output invariance
    if (inputShaderVersion == 100 && outputShaderType == ShaderType::Vertex &&
        inputShaderType == ShaderType::Fragment)
    {
        return LinkValidateBuiltInVaryingsInvariant(outputVaryings, inputVaryings,
                                                    outputShaderVersion, infoLog);
    }

    uint32_t sizeClipDistance = 0;
    uint32_t sizeCullDistance = 0;

    for (const sh::ShaderVariable &varying : outputVaryings)
    {
        if (!varying.isBuiltIn())
        {
            continue;
        }
        if (varying.name.compare("gl_ClipDistance") == 0)
        {
            sizeClipDistance = varying.getOutermostArraySize();
        }
        else if (varying.name.compare("gl_CullDistance") == 0)
        {
            sizeCullDistance = varying.getOutermostArraySize();
        }
    }

    for (const sh::ShaderVariable &varying : inputVaryings)
    {
        if (!varying.isBuiltIn())
        {
            continue;
        }
        if (varying.name.compare("gl_ClipDistance") == 0)
        {
            if (sizeClipDistance != varying.getOutermostArraySize())
            {
                infoLog
                    << "If a fragment shader statically uses the gl_ClipDistance built-in array, "
                       "the array must have the same size as in the previous shader stage. "
                    << "Output size " << sizeClipDistance << ", input size "
                    << varying.getOutermostArraySize() << ".";
                return false;
            }
        }
        else if (varying.name.compare("gl_CullDistance") == 0)
        {
            if (sizeCullDistance != varying.getOutermostArraySize())
            {
                infoLog
                    << "If a fragment shader statically uses the gl_ClipDistance built-in array, "
                       "the array must have the same size as in the previous shader stage. "
                    << "Output size " << sizeCullDistance << ", input size "
                    << varying.getOutermostArraySize() << ".";

                return false;
            }
        }
    }
    return true;
}

void LogAmbiguousFieldLinkMismatch(InfoLog &infoLog,
                                   const std::string &blockName1,
                                   const std::string &blockName2,
                                   const std::string &fieldName,
                                   ShaderType shaderType1,
                                   ShaderType shaderType2)
{
    infoLog << "Ambiguous field '" << fieldName << "' in blocks '" << blockName1 << "' ("
            << GetShaderTypeString(shaderType1) << " shader) and '" << blockName2 << "' ("
            << GetShaderTypeString(shaderType2) << " shader) which don't have instance names.";
}

bool ValidateInstancelessGraphicsInterfaceBlocksPerShader(
    const std::vector<sh::InterfaceBlock> &interfaceBlocks,
    ShaderType shaderType,
    InterfaceBlockMap *instancelessBlocksFields,
    InfoLog &infoLog)
{
    ASSERT(instancelessBlocksFields);

    for (const sh::InterfaceBlock &block : interfaceBlocks)
    {
        if (!block.instanceName.empty())
        {
            continue;
        }

        for (const sh::ShaderVariable &field : block.fields)
        {
            const auto &entry = instancelessBlocksFields->find(field.name);
            if (entry != instancelessBlocksFields->end())
            {
                const sh::InterfaceBlock &linkedBlock = *(entry->second.second);
                if (block.name != linkedBlock.name)
                {
                    LogAmbiguousFieldLinkMismatch(infoLog, block.name, linkedBlock.name, field.name,
                                                  entry->second.first, shaderType);
                    return false;
                }
            }
            else
            {
                (*instancelessBlocksFields)[field.name] = std::make_pair(shaderType, &block);
            }
        }
    }

    return true;
}

LinkMismatchError LinkValidateInterfaceBlockFields(const sh::ShaderVariable &blockField1,
                                                   const sh::ShaderVariable &blockField2,
                                                   bool webglCompatibility,
                                                   std::string *mismatchedBlockFieldName)
{
    if (blockField1.name != blockField2.name)
    {
        return LinkMismatchError::FIELD_NAME_MISMATCH;
    }

    // If webgl, validate precision of UBO fields, otherwise don't.  See Khronos bug 10287.
    LinkMismatchError linkError = LinkValidateProgramVariables(
        blockField1, blockField2, webglCompatibility, false, false, mismatchedBlockFieldName);
    if (linkError != LinkMismatchError::NO_MISMATCH)
    {
        AddProgramVariableParentPrefix(blockField1.name, mismatchedBlockFieldName);
        return linkError;
    }

    if (blockField1.isRowMajorLayout != blockField2.isRowMajorLayout)
    {
        AddProgramVariableParentPrefix(blockField1.name, mismatchedBlockFieldName);
        return LinkMismatchError::MATRIX_PACKING_MISMATCH;
    }

    return LinkMismatchError::NO_MISMATCH;
}

LinkMismatchError AreMatchingInterfaceBlocks(const sh::InterfaceBlock &interfaceBlock1,
                                             const sh::InterfaceBlock &interfaceBlock2,
                                             bool webglCompatibility,
                                             std::string *mismatchedBlockFieldName)
{
    // validate blocks for the same member types
    if (interfaceBlock1.fields.size() != interfaceBlock2.fields.size())
    {
        return LinkMismatchError::FIELD_NUMBER_MISMATCH;
    }
    if (interfaceBlock1.arraySize != interfaceBlock2.arraySize)
    {
        return LinkMismatchError::ARRAY_SIZE_MISMATCH;
    }
    if (interfaceBlock1.layout != interfaceBlock2.layout ||
        interfaceBlock1.binding != interfaceBlock2.binding)
    {
        return LinkMismatchError::LAYOUT_QUALIFIER_MISMATCH;
    }
    if (interfaceBlock1.instanceName.empty() != interfaceBlock2.instanceName.empty())
    {
        return LinkMismatchError::INSTANCE_NAME_MISMATCH;
    }
    const unsigned int numBlockMembers = static_cast<unsigned int>(interfaceBlock1.fields.size());
    for (unsigned int blockMemberIndex = 0; blockMemberIndex < numBlockMembers; blockMemberIndex++)
    {
        const sh::ShaderVariable &member1 = interfaceBlock1.fields[blockMemberIndex];
        const sh::ShaderVariable &member2 = interfaceBlock2.fields[blockMemberIndex];

        LinkMismatchError linkError = LinkValidateInterfaceBlockFields(
            member1, member2, webglCompatibility, mismatchedBlockFieldName);
        if (linkError != LinkMismatchError::NO_MISMATCH)
        {
            return linkError;
        }
    }
    return LinkMismatchError::NO_MISMATCH;
}

void InitializeInterfaceBlockMap(const std::vector<sh::InterfaceBlock> &interfaceBlocks,
                                 ShaderType shaderType,
                                 InterfaceBlockMap *linkedInterfaceBlocks)
{
    ASSERT(linkedInterfaceBlocks);

    for (const sh::InterfaceBlock &interfaceBlock : interfaceBlocks)
    {
        (*linkedInterfaceBlocks)[interfaceBlock.name] = std::make_pair(shaderType, &interfaceBlock);
    }
}

bool ValidateGraphicsInterfaceBlocksPerShader(
    const std::vector<sh::InterfaceBlock> &interfaceBlocksToLink,
    ShaderType shaderType,
    bool webglCompatibility,
    InterfaceBlockMap *linkedBlocks,
    InfoLog &infoLog)
{
    ASSERT(linkedBlocks);

    for (const sh::InterfaceBlock &block : interfaceBlocksToLink)
    {
        const auto &entry = linkedBlocks->find(block.name);
        if (entry != linkedBlocks->end())
        {
            const sh::InterfaceBlock &linkedBlock = *(entry->second.second);
            std::string mismatchedStructFieldName;
            LinkMismatchError linkError = AreMatchingInterfaceBlocks(
                block, linkedBlock, webglCompatibility, &mismatchedStructFieldName);
            if (linkError != LinkMismatchError::NO_MISMATCH)
            {
                LogLinkMismatch(infoLog, block.name, GetInterfaceBlockTypeString(block.blockType),
                                linkError, mismatchedStructFieldName, entry->second.first,
                                shaderType);
                return false;
            }
        }
        else
        {
            (*linkedBlocks)[block.name] = std::make_pair(shaderType, &block);
        }
    }

    return true;
}

bool ValidateInterfaceBlocksMatch(
    GLuint numShadersHasInterfaceBlocks,
    const ShaderMap<const std::vector<sh::InterfaceBlock> *> &shaderInterfaceBlocks,
    InfoLog &infoLog,
    bool webglCompatibility,
    InterfaceBlockMap *instancelessInterfaceBlocksFields)
{
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        // Validate that instanceless blocks of different names don't have fields of the same name.
        if (shaderInterfaceBlocks[shaderType] &&
            !ValidateInstancelessGraphicsInterfaceBlocksPerShader(
                *shaderInterfaceBlocks[shaderType], shaderType, instancelessInterfaceBlocksFields,
                infoLog))
        {
            return false;
        }
    }

    if (numShadersHasInterfaceBlocks < 2u)
    {
        return true;
    }

    ASSERT(!shaderInterfaceBlocks[ShaderType::Compute]);

    // Check that interface blocks defined in the graphics shaders are identical

    InterfaceBlockMap linkedInterfaceBlocks;

    bool interfaceBlockMapInitialized = false;
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        if (!shaderInterfaceBlocks[shaderType])
        {
            continue;
        }

        if (!interfaceBlockMapInitialized)
        {
            InitializeInterfaceBlockMap(*shaderInterfaceBlocks[shaderType], shaderType,
                                        &linkedInterfaceBlocks);
            interfaceBlockMapInitialized = true;
        }
        else if (!ValidateGraphicsInterfaceBlocksPerShader(*shaderInterfaceBlocks[shaderType],
                                                           shaderType, webglCompatibility,
                                                           &linkedInterfaceBlocks, infoLog))
        {
            return false;
        }
    }

    return true;
}

bool LinkValidateProgramInterfaceBlocks(const Caps &caps,
                                        const Version &clientVersion,
                                        bool webglCompatibility,
                                        ShaderBitSet activeProgramStages,
                                        const ProgramLinkedResources &resources,
                                        InfoLog &infoLog,
                                        GLuint *combinedShaderStorageBlocksCountOut)
{
    ASSERT(combinedShaderStorageBlocksCountOut);

    GLuint combinedUniformBlocksCount                                         = 0u;
    GLuint numShadersHasUniformBlocks                                         = 0u;
    ShaderMap<const std::vector<sh::InterfaceBlock> *> allShaderUniformBlocks = {};
    InterfaceBlockMap instancelessInterfaceBlocksFields;

    for (ShaderType shaderType : activeProgramStages)
    {
        const std::vector<sh::InterfaceBlock> &uniformBlocks =
            resources.uniformBlockLinker.getShaderBlocks(shaderType);
        if (!uniformBlocks.empty())
        {
            if (!ValidateInterfaceBlocksCount(
                    static_cast<GLuint>(caps.maxShaderUniformBlocks[shaderType]), uniformBlocks,
                    shaderType, sh::BlockType::kBlockUniform, &combinedUniformBlocksCount, infoLog))
            {
                return false;
            }

            allShaderUniformBlocks[shaderType] = &uniformBlocks;
            ++numShadersHasUniformBlocks;
        }
    }

    if (combinedUniformBlocksCount > static_cast<GLuint>(caps.maxCombinedUniformBlocks))
    {
        infoLog << "The sum of the number of active uniform blocks exceeds "
                   "MAX_COMBINED_UNIFORM_BLOCKS ("
                << caps.maxCombinedUniformBlocks << ").";
        return false;
    }

    if (!ValidateInterfaceBlocksMatch(numShadersHasUniformBlocks, allShaderUniformBlocks, infoLog,
                                      webglCompatibility, &instancelessInterfaceBlocksFields))
    {
        return false;
    }

    if (clientVersion >= Version(3, 1))
    {
        *combinedShaderStorageBlocksCountOut                                      = 0u;
        GLuint numShadersHasShaderStorageBlocks                                   = 0u;
        ShaderMap<const std::vector<sh::InterfaceBlock> *> allShaderStorageBlocks = {};
        for (ShaderType shaderType : activeProgramStages)
        {
            const std::vector<sh::InterfaceBlock> &shaderStorageBlocks =
                resources.shaderStorageBlockLinker.getShaderBlocks(shaderType);
            if (!shaderStorageBlocks.empty())
            {
                if (!ValidateInterfaceBlocksCount(
                        static_cast<GLuint>(caps.maxShaderStorageBlocks[shaderType]),
                        shaderStorageBlocks, shaderType, sh::BlockType::kBlockBuffer,
                        combinedShaderStorageBlocksCountOut, infoLog))
                {
                    return false;
                }

                allShaderStorageBlocks[shaderType] = &shaderStorageBlocks;
                ++numShadersHasShaderStorageBlocks;
            }
        }

        if (*combinedShaderStorageBlocksCountOut >
            static_cast<GLuint>(caps.maxCombinedShaderStorageBlocks))
        {
            infoLog << "The sum of the number of active shader storage blocks exceeds "
                       "MAX_COMBINED_SHADER_STORAGE_BLOCKS ("
                    << caps.maxCombinedShaderStorageBlocks << ").";
            return false;
        }

        if (!ValidateInterfaceBlocksMatch(numShadersHasShaderStorageBlocks, allShaderStorageBlocks,
                                          infoLog, webglCompatibility,
                                          &instancelessInterfaceBlocksFields))
        {
            return false;
        }
    }

    return true;
}

}  // namespace gl
