//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_msl_utils.h: Utilities to manipulate MSL.
//

#import <Foundation/Foundation.h>

#include <variant>
#include "common/string_utils.h"
#include "common/utilities.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/msl/TranslatorMSL.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/ShaderMtl.h"
#include "libANGLE/renderer/metal/mtl_msl_utils.h"

namespace rx
{
namespace
{
constexpr char kXfbBindingsMarker[]     = "@@XFB-Bindings@@";
constexpr char kXfbOutMarker[]          = "ANGLE_@@XFB-OUT@@";
constexpr char kUserDefinedNamePrefix[] = "_u";  // Defined in GLSLANG/ShaderLang.h
constexpr char kAttribBindingsMarker[]  = "@@Attrib-Bindings@@\n";

std::string GetXfbBufferNameMtl(const uint32_t bufferIndex)
{
    return "xfbBuffer" + Str(bufferIndex);
}

// Name format needs to match sh::Name.
struct UserDefinedNameExpr
{
    std::string name;
};

std::ostream &operator<<(std::ostream &stream, const UserDefinedNameExpr &expr)
{
    return stream << kUserDefinedNamePrefix << expr.name;
}

struct UserDefinedNameComponentExpr
{
    UserDefinedNameExpr name;
    const int component;
};

std::ostream &operator<<(std::ostream &stream, const UserDefinedNameComponentExpr &expr)
{
    return stream << expr.name << '[' << expr.component << ']';
}

struct InternalNameExpr
{
    std::string name;
};

std::ostream &operator<<(std::ostream &stream, const InternalNameExpr &expr)
{
    return stream << sh::kAngleInternalPrefix << '_' << expr.name;
}

struct InternalNameComponentExpr
{
    InternalNameExpr name;
    const int component;
};

std::ostream &operator<<(std::ostream &stream, const InternalNameComponentExpr &expr)
{
    return stream << expr.name << '_' << expr.component;
}

// ModifyStructs phase forwarded a single-component user-defined name or created a new AngleInternal
// field name to support multi-component fields as multiple single-component fields.
std::variant<UserDefinedNameExpr, InternalNameComponentExpr>
ResolveModifiedAttributeName(const std::string &name, int registerIndex, int registerCount)
{
    if (registerCount < 2)
    {
        return UserDefinedNameExpr{name};
    }
    return InternalNameComponentExpr{InternalNameExpr{name}, registerIndex};
}

std::variant<UserDefinedNameExpr, InternalNameComponentExpr>
ResolveModifiedOutputName(const std::string &name, int component, int componentCount)
{
    if (componentCount == 0)
    {
        return UserDefinedNameExpr{name};
    }
    return InternalNameComponentExpr{InternalNameExpr{name}, component};
}

// Accessing unmodified structs uses user-defined name, business as usual.
std::variant<UserDefinedNameExpr, UserDefinedNameComponentExpr>
ResolveUserDefinedName(const std::string &name, int component, int componentCount)
{
    if (componentCount == 0)
    {
        return UserDefinedNameExpr{name};
    }
    return UserDefinedNameComponentExpr{{name}, component};
}

template <class T>
struct ApplyOStream
{
    const T &value;
};

template <class T>
ApplyOStream(T) -> ApplyOStream<T>;

template <class T>
std::ostream &operator<<(std::ostream &stream, ApplyOStream<T> sv)
{
    stream << sv.value;
    return stream;
}

template <class... Ts>
std::ostream &operator<<(std::ostream &stream, ApplyOStream<std::variant<Ts...>> sv)
{
    std::visit([&stream](auto &&v) { stream << ApplyOStream{v}; }, sv.value);
    return stream;
}

}  // namespace

namespace mtl
{

void TranslatedShaderInfo::reset()
{
    metalShaderSource    = nullptr;
    metalLibrary         = nil;
    hasUBOArgumentBuffer = false;
    hasIsnanOrIsinf      = false;
    hasInvariant         = false;
    for (mtl::SamplerBinding &binding : actualSamplerBindings)
    {
        binding.textureBinding = mtl::kMaxShaderSamplers;
        binding.samplerBinding = 0;
    }
    for (int &rwTextureBinding : actualImageBindings)
    {
        rwTextureBinding = -1;
    }
    for (uint32_t &binding : actualUBOBindings)
    {
        binding = mtl::kMaxShaderBuffers;
    }

    for (uint32_t &binding : actualXFBBindings)
    {
        binding = mtl::kMaxShaderBuffers;
    }
}

// Original mapping of front end from sampler name to multiple sampler slots (in form of
// slot:count pair)
using OriginalSamplerBindingMap =
    std::unordered_map<std::string, std::vector<std::pair<uint32_t, uint32_t>>>;

bool MappedSamplerNameNeedsUserDefinedPrefix(const std::string &originalName)
{
    return originalName.find('.') == std::string::npos;
}

static std::string MSLGetMappedSamplerName(const std::string &originalName)
{
    std::string samplerName = originalName;

    // Samplers in structs are extracted.
    std::replace(samplerName.begin(), samplerName.end(), '.', '_');

    // Remove array elements
    auto out = samplerName.begin();
    for (auto in = samplerName.begin(); in != samplerName.end(); in++)
    {
        if (*in == '[')
        {
            while (*in != ']')
            {
                in++;
                ASSERT(in != samplerName.end());
            }
        }
        else
        {
            *out++ = *in;
        }
    }

    samplerName.erase(out, samplerName.end());

    if (MappedSamplerNameNeedsUserDefinedPrefix(originalName))
    {
        samplerName = sh::kUserDefinedNamePrefix + samplerName;
    }

    return samplerName;
}

void MSLGetShaderSource(const gl::ProgramState &programState,
                        const gl::ProgramLinkedResources &resources,
                        gl::ShaderMap<std::string> *shaderSourcesOut)
{
    for (const gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::SharedCompiledShaderState &glShader = programState.getAttachedShader(shaderType);
        (*shaderSourcesOut)[shaderType]               = glShader ? glShader->translatedSource : "";
    }
}

void GetAssignedSamplerBindings(const sh::TranslatorMetalReflection *reflection,
                                const OriginalSamplerBindingMap &originalBindings,
                                std::unordered_set<std::string> &structSamplers,
                                std::array<SamplerBinding, mtl::kMaxGLSamplerBindings> *bindings)
{
    for (auto &sampler : reflection->getSamplerBindings())
    {
        const std::string &name          = sampler.first;
        const uint32_t actualSamplerSlot = (uint32_t)reflection->getSamplerBinding(name);
        const uint32_t actualTextureSlot = (uint32_t)reflection->getTextureBinding(name);

        // Assign sequential index for subsequent array elements
        const bool structSampler = structSamplers.find(name) != structSamplers.end();
        const std::string mappedName =
            structSampler ? name : MSLGetMappedSamplerName(sh::kUserDefinedNamePrefix + name);
        auto original = originalBindings.find(mappedName);
        if (original != originalBindings.end())
        {
            const std::vector<std::pair<uint32_t, uint32_t>> &resOrignalBindings =
                originalBindings.at(mappedName);
            uint32_t currentTextureSlot = actualTextureSlot;
            uint32_t currentSamplerSlot = actualSamplerSlot;
            for (const std::pair<uint32_t, uint32_t> &originalBindingRange : resOrignalBindings)
            {
                SamplerBinding &actualBinding = bindings->at(originalBindingRange.first);
                actualBinding.textureBinding  = currentTextureSlot;
                actualBinding.samplerBinding  = currentSamplerSlot;

                currentTextureSlot += originalBindingRange.second;
                currentSamplerSlot += originalBindingRange.second;
            }
        }
    }
}

std::string UpdateAliasedShaderAttributes(std::string shaderSourceIn,
                                          const gl::ProgramExecutable &executable)
{
    // Cache max number of components for each attribute location
    std::array<uint8_t, gl::MAX_VERTEX_ATTRIBS> maxComponents{};
    for (auto &attribute : executable.getProgramInputs())
    {
        const int location       = attribute.getLocation();
        const int registers      = gl::VariableRegisterCount(attribute.getType());
        const uint8_t components = gl::VariableColumnCount(attribute.getType());
        for (int i = 0; i < registers; ++i)
        {
            ASSERT(location + i < static_cast<int>(maxComponents.size()));
            maxComponents[location + i] = std::max(maxComponents[location + i], components);
        }
    }

    // Define aliased names pointing to real attributes with swizzles as needed
    std::ostringstream stream;
    for (auto &attribute : executable.getProgramInputs())
    {
        const int location       = attribute.getLocation();
        const int registers      = gl::VariableRegisterCount(attribute.getType());
        const uint8_t components = gl::VariableColumnCount(attribute.getType());
        for (int i = 0; i < registers; i++)
        {
            stream << "#define ANGLE_ALIASED_"
                   << ApplyOStream{ResolveModifiedAttributeName(attribute.name, i, registers)}
                   << " ANGLE_modified.ANGLE_ATTRIBUTE_" << (location + i);
            if (components != maxComponents[location + i])
            {
                ASSERT(components < maxComponents[location + i]);
                switch (components)
                {
                    case 1:
                        stream << ".x";
                        break;
                    case 2:
                        stream << ".xy";
                        break;
                    case 3:
                        stream << ".xyz";
                        break;
                }
            }
            stream << "\n";
        }
    }

    // Declare actual MSL attributes
    for (size_t i : executable.getActiveAttribLocationsMask())
    {
        stream << "  float";
        if (maxComponents[i] > 1)
        {
            stream << static_cast<int>(maxComponents[i]);
        }
        stream << " ANGLE_ATTRIBUTE_" << i << "[[attribute(" << i << ")]];\n";
    }

    std::string outputSource = shaderSourceIn;
    size_t markerFound       = outputSource.find(kAttribBindingsMarker);
    ASSERT(markerFound != std::string::npos);
    outputSource.replace(markerFound, angle::ConstStrLen(kAttribBindingsMarker), stream.str());
    return outputSource;
}

std::string updateShaderAttributes(std::string shaderSourceIn,
                                   const gl::ProgramExecutable &executable)
{
    // Build string to attrib map.
    const auto &programAttributes = executable.getProgramInputs();
    std::ostringstream stream;
    std::unordered_map<std::string, uint32_t> attributeBindings;
    for (auto &attribute : programAttributes)
    {
        const int registers = gl::VariableRegisterCount(attribute.getType());
        for (int i = 0; i < registers; i++)
        {
            stream.str("");
            stream << ' '
                   << ApplyOStream{ResolveModifiedAttributeName(attribute.name, i, registers)}
                   << sh::kUnassignedAttributeString;
            attributeBindings.insert({stream.str(), i + attribute.getLocation()});
        }
    }
    // Rewrite attributes
    std::string outputSource = shaderSourceIn;
    for (auto it = attributeBindings.begin(); it != attributeBindings.end(); ++it)
    {
        std::size_t attribFound = outputSource.find(it->first);
        if (attribFound != std::string::npos)
        {
            stream.str("");
            stream << "[[attribute(" << it->second << ")]]";
            outputSource = outputSource.replace(
                attribFound + it->first.length() -
                    angle::ConstStrLen(sh::kUnassignedAttributeString),
                angle::ConstStrLen(sh::kUnassignedAttributeString), stream.str());
        }
    }
    return outputSource;
}

std::string UpdateFragmentShaderOutputs(std::string shaderSourceIn,
                                        const gl::ProgramExecutable &executable,
                                        bool defineAlpha0)
{
    std::ostringstream stream;
    std::string outputSource    = shaderSourceIn;
    const auto &outputVariables = executable.getOutputVariables();

    // For alpha-to-coverage emulation, a reference to the alpha channel
    // of color output 0 is needed. For ESSL 1.00, it is gl_FragColor or
    // gl_FragData[0]; for ESSL 3.xx, it is a user-defined output.
    std::string alphaOutputName;

    auto assignLocations = [&](const std::vector<gl::VariableLocation> &locations, bool secondary) {
        for (auto &outputLocation : locations)
        {
            if (!outputLocation.used())
            {
                continue;
            }
            const int index                    = outputLocation.arrayIndex;
            const gl::ProgramOutput &outputVar = outputVariables[outputLocation.index];
            ASSERT(outputVar.pod.location >= 0);
            const int location  = outputVar.pod.location + index;
            const int arraySize = outputVar.getOutermostArraySize();
            stream.str("");
            stream << ApplyOStream{ResolveModifiedOutputName(outputVar.name, index, arraySize)}
                   << " [[" << sh::kUnassignedFragmentOutputString;
            const std::string placeholder(stream.str());

            size_t outputFound = outputSource.find(placeholder);
            if (outputFound != std::string::npos)
            {
                stream.str("");
                stream << "color(" << location << (secondary ? "), index(1)" : ")");
                outputSource = outputSource.replace(
                    outputFound + placeholder.length() -
                        angle::ConstStrLen(sh::kUnassignedFragmentOutputString),
                    angle::ConstStrLen(sh::kUnassignedFragmentOutputString), stream.str());
            }

            if (defineAlpha0 && location == 0 && !secondary && outputVar.pod.type == GL_FLOAT_VEC4)
            {
                ASSERT(index == 0);
                ASSERT(alphaOutputName.empty());
                std::ostringstream nameStream;
                nameStream << "ANGLE_fragmentOut."
                           << ApplyOStream{ResolveUserDefinedName(outputVar.name, index, arraySize)}
                           << ".a";
                alphaOutputName = nameStream.str();
            }
        }
    };
    assignLocations(executable.getOutputLocations(), false);
    assignLocations(executable.getSecondaryOutputLocations(), true);

    if (defineAlpha0)
    {
        // Locations are empty for ESSL 1.00 shaders, try built-in outputs
        if (alphaOutputName.empty())
        {
            for (auto &v : outputVariables)
            {
                if (v.name == "gl_FragColor")
                {
                    alphaOutputName = "ANGLE_fragmentOut.gl_FragColor.a";
                    break;
                }
                else if (v.name == "gl_FragData")
                {
                    alphaOutputName = "ANGLE_fragmentOut.ANGLE_gl_FragData_0.a";
                    break;
                }
            }
        }

        // Set a value used for alpha-to-coverage emulation
        const std::string alphaPlaceholder("#define ANGLE_ALPHA0");
        size_t alphaFound = outputSource.find(alphaPlaceholder);
        ASSERT(alphaFound != std::string::npos);

        std::ostringstream alphaStream;
        alphaStream << alphaPlaceholder << " ";
        alphaStream << (alphaOutputName.empty() ? "1.0" : alphaOutputName);
        outputSource =
            outputSource.replace(alphaFound, alphaPlaceholder.length(), alphaStream.str());
    }

    return outputSource;
}

std::string SubstituteTransformFeedbackMarkers(const std::string &originalSource,
                                               const std::string &xfbBindings,
                                               const std::string &xfbOut)
{
    const size_t xfbBindingsMarkerStart = originalSource.find(kXfbBindingsMarker);
    bool hasBindingsMarker              = xfbBindingsMarkerStart != std::string::npos;
    const size_t xfbBindingsMarkerEnd =
        xfbBindingsMarkerStart + angle::ConstStrLen(kXfbBindingsMarker);

    const size_t xfbOutMarkerStart = originalSource.find(kXfbOutMarker, xfbBindingsMarkerStart);
    bool hasOutMarker              = xfbOutMarkerStart != std::string::npos;
    const size_t xfbOutMarkerEnd   = xfbOutMarkerStart + angle::ConstStrLen(kXfbOutMarker);

    // The shader is the following form:
    //
    // ..part1..
    // @@ XFB-BINDINGS @@
    // ..part2..
    // @@ XFB-OUT @@;
    // ..part3..
    //
    // Construct the string by concatenating these five pieces, replacing the markers with the given
    // values.
    std::string result;
    if (hasBindingsMarker && hasOutMarker)
    {
        result.append(&originalSource[0], &originalSource[xfbBindingsMarkerStart]);
        result.append(xfbBindings);
        result.append(&originalSource[xfbBindingsMarkerEnd], &originalSource[xfbOutMarkerStart]);
        result.append(xfbOut);
        result.append(&originalSource[xfbOutMarkerEnd], &originalSource[originalSource.size()]);
        return result;
    }
    return originalSource;
}

std::string GenerateTransformFeedbackVaryingOutput(const gl::TransformFeedbackVarying &varying,
                                                   const gl::UniformTypeInfo &info,
                                                   size_t strideBytes,
                                                   size_t offset,
                                                   const std::string &bufferIndex)
{
    std::ostringstream result;

    ASSERT(strideBytes % 4 == 0);
    size_t stride = strideBytes / 4;

    const size_t arrayIndexStart = varying.arrayIndex == GL_INVALID_INDEX ? 0 : varying.arrayIndex;
    const size_t arrayIndexEnd   = arrayIndexStart + varying.size();

    for (size_t arrayIndex = arrayIndexStart; arrayIndex < arrayIndexEnd; ++arrayIndex)
    {
        for (int col = 0; col < info.columnCount; ++col)
        {
            for (int row = 0; row < info.rowCount; ++row)
            {
                result << "        ";
                result << "ANGLE_" << "xfbBuffer" << bufferIndex << "[" << "ANGLE_"
                       << std::string(sh::kUniformsVar) << ".ANGLE_xfbBufferOffsets[" << bufferIndex
                       << "] + (ANGLE_vertexIDMetal + (ANGLE_instanceIdMod - ANGLE_baseInstance) * "
                       << "ANGLE_" << std::string(sh::kUniformsVar)
                       << ".ANGLE_xfbVerticesPerInstance) * " << stride << " + " << offset
                       << "] = " << "as_type<float>" << "(" << "ANGLE_vertexOut.";
                if (!varying.isBuiltIn())
                {
                    result << kUserDefinedNamePrefix;
                }
                result << varying.name;

                if (varying.isArray())
                {
                    result << "[" << arrayIndex << "]";
                }

                if (info.columnCount > 1)
                {
                    result << "[" << col << "]";
                }

                if (info.rowCount > 1)
                {
                    result << "[" << row << "]";
                }

                result << ");\n";
                ++offset;
            }
        }
    }

    return result.str();
}

void GenerateTransformFeedbackEmulationOutputs(
    const gl::ProgramExecutable &executable,
    std::string *vertexShader,
    std::array<uint32_t, kMaxShaderXFBs> *xfbBindingRemapOut)
{
    const std::vector<gl::TransformFeedbackVarying> &varyings =
        executable.getLinkedTransformFeedbackVaryings();
    const std::vector<GLsizei> &bufferStrides = executable.getTransformFeedbackStrides();
    const bool isInterleaved =
        executable.getTransformFeedbackBufferMode() == GL_INTERLEAVED_ATTRIBS;
    const size_t bufferCount = isInterleaved ? 1 : varyings.size();

    std::vector<std::string> xfbIndices(bufferCount);

    std::string xfbBindings;

    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
    {
        const std::string xfbBinding = Str(0);
        xfbIndices[bufferIndex]      = Str(bufferIndex);

        std::string bufferName = GetXfbBufferNameMtl(bufferIndex);

        xfbBindings += ", ";
        // TODO: offset from last used buffer binding from front end
        // XFB buffer is allocated slot starting from last discrete Metal buffer slot.
        uint32_t bindingPoint               = kMaxShaderBuffers - 1 - bufferIndex;
        xfbBindingRemapOut->at(bufferIndex) = bindingPoint;
        xfbBindings +=
            "device float* ANGLE_" + bufferName + " [[buffer(" + Str(bindingPoint) + ")]]";
    }

    std::string xfbOut  = "#if TRANSFORM_FEEDBACK_ENABLED\n    {\n";
    size_t outputOffset = 0;
    for (size_t varyingIndex = 0; varyingIndex < varyings.size(); ++varyingIndex)
    {
        const size_t bufferIndex                    = isInterleaved ? 0 : varyingIndex;
        const gl::TransformFeedbackVarying &varying = varyings[varyingIndex];

        // For every varying, output to the respective buffer packed.  If interleaved, the output is
        // always to the same buffer, but at different offsets.
        const gl::UniformTypeInfo &info = gl::GetUniformTypeInfo(varying.type);
        xfbOut += GenerateTransformFeedbackVaryingOutput(varying, info, bufferStrides[bufferIndex],
                                                         outputOffset, xfbIndices[bufferIndex]);

        if (isInterleaved)
        {
            outputOffset += info.columnCount * info.rowCount * varying.size();
        }
    }
    xfbOut += "    }\n#endif\n";

    *vertexShader = SubstituteTransformFeedbackMarkers(*vertexShader, xfbBindings, xfbOut);
}

angle::Result MTLGetMSL(const angle::FeaturesMtl &features,
                        const gl::ProgramExecutable &executable,
                        const gl::ShaderMap<std::string> &shaderSources,
                        const gl::ShaderMap<SharedCompiledShaderStateMtl> &shadersState,
                        gl::ShaderMap<TranslatedShaderInfo> *mslShaderInfoOut)
{
    // Retrieve original uniform buffer bindings generated by front end. We will need to do a remap.
    std::unordered_map<std::string, uint32_t> uboOriginalBindings;
    const std::vector<gl::InterfaceBlock> &blocks = executable.getUniformBlocks();
    for (uint32_t bufferIdx = 0; bufferIdx < blocks.size(); ++bufferIdx)
    {
        const gl::InterfaceBlock &block = blocks[bufferIdx];
        if (!uboOriginalBindings.count(block.name))
        {
            uboOriginalBindings[block.name] = bufferIdx;
        }
    }
    // Retrieve original sampler bindings produced by front end.
    OriginalSamplerBindingMap originalSamplerBindings;
    const std::vector<gl::SamplerBinding> &samplerBindings = executable.getSamplerBindings();
    std::unordered_set<std::string> structSamplers         = {};

    for (uint32_t textureIndex = 0; textureIndex < samplerBindings.size(); ++textureIndex)
    {
        const gl::SamplerBinding &samplerBinding = samplerBindings[textureIndex];
        uint32_t uniformIndex          = executable.getUniformIndexFromSamplerIndex(textureIndex);
        const std::string &uniformName = executable.getUniformNames()[uniformIndex];
        const std::string &uniformMappedName = executable.getUniformMappedNames()[uniformIndex];
        bool isSamplerInStruct               = uniformName.find('.') != std::string::npos;
        std::string mappedSamplerName        = isSamplerInStruct
                                                   ? MSLGetMappedSamplerName(uniformName)
                                                   : MSLGetMappedSamplerName(uniformMappedName);
        // These need to be prefixed later seperately
        if (isSamplerInStruct)
            structSamplers.insert(mappedSamplerName);
        originalSamplerBindings[mappedSamplerName].push_back(
            {textureIndex, static_cast<uint32_t>(samplerBinding.textureUnitsCount)});
    }
    for (gl::ShaderType type : {gl::ShaderType::Vertex, gl::ShaderType::Fragment})
    {
        std::string source;
        if (type == gl::ShaderType::Vertex)
        {
            source =
                shadersState[gl::ShaderType::Vertex]->translatorMetalReflection.hasAttributeAliasing
                    ? UpdateAliasedShaderAttributes(shaderSources[type], executable)
                    : updateShaderAttributes(shaderSources[type], executable);
            // Write transform feedback output code.
            if (!source.empty())
            {
                if (executable.getLinkedTransformFeedbackVaryings().empty())
                {
                    source = SubstituteTransformFeedbackMarkers(source, "", "");
                }
                else
                {
                    GenerateTransformFeedbackEmulationOutputs(
                        executable, &source, &(*mslShaderInfoOut)[type].actualXFBBindings);
                }
            }
        }
        else
        {
            ASSERT(type == gl::ShaderType::Fragment);
            const bool defineAlpha0 = features.emulateAlphaToCoverage.enabled ||
                                      features.generateShareableShaders.enabled;
            source = UpdateFragmentShaderOutputs(shaderSources[type], executable, defineAlpha0);
        }
        (*mslShaderInfoOut)[type].metalShaderSource =
            std::make_shared<const std::string>(std::move(source));
        const sh::TranslatorMetalReflection *reflection =
            &shadersState[type]->translatorMetalReflection;
        if (reflection->hasUBOs)
        {
            (*mslShaderInfoOut)[type].hasUBOArgumentBuffer = true;

            for (auto &uboBinding : reflection->getUniformBufferBindings())
            {
                const std::string &uboName         = uboBinding.first;
                const sh::UBOBindingInfo &bindInfo = uboBinding.second;
                const uint32_t uboBindIndex        = static_cast<uint32_t>(bindInfo.bindIndex);
                const uint32_t uboArraySize        = static_cast<uint32_t>(bindInfo.arraySize);
                const uint32_t originalBinding     = uboOriginalBindings.at(uboName);
                uint32_t currentSlot               = static_cast<uint>(uboBindIndex);
                for (uint32_t i = 0; i < uboArraySize; ++i)
                {
                    // Use consecutive slot for member in array
                    (*mslShaderInfoOut)[type].actualUBOBindings[originalBinding + i] =
                        currentSlot + i;
                }
            }
        }
        // Retrieve automatic texture slot assignments
        if (originalSamplerBindings.size() > 0)
        {
            GetAssignedSamplerBindings(reflection, originalSamplerBindings, structSamplers,
                                       &mslShaderInfoOut->at(type).actualSamplerBindings);
        }
        for (uint32_t i = 0; i < kMaxShaderImages; ++i)
        {
            mslShaderInfoOut->at(type).actualImageBindings[i] = reflection->getRWTextureBinding(i);
        }
        (*mslShaderInfoOut)[type].hasIsnanOrIsinf = reflection->hasIsnanOrIsinf;
        (*mslShaderInfoOut)[type].hasInvariant    = reflection->hasInvariance;
    }
    return angle::Result::Continue;
}

uint MslGetShaderShadowCompareMode(GLenum mode, GLenum func)
{
    // See SpirvToMslCompiler::emit_header()
    if (mode == GL_NONE)
    {
        return 0;
    }
    else
    {
        switch (func)
        {
            case GL_LESS:
                return 1;
            case GL_LEQUAL:
                return 2;
            case GL_GREATER:
                return 3;
            case GL_GEQUAL:
                return 4;
            case GL_NEVER:
                return 5;
            case GL_ALWAYS:
                return 6;
            case GL_EQUAL:
                return 7;
            case GL_NOTEQUAL:
                return 8;
            default:
                UNREACHABLE();
                return 1;
        }
    }
}

}  // namespace mtl
}  // namespace rx
