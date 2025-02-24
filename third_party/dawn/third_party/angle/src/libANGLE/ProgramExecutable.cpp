//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutable.cpp: Collects the interfaces common to both Programs and
// ProgramPipelines in order to execute/draw with either.

#include "libANGLE/ProgramExecutable.h"

#include "common/string_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Program.h"
#include "libANGLE/Shader.h"
#include "libANGLE/queryconversions.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/ProgramExecutableImpl.h"
#include "libANGLE/renderer/ProgramImpl.h"

namespace gl
{
namespace
{
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS
// A placeholder struct just to ensure sh::BlockMemberInfo is tightly packed since vulkan backend
// uses it and memcpy the entire vector which requires it tightly packed to make msan happy.
struct BlockMemberInfoPaddingTest
{
    sh::BlockMemberInfo blockMemberInfo;
};
ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

bool IncludeSameArrayElement(const std::set<std::string> &nameSet, const std::string &name)
{
    std::vector<unsigned int> subscripts;
    std::string baseName = ParseResourceName(name, &subscripts);
    for (const std::string &nameInSet : nameSet)
    {
        std::vector<unsigned int> arrayIndices;
        std::string arrayName = ParseResourceName(nameInSet, &arrayIndices);
        if (baseName == arrayName &&
            (subscripts.empty() || arrayIndices.empty() || subscripts == arrayIndices))
        {
            return true;
        }
    }
    return false;
}

// Find the matching varying or field by name.
const sh::ShaderVariable *FindOutputVaryingOrField(const ProgramMergedVaryings &varyings,
                                                   ShaderType stage,
                                                   const std::string &name)
{
    const sh::ShaderVariable *var = nullptr;
    for (const ProgramVaryingRef &ref : varyings)
    {
        if (ref.frontShaderStage != stage)
        {
            continue;
        }

        const sh::ShaderVariable *varying = ref.get(stage);
        if (varying->name == name)
        {
            var = varying;
            break;
        }
        GLuint fieldIndex = 0;
        var               = varying->findField(name, &fieldIndex);
        if (var != nullptr)
        {
            break;
        }
    }
    return var;
}

bool FindUsedOutputLocation(std::vector<VariableLocation> &outputLocations,
                            unsigned int baseLocation,
                            unsigned int elementCount,
                            const std::vector<VariableLocation> &reservedLocations,
                            unsigned int variableIndex)
{
    if (baseLocation + elementCount > outputLocations.size())
    {
        elementCount = baseLocation < outputLocations.size()
                           ? static_cast<unsigned int>(outputLocations.size() - baseLocation)
                           : 0;
    }
    for (unsigned int elementIndex = 0; elementIndex < elementCount; elementIndex++)
    {
        const unsigned int location = baseLocation + elementIndex;
        if (outputLocations[location].used())
        {
            VariableLocation locationInfo(elementIndex, variableIndex);
            if (std::find(reservedLocations.begin(), reservedLocations.end(), locationInfo) ==
                reservedLocations.end())
            {
                return true;
            }
        }
    }
    return false;
}

void AssignOutputLocations(std::vector<VariableLocation> &outputLocations,
                           unsigned int baseLocation,
                           unsigned int elementCount,
                           const std::vector<VariableLocation> &reservedLocations,
                           unsigned int variableIndex,
                           bool locationAssignedByApi,
                           ProgramOutput &outputVariable)
{
    if (baseLocation + elementCount > outputLocations.size())
    {
        outputLocations.resize(baseLocation + elementCount);
    }
    for (unsigned int elementIndex = 0; elementIndex < elementCount; elementIndex++)
    {
        VariableLocation locationInfo(elementIndex, variableIndex);
        if (std::find(reservedLocations.begin(), reservedLocations.end(), locationInfo) ==
            reservedLocations.end())
        {
            outputVariable.pod.location = baseLocation;
            const unsigned int location = baseLocation + elementIndex;
            outputLocations[location]   = locationInfo;
        }
    }
    outputVariable.pod.hasApiAssignedLocation = locationAssignedByApi;
}

int GetOutputLocationForLink(const ProgramAliasedBindings &fragmentOutputLocations,
                             const ProgramOutput &outputVariable)
{
    if (outputVariable.pod.location != -1)
    {
        return outputVariable.pod.location;
    }
    int apiLocation = fragmentOutputLocations.getBinding(outputVariable);
    if (apiLocation != -1)
    {
        return apiLocation;
    }
    return -1;
}

void AssignOutputIndex(const ProgramAliasedBindings &fragmentOutputIndexes,
                       ProgramOutput &outputVariable)
{
    if (outputVariable.pod.hasShaderAssignedLocation)
    {
        // Already assigned through a layout qualifier
        ASSERT(outputVariable.pod.index == 0 || outputVariable.pod.index == 1);
        return;
    }

    int apiIndex = fragmentOutputIndexes.getBinding(outputVariable);
    if (apiIndex != -1)
    {
        // Index layout qualifier from the shader takes precedence, so the index from the API is
        // checked only if the index was not set in the shader. This is not specified in the EXT
        // spec, but is specified in desktop OpenGL specs.
        ASSERT(apiIndex == 0 || apiIndex == 1);
        outputVariable.pod.index = apiIndex;
        return;
    }

    // EXT_blend_func_extended: Outputs get index 0 by default.
    outputVariable.pod.index = 0;
}

RangeUI AddUniforms(const ShaderMap<SharedProgramExecutable> &executables,
                    ShaderBitSet activeShaders,
                    std::vector<LinkedUniform> *outputUniforms,
                    std::vector<std::string> *outputUniformNames,
                    std::vector<std::string> *outputUniformMappedNames,
                    const std::function<RangeUI(const ProgramExecutable &)> &getRange)
{
    unsigned int startRange = static_cast<unsigned int>(outputUniforms->size());
    for (ShaderType shaderType : activeShaders)
    {
        const ProgramExecutable &executable = *executables[shaderType];
        const RangeUI uniformRange          = getRange(executable);

        const std::vector<LinkedUniform> &programUniforms = executable.getUniforms();
        outputUniforms->insert(outputUniforms->end(), programUniforms.begin() + uniformRange.low(),
                               programUniforms.begin() + uniformRange.high());

        const std::vector<std::string> &uniformNames = executable.getUniformNames();
        outputUniformNames->insert(outputUniformNames->end(),
                                   uniformNames.begin() + uniformRange.low(),
                                   uniformNames.begin() + uniformRange.high());

        const std::vector<std::string> &uniformMappedNames = executable.getUniformMappedNames();
        outputUniformMappedNames->insert(outputUniformMappedNames->end(),
                                         uniformMappedNames.begin() + uniformRange.low(),
                                         uniformMappedNames.begin() + uniformRange.high());
    }
    return RangeUI(startRange, static_cast<unsigned int>(outputUniforms->size()));
}

template <typename BlockT>
void AppendActiveBlocks(ShaderType shaderType,
                        const std::vector<BlockT> &blocksIn,
                        std::vector<BlockT> &blocksOut,
                        ProgramUniformBlockArray<GLuint> *ppoBlockMap)
{
    for (size_t index = 0; index < blocksIn.size(); ++index)
    {
        const BlockT &block = blocksIn[index];
        if (block.isActive(shaderType))
        {
            // Have a way for the PPO to know how to map the program's UBO index into its own UBO
            // array.  This is used to propagate changes to the program's UBOs to the PPO's UBO
            // list.
            if (ppoBlockMap != nullptr)
            {
                (*ppoBlockMap)[static_cast<uint32_t>(index)] =
                    static_cast<uint32_t>(blocksOut.size());
            }

            blocksOut.push_back(block);
        }
    }
}

void SaveProgramInputs(BinaryOutputStream *stream, const std::vector<ProgramInput> &programInputs)
{
    stream->writeInt(programInputs.size());
    for (const ProgramInput &attrib : programInputs)
    {
        stream->writeString(attrib.name);
        stream->writeString(attrib.mappedName);
        stream->writeStruct(attrib.pod);
    }
}
void LoadProgramInputs(BinaryInputStream *stream, std::vector<ProgramInput> *programInputs)
{
    size_t attribCount = stream->readInt<size_t>();
    ASSERT(programInputs->empty());
    if (attribCount > 0)
    {
        programInputs->resize(attribCount);
        for (size_t attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            ProgramInput &attrib = (*programInputs)[attribIndex];
            stream->readString(&attrib.name);
            stream->readString(&attrib.mappedName);
            stream->readStruct(&attrib.pod);
        }
    }
}

void SaveUniforms(BinaryOutputStream *stream,
                  const std::vector<LinkedUniform> &uniforms,
                  const std::vector<std::string> &uniformNames,
                  const std::vector<std::string> &uniformMappedNames,
                  const std::vector<VariableLocation> &uniformLocations)
{
    stream->writeVector(uniforms);
    ASSERT(uniforms.size() == uniformNames.size());
    ASSERT(uniforms.size() == uniformMappedNames.size());
    for (const std::string &name : uniformNames)
    {
        stream->writeString(name);
    }
    for (const std::string &name : uniformMappedNames)
    {
        stream->writeString(name);
    }
    stream->writeVector(uniformLocations);
}
void LoadUniforms(BinaryInputStream *stream,
                  std::vector<LinkedUniform> *uniforms,
                  std::vector<std::string> *uniformNames,
                  std::vector<std::string> *uniformMappedNames,
                  std::vector<VariableLocation> *uniformLocations)
{
    stream->readVector(uniforms);
    if (!uniforms->empty())
    {
        uniformNames->resize(uniforms->size());
        for (size_t uniformIndex = 0; uniformIndex < uniforms->size(); ++uniformIndex)
        {
            stream->readString(&(*uniformNames)[uniformIndex]);
        }
        uniformMappedNames->resize(uniforms->size());
        for (size_t uniformIndex = 0; uniformIndex < uniforms->size(); ++uniformIndex)
        {
            stream->readString(&(*uniformMappedNames)[uniformIndex]);
        }
    }
    stream->readVector(uniformLocations);
}

void SaveSamplerBindings(BinaryOutputStream *stream,
                         const std::vector<SamplerBinding> &samplerBindings,
                         const std::vector<GLuint> &samplerBoundTextureUnits)
{
    stream->writeVector(samplerBindings);
    stream->writeInt(samplerBoundTextureUnits.size());
}
void LoadSamplerBindings(BinaryInputStream *stream,
                         std::vector<SamplerBinding> *samplerBindings,
                         std::vector<GLuint> *samplerBoundTextureUnits)
{
    stream->readVector(samplerBindings);
    ASSERT(samplerBoundTextureUnits->empty());
    size_t boundTextureUnitsCount = stream->readInt<size_t>();
    samplerBoundTextureUnits->resize(boundTextureUnitsCount, 0);
}

void WriteBufferVariable(BinaryOutputStream *stream, const BufferVariable &var)
{
    stream->writeString(var.name);
    stream->writeString(var.mappedName);
    stream->writeStruct(var.pod);
}

void LoadBufferVariable(BinaryInputStream *stream, BufferVariable *var)
{
    var->name       = stream->readString();
    var->mappedName = stream->readString();
    stream->readStruct(&var->pod);
}

void WriteAtomicCounterBuffer(BinaryOutputStream *stream, const AtomicCounterBuffer &var)
{
    stream->writeVector(var.memberIndexes);
    stream->writeStruct(var.pod);
}

void LoadAtomicCounterBuffer(BinaryInputStream *stream, AtomicCounterBuffer *var)
{
    stream->readVector(&var->memberIndexes);
    stream->readStruct(&var->pod);
}

void WriteInterfaceBlock(BinaryOutputStream *stream, const InterfaceBlock &block)
{
    stream->writeString(block.name);
    stream->writeString(block.mappedName);
    stream->writeVector(block.memberIndexes);
    stream->writeStruct(block.pod);
}

void LoadInterfaceBlock(BinaryInputStream *stream, InterfaceBlock *block)
{
    block->name       = stream->readString();
    block->mappedName = stream->readString();
    stream->readVector(&block->memberIndexes);
    stream->readStruct(&block->pod);
}

void CopyStringToBuffer(GLchar *buffer,
                        const std::string &string,
                        GLsizei bufSize,
                        GLsizei *lengthOut)
{
    ASSERT(bufSize > 0);
    size_t length = std::min<size_t>(bufSize - 1, string.length());
    memcpy(buffer, string.c_str(), length);
    buffer[length] = '\0';

    if (lengthOut)
    {
        *lengthOut = static_cast<GLsizei>(length);
    }
}

template <typename T>
GLuint GetResourceMaxNameSize(const T &resource, GLint max)
{
    if (resource.isArray())
    {
        return std::max(max, clampCast<GLint>((resource.name + "[0]").size()));
    }
    else
    {
        return std::max(max, clampCast<GLint>((resource.name).size()));
    }
}

template <typename T>
GLuint GetResourceLocation(const GLchar *name, const T &variable, GLint location)
{
    if (variable.isBuiltIn())
    {
        return GL_INVALID_INDEX;
    }

    if (variable.isArray())
    {
        size_t nameLengthWithoutArrayIndexOut;
        size_t arrayIndex = ParseArrayIndex(name, &nameLengthWithoutArrayIndexOut);
        // The 'name' string may not contain the array notation "[0]"
        if (arrayIndex != GL_INVALID_INDEX)
        {
            location += arrayIndex;
        }
    }

    return location;
}

template <typename T>
const std::string GetResourceName(const T &resource)
{
    std::string resourceName = resource.name;

    if (resource.isArray())
    {
        resourceName += "[0]";
    }

    return resourceName;
}

GLint GetVariableLocation(const std::vector<gl::ProgramOutput> &list,
                          const std::vector<VariableLocation> &locationList,
                          const std::string &name)
{
    size_t nameLengthWithoutArrayIndex;
    unsigned int arrayIndex = ParseArrayIndex(name, &nameLengthWithoutArrayIndex);

    for (size_t location = 0u; location < locationList.size(); ++location)
    {
        const VariableLocation &variableLocation = locationList[location];
        if (!variableLocation.used())
        {
            continue;
        }

        const gl::ProgramOutput &variable = list[variableLocation.index];

        // Array output variables may be bound out of order, so we need to ensure we only pick the
        // first element if given the base name.
        if ((variable.name == name) && (variableLocation.arrayIndex == 0))
        {
            return static_cast<GLint>(location);
        }
        if (variable.isArray() && variableLocation.arrayIndex == arrayIndex &&
            angle::BeginsWith(variable.name, name, nameLengthWithoutArrayIndex))
        {
            return static_cast<GLint>(location);
        }
    }

    return -1;
}

template <typename VarT>
GLuint GetResourceIndexFromName(const std::vector<VarT> &list, const std::string &name)
{
    std::string nameAsArrayName = name + "[0]";
    for (size_t index = 0; index < list.size(); index++)
    {
        const VarT &resource = list[index];
        if (resource.name == name || (resource.isArray() && resource.name == nameAsArrayName))
        {
            return static_cast<GLuint>(index);
        }
    }

    return GL_INVALID_INDEX;
}

GLuint GetUniformIndexFromName(const std::vector<LinkedUniform> &uniformList,
                               const std::vector<std::string> &nameList,
                               const std::string &name)
{
    std::string nameAsArrayName = name + "[0]";
    for (size_t index = 0; index < nameList.size(); index++)
    {
        const std::string &uniformName = nameList[index];
        if (uniformName == name || (uniformList[index].isArray() && uniformName == nameAsArrayName))
        {
            return static_cast<GLuint>(index);
        }
    }

    return GL_INVALID_INDEX;
}

GLint GetUniformLocation(const std::vector<LinkedUniform> &uniformList,
                         const std::vector<std::string> &nameList,
                         const std::vector<VariableLocation> &locationList,
                         const std::string &name)
{
    size_t nameLengthWithoutArrayIndex;
    unsigned int arrayIndex = ParseArrayIndex(name, &nameLengthWithoutArrayIndex);

    for (size_t location = 0u; location < locationList.size(); ++location)
    {
        const VariableLocation &variableLocation = locationList[location];
        if (!variableLocation.used())
        {
            continue;
        }

        const LinkedUniform &variable  = uniformList[variableLocation.index];
        const std::string &uniformName = nameList[variableLocation.index];

        // Array output variables may be bound out of order, so we need to ensure we only pick the
        // first element if given the base name. Uniforms don't allow this behavior and some code
        // seemingly depends on the opposite behavior, so only enable it for output variables.
        if (angle::BeginsWith(uniformName, name) && (variableLocation.arrayIndex == 0))
        {
            if (name.length() == uniformName.length())
            {
                ASSERT(name == uniformName);
                // GLES 3.1 November 2016 page 87.
                // The string exactly matches the name of the active variable.
                return static_cast<GLint>(location);
            }
            if (name.length() + 3u == uniformName.length() && variable.isArray())
            {
                ASSERT(name + "[0]" == uniformName);
                // The string identifies the base name of an active array, where the string would
                // exactly match the name of the variable if the suffix "[0]" were appended to the
                // string.
                return static_cast<GLint>(location);
            }
        }
        if (variable.isArray() && variableLocation.arrayIndex == arrayIndex &&
            nameLengthWithoutArrayIndex + 3u == uniformName.length() &&
            angle::BeginsWith(uniformName, name, nameLengthWithoutArrayIndex))
        {
            ASSERT(name.substr(0u, nameLengthWithoutArrayIndex) + "[0]" == uniformName);
            // The string identifies an active element of the array, where the string ends with the
            // concatenation of the "[" character, an integer (with no "+" sign, extra leading
            // zeroes, or whitespace) identifying an array element, and the "]" character, the
            // integer is less than the number of active elements of the array variable, and where
            // the string would exactly match the enumerated name of the array if the decimal
            // integer were replaced with zero.
            return static_cast<GLint>(location);
        }
    }

    return -1;
}

GLuint GetInterfaceBlockIndex(const std::vector<InterfaceBlock> &list, const std::string &name)
{
    std::vector<unsigned int> subscripts;
    std::string baseName = ParseResourceName(name, &subscripts);

    unsigned int numBlocks = static_cast<unsigned int>(list.size());
    for (unsigned int blockIndex = 0; blockIndex < numBlocks; blockIndex++)
    {
        const auto &block = list[blockIndex];
        if (block.name == baseName)
        {
            const bool arrayElementZero =
                (subscripts.empty() && (!block.pod.isArray || block.pod.arrayElement == 0));
            const bool arrayElementMatches =
                (subscripts.size() == 1 && subscripts[0] == block.pod.arrayElement);
            if (arrayElementMatches || arrayElementZero)
            {
                return blockIndex;
            }
        }
    }

    return GL_INVALID_INDEX;
}

void GetInterfaceBlockName(const UniformBlockIndex index,
                           const std::vector<InterfaceBlock> &list,
                           GLsizei bufSize,
                           GLsizei *length,
                           GLchar *name)
{
    ASSERT(index.value < list.size());

    const auto &block = list[index.value];

    if (bufSize > 0)
    {
        std::string blockName = block.name;

        if (block.pod.isArray)
        {
            blockName += ArrayString(block.pod.arrayElement);
        }
        CopyStringToBuffer(name, blockName, bufSize, length);
    }
}

template <typename T>
GLint GetActiveInterfaceBlockMaxNameLength(const std::vector<T> &resources)
{
    int maxLength = 0;

    for (const T &resource : resources)
    {
        if (!resource.name.empty())
        {
            int length = static_cast<int>(resource.nameWithArrayIndex().length());
            maxLength  = std::max(length + 1, maxLength);
        }
    }

    return maxLength;
}

// This simplified cast function doesn't need to worry about advanced concepts like
// depth range values, or casting to bool.
template <typename DestT, typename SrcT>
DestT UniformStateQueryCast(SrcT value);

// From-Float-To-Integer Casts
template <>
GLint UniformStateQueryCast(GLfloat value)
{
    return clampCast<GLint>(roundf(value));
}

template <>
GLuint UniformStateQueryCast(GLfloat value)
{
    return clampCast<GLuint>(roundf(value));
}

// From-Integer-to-Integer Casts
template <>
GLint UniformStateQueryCast(GLuint value)
{
    return clampCast<GLint>(value);
}

template <>
GLuint UniformStateQueryCast(GLint value)
{
    return clampCast<GLuint>(value);
}

// From-Boolean-to-Anything Casts
template <>
GLfloat UniformStateQueryCast(GLboolean value)
{
    return (ConvertToBool(value) ? 1.0f : 0.0f);
}

template <>
GLint UniformStateQueryCast(GLboolean value)
{
    return (ConvertToBool(value) ? 1 : 0);
}

template <>
GLuint UniformStateQueryCast(GLboolean value)
{
    return (ConvertToBool(value) ? 1u : 0u);
}

// Default to static_cast
template <typename DestT, typename SrcT>
DestT UniformStateQueryCast(SrcT value)
{
    return static_cast<DestT>(value);
}

template <typename SrcT, typename DestT>
void UniformStateQueryCastLoop(DestT *dataOut, const uint8_t *srcPointer, int components)
{
    for (int comp = 0; comp < components; ++comp)
    {
        // We only work with strides of 4 bytes for uniform components. (GLfloat/GLint)
        // Don't use SrcT stride directly since GLboolean has a stride of 1 byte.
        size_t offset               = comp * 4;
        const SrcT *typedSrcPointer = reinterpret_cast<const SrcT *>(&srcPointer[offset]);
        dataOut[comp]               = UniformStateQueryCast<DestT>(*typedSrcPointer);
    }
}
}  // anonymous namespace

// ImageBinding implementation.
ImageBinding::ImageBinding(GLuint imageUnit, size_t count, TextureType textureTypeIn)
    : textureType(textureTypeIn)
{
    for (size_t index = 0; index < count; ++index)
    {
        boundImageUnits.push_back(imageUnit + static_cast<GLuint>(index));
    }
}

// ProgramInput implementation.
ProgramInput::ProgramInput(const sh::ShaderVariable &var)
{
    ASSERT(!var.isStruct());

    name       = var.name;
    mappedName = var.mappedName;

    SetBitField(pod.type, var.type);
    pod.location = var.hasImplicitLocation ? -1 : var.location;
    SetBitField(pod.interpolation, var.interpolation);
    pod.flagBitsAsUByte              = 0;
    pod.flagBits.active              = var.active;
    pod.flagBits.isPatch             = var.isPatch;
    pod.flagBits.hasImplicitLocation = var.hasImplicitLocation;
    pod.flagBits.isArray             = var.isArray();
    pod.flagBits.isBuiltIn           = IsBuiltInName(var.name);
    SetBitField(pod.basicTypeElementCount, var.getBasicTypeElementCount());
    pod.id = var.id;
    SetBitField(pod.arraySizeProduct, var.getArraySizeProduct());
}

// ProgramOutput implementation.
ProgramOutput::ProgramOutput(const sh::ShaderVariable &var)
{
    name       = var.name;
    mappedName = var.mappedName;

    pod.type     = var.type;
    pod.location = var.location;
    pod.index    = var.index;
    pod.id       = var.id;

    SetBitField(pod.outermostArraySize, var.getOutermostArraySize());
    SetBitField(pod.basicTypeElementCount, var.getBasicTypeElementCount());

    SetBitField(pod.isPatch, var.isPatch);
    SetBitField(pod.yuv, var.yuv);
    SetBitField(pod.isBuiltIn, IsBuiltInName(var.name));
    SetBitField(pod.isArray, var.isArray());
    SetBitField(pod.hasImplicitLocation, var.hasImplicitLocation);
    SetBitField(pod.hasShaderAssignedLocation, var.location != -1);
    SetBitField(pod.hasApiAssignedLocation, false);
    SetBitField(pod.pad, 0);

    if (pod.hasShaderAssignedLocation && pod.index == -1)
    {
        // Location was assigned but index was not. Equivalent to setting index to 0.
        pod.index = 0;
    }
}

// ProgramExecutable implementation.
ProgramExecutable::ProgramExecutable(rx::GLImplFactory *factory, InfoLog *infoLog)
    : mImplementation(factory->createProgramExecutable(this)),
      mInfoLog(infoLog),
      mCachedBaseVertex(0),
      mCachedBaseInstance(0),
      mIsPPO(false)
{
    memset(&mPod, 0, sizeof(mPod));
    reset();
}

ProgramExecutable::~ProgramExecutable()
{
    ASSERT(mPostLinkSubTasks.empty());
    ASSERT(mPostLinkSubTaskWaitableEvents.empty());
    ASSERT(mImplementation == nullptr);
}

void ProgramExecutable::destroy(const Context *context)
{
    ASSERT(mImplementation != nullptr);

    for (SharedProgramExecutable &executable : mPPOProgramExecutables)
    {
        if (executable)
        {
            UninstallExecutable(context, &executable);
        }
    }

    mImplementation->destroy(context);
    SafeDelete(mImplementation);
}

void ProgramExecutable::reset()
{
    mPod.activeAttribLocationsMask.reset();
    mPod.attributesTypeMask.reset();
    mPod.attributesMask.reset();
    mPod.maxActiveAttribLocation = 0;
    mPod.activeOutputVariablesMask.reset();
    mPod.activeSecondaryOutputVariablesMask.reset();

    mPod.defaultUniformRange       = RangeUI(0, 0);
    mPod.samplerUniformRange       = RangeUI(0, 0);
    mPod.imageUniformRange         = RangeUI(0, 0);
    mPod.atomicCounterUniformRange = RangeUI(0, 0);

    mPod.fragmentInoutIndices.reset();

    mPod.hasClipDistance         = false;
    mPod.hasDiscard              = false;
    mPod.enablesPerSampleShading = false;
    mPod.hasYUVOutput            = false;
    mPod.hasDepthInputAttachment   = false;
    mPod.hasStencilInputAttachment = false;

    mPod.advancedBlendEquations.reset();

    mPod.geometryShaderInputPrimitiveType  = PrimitiveMode::Triangles;
    mPod.geometryShaderOutputPrimitiveType = PrimitiveMode::TriangleStrip;
    mPod.geometryShaderInvocations         = 1;
    mPod.geometryShaderMaxVertices         = 0;

    mPod.transformFeedbackBufferMode = GL_INTERLEAVED_ATTRIBS;

    mPod.numViews = -1;

    mPod.drawIDLocation = -1;

    mPod.baseVertexLocation   = -1;
    mPod.baseInstanceLocation = -1;

    mPod.tessControlShaderVertices = 0;
    mPod.tessGenMode               = GL_NONE;
    mPod.tessGenSpacing            = GL_NONE;
    mPod.tessGenVertexOrder        = GL_NONE;
    mPod.tessGenPointMode          = GL_NONE;
    mPod.drawBufferTypeMask.reset();
    mPod.computeShaderLocalSize.fill(1);

    mPod.specConstUsageBits.reset();

    mActiveSamplersMask.reset();
    mActiveSamplerRefCounts = {};
    mActiveSamplerTypes.fill(TextureType::InvalidEnum);
    mActiveSamplerYUV.reset();
    mActiveSamplerFormats.fill(SamplerFormat::InvalidEnum);

    mActiveImagesMask.reset();

    mUniformBlockIndexToBufferBinding = {};

    mProgramInputs.clear();
    mLinkedTransformFeedbackVaryings.clear();
    mTransformFeedbackStrides.clear();
    mUniforms.clear();
    mUniformNames.clear();
    mUniformMappedNames.clear();
    mUniformBlocks.clear();
    mUniformLocations.clear();
    mShaderStorageBlocks.clear();
    mAtomicCounterBuffers.clear();
    mBufferVariables.clear();
    mOutputVariables.clear();
    mOutputLocations.clear();
    mSecondaryOutputLocations.clear();
    mSamplerBindings.clear();
    mSamplerBoundTextureUnits.clear();
    mImageBindings.clear();
    mPixelLocalStorageFormats.clear();

    mPostLinkSubTasks.clear();
    mPostLinkSubTaskWaitableEvents.clear();
}

void ProgramExecutable::load(gl::BinaryInputStream *stream)
{
    static_assert(MAX_VERTEX_ATTRIBS * 2 <= sizeof(uint32_t) * 8,
                  "Too many vertex attribs for mask: All bits of mAttributesTypeMask types and "
                  "mask fit into 32 bits each");
    static_assert(IMPLEMENTATION_MAX_DRAW_BUFFERS * 2 <= 8 * sizeof(uint32_t),
                  "All bits of mDrawBufferTypeMask and mActiveOutputVariables types and mask fit "
                  "into 32 bits each");

    stream->readStruct(&mPod);

    LoadProgramInputs(stream, &mProgramInputs);
    LoadUniforms(stream, &mUniforms, &mUniformNames, &mUniformMappedNames, &mUniformLocations);

    size_t uniformBlockCount = stream->readInt<size_t>();
    ASSERT(getUniformBlocks().empty());
    mUniformBlocks.resize(uniformBlockCount);
    for (size_t uniformBlockIndex = 0; uniformBlockIndex < uniformBlockCount; ++uniformBlockIndex)
    {
        InterfaceBlock &uniformBlock = mUniformBlocks[uniformBlockIndex];
        LoadInterfaceBlock(stream, &uniformBlock);
    }

    size_t shaderStorageBlockCount = stream->readInt<size_t>();
    ASSERT(getShaderStorageBlocks().empty());
    mShaderStorageBlocks.resize(shaderStorageBlockCount);
    for (size_t shaderStorageBlockIndex = 0; shaderStorageBlockIndex < shaderStorageBlockCount;
         ++shaderStorageBlockIndex)
    {
        InterfaceBlock &shaderStorageBlock = mShaderStorageBlocks[shaderStorageBlockIndex];
        LoadInterfaceBlock(stream, &shaderStorageBlock);
    }

    size_t atomicCounterBufferCount = stream->readInt<size_t>();
    ASSERT(getAtomicCounterBuffers().empty());
    mAtomicCounterBuffers.resize(atomicCounterBufferCount);
    for (size_t bufferIndex = 0; bufferIndex < atomicCounterBufferCount; ++bufferIndex)
    {
        AtomicCounterBuffer &atomicCounterBuffer = mAtomicCounterBuffers[bufferIndex];
        LoadAtomicCounterBuffer(stream, &atomicCounterBuffer);
    }

    size_t bufferVariableCount = stream->readInt<size_t>();
    ASSERT(getBufferVariables().empty());
    mBufferVariables.resize(bufferVariableCount);
    for (size_t bufferVarIndex = 0; bufferVarIndex < bufferVariableCount; ++bufferVarIndex)
    {
        LoadBufferVariable(stream, &mBufferVariables[bufferVarIndex]);
    }

    size_t transformFeedbackVaryingCount = stream->readInt<size_t>();
    ASSERT(mLinkedTransformFeedbackVaryings.empty());
    mLinkedTransformFeedbackVaryings.resize(transformFeedbackVaryingCount);
    for (size_t transformFeedbackVaryingIndex = 0;
         transformFeedbackVaryingIndex < transformFeedbackVaryingCount;
         ++transformFeedbackVaryingIndex)
    {
        TransformFeedbackVarying &varying =
            mLinkedTransformFeedbackVaryings[transformFeedbackVaryingIndex];
        stream->readVector(&varying.arraySizes);
        stream->readInt(&varying.type);
        stream->readString(&varying.name);
        varying.arrayIndex = stream->readInt<GLuint>();
    }

    size_t outputCount = stream->readInt<size_t>();
    ASSERT(getOutputVariables().empty());
    mOutputVariables.resize(outputCount);
    for (size_t outputIndex = 0; outputIndex < outputCount; ++outputIndex)
    {
        ProgramOutput &output = mOutputVariables[outputIndex];
        stream->readString(&output.name);
        stream->readString(&output.mappedName);
        stream->readStruct(&output.pod);
    }

    stream->readVector(&mOutputLocations);
    stream->readVector(&mSecondaryOutputLocations);
    LoadSamplerBindings(stream, &mSamplerBindings, &mSamplerBoundTextureUnits);

    size_t imageBindingCount = stream->readInt<size_t>();
    ASSERT(mImageBindings.empty());
    mImageBindings.resize(imageBindingCount);
    for (size_t imageIndex = 0; imageIndex < imageBindingCount; ++imageIndex)
    {
        ImageBinding &imageBinding = mImageBindings[imageIndex];
        size_t elementCount        = stream->readInt<size_t>();
        imageBinding.textureType   = static_cast<TextureType>(stream->readInt<unsigned int>());
        imageBinding.boundImageUnits.resize(elementCount);
        for (size_t elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            imageBinding.boundImageUnits[elementIndex] = stream->readInt<unsigned int>();
        }
    }

    // ANGLE_shader_pixel_local_storage.
    size_t plsCount = stream->readInt<size_t>();
    ASSERT(mPixelLocalStorageFormats.empty());
    mPixelLocalStorageFormats.resize(plsCount);
    stream->readBytes(reinterpret_cast<uint8_t *>(mPixelLocalStorageFormats.data()), plsCount);

    // These values are currently only used by PPOs, so only load them when the program is marked
    // separable to save memory.
    if (mPod.isSeparable)
    {
        for (ShaderType shaderType : getLinkedShaderStages())
        {
            mLinkedOutputVaryings[shaderType].resize(stream->readInt<size_t>());
            for (sh::ShaderVariable &variable : mLinkedOutputVaryings[shaderType])
            {
                LoadShaderVar(stream, &variable);
            }
            mLinkedInputVaryings[shaderType].resize(stream->readInt<size_t>());
            for (sh::ShaderVariable &variable : mLinkedInputVaryings[shaderType])
            {
                LoadShaderVar(stream, &variable);
            }
            mLinkedUniforms[shaderType].resize(stream->readInt<size_t>());
            for (sh::ShaderVariable &variable : mLinkedUniforms[shaderType])
            {
                LoadShaderVar(stream, &variable);
            }
            mLinkedUniformBlocks[shaderType].resize(stream->readInt<size_t>());
            for (sh::InterfaceBlock &shaderStorageBlock : mLinkedUniformBlocks[shaderType])
            {
                LoadShInterfaceBlock(stream, &shaderStorageBlock);
            }
        }
    }
}

void ProgramExecutable::save(gl::BinaryOutputStream *stream) const
{
    static_assert(MAX_VERTEX_ATTRIBS * 2 <= sizeof(uint32_t) * 8,
                  "All bits of mAttributesTypeMask types and mask fit into 32 bits each");
    static_assert(
        IMPLEMENTATION_MAX_DRAW_BUFFERS * 2 <= 8 * sizeof(uint32_t),
        "All bits of mDrawBufferTypeMask and mActiveOutputVariables can be contained in 32 bits");

    ASSERT(mPod.geometryShaderInvocations >= 1 && mPod.geometryShaderMaxVertices >= 0);
    stream->writeStruct(mPod);

    SaveProgramInputs(stream, mProgramInputs);
    SaveUniforms(stream, mUniforms, mUniformNames, mUniformMappedNames, mUniformLocations);

    stream->writeInt(getUniformBlocks().size());
    for (const InterfaceBlock &uniformBlock : getUniformBlocks())
    {
        WriteInterfaceBlock(stream, uniformBlock);
    }

    stream->writeInt(getShaderStorageBlocks().size());
    for (const InterfaceBlock &shaderStorageBlock : getShaderStorageBlocks())
    {
        WriteInterfaceBlock(stream, shaderStorageBlock);
    }

    stream->writeInt(mAtomicCounterBuffers.size());
    for (const AtomicCounterBuffer &atomicCounterBuffer : getAtomicCounterBuffers())
    {
        WriteAtomicCounterBuffer(stream, atomicCounterBuffer);
    }

    stream->writeInt(getBufferVariables().size());
    for (const BufferVariable &bufferVariable : getBufferVariables())
    {
        WriteBufferVariable(stream, bufferVariable);
    }

    stream->writeInt(getLinkedTransformFeedbackVaryings().size());
    for (const auto &var : getLinkedTransformFeedbackVaryings())
    {
        stream->writeVector(var.arraySizes);
        stream->writeInt(var.type);
        stream->writeString(var.name);

        stream->writeIntOrNegOne(var.arrayIndex);
    }

    stream->writeInt(getOutputVariables().size());
    for (const ProgramOutput &output : getOutputVariables())
    {
        stream->writeString(output.name);
        stream->writeString(output.mappedName);
        stream->writeStruct(output.pod);
    }

    stream->writeVector(mOutputLocations);
    stream->writeVector(mSecondaryOutputLocations);
    SaveSamplerBindings(stream, mSamplerBindings, mSamplerBoundTextureUnits);

    stream->writeInt(getImageBindings().size());
    for (const auto &imageBinding : getImageBindings())
    {
        stream->writeInt(imageBinding.boundImageUnits.size());
        stream->writeInt(static_cast<unsigned int>(imageBinding.textureType));
        for (size_t i = 0; i < imageBinding.boundImageUnits.size(); ++i)
        {
            stream->writeInt(imageBinding.boundImageUnits[i]);
        }
    }

    // ANGLE_shader_pixel_local_storage.
    stream->writeInt<size_t>(mPixelLocalStorageFormats.size());
    stream->writeBytes(reinterpret_cast<const uint8_t *>(mPixelLocalStorageFormats.data()),
                       mPixelLocalStorageFormats.size());

    // These values are currently only used by PPOs, so only save them when the program is marked
    // separable to save memory.
    if (mPod.isSeparable)
    {
        for (ShaderType shaderType : getLinkedShaderStages())
        {
            stream->writeInt(mLinkedOutputVaryings[shaderType].size());
            for (const sh::ShaderVariable &shaderVariable : mLinkedOutputVaryings[shaderType])
            {
                WriteShaderVar(stream, shaderVariable);
            }
            stream->writeInt(mLinkedInputVaryings[shaderType].size());
            for (const sh::ShaderVariable &shaderVariable : mLinkedInputVaryings[shaderType])
            {
                WriteShaderVar(stream, shaderVariable);
            }
            stream->writeInt(mLinkedUniforms[shaderType].size());
            for (const sh::ShaderVariable &shaderVariable : mLinkedUniforms[shaderType])
            {
                WriteShaderVar(stream, shaderVariable);
            }
            stream->writeInt(mLinkedUniformBlocks[shaderType].size());
            for (const sh::InterfaceBlock &shaderStorageBlock : mLinkedUniformBlocks[shaderType])
            {
                WriteShInterfaceBlock(stream, shaderStorageBlock);
            }
        }
    }
}

std::string ProgramExecutable::getInfoLogString() const
{
    return mInfoLog->str();
}

ShaderType ProgramExecutable::getFirstLinkedShaderStageType() const
{
    const ShaderBitSet linkedStages = mPod.linkedShaderStages;
    if (linkedStages.none())
    {
        return ShaderType::InvalidEnum;
    }

    return linkedStages.first();
}

ShaderType ProgramExecutable::getLastLinkedShaderStageType() const
{
    const ShaderBitSet linkedStages = mPod.linkedShaderStages;
    if (linkedStages.none())
    {
        return ShaderType::InvalidEnum;
    }

    return linkedStages.last();
}

void ProgramExecutable::setActive(size_t textureUnit,
                                  const SamplerBinding &samplerBinding,
                                  const gl::LinkedUniform &samplerUniform)
{
    mActiveSamplersMask.set(textureUnit);
    mActiveSamplerTypes[textureUnit]      = samplerBinding.textureType;
    mActiveSamplerYUV[textureUnit]        = IsSamplerYUVType(samplerBinding.samplerType);
    mActiveSamplerFormats[textureUnit]    = samplerBinding.format;
    mActiveSamplerShaderBits[textureUnit] = samplerUniform.activeShaders();
}

void ProgramExecutable::setInactive(size_t textureUnit)
{
    mActiveSamplersMask.reset(textureUnit);
    mActiveSamplerTypes[textureUnit] = TextureType::InvalidEnum;
    mActiveSamplerYUV.reset(textureUnit);
    mActiveSamplerFormats[textureUnit] = SamplerFormat::InvalidEnum;
    mActiveSamplerShaderBits[textureUnit].reset();
}

void ProgramExecutable::hasSamplerTypeConflict(size_t textureUnit)
{
    // Conflicts are marked with InvalidEnum
    mActiveSamplerYUV.reset(textureUnit);
    mActiveSamplerTypes[textureUnit] = TextureType::InvalidEnum;
}

void ProgramExecutable::hasSamplerFormatConflict(size_t textureUnit)
{
    // Conflicts are marked with InvalidEnum
    mActiveSamplerFormats[textureUnit] = SamplerFormat::InvalidEnum;
}

void ProgramExecutable::updateActiveSamplers(const ProgramExecutable &executable)
{
    const std::vector<SamplerBinding> &samplerBindings = executable.getSamplerBindings();
    const std::vector<GLuint> &boundTextureUnits       = executable.getSamplerBoundTextureUnits();

    for (uint32_t samplerIndex = 0; samplerIndex < samplerBindings.size(); ++samplerIndex)
    {
        const SamplerBinding &samplerBinding = samplerBindings[samplerIndex];

        for (uint16_t index = 0; index < samplerBinding.textureUnitsCount; index++)
        {
            GLint textureUnit = samplerBinding.getTextureUnit(boundTextureUnits, index);
            if (++mActiveSamplerRefCounts[textureUnit] == 1)
            {
                uint32_t uniformIndex = executable.getUniformIndexFromSamplerIndex(samplerIndex);
                setActive(textureUnit, samplerBinding, executable.getUniforms()[uniformIndex]);
            }
            else
            {
                if (mActiveSamplerTypes[textureUnit] != samplerBinding.textureType ||
                    mActiveSamplerYUV.test(textureUnit) !=
                        IsSamplerYUVType(samplerBinding.samplerType))
                {
                    hasSamplerTypeConflict(textureUnit);
                }

                if (mActiveSamplerFormats[textureUnit] != samplerBinding.format)
                {
                    hasSamplerFormatConflict(textureUnit);
                }
            }
            mActiveSamplersMask.set(textureUnit);
        }
    }

    // Invalidate the validation cache.
    resetCachedValidateSamplersResult();
}

void ProgramExecutable::updateActiveImages(const ProgramExecutable &executable)
{
    const std::vector<ImageBinding> &imageBindings = executable.getImageBindings();
    for (uint32_t imageIndex = 0; imageIndex < imageBindings.size(); ++imageIndex)
    {
        const gl::ImageBinding &imageBinding = imageBindings.at(imageIndex);

        uint32_t uniformIndex = executable.getUniformIndexFromImageIndex(imageIndex);
        const gl::LinkedUniform &imageUniform = executable.getUniforms()[uniformIndex];
        const ShaderBitSet shaderBits         = imageUniform.activeShaders();
        for (GLint imageUnit : imageBinding.boundImageUnits)
        {
            mActiveImagesMask.set(imageUnit);
            mActiveImageShaderBits[imageUnit] |= shaderBits;
        }
    }
}

void ProgramExecutable::setSamplerUniformTextureTypeAndFormat(size_t textureUnitIndex)
{
    bool foundBinding         = false;
    TextureType foundType     = TextureType::InvalidEnum;
    bool foundYUV             = false;
    SamplerFormat foundFormat = SamplerFormat::InvalidEnum;

    for (uint32_t samplerIndex = 0; samplerIndex < mSamplerBindings.size(); ++samplerIndex)
    {
        const SamplerBinding &binding = mSamplerBindings[samplerIndex];

        // A conflict exists if samplers of different types are sourced by the same texture unit.
        // We need to check all bound textures to detect this error case.
        for (uint16_t index = 0; index < binding.textureUnitsCount; index++)
        {
            GLuint textureUnit = binding.getTextureUnit(mSamplerBoundTextureUnits, index);
            if (textureUnit != textureUnitIndex)
            {
                continue;
            }

            if (!foundBinding)
            {
                foundBinding          = true;
                foundType             = binding.textureType;
                foundYUV              = IsSamplerYUVType(binding.samplerType);
                foundFormat           = binding.format;
                uint32_t uniformIndex = getUniformIndexFromSamplerIndex(samplerIndex);
                setActive(textureUnit, binding, mUniforms[uniformIndex]);
            }
            else
            {
                if (foundType != binding.textureType ||
                    foundYUV != IsSamplerYUVType(binding.samplerType))
                {
                    hasSamplerTypeConflict(textureUnit);
                }

                if (foundFormat != binding.format)
                {
                    hasSamplerFormatConflict(textureUnit);
                }
            }
        }
    }
}

void ProgramExecutable::saveLinkedStateInfo(const ProgramState &state)
{
    for (ShaderType shaderType : getLinkedShaderStages())
    {
        const SharedCompiledShaderState &shader = state.getAttachedShader(shaderType);
        ASSERT(shader);
        mPod.linkedShaderVersions[shaderType] = shader->shaderVersion;
        mLinkedOutputVaryings[shaderType]     = shader->outputVaryings;
        mLinkedInputVaryings[shaderType]      = shader->inputVaryings;
        mLinkedUniforms[shaderType]           = shader->uniforms;
        mLinkedUniformBlocks[shaderType]      = shader->uniformBlocks;
    }
}

bool ProgramExecutable::linkMergedVaryings(const Caps &caps,
                                           const Limitations &limitations,
                                           const Version &clientVersion,
                                           bool webglCompatibility,
                                           const ProgramMergedVaryings &mergedVaryings,
                                           const LinkingVariables &linkingVariables,
                                           ProgramVaryingPacking *varyingPacking)
{
    ShaderType tfStage = GetLastPreFragmentStage(linkingVariables.isShaderStageUsedBitset);

    if (!linkValidateTransformFeedback(caps, clientVersion, mergedVaryings, tfStage))
    {
        return false;
    }

    // Map the varyings to the register file
    // In WebGL, we use a slightly different handling for packing variables.
    gl::PackMode packMode = PackMode::ANGLE_RELAXED;
    if (limitations.noFlexibleVaryingPacking)
    {
        // D3D9 pack mode is strictly more strict than WebGL, so takes priority.
        packMode = PackMode::ANGLE_NON_CONFORMANT_D3D9;
    }
    else if (webglCompatibility)
    {
        packMode = PackMode::WEBGL_STRICT;
    }

    // Build active shader stage map.
    ShaderBitSet activeShadersMask;
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        // - Check for attached shaders to handle the case of a Program linking the currently
        // attached shaders.
        // - Check for linked shaders to handle the case of a PPO linking separable programs before
        // drawing.
        if (linkingVariables.isShaderStageUsedBitset[shaderType] ||
            getLinkedShaderStages().test(shaderType))
        {
            activeShadersMask[shaderType] = true;
        }
    }

    if (!varyingPacking->collectAndPackUserVaryings(*mInfoLog, caps, packMode, activeShadersMask,
                                                    mergedVaryings, mTransformFeedbackVaryingNames,
                                                    mPod.isSeparable))
    {
        return false;
    }

    gatherTransformFeedbackVaryings(mergedVaryings, tfStage);
    updateTransformFeedbackStrides();

    return true;
}

bool ProgramExecutable::linkValidateTransformFeedback(const Caps &caps,
                                                      const Version &clientVersion,
                                                      const ProgramMergedVaryings &varyings,
                                                      ShaderType stage)
{
    // Validate the tf names regardless of the actual program varyings.
    std::set<std::string> uniqueNames;
    for (const std::string &tfVaryingName : mTransformFeedbackVaryingNames)
    {
        if (clientVersion < Version(3, 1) && tfVaryingName.find('[') != std::string::npos)
        {
            *mInfoLog << "Capture of array elements is undefined and not supported.";
            return false;
        }
        if (clientVersion >= Version(3, 1))
        {
            if (IncludeSameArrayElement(uniqueNames, tfVaryingName))
            {
                *mInfoLog << "Two transform feedback varyings include the same array element ("
                          << tfVaryingName << ").";
                return false;
            }
        }
        else
        {
            if (uniqueNames.count(tfVaryingName) > 0)
            {
                *mInfoLog << "Two transform feedback varyings specify the same output variable ("
                          << tfVaryingName << ").";
                return false;
            }
        }
        uniqueNames.insert(tfVaryingName);
    }

    // From OpneGLES spec. 11.1.2.1: A program will fail to link if:
    // the count specified by TransformFeedbackVaryings is non-zero, but the
    // program object has no vertex, tessellation evaluation, or geometry shader
    if (mTransformFeedbackVaryingNames.size() > 0 &&
        !gl::ShaderTypeSupportsTransformFeedback(getLinkedTransformFeedbackStage()))
    {
        *mInfoLog << "Linked transform feedback stage " << getLinkedTransformFeedbackStage()
                  << " does not support transform feedback varying.";
        return false;
    }

    // Validate against program varyings.
    size_t totalComponents = 0;
    for (const std::string &tfVaryingName : mTransformFeedbackVaryingNames)
    {
        std::vector<unsigned int> subscripts;
        std::string baseName = ParseResourceName(tfVaryingName, &subscripts);

        const sh::ShaderVariable *var = FindOutputVaryingOrField(varyings, stage, baseName);
        if (var == nullptr)
        {
            *mInfoLog << "Transform feedback varying " << tfVaryingName
                      << " does not exist in the vertex shader.";
            return false;
        }

        // Validate the matching variable.
        if (var->isStruct())
        {
            *mInfoLog << "Struct cannot be captured directly (" << baseName << ").";
            return false;
        }

        size_t elementCount   = 0;
        size_t componentCount = 0;

        if (var->isArray())
        {
            if (clientVersion < Version(3, 1))
            {
                *mInfoLog << "Capture of arrays is undefined and not supported.";
                return false;
            }

            // GLSL ES 3.10 section 4.3.6: A vertex output can't be an array of arrays.
            ASSERT(!var->isArrayOfArrays());

            if (!subscripts.empty() && subscripts[0] >= var->getOutermostArraySize())
            {
                *mInfoLog << "Cannot capture outbound array element '" << tfVaryingName << "'.";
                return false;
            }
            elementCount = (subscripts.empty() ? var->getOutermostArraySize() : 1);
        }
        else
        {
            if (!subscripts.empty())
            {
                *mInfoLog << "Varying '" << baseName
                          << "' is not an array to be captured by element.";
                return false;
            }
            elementCount = 1;
        }

        componentCount = VariableComponentCount(var->type) * elementCount;
        if (mPod.transformFeedbackBufferMode == GL_SEPARATE_ATTRIBS &&
            componentCount > static_cast<GLuint>(caps.maxTransformFeedbackSeparateComponents))
        {
            *mInfoLog << "Transform feedback varying " << tfVaryingName << " components ("
                      << componentCount << ") exceed the maximum separate components ("
                      << caps.maxTransformFeedbackSeparateComponents << ").";
            return false;
        }

        totalComponents += componentCount;
        if (mPod.transformFeedbackBufferMode == GL_INTERLEAVED_ATTRIBS &&
            totalComponents > static_cast<GLuint>(caps.maxTransformFeedbackInterleavedComponents))
        {
            *mInfoLog << "Transform feedback varying total components (" << totalComponents
                      << ") exceed the maximum interleaved components ("
                      << caps.maxTransformFeedbackInterleavedComponents << ").";
            return false;
        }
    }
    return true;
}

void ProgramExecutable::gatherTransformFeedbackVaryings(const ProgramMergedVaryings &varyings,
                                                        ShaderType stage)
{
    // Gather the linked varyings that are used for transform feedback, they should all exist.
    mLinkedTransformFeedbackVaryings.clear();
    for (const std::string &tfVaryingName : mTransformFeedbackVaryingNames)
    {
        std::vector<unsigned int> subscripts;
        std::string baseName = ParseResourceName(tfVaryingName, &subscripts);
        size_t subscript     = GL_INVALID_INDEX;
        if (!subscripts.empty())
        {
            subscript = subscripts.back();
        }
        for (const ProgramVaryingRef &ref : varyings)
        {
            if (ref.frontShaderStage != stage)
            {
                continue;
            }

            const sh::ShaderVariable *varying = ref.get(stage);
            if (baseName == varying->name)
            {
                mLinkedTransformFeedbackVaryings.emplace_back(*varying,
                                                              static_cast<GLuint>(subscript));
                break;
            }
            else if (varying->isStruct())
            {
                GLuint fieldIndex = 0;
                const auto *field = varying->findField(tfVaryingName, &fieldIndex);
                if (field != nullptr)
                {
                    mLinkedTransformFeedbackVaryings.emplace_back(*field, *varying);
                    break;
                }
            }
        }
    }
}

void ProgramExecutable::updateTransformFeedbackStrides()
{
    if (mLinkedTransformFeedbackVaryings.empty())
    {
        return;
    }

    if (mPod.transformFeedbackBufferMode == GL_INTERLEAVED_ATTRIBS)
    {
        mTransformFeedbackStrides.resize(1);
        size_t totalSize = 0;
        for (const TransformFeedbackVarying &varying : mLinkedTransformFeedbackVaryings)
        {
            totalSize += varying.size() * VariableExternalSize(varying.type);
        }
        mTransformFeedbackStrides[0] = static_cast<GLsizei>(totalSize);
    }
    else
    {
        mTransformFeedbackStrides.resize(mLinkedTransformFeedbackVaryings.size());
        for (size_t i = 0; i < mLinkedTransformFeedbackVaryings.size(); i++)
        {
            TransformFeedbackVarying &varying = mLinkedTransformFeedbackVaryings[i];
            mTransformFeedbackStrides[i] =
                static_cast<GLsizei>(varying.size() * VariableExternalSize(varying.type));
        }
    }
}

bool ProgramExecutable::validateSamplersImpl(const Caps &caps) const
{
    // if any two active samplers in a program are of different types, but refer to the same
    // texture image unit, and this is the current program, then ValidateProgram will fail, and
    // DrawArrays and DrawElements will issue the INVALID_OPERATION error.
    for (size_t textureUnit : mActiveSamplersMask)
    {
        if (mActiveSamplerTypes[textureUnit] == TextureType::InvalidEnum)
        {
            mCachedValidateSamplersResult = false;
            return false;
        }

        if (mActiveSamplerFormats[textureUnit] == SamplerFormat::InvalidEnum)
        {
            mCachedValidateSamplersResult = false;
            return false;
        }
    }

    mCachedValidateSamplersResult = true;
    return true;
}

bool ProgramExecutable::linkValidateOutputVariables(
    const Caps &caps,
    const Version &version,
    GLuint combinedImageUniformsCount,
    GLuint combinedShaderStorageBlocksCount,
    int fragmentShaderVersion,
    const ProgramAliasedBindings &fragmentOutputLocations,
    const ProgramAliasedBindings &fragmentOutputIndices)
{
    ASSERT(mPod.activeOutputVariablesMask.none());
    ASSERT(mPod.activeSecondaryOutputVariablesMask.none());
    ASSERT(mPod.drawBufferTypeMask.none());
    ASSERT(!mPod.hasYUVOutput);

    if (fragmentShaderVersion == 100)
    {
        return gatherOutputTypes();
    }

    // EXT_blend_func_extended doesn't specify anything related to binding specific elements of an
    // output array in explicit terms.
    //
    // Assuming fragData is an output array, you can defend the position that:
    // P1) you must support binding "fragData" because it's specified
    // P2) you must support querying "fragData[x]" because it's specified
    // P3) you must support binding "fragData[0]" because it's a frequently used pattern
    //
    // Then you can make the leap of faith:
    // P4) you must support binding "fragData[x]" because you support "fragData[0]"
    // P5) you must support binding "fragData[x]" because you support querying "fragData[x]"
    //
    // The spec brings in the "world of arrays" when it mentions binding the arrays and the
    // automatic binding. Thus it must be interpreted that the thing is not undefined, rather you
    // must infer the only possible interpretation (?). Note again: this need of interpretation
    // might be completely off of what GL spec logic is.
    //
    // The other complexity is that unless you implement this feature, it's hard to understand what
    // should happen when the client invokes the feature. You cannot add an additional error as it
    // is not specified. One can ignore it, but obviously it creates the discrepancies...

    std::vector<VariableLocation> reservedLocations;

    // Process any output API bindings for arrays that don't alias to the first element.
    for (const auto &bindingPair : fragmentOutputLocations)
    {
        const std::string &name       = bindingPair.first;
        const ProgramBinding &binding = bindingPair.second;

        size_t nameLengthWithoutArrayIndex;
        unsigned int arrayIndex = ParseArrayIndex(name, &nameLengthWithoutArrayIndex);
        if (arrayIndex == 0 || arrayIndex == GL_INVALID_INDEX)
        {
            continue;
        }
        for (unsigned int outputVariableIndex = 0; outputVariableIndex < mOutputVariables.size();
             outputVariableIndex++)
        {
            const ProgramOutput &outputVariable = mOutputVariables[outputVariableIndex];
            // Check that the binding corresponds to an output array and its array index fits.
            if (outputVariable.isBuiltIn() || !outputVariable.isArray() ||
                !angle::BeginsWith(outputVariable.name, name, nameLengthWithoutArrayIndex) ||
                arrayIndex >= outputVariable.getOutermostArraySize())
            {
                continue;
            }

            // Get the API index that corresponds to this exact binding.
            // This index may differ from the index used for the array's base.
            std::vector<VariableLocation> &outputLocations =
                fragmentOutputIndices.getBindingByName(name) == 1 ? mSecondaryOutputLocations
                                                                  : mOutputLocations;
            unsigned int location = binding.location;
            VariableLocation locationInfo(arrayIndex, outputVariableIndex);
            if (location >= outputLocations.size())
            {
                outputLocations.resize(location + 1);
            }
            if (outputLocations[location].used())
            {
                *mInfoLog << "Location of variable " << outputVariable.name
                          << " conflicts with another variable.";
                return false;
            }
            outputLocations[location] = locationInfo;

            // Note the array binding location so that it can be skipped later.
            reservedLocations.push_back(locationInfo);
        }
    }

    // Reserve locations for output variables whose location is fixed in the shader or through the
    // API. Otherwise, the remaining unallocated outputs will be processed later.
    for (unsigned int outputVariableIndex = 0; outputVariableIndex < mOutputVariables.size();
         outputVariableIndex++)
    {
        ProgramOutput &outputVariable = mOutputVariables[outputVariableIndex];

        // Don't store outputs for gl_FragDepth, gl_FragColor, etc.
        if (outputVariable.isBuiltIn())
            continue;

        int fixedLocation = GetOutputLocationForLink(fragmentOutputLocations, outputVariable);
        if (fixedLocation == -1)
        {
            // Here we're only reserving locations for variables whose location is fixed.
            continue;
        }
        unsigned int baseLocation = static_cast<unsigned int>(fixedLocation);

        AssignOutputIndex(fragmentOutputIndices, outputVariable);
        ASSERT(outputVariable.pod.index == 0 || outputVariable.pod.index == 1);
        std::vector<VariableLocation> &outputLocations =
            outputVariable.pod.index == 0 ? mOutputLocations : mSecondaryOutputLocations;

        // GLSL ES 3.10 section 4.3.6: Output variables cannot be arrays of arrays or arrays of
        // structures, so we may use getBasicTypeElementCount().
        unsigned int elementCount = outputVariable.pod.basicTypeElementCount;
        if (FindUsedOutputLocation(outputLocations, baseLocation, elementCount, reservedLocations,
                                   outputVariableIndex))
        {
            *mInfoLog << "Location of variable " << outputVariable.name
                      << " conflicts with another variable.";
            return false;
        }
        bool hasApiAssignedLocation = !outputVariable.pod.hasShaderAssignedLocation &&
                                      (fragmentOutputLocations.getBinding(outputVariable) != -1);
        AssignOutputLocations(outputLocations, baseLocation, elementCount, reservedLocations,
                              outputVariableIndex, hasApiAssignedLocation, outputVariable);
    }

    // Here we assign locations for the output variables that don't yet have them. Note that we're
    // not necessarily able to fit the variables optimally, since then we might have to try
    // different arrangements of output arrays. Now we just assign the locations in the order that
    // we got the output variables. The spec isn't clear on what kind of algorithm is required for
    // finding locations for the output variables, so this should be acceptable at least for now.
    GLuint maxLocation = static_cast<GLuint>(caps.maxDrawBuffers);
    if (!mSecondaryOutputLocations.empty())
    {
        // EXT_blend_func_extended: Program outputs will be validated against
        // MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT if there's even one output with index one.
        maxLocation = caps.maxDualSourceDrawBuffers;
    }

    for (unsigned int outputVariableIndex = 0; outputVariableIndex < mOutputVariables.size();
         outputVariableIndex++)
    {
        ProgramOutput &outputVariable = mOutputVariables[outputVariableIndex];

        // Don't store outputs for gl_FragDepth, gl_FragColor, etc.
        if (outputVariable.isBuiltIn())
            continue;

        AssignOutputIndex(fragmentOutputIndices, outputVariable);
        ASSERT(outputVariable.pod.index == 0 || outputVariable.pod.index == 1);
        std::vector<VariableLocation> &outputLocations =
            outputVariable.pod.index == 0 ? mOutputLocations : mSecondaryOutputLocations;

        int fixedLocation = GetOutputLocationForLink(fragmentOutputLocations, outputVariable);
        unsigned int baseLocation = 0;
        unsigned int elementCount = outputVariable.pod.basicTypeElementCount;
        if (fixedLocation != -1)
        {
            // Secondary inputs might have caused the max location to drop below what has already
            // been explicitly assigned locations. Check for any fixed locations above the max
            // that should cause linking to fail.
            baseLocation = static_cast<unsigned int>(fixedLocation);
        }
        else
        {
            // No fixed location, so try to fit the output in unassigned locations.
            // Try baseLocations starting from 0 one at a time and see if the variable fits.
            while (FindUsedOutputLocation(outputLocations, baseLocation, elementCount,
                                          reservedLocations, outputVariableIndex))
            {
                baseLocation++;
            }
            AssignOutputLocations(outputLocations, baseLocation, elementCount, reservedLocations,
                                  outputVariableIndex, false, outputVariable);
        }

        // Check for any elements assigned above the max location that are actually used.
        if (baseLocation + elementCount > maxLocation &&
            (baseLocation >= maxLocation ||
             FindUsedOutputLocation(outputLocations, maxLocation,
                                    baseLocation + elementCount - maxLocation, reservedLocations,
                                    outputVariableIndex)))
        {
            // EXT_blend_func_extended: Linking can fail:
            // "if the explicit binding assignments do not leave enough space for the linker to
            // automatically assign a location for a varying out array, which requires multiple
            // contiguous locations."
            *mInfoLog << "Could not fit output variable into available locations: "
                      << outputVariable.name;
            return false;
        }
    }

    if (!gatherOutputTypes())
    {
        return false;
    }

    if (version >= ES_3_1)
    {
        // [OpenGL ES 3.1] Chapter 8.22 Page 203:
        // A link error will be generated if the sum of the number of active image uniforms used in
        // all shaders, the number of active shader storage blocks, and the number of active
        // fragment shader outputs exceeds the implementation-dependent value of
        // MAX_COMBINED_SHADER_OUTPUT_RESOURCES.
        if (combinedImageUniformsCount + combinedShaderStorageBlocksCount +
                mPod.activeOutputVariablesMask.count() >
            static_cast<GLuint>(caps.maxCombinedShaderOutputResources))
        {
            *mInfoLog
                << "The sum of the number of active image uniforms, active shader storage blocks "
                   "and active fragment shader outputs exceeds "
                   "MAX_COMBINED_SHADER_OUTPUT_RESOURCES ("
                << caps.maxCombinedShaderOutputResources << ")";
            return false;
        }
    }

    return true;
}

bool ProgramExecutable::gatherOutputTypes()
{
    for (const ProgramOutput &outputVariable : mOutputVariables)
    {
        if (outputVariable.isBuiltIn() && outputVariable.name != "gl_FragColor" &&
            outputVariable.name != "gl_FragData" &&
            outputVariable.name != "gl_SecondaryFragColorEXT" &&
            outputVariable.name != "gl_SecondaryFragDataEXT")
        {
            continue;
        }

        unsigned int baseLocation = (outputVariable.pod.location == -1
                                         ? 0u
                                         : static_cast<unsigned int>(outputVariable.pod.location));

        const bool secondary =
            outputVariable.pod.index == 1 || (outputVariable.name == "gl_SecondaryFragColorEXT" ||
                                              outputVariable.name == "gl_SecondaryFragDataEXT");

        const ComponentType componentType =
            GLenumToComponentType(VariableComponentType(outputVariable.pod.type));

        // GLSL ES 3.10 section 4.3.6: Output variables cannot be arrays of arrays or arrays of
        // structures, so we may use getBasicTypeElementCount().
        unsigned int elementCount = outputVariable.pod.basicTypeElementCount;
        for (unsigned int elementIndex = 0; elementIndex < elementCount; elementIndex++)
        {
            const unsigned int location = baseLocation + elementIndex;
            ASSERT(location < mPod.activeOutputVariablesMask.size());
            ASSERT(location < mPod.activeSecondaryOutputVariablesMask.size());
            if (secondary)
            {
                mPod.activeSecondaryOutputVariablesMask.set(location);
            }
            else
            {
                mPod.activeOutputVariablesMask.set(location);
            }
            const ComponentType storedComponentType =
                gl::GetComponentTypeMask(mPod.drawBufferTypeMask, location);
            if (storedComponentType == ComponentType::InvalidEnum)
            {
                SetComponentTypeMask(componentType, location, &mPod.drawBufferTypeMask);
            }
            else if (storedComponentType != componentType)
            {
                *mInfoLog << "Inconsistent component types for fragment outputs at location "
                          << location;
                return false;
            }
        }

        if (outputVariable.pod.yuv)
        {
            ASSERT(mOutputVariables.size() == 1);
            mPod.hasYUVOutput = true;
        }
    }

    return true;
}

bool ProgramExecutable::linkUniforms(
    const Caps &caps,
    const ShaderMap<std::vector<sh::ShaderVariable>> &shaderUniforms,
    const ProgramAliasedBindings &uniformLocationBindings,
    GLuint *combinedImageUniformsCountOut,
    std::vector<UnusedUniform> *unusedUniformsOutOrNull)
{
    UniformLinker linker(mPod.linkedShaderStages, shaderUniforms);
    if (!linker.link(caps, *mInfoLog, uniformLocationBindings))
    {
        return false;
    }

    linker.getResults(&mUniforms, &mUniformNames, &mUniformMappedNames, unusedUniformsOutOrNull,
                      &mUniformLocations);

    linkSamplerAndImageBindings(combinedImageUniformsCountOut);

    if (!linkAtomicCounterBuffers(caps))
    {
        return false;
    }

    return true;
}

void ProgramExecutable::linkSamplerAndImageBindings(GLuint *combinedImageUniforms)
{
    ASSERT(combinedImageUniforms);

    // Iterate over mExecutable->mUniforms from the back, and find the range of subpass inputs,
    // atomic counters, images and samplers in that order.
    auto highIter = mUniforms.rbegin();
    auto lowIter  = highIter;

    unsigned int high = static_cast<unsigned int>(mUniforms.size());
    unsigned int low  = high;

    // Note that uniform block uniforms are not yet appended to this list.
    ASSERT(mUniforms.empty() || highIter->isAtomicCounter() || highIter->isImage() ||
           highIter->isSampler() || highIter->isInDefaultBlock());

    for (; lowIter != mUniforms.rend() && lowIter->isAtomicCounter(); ++lowIter)
    {
        --low;
    }

    mPod.atomicCounterUniformRange = RangeUI(low, high);

    highIter = lowIter;
    high     = low;

    for (; lowIter != mUniforms.rend() && lowIter->isImage(); ++lowIter)
    {
        --low;
    }

    mPod.imageUniformRange = RangeUI(low, high);
    *combinedImageUniforms = 0u;
    // If uniform is a image type, insert it into the mImageBindings array.
    for (unsigned int imageIndex : mPod.imageUniformRange)
    {
        // ES3.1 (section 7.6.1) and GLSL ES3.1 (section 4.4.5), Uniform*i{v} commands
        // cannot load values into a uniform defined as an image. if declare without a
        // binding qualifier, any uniform image variable (include all elements of
        // unbound image array) should be bound to unit zero.
        auto &imageUniform      = mUniforms[imageIndex];
        TextureType textureType = ImageTypeToTextureType(imageUniform.getType());
        const GLuint arraySize  = imageUniform.getBasicTypeElementCount();

        if (imageUniform.getBinding() == -1)
        {
            mImageBindings.emplace_back(
                ImageBinding(imageUniform.getBasicTypeElementCount(), textureType));
        }
        else
        {
            // The arrays of arrays are flattened to arrays, it needs to record the array offset for
            // the correct binding image unit.
            mImageBindings.emplace_back(ImageBinding(
                imageUniform.getBinding() + imageUniform.pod.parentArrayIndex * arraySize,
                imageUniform.getBasicTypeElementCount(), textureType));
        }

        *combinedImageUniforms += imageUniform.activeShaderCount() * arraySize;
    }

    highIter = lowIter;
    high     = low;

    for (; lowIter != mUniforms.rend() && lowIter->isSampler(); ++lowIter)
    {
        --low;
    }

    mPod.samplerUniformRange = RangeUI(low, high);

    // If uniform is a sampler type, insert it into the mSamplerBindings array.
    uint16_t totalCount = 0;
    for (unsigned int samplerIndex : mPod.samplerUniformRange)
    {
        const auto &samplerUniform = mUniforms[samplerIndex];
        TextureType textureType    = SamplerTypeToTextureType(samplerUniform.getType());
        GLenum samplerType         = samplerUniform.getType();
        uint16_t elementCount      = samplerUniform.getBasicTypeElementCount();
        SamplerFormat format       = GetUniformTypeInfo(samplerType).samplerFormat;
        mSamplerBindings.emplace_back(textureType, samplerType, format, totalCount, elementCount);
        totalCount += elementCount;
    }
    mSamplerBoundTextureUnits.resize(totalCount, 0);

    // Whatever is left constitutes the default uniforms.
    mPod.defaultUniformRange = RangeUI(0, low);
}

bool ProgramExecutable::linkAtomicCounterBuffers(const Caps &caps)
{
    for (unsigned int index : mPod.atomicCounterUniformRange)
    {
        auto &uniform = mUniforms[index];

        uniform.pod.blockOffset                    = uniform.getOffset();
        uniform.pod.blockArrayStride               = uniform.isArray() ? 4 : 0;
        uniform.pod.blockMatrixStride              = 0;
        uniform.pod.flagBits.blockIsRowMajorMatrix = false;
        uniform.pod.flagBits.isBlock               = true;

        bool found = false;
        for (size_t bufferIndex = 0; bufferIndex < mAtomicCounterBuffers.size(); ++bufferIndex)
        {
            AtomicCounterBuffer &buffer = mAtomicCounterBuffers[bufferIndex];
            if (buffer.pod.inShaderBinding == uniform.getBinding())
            {
                buffer.memberIndexes.push_back(index);
                SetBitField(uniform.pod.bufferIndex, bufferIndex);
                found = true;
                buffer.unionReferencesWith(uniform);
                break;
            }
        }
        if (!found)
        {
            AtomicCounterBuffer atomicCounterBuffer;
            atomicCounterBuffer.pod.inShaderBinding = uniform.getBinding();
            atomicCounterBuffer.memberIndexes.push_back(index);
            atomicCounterBuffer.unionReferencesWith(uniform);
            mAtomicCounterBuffers.push_back(atomicCounterBuffer);
            SetBitField(uniform.pod.bufferIndex, mAtomicCounterBuffers.size() - 1);
        }
    }

    // Count each atomic counter buffer to validate against
    // per-stage and combined gl_Max*AtomicCounterBuffers.
    GLint combinedShaderACBCount           = 0;
    gl::ShaderMap<GLint> perShaderACBCount = {};
    for (size_t bufferIndex = 0; bufferIndex < mAtomicCounterBuffers.size(); ++bufferIndex)
    {
        AtomicCounterBuffer &acb        = mAtomicCounterBuffers[bufferIndex];
        const ShaderBitSet shaderStages = acb.activeShaders();
        for (gl::ShaderType shaderType : shaderStages)
        {
            ++perShaderACBCount[shaderType];
        }
        ++combinedShaderACBCount;
    }
    if (combinedShaderACBCount > caps.maxCombinedAtomicCounterBuffers)
    {
        *mInfoLog << " combined AtomicCounterBuffers count exceeds limit";
        return false;
    }
    for (gl::ShaderType stage : gl::AllShaderTypes())
    {
        if (perShaderACBCount[stage] > caps.maxShaderAtomicCounterBuffers[stage])
        {
            *mInfoLog << GetShaderTypeString(stage)
                      << " shader AtomicCounterBuffers count exceeds limit";
            return false;
        }
    }
    return true;
}

void ProgramExecutable::copyInputsFromProgram(const ProgramExecutable &executable)
{
    mProgramInputs = executable.getProgramInputs();
}

void ProgramExecutable::copyUniformBuffersFromProgram(
    const ProgramExecutable &executable,
    ShaderType shaderType,
    ProgramUniformBlockArray<GLuint> *ppoUniformBlockMap)
{
    AppendActiveBlocks(shaderType, executable.getUniformBlocks(), mUniformBlocks,
                       ppoUniformBlockMap);

    const std::vector<InterfaceBlock> &blocks = executable.getUniformBlocks();
    for (size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex)
    {
        if (!blocks[blockIndex].isActive(shaderType))
        {
            continue;
        }
        const uint32_t blockIndexInPPO = (*ppoUniformBlockMap)[static_cast<uint32_t>(blockIndex)];
        ASSERT(blockIndexInPPO < mUniformBlocks.size());

        // Set the block buffer binding in the PPO to the same binding as the program's.
        remapUniformBlockBinding({blockIndexInPPO}, executable.getUniformBlockBinding(blockIndex));
    }
}

void ProgramExecutable::copyStorageBuffersFromProgram(const ProgramExecutable &executable,
                                                      ShaderType shaderType)
{
    AppendActiveBlocks(shaderType, executable.getShaderStorageBlocks(), mShaderStorageBlocks,
                       nullptr);
    AppendActiveBlocks(shaderType, executable.getAtomicCounterBuffers(), mAtomicCounterBuffers,
                       nullptr);

    // Buffer variable info is queried through the program, and program pipelines don't access it.
    ASSERT(mBufferVariables.empty());
}

void ProgramExecutable::clearSamplerBindings()
{
    mSamplerBindings.clear();
    mSamplerBoundTextureUnits.clear();
}

void ProgramExecutable::copySamplerBindingsFromProgram(const ProgramExecutable &executable)
{
    const std::vector<SamplerBinding> &bindings = executable.getSamplerBindings();
    const std::vector<GLuint> &textureUnits     = executable.getSamplerBoundTextureUnits();
    uint16_t adjustedStartIndex                 = mSamplerBoundTextureUnits.size();
    mSamplerBoundTextureUnits.insert(mSamplerBoundTextureUnits.end(), textureUnits.begin(),
                                     textureUnits.end());
    for (const SamplerBinding &binding : bindings)
    {
        mSamplerBindings.push_back(binding);
        mSamplerBindings.back().textureUnitsStartIndex += adjustedStartIndex;
    }
}

void ProgramExecutable::copyImageBindingsFromProgram(const ProgramExecutable &executable)
{
    const std::vector<ImageBinding> &bindings = executable.getImageBindings();
    mImageBindings.insert(mImageBindings.end(), bindings.begin(), bindings.end());
}

void ProgramExecutable::copyOutputsFromProgram(const ProgramExecutable &executable)
{
    mOutputVariables          = executable.getOutputVariables();
    mOutputLocations          = executable.getOutputLocations();
    mSecondaryOutputLocations = executable.getSecondaryOutputLocations();
}

void ProgramExecutable::copyUniformsFromProgramMap(
    const ShaderMap<SharedProgramExecutable> &executables)
{
    // Merge default uniforms.
    auto getDefaultRange = [](const ProgramExecutable &state) {
        return state.getDefaultUniformRange();
    };
    mPod.defaultUniformRange = AddUniforms(executables, mPod.linkedShaderStages, &mUniforms,
                                           &mUniformNames, &mUniformMappedNames, getDefaultRange);

    // Merge sampler uniforms.
    auto getSamplerRange = [](const ProgramExecutable &state) {
        return state.getSamplerUniformRange();
    };
    mPod.samplerUniformRange = AddUniforms(executables, mPod.linkedShaderStages, &mUniforms,
                                           &mUniformNames, &mUniformMappedNames, getSamplerRange);

    // Merge image uniforms.
    auto getImageRange = [](const ProgramExecutable &state) {
        return state.getImageUniformRange();
    };
    mPod.imageUniformRange = AddUniforms(executables, mPod.linkedShaderStages, &mUniforms,
                                         &mUniformNames, &mUniformMappedNames, getImageRange);

    // Merge atomic counter uniforms.
    auto getAtomicRange = [](const ProgramExecutable &state) {
        return state.getAtomicCounterUniformRange();
    };
    mPod.atomicCounterUniformRange =
        AddUniforms(executables, mPod.linkedShaderStages, &mUniforms, &mUniformNames,
                    &mUniformMappedNames, getAtomicRange);

    // Note: uniforms are set through the program, and the program pipeline never needs it.
    ASSERT(mUniformLocations.empty());
}

void ProgramExecutable::getResourceName(const std::string name,
                                        GLsizei bufSize,
                                        GLsizei *length,
                                        GLchar *dest) const
{
    if (length)
    {
        *length = 0;
    }

    if (bufSize > 0)
    {
        CopyStringToBuffer(dest, name, bufSize, length);
    }
}

GLuint ProgramExecutable::getInputResourceIndex(const GLchar *name) const
{
    const std::string nameString = StripLastArrayIndex(name);

    for (size_t index = 0; index < mProgramInputs.size(); index++)
    {
        if (mProgramInputs[index].name == nameString)
        {
            return static_cast<GLuint>(index);
        }
    }

    return GL_INVALID_INDEX;
}

GLuint ProgramExecutable::getInputResourceMaxNameSize() const
{
    GLint max = 0;

    for (const ProgramInput &resource : mProgramInputs)
    {
        max = GetResourceMaxNameSize(resource, max);
    }

    return max;
}

GLuint ProgramExecutable::getOutputResourceMaxNameSize() const
{
    GLint max = 0;

    for (const gl::ProgramOutput &resource : mOutputVariables)
    {
        max = GetResourceMaxNameSize(resource, max);
    }

    return max;
}

GLuint ProgramExecutable::getInputResourceLocation(const GLchar *name) const
{
    const GLuint index = getInputResourceIndex(name);
    if (index == GL_INVALID_INDEX)
    {
        return index;
    }

    const ProgramInput &variable = getInputResource(index);

    return GetResourceLocation(name, variable, variable.getLocation());
}

GLuint ProgramExecutable::getOutputResourceLocation(const GLchar *name) const
{
    const GLuint index = getOutputResourceIndex(name);
    if (index == GL_INVALID_INDEX)
    {
        return index;
    }

    const gl::ProgramOutput &variable = getOutputResource(index);

    return GetResourceLocation(name, variable, variable.pod.location);
}

GLuint ProgramExecutable::getOutputResourceIndex(const GLchar *name) const
{
    const std::string nameString = StripLastArrayIndex(name);

    for (size_t index = 0; index < mOutputVariables.size(); index++)
    {
        if (mOutputVariables[index].name == nameString)
        {
            return static_cast<GLuint>(index);
        }
    }

    return GL_INVALID_INDEX;
}

void ProgramExecutable::getInputResourceName(GLuint index,
                                             GLsizei bufSize,
                                             GLsizei *length,
                                             GLchar *name) const
{
    getResourceName(getInputResourceName(index), bufSize, length, name);
}

void ProgramExecutable::getOutputResourceName(GLuint index,
                                              GLsizei bufSize,
                                              GLsizei *length,
                                              GLchar *name) const
{
    getResourceName(getOutputResourceName(index), bufSize, length, name);
}

void ProgramExecutable::getUniformResourceName(GLuint index,
                                               GLsizei bufSize,
                                               GLsizei *length,
                                               GLchar *name) const
{
    getResourceName(getUniformNameByIndex(index), bufSize, length, name);
}

void ProgramExecutable::getBufferVariableResourceName(GLuint index,
                                                      GLsizei bufSize,
                                                      GLsizei *length,
                                                      GLchar *name) const
{
    ASSERT(index < mBufferVariables.size());
    getResourceName(mBufferVariables[index].name, bufSize, length, name);
}

const std::string ProgramExecutable::getInputResourceName(GLuint index) const
{
    return GetResourceName(getInputResource(index));
}

const std::string ProgramExecutable::getOutputResourceName(GLuint index) const
{
    return GetResourceName(getOutputResource(index));
}

GLint ProgramExecutable::getFragDataLocation(const std::string &name) const
{
    const GLint primaryLocation = GetVariableLocation(mOutputVariables, mOutputLocations, name);
    if (primaryLocation != -1)
    {
        return primaryLocation;
    }
    return GetVariableLocation(mOutputVariables, mSecondaryOutputLocations, name);
}

GLint ProgramExecutable::getFragDataIndex(const std::string &name) const
{
    if (GetVariableLocation(mOutputVariables, mOutputLocations, name) != -1)
    {
        return 0;
    }
    if (GetVariableLocation(mOutputVariables, mSecondaryOutputLocations, name) != -1)
    {
        return 1;
    }
    return -1;
}

GLsizei ProgramExecutable::getTransformFeedbackVaryingMaxLength() const
{
    GLsizei maxSize = 0;
    for (const TransformFeedbackVarying &var : mLinkedTransformFeedbackVaryings)
    {
        maxSize = std::max(maxSize, static_cast<GLsizei>(var.nameWithArrayIndex().length() + 1));
    }

    return maxSize;
}

GLuint ProgramExecutable::getTransformFeedbackVaryingResourceIndex(const GLchar *name) const
{
    for (GLuint tfIndex = 0; tfIndex < mLinkedTransformFeedbackVaryings.size(); ++tfIndex)
    {
        if (mLinkedTransformFeedbackVaryings[tfIndex].nameWithArrayIndex() == name)
        {
            return tfIndex;
        }
    }
    return GL_INVALID_INDEX;
}

const TransformFeedbackVarying &ProgramExecutable::getTransformFeedbackVaryingResource(
    GLuint index) const
{
    ASSERT(index < mLinkedTransformFeedbackVaryings.size());
    return mLinkedTransformFeedbackVaryings[index];
}

void ProgramExecutable::getTransformFeedbackVarying(GLuint index,
                                                    GLsizei bufSize,
                                                    GLsizei *length,
                                                    GLsizei *size,
                                                    GLenum *type,
                                                    GLchar *name) const
{
    if (mLinkedTransformFeedbackVaryings.empty())
    {
        // Program is not successfully linked
        return;
    }

    ASSERT(index < mLinkedTransformFeedbackVaryings.size());
    const auto &var     = mLinkedTransformFeedbackVaryings[index];
    std::string varName = var.nameWithArrayIndex();
    GLsizei lastNameIdx = std::min(bufSize - 1, static_cast<GLsizei>(varName.length()));
    if (length)
    {
        *length = lastNameIdx;
    }
    if (size)
    {
        *size = var.size();
    }
    if (type)
    {
        *type = var.type;
    }
    if (name)
    {
        memcpy(name, varName.c_str(), lastNameIdx);
        name[lastNameIdx] = '\0';
    }
}

void ProgramExecutable::getActiveAttribute(GLuint index,
                                           GLsizei bufsize,
                                           GLsizei *length,
                                           GLint *size,
                                           GLenum *type,
                                           GLchar *name) const
{
    if (mProgramInputs.empty())
    {
        // Program is not successfully linked
        if (bufsize > 0)
        {
            name[0] = '\0';
        }

        if (length)
        {
            *length = 0;
        }

        *type = GL_NONE;
        *size = 1;
        return;
    }

    ASSERT(index < mProgramInputs.size());
    const ProgramInput &attrib = mProgramInputs[index];

    if (bufsize > 0)
    {
        CopyStringToBuffer(name, attrib.name, bufsize, length);
    }

    // Always a single 'type' instance
    *size = 1;
    *type = attrib.getType();
}

GLint ProgramExecutable::getActiveAttributeMaxLength() const
{
    size_t maxLength = 0;

    for (const ProgramInput &attrib : mProgramInputs)
    {
        maxLength = std::max(attrib.name.length() + 1, maxLength);
    }

    return static_cast<GLint>(maxLength);
}

GLuint ProgramExecutable::getAttributeLocation(const std::string &name) const
{
    for (const ProgramInput &attribute : mProgramInputs)
    {
        if (attribute.name == name)
        {
            return attribute.getLocation();
        }
    }

    return static_cast<GLuint>(-1);
}

void ProgramExecutable::getActiveUniform(GLuint index,
                                         GLsizei bufsize,
                                         GLsizei *length,
                                         GLint *size,
                                         GLenum *type,
                                         GLchar *name) const
{
    if (mUniforms.empty())
    {
        // Program is not successfully linked
        if (bufsize > 0)
        {
            name[0] = '\0';
        }

        if (length)
        {
            *length = 0;
        }

        *size = 0;
        *type = GL_NONE;
    }

    ASSERT(index < mUniforms.size());
    const LinkedUniform &uniform = mUniforms[index];

    if (bufsize > 0)
    {
        const std::string &string = getUniformNameByIndex(index);
        CopyStringToBuffer(name, string, bufsize, length);
    }

    *size = clampCast<GLint>(uniform.getBasicTypeElementCount());
    *type = uniform.getType();
}

GLint ProgramExecutable::getActiveUniformMaxLength() const
{
    size_t maxLength = 0;

    for (GLuint index = 0; index < static_cast<size_t>(mUniformNames.size()); index++)
    {
        const std::string &uniformName = getUniformNameByIndex(index);
        if (!uniformName.empty())
        {
            size_t length = uniformName.length() + 1u;
            if (getUniformByIndex(index).isArray())
            {
                length += 3;  // Counting in "[0]".
            }
            maxLength = std::max(length, maxLength);
        }
    }

    return static_cast<GLint>(maxLength);
}

bool ProgramExecutable::isValidUniformLocation(UniformLocation location) const
{
    ASSERT(angle::IsValueInRangeForNumericType<GLint>(mUniformLocations.size()));
    return location.value >= 0 && static_cast<size_t>(location.value) < mUniformLocations.size() &&
           mUniformLocations[location.value].used();
}

const LinkedUniform &ProgramExecutable::getUniformByLocation(UniformLocation location) const
{
    ASSERT(location.value >= 0 && static_cast<size_t>(location.value) < mUniformLocations.size());
    return mUniforms[getUniformIndexFromLocation(location)];
}

const VariableLocation &ProgramExecutable::getUniformLocation(UniformLocation location) const
{
    ASSERT(location.value >= 0 && static_cast<size_t>(location.value) < mUniformLocations.size());
    return mUniformLocations[location.value];
}

UniformLocation ProgramExecutable::getUniformLocation(const std::string &name) const
{
    return {GetUniformLocation(mUniforms, mUniformNames, mUniformLocations, name)};
}

GLuint ProgramExecutable::getUniformIndex(const std::string &name) const
{
    return getUniformIndexFromName(name);
}

bool ProgramExecutable::shouldIgnoreUniform(UniformLocation location) const
{
    // Casting to size_t will convert negative values to large positive avoiding double check.
    // Adding ERR() log to report out of bound location harms performance on Android.
    return ANGLE_UNLIKELY(static_cast<size_t>(location.value) >= mUniformLocations.size() ||
                          mUniformLocations[location.value].ignored);
}

GLuint ProgramExecutable::getUniformIndexFromName(const std::string &name) const
{
    return GetUniformIndexFromName(mUniforms, mUniformNames, name);
}

GLuint ProgramExecutable::getBufferVariableIndexFromName(const std::string &name) const
{
    return GetResourceIndexFromName(mBufferVariables, name);
}

GLuint ProgramExecutable::getUniformIndexFromLocation(UniformLocation location) const
{
    ASSERT(location.value >= 0 && static_cast<size_t>(location.value) < mUniformLocations.size());
    return mUniformLocations[location.value].index;
}

Optional<GLuint> ProgramExecutable::getSamplerIndex(UniformLocation location) const
{
    GLuint index = getUniformIndexFromLocation(location);
    if (!isSamplerUniformIndex(index))
    {
        return Optional<GLuint>::Invalid();
    }

    return getSamplerIndexFromUniformIndex(index);
}

bool ProgramExecutable::isSamplerUniformIndex(GLuint index) const
{
    return mPod.samplerUniformRange.contains(index);
}

GLuint ProgramExecutable::getSamplerIndexFromUniformIndex(GLuint uniformIndex) const
{
    ASSERT(isSamplerUniformIndex(uniformIndex));
    return uniformIndex - mPod.samplerUniformRange.low();
}

bool ProgramExecutable::isImageUniformIndex(GLuint index) const
{
    return mPod.imageUniformRange.contains(index);
}

GLuint ProgramExecutable::getImageIndexFromUniformIndex(GLuint uniformIndex) const
{
    ASSERT(isImageUniformIndex(uniformIndex));
    return uniformIndex - mPod.imageUniformRange.low();
}

void ProgramExecutable::getActiveUniformBlockName(const Context *context,
                                                  const UniformBlockIndex blockIndex,
                                                  GLsizei bufSize,
                                                  GLsizei *length,
                                                  GLchar *blockName) const
{
    GetInterfaceBlockName(blockIndex, mUniformBlocks, bufSize, length, blockName);
}

void ProgramExecutable::getActiveShaderStorageBlockName(const GLuint blockIndex,
                                                        GLsizei bufSize,
                                                        GLsizei *length,
                                                        GLchar *blockName) const
{
    GetInterfaceBlockName({blockIndex}, mShaderStorageBlocks, bufSize, length, blockName);
}

GLint ProgramExecutable::getActiveUniformBlockMaxNameLength() const
{
    return GetActiveInterfaceBlockMaxNameLength(mUniformBlocks);
}

GLint ProgramExecutable::getActiveShaderStorageBlockMaxNameLength() const
{
    return GetActiveInterfaceBlockMaxNameLength(mShaderStorageBlocks);
}

GLuint ProgramExecutable::getUniformBlockIndex(const std::string &name) const
{
    return GetInterfaceBlockIndex(mUniformBlocks, name);
}

GLuint ProgramExecutable::getShaderStorageBlockIndex(const std::string &name) const
{
    return GetInterfaceBlockIndex(mShaderStorageBlocks, name);
}

GLuint ProgramExecutable::getSamplerUniformBinding(const VariableLocation &uniformLocation) const
{
    GLuint samplerIndex                  = getSamplerIndexFromUniformIndex(uniformLocation.index);
    const SamplerBinding &samplerBinding = mSamplerBindings[samplerIndex];
    if (uniformLocation.arrayIndex >= samplerBinding.textureUnitsCount)
    {
        return 0;
    }

    const std::vector<GLuint> &boundTextureUnits = mSamplerBoundTextureUnits;
    return samplerBinding.getTextureUnit(boundTextureUnits, uniformLocation.arrayIndex);
}

GLuint ProgramExecutable::getImageUniformBinding(const VariableLocation &uniformLocation) const
{
    GLuint imageIndex = getImageIndexFromUniformIndex(uniformLocation.index);

    const std::vector<GLuint> &boundImageUnits = mImageBindings[imageIndex].boundImageUnits;
    return boundImageUnits[uniformLocation.arrayIndex];
}

template <typename UniformT,
          GLint UniformSize,
          void (rx::ProgramExecutableImpl::*SetUniformFunc)(GLint, GLsizei, const UniformT *)>
void ProgramExecutable::setUniformGeneric(UniformLocation location,
                                          GLsizei count,
                                          const UniformT *v)
{
    if (shouldIgnoreUniform(location))
    {
        return;
    }

    const VariableLocation &locationInfo = mUniformLocations[location.value];
    GLsizei clampedCount                 = clampUniformCount(locationInfo, count, UniformSize, v);
    (mImplementation->*SetUniformFunc)(location.value, clampedCount, v);
}

void ProgramExecutable::setUniform1fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    setUniformGeneric<GLfloat, 1, &rx::ProgramExecutableImpl::setUniform1fv>(location, count, v);
}

void ProgramExecutable::setUniform2fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    setUniformGeneric<GLfloat, 2, &rx::ProgramExecutableImpl::setUniform2fv>(location, count, v);
}

void ProgramExecutable::setUniform3fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    setUniformGeneric<GLfloat, 3, &rx::ProgramExecutableImpl::setUniform3fv>(location, count, v);
}

void ProgramExecutable::setUniform4fv(UniformLocation location, GLsizei count, const GLfloat *v)
{
    setUniformGeneric<GLfloat, 4, &rx::ProgramExecutableImpl::setUniform4fv>(location, count, v);
}

void ProgramExecutable::setUniform1iv(Context *context,
                                      UniformLocation location,
                                      GLsizei count,
                                      const GLint *v)
{
    if (shouldIgnoreUniform(location))
    {
        return;
    }

    const VariableLocation &locationInfo = mUniformLocations[location.value];
    GLsizei clampedCount                 = clampUniformCount(locationInfo, count, 1, v);

    mImplementation->setUniform1iv(location.value, clampedCount, v);

    if (isSamplerUniformIndex(locationInfo.index))
    {
        updateSamplerUniform(context, locationInfo, clampedCount, v);
    }
}

void ProgramExecutable::setUniform2iv(UniformLocation location, GLsizei count, const GLint *v)
{
    setUniformGeneric<GLint, 2, &rx::ProgramExecutableImpl::setUniform2iv>(location, count, v);
}

void ProgramExecutable::setUniform3iv(UniformLocation location, GLsizei count, const GLint *v)
{
    setUniformGeneric<GLint, 3, &rx::ProgramExecutableImpl::setUniform3iv>(location, count, v);
}

void ProgramExecutable::setUniform4iv(UniformLocation location, GLsizei count, const GLint *v)
{
    setUniformGeneric<GLint, 4, &rx::ProgramExecutableImpl::setUniform4iv>(location, count, v);
}

void ProgramExecutable::setUniform1uiv(UniformLocation location, GLsizei count, const GLuint *v)
{
    setUniformGeneric<GLuint, 1, &rx::ProgramExecutableImpl::setUniform1uiv>(location, count, v);
}

void ProgramExecutable::setUniform2uiv(UniformLocation location, GLsizei count, const GLuint *v)
{
    setUniformGeneric<GLuint, 2, &rx::ProgramExecutableImpl::setUniform2uiv>(location, count, v);
}

void ProgramExecutable::setUniform3uiv(UniformLocation location, GLsizei count, const GLuint *v)
{
    setUniformGeneric<GLuint, 3, &rx::ProgramExecutableImpl::setUniform3uiv>(location, count, v);
}

void ProgramExecutable::setUniform4uiv(UniformLocation location, GLsizei count, const GLuint *v)
{
    setUniformGeneric<GLuint, 4, &rx::ProgramExecutableImpl::setUniform4uiv>(location, count, v);
}

template <typename UniformT,
          GLint MatrixC,
          GLint MatrixR,
          void (rx::ProgramExecutableImpl::*
                    SetUniformMatrixFunc)(GLint, GLsizei, GLboolean, const UniformT *)>
void ProgramExecutable::setUniformMatrixGeneric(UniformLocation location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const UniformT *v)
{
    if (shouldIgnoreUniform(location))
    {
        return;
    }

    GLsizei clampedCount = clampMatrixUniformCount<MatrixC, MatrixR>(location, count, transpose, v);
    (mImplementation->*SetUniformMatrixFunc)(location.value, clampedCount, transpose, v);
}

void ProgramExecutable::setUniformMatrix2fv(UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 2, 2, &rx::ProgramExecutableImpl::setUniformMatrix2fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix3fv(UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 3, 3, &rx::ProgramExecutableImpl::setUniformMatrix3fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix4fv(UniformLocation location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 4, 4, &rx::ProgramExecutableImpl::setUniformMatrix4fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix2x3fv(UniformLocation location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 2, 3, &rx::ProgramExecutableImpl::setUniformMatrix2x3fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix2x4fv(UniformLocation location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 2, 4, &rx::ProgramExecutableImpl::setUniformMatrix2x4fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix3x2fv(UniformLocation location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 3, 2, &rx::ProgramExecutableImpl::setUniformMatrix3x2fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix3x4fv(UniformLocation location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 3, 4, &rx::ProgramExecutableImpl::setUniformMatrix3x4fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix4x2fv(UniformLocation location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 4, 2, &rx::ProgramExecutableImpl::setUniformMatrix4x2fv>(
        location, count, transpose, v);
}

void ProgramExecutable::setUniformMatrix4x3fv(UniformLocation location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *v)
{
    setUniformMatrixGeneric<GLfloat, 4, 3, &rx::ProgramExecutableImpl::setUniformMatrix4x3fv>(
        location, count, transpose, v);
}

void ProgramExecutable::getUniformfv(const Context *context,
                                     UniformLocation location,
                                     GLfloat *v) const
{
    const VariableLocation &uniformLocation = mUniformLocations[location.value];
    const LinkedUniform &uniform            = mUniforms[uniformLocation.index];

    if (uniform.isSampler())
    {
        *v = static_cast<GLfloat>(getSamplerUniformBinding(uniformLocation));
        return;
    }
    else if (uniform.isImage())
    {
        *v = static_cast<GLfloat>(getImageUniformBinding(uniformLocation));
        return;
    }

    const GLenum nativeType = VariableComponentType(uniform.getType());
    if (nativeType == GL_FLOAT)
    {
        mImplementation->getUniformfv(context, location.value, v);
    }
    else
    {
        getUniformInternal(context, v, location, nativeType,
                           VariableComponentCount(uniform.getType()));
    }
}

void ProgramExecutable::getUniformiv(const Context *context,
                                     UniformLocation location,
                                     GLint *v) const
{
    const VariableLocation &uniformLocation = mUniformLocations[location.value];
    const LinkedUniform &uniform            = mUniforms[uniformLocation.index];

    if (uniform.isSampler())
    {
        *v = static_cast<GLint>(getSamplerUniformBinding(uniformLocation));
        return;
    }
    else if (uniform.isImage())
    {
        *v = static_cast<GLint>(getImageUniformBinding(uniformLocation));
        return;
    }

    const GLenum nativeType = VariableComponentType(uniform.getType());
    if (nativeType == GL_INT || nativeType == GL_BOOL)
    {
        mImplementation->getUniformiv(context, location.value, v);
    }
    else
    {
        getUniformInternal(context, v, location, nativeType,
                           VariableComponentCount(uniform.getType()));
    }
}

void ProgramExecutable::getUniformuiv(const Context *context,
                                      UniformLocation location,
                                      GLuint *v) const
{
    const VariableLocation &uniformLocation = mUniformLocations[location.value];
    const LinkedUniform &uniform            = mUniforms[uniformLocation.index];

    if (uniform.isSampler())
    {
        *v = getSamplerUniformBinding(uniformLocation);
        return;
    }
    else if (uniform.isImage())
    {
        *v = getImageUniformBinding(uniformLocation);
        return;
    }

    const GLenum nativeType = VariableComponentType(uniform.getType());
    if (nativeType == GL_UNSIGNED_INT)
    {
        mImplementation->getUniformuiv(context, location.value, v);
    }
    else
    {
        getUniformInternal(context, v, location, nativeType,
                           VariableComponentCount(uniform.getType()));
    }
}

void ProgramExecutable::initInterfaceBlockBindings()
{
    // Set initial bindings from shader.
    for (size_t blockIndex = 0; blockIndex < mUniformBlocks.size(); blockIndex++)
    {
        InterfaceBlock &uniformBlock = mUniformBlocks[blockIndex];
        // All interface blocks either have |binding| defined, or default to binding 0.
        ASSERT(uniformBlock.pod.inShaderBinding >= 0);
        remapUniformBlockBinding({static_cast<uint32_t>(blockIndex)},
                                 uniformBlock.pod.inShaderBinding);

        // This is called on program link/binary, which means the executable has changed.  There is
        // no need to send any additional notifications to the contexts (where the program may be
        // current) or program pipeline objects (that have this program attached), because they
        // already assume all blocks are dirty.
    }
}

void ProgramExecutable::remapUniformBlockBinding(UniformBlockIndex uniformBlockIndex,
                                                 GLuint uniformBlockBinding)
{
    // Remove previous binding
    const GLuint previousBinding = mUniformBlockIndexToBufferBinding[uniformBlockIndex.value];
    mUniformBufferBindingToUniformBlocks[previousBinding].reset(uniformBlockIndex.value);

    // Set new binding
    mUniformBlockIndexToBufferBinding[uniformBlockIndex.value] = uniformBlockBinding;
    mUniformBufferBindingToUniformBlocks[uniformBlockBinding].set(uniformBlockIndex.value);
}

void ProgramExecutable::setUniformValuesFromBindingQualifiers()
{
    for (unsigned int samplerIndex : mPod.samplerUniformRange)
    {
        const auto &samplerUniform = mUniforms[samplerIndex];
        if (samplerUniform.getBinding() != -1)
        {
            const std::string &uniformName = getUniformNameByIndex(samplerIndex);
            UniformLocation location       = getUniformLocation(uniformName);
            ASSERT(location.value != -1);
            std::vector<GLint> boundTextureUnits;
            for (unsigned int elementIndex = 0;
                 elementIndex < samplerUniform.getBasicTypeElementCount(); ++elementIndex)
            {
                boundTextureUnits.push_back(samplerUniform.getBinding() + elementIndex);
            }

            // Here we pass nullptr to avoid a large chain of calls that need a non-const Context.
            // We know it's safe not to notify the Context because this is only called after link.
            setUniform1iv(nullptr, location, static_cast<GLsizei>(boundTextureUnits.size()),
                          boundTextureUnits.data());
        }
    }
}

template <typename T>
GLsizei ProgramExecutable::clampUniformCount(const VariableLocation &locationInfo,
                                             GLsizei count,
                                             int vectorSize,
                                             const T *v)
{
    if (count == 1)
        return 1;

    const LinkedUniform &linkedUniform = mUniforms[locationInfo.index];

    // OpenGL ES 3.0.4 spec pg 67: "Values for any array element that exceeds the highest array
    // element index used, as reported by GetActiveUniform, will be ignored by the GL."
    unsigned int remainingElements =
        linkedUniform.getBasicTypeElementCount() - locationInfo.arrayIndex;
    GLsizei maxElementCount =
        static_cast<GLsizei>(remainingElements * linkedUniform.getElementComponents());

    if (count * vectorSize > maxElementCount)
    {
        return maxElementCount / vectorSize;
    }

    return count;
}

template <size_t cols, size_t rows, typename T>
GLsizei ProgramExecutable::clampMatrixUniformCount(UniformLocation location,
                                                   GLsizei count,
                                                   GLboolean transpose,
                                                   const T *v)
{
    const VariableLocation &locationInfo = mUniformLocations[location.value];

    if (!transpose)
    {
        return clampUniformCount(locationInfo, count, cols * rows, v);
    }

    const LinkedUniform &linkedUniform = mUniforms[locationInfo.index];

    // OpenGL ES 3.0.4 spec pg 67: "Values for any array element that exceeds the highest array
    // element index used, as reported by GetActiveUniform, will be ignored by the GL."
    unsigned int remainingElements =
        linkedUniform.getBasicTypeElementCount() - locationInfo.arrayIndex;
    return std::min(count, static_cast<GLsizei>(remainingElements));
}

void ProgramExecutable::updateSamplerUniform(Context *context,
                                             const VariableLocation &locationInfo,
                                             GLsizei clampedCount,
                                             const GLint *v)
{
    ASSERT(isSamplerUniformIndex(locationInfo.index));
    GLuint samplerIndex                    = getSamplerIndexFromUniformIndex(locationInfo.index);
    const SamplerBinding &samplerBinding   = mSamplerBindings[samplerIndex];
    std::vector<GLuint> &boundTextureUnits = mSamplerBoundTextureUnits;

    if (locationInfo.arrayIndex >= samplerBinding.textureUnitsCount)
    {
        return;
    }
    GLsizei safeUniformCount =
        std::min(clampedCount,
                 static_cast<GLsizei>(samplerBinding.textureUnitsCount - locationInfo.arrayIndex));

    // Update the sampler uniforms.
    for (uint16_t arrayIndex = 0; arrayIndex < safeUniformCount; ++arrayIndex)
    {
        GLint oldTextureUnit =
            samplerBinding.getTextureUnit(boundTextureUnits, arrayIndex + locationInfo.arrayIndex);
        GLint newTextureUnit = v[arrayIndex];

        if (oldTextureUnit == newTextureUnit)
        {
            continue;
        }

        // Update sampler's bound textureUnit
        boundTextureUnits[samplerBinding.textureUnitsStartIndex + arrayIndex +
                          locationInfo.arrayIndex] = newTextureUnit;

        // Update the reference counts.
        uint32_t &oldRefCount = mActiveSamplerRefCounts[oldTextureUnit];
        uint32_t &newRefCount = mActiveSamplerRefCounts[newTextureUnit];
        ASSERT(oldRefCount > 0);
        ASSERT(newRefCount < std::numeric_limits<uint32_t>::max());
        oldRefCount--;
        newRefCount++;

        // Check for binding type change.
        TextureType newSamplerType     = mActiveSamplerTypes[newTextureUnit];
        TextureType oldSamplerType     = mActiveSamplerTypes[oldTextureUnit];
        SamplerFormat newSamplerFormat = mActiveSamplerFormats[newTextureUnit];
        SamplerFormat oldSamplerFormat = mActiveSamplerFormats[oldTextureUnit];
        bool newSamplerYUV             = mActiveSamplerYUV.test(newTextureUnit);

        if (newRefCount == 1)
        {
            setActive(newTextureUnit, samplerBinding, mUniforms[locationInfo.index]);
        }
        else
        {
            if (newSamplerType != samplerBinding.textureType ||
                newSamplerYUV != IsSamplerYUVType(samplerBinding.samplerType))
            {
                hasSamplerTypeConflict(newTextureUnit);
            }

            if (newSamplerFormat != samplerBinding.format)
            {
                hasSamplerFormatConflict(newTextureUnit);
            }
        }

        // Unset previously active sampler.
        if (oldRefCount == 0)
        {
            setInactive(oldTextureUnit);
        }
        else
        {
            if (oldSamplerType == TextureType::InvalidEnum ||
                oldSamplerFormat == SamplerFormat::InvalidEnum)
            {
                // Previous conflict. Check if this new change fixed the conflict.
                setSamplerUniformTextureTypeAndFormat(oldTextureUnit);
            }
        }

        // Update the observing PPO's executable, if any.
        // Do this before any of the Context work, since that uses the current ProgramExecutable,
        // which will be the PPO's if this Program is bound to it, rather than this Program's.
        if (mPod.isSeparable)
        {
            onStateChange(angle::SubjectMessage::ProgramTextureOrImageBindingChanged);
        }

        // Notify context.
        if (context)
        {
            context->onSamplerUniformChange(newTextureUnit);
            context->onSamplerUniformChange(oldTextureUnit);
        }
    }

    // Invalidate the validation cache.
    resetCachedValidateSamplersResult();
    // Inform any PPOs this Program may be bound to.
    onStateChange(angle::SubjectMessage::SamplerUniformsUpdated);
}

// Driver differences mean that doing the uniform value cast ourselves gives consistent results.
// EG: on NVIDIA drivers, it was observed that getUniformi for MAX_INT+1 returned MIN_INT.
template <typename DestT>
void ProgramExecutable::getUniformInternal(const Context *context,
                                           DestT *dataOut,
                                           UniformLocation location,
                                           GLenum nativeType,
                                           int components) const
{
    switch (nativeType)
    {
        case GL_BOOL:
        {
            GLint tempValue[16] = {0};
            mImplementation->getUniformiv(context, location.value, tempValue);
            UniformStateQueryCastLoop<GLboolean>(
                dataOut, reinterpret_cast<const uint8_t *>(tempValue), components);
            break;
        }
        case GL_INT:
        {
            GLint tempValue[16] = {0};
            mImplementation->getUniformiv(context, location.value, tempValue);
            UniformStateQueryCastLoop<GLint>(dataOut, reinterpret_cast<const uint8_t *>(tempValue),
                                             components);
            break;
        }
        case GL_UNSIGNED_INT:
        {
            GLuint tempValue[16] = {0};
            mImplementation->getUniformuiv(context, location.value, tempValue);
            UniformStateQueryCastLoop<GLuint>(dataOut, reinterpret_cast<const uint8_t *>(tempValue),
                                              components);
            break;
        }
        case GL_FLOAT:
        {
            GLfloat tempValue[16] = {0};
            mImplementation->getUniformfv(context, location.value, tempValue);
            UniformStateQueryCastLoop<GLfloat>(
                dataOut, reinterpret_cast<const uint8_t *>(tempValue), components);
            break;
        }
        default:
            UNREACHABLE();
            break;
    }
}

void ProgramExecutable::setDrawIDUniform(GLint drawid)
{
    ASSERT(hasDrawIDUniform());
    mImplementation->setUniform1iv(mPod.drawIDLocation, 1, &drawid);
}

void ProgramExecutable::setBaseVertexUniform(GLint baseVertex)
{
    ASSERT(hasBaseVertexUniform());
    if (baseVertex == mCachedBaseVertex)
    {
        return;
    }
    mCachedBaseVertex = baseVertex;
    mImplementation->setUniform1iv(mPod.baseVertexLocation, 1, &baseVertex);
}

void ProgramExecutable::setBaseInstanceUniform(GLuint baseInstance)
{
    ASSERT(hasBaseInstanceUniform());
    if (baseInstance == mCachedBaseInstance)
    {
        return;
    }
    mCachedBaseInstance   = baseInstance;
    GLint baseInstanceInt = baseInstance;
    mImplementation->setUniform1iv(mPod.baseInstanceLocation, 1, &baseInstanceInt);
}

void ProgramExecutable::waitForPostLinkTasks(const Context *context)
{
    if (mPostLinkSubTasks.empty())
    {
        return;
    }

    mImplementation->waitForPostLinkTasks(context);

    // Implementation is expected to call |onPostLinkTasksComplete|.
    ASSERT(mPostLinkSubTasks.empty());
}

void InstallExecutable(const Context *context,
                       const SharedProgramExecutable &toInstall,
                       SharedProgramExecutable *executable)
{
    // There should never be a need to re-install the same executable.
    ASSERT(toInstall.get() != executable->get());

    // Destroy the old executable before it gets deleted.
    UninstallExecutable(context, executable);

    // Install the new executable.
    *executable = toInstall;
}

void UninstallExecutable(const Context *context, SharedProgramExecutable *executable)
{
    if (executable->use_count() == 1)
    {
        (*executable)->destroy(context);
    }

    executable->reset();
}

}  // namespace gl
