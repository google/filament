//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/wgsl/RewritePipelineVariables.h"

#include <string>
#include <utility>

#include "GLES2/gl2.h"
#include "GLSLANG/ShaderLang.h"
#include "GLSLANG/ShaderVars.h"
#include "anglebase/no_destructor.h"
#include "common/angleutils.h"
#include "common/log_utils.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/OutputTree.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolUniqueId.h"
#include "compiler/translator/Types.h"
#include "compiler/translator/tree_util/BuiltIn_autogen.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/util.h"
#include "compiler/translator/wgsl/Utils.h"

namespace sh
{

namespace
{

const bool kOutputVariableUses = false;

struct LocationAnnotation
{
    // Most variables will not be assigned a location until link time, but some variables (like
    // gl_FragColor) imply an output location.
    int location = -1;
};
struct BuiltinAnnotation
{
    ImmutableString wgslBuiltinName;
};
struct NoAnnotation
{};

using PipelineAnnotation = std::variant<LocationAnnotation, BuiltinAnnotation, NoAnnotation>;

enum class IOType
{
    Input,
    Output
};

struct GlslToWgslBuiltinMapping
{
    ImmutableString glslBuiltinName{nullptr};
    PipelineAnnotation wgslPipelineAnnotation;
    IOType ioType;
    const TVariable *builtinVar;
    // The type from the WGSL spec that corresponds to `wgslPipelineAnnotation`.
    ImmutableString wgslBuiltinType{nullptr};
    // The type that is expected by the shader in the AST, i.e. the type of `builtinVar`. If
    // nullptr, is the same as `wgslBuiltinType`.
    // TODO(anglebug.com/42267100): delete this and convert `builtinVar`'s type to a WGSL type.
    ImmutableString wgslTypeExpectedByShader{nullptr};
    // A function to apply that does one of two thing:
    //   1. for an input builtin: converts the builtin, as supplied by WGPU, into the variable that
    //   the GLSL shader expects.
    //   2. for an output builtin: converts the output variable from the GLSL shader into the
    //   builtin supplied back to WGPU.
    // Can be nullptr for no conversion.
    ImmutableString conversionFunc{nullptr};
};

bool GetWgslBuiltinName(std::string glslBuiltinName,
                        GLenum shaderType,
                        GlslToWgslBuiltinMapping *outMapping)
{
    static const angle::base::NoDestructor<angle::HashMap<std::string, GlslToWgslBuiltinMapping>>
        kGlslBuiltinToWgslBuiltinVertex(
            {{"gl_VertexID",
              GlslToWgslBuiltinMapping{ImmutableString("gl_VertexID"),
                                       BuiltinAnnotation{ImmutableString("vertex_index")},
                                       IOType::Input, BuiltInVariable::gl_VertexID(),
                                       ImmutableString("u32"), ImmutableString("i32"),
                                       ImmutableString("i32")}},
             {"gl_InstanceID",
              GlslToWgslBuiltinMapping{ImmutableString("gl_InstanceID"),
                                       BuiltinAnnotation{ImmutableString("instance_index")},
                                       IOType::Input, BuiltInVariable::gl_InstanceID(),
                                       ImmutableString("u32"), ImmutableString("i32"),
                                       ImmutableString("i32")}},
             {"gl_Position",
              GlslToWgslBuiltinMapping{
                  ImmutableString("gl_Position"), BuiltinAnnotation{ImmutableString("position")},
                  IOType::Output, BuiltInVariable::gl_Position(), ImmutableString("vec4<f32>"),
                  ImmutableString(nullptr), ImmutableString(nullptr)}},
             {"gl_PointSize",
              GlslToWgslBuiltinMapping{ImmutableString("gl_PointSize"), NoAnnotation{},
                                       IOType::Output, BuiltInVariable::gl_PointSize(),
                                       ImmutableString("f32"), ImmutableString(nullptr),
                                       ImmutableString(nullptr)}},
             // TODO(anglebug.com/42267100): might have to emulate clip_distances, see
             // Metal's
             // https://source.chromium.org/chromium/chromium/src/+/main:third_party/angle/src/compiler/translator/msl/TranslatorMSL.cpp?q=symbol%3A%5Cbsh%3A%3AEmulateClipDistanceVaryings%5Cb%20case%3Ayes
             {"gl_ClipDistance",
              GlslToWgslBuiltinMapping{ImmutableString("gl_ClipDistance"),
                                       BuiltinAnnotation{ImmutableString("clip_distances")},
                                       IOType::Output, nullptr, ImmutableString("TODO"),
                                       ImmutableString(nullptr), ImmutableString(nullptr)}}});
    static const angle::base::NoDestructor<angle::HashMap<std::string, GlslToWgslBuiltinMapping>>
        kGlslBuiltinToWgslBuiltinFragment({
            {"gl_FragCoord",
             GlslToWgslBuiltinMapping{ImmutableString("gl_FragCoord"),
                                      BuiltinAnnotation{ImmutableString("position")}, IOType::Input,
                                      BuiltInVariable::gl_FragCoord(), ImmutableString("vec4<f32>"),
                                      ImmutableString(nullptr), ImmutableString(nullptr)}},
            {"gl_FrontFacing",
             GlslToWgslBuiltinMapping{ImmutableString("gl_FrontFacing"),
                                      BuiltinAnnotation{ImmutableString("front_facing")},
                                      IOType::Input, BuiltInVariable::gl_FrontFacing(),
                                      ImmutableString("bool"), ImmutableString(nullptr),
                                      ImmutableString(nullptr)}},
            {"gl_SampleID",
             GlslToWgslBuiltinMapping{
                 ImmutableString("gl_SampleID"), BuiltinAnnotation{ImmutableString("sample_index")},
                 IOType::Input, BuiltInVariable::gl_SampleID(), ImmutableString("u32"),
                 ImmutableString("i32"), ImmutableString("i32")}},
            // TODO(anglebug.com/42267100): gl_SampleMask is GLSL 4.00 or ARB_sample_shading and
            // requires some special handling (see Metal).
            {"gl_SampleMaskIn",
             GlslToWgslBuiltinMapping{ImmutableString("gl_SampleMaskIn"),
                                      BuiltinAnnotation{ImmutableString("sample_mask")},
                                      IOType::Input, nullptr, ImmutableString("u32"),
                                      ImmutableString("i32"), ImmutableString("i32")}},
            // Just translate FragColor into a location = 0 out variable.
            // TODO(anglebug.com/42267100): maybe ASSERT that there are no user-defined output
            // variables? Is it possible for there to be other output variables when using
            // FragColor?
            {"gl_FragColor",
             GlslToWgslBuiltinMapping{ImmutableString("gl_FragColor"), LocationAnnotation{0},
                                      IOType::Output, BuiltInVariable::gl_FragColor(),
                                      ImmutableString("vec4<f32>"), ImmutableString(nullptr),
                                      ImmutableString(nullptr)}},
            {"gl_SampleMask",
             GlslToWgslBuiltinMapping{ImmutableString("gl_SampleMask"),
                                      BuiltinAnnotation{ImmutableString("sample_mask")},
                                      IOType::Output, nullptr, ImmutableString("u32"),
                                      ImmutableString("i32"), ImmutableString("i32")}},
            {"gl_FragDepth",
             GlslToWgslBuiltinMapping{
                 ImmutableString("gl_FragDepth"), BuiltinAnnotation{ImmutableString("frag_depth")},
                 IOType::Output, BuiltInVariable::gl_FragDepth(), ImmutableString("f32"),
                 ImmutableString(nullptr), ImmutableString(nullptr)}},
        });
    // TODO(anglebug.com/42267100): gl_FragData needs to be emulated. Need something
    // like spir-v's
    // third_party/angle/src/compiler/translator/tree_ops/spirv/EmulateFragColorData.h.

    if (shaderType == GL_VERTEX_SHADER)
    {
        auto it = kGlslBuiltinToWgslBuiltinVertex->find(glslBuiltinName);
        if (it == kGlslBuiltinToWgslBuiltinVertex->end())
        {
            return false;
        }
        *outMapping = it->second;
        return true;
    }
    else if (shaderType == GL_FRAGMENT_SHADER)
    {
        auto it = kGlslBuiltinToWgslBuiltinFragment->find(glslBuiltinName);
        if (it == kGlslBuiltinToWgslBuiltinFragment->end())
        {
            return false;
        }
        *outMapping = it->second;
        return true;
    }
    else
    {
        UNREACHABLE();
        return false;
    }
}

ImmutableString CreateNameToReplaceBuiltin(ImmutableString glslBuiltinName)
{
    ImmutableStringBuilder newName(glslBuiltinName.length() + 1);
    newName << glslBuiltinName << '_';
    return newName;
}

}  // namespace

// Friended by RewritePipelineVarOutput
class RewritePipelineVarOutputBuilder
{
  public:
    static bool GenerateMainFunctionAndIOStructs(TCompiler &compiler,
                                                 TIntermBlock &root,
                                                 RewritePipelineVarOutput &outVarReplacements);

  private:
    static bool GeneratePipelineStructStrings(
        RewritePipelineVarOutput::WgslIOBlock *ioblock,
        RewritePipelineVarOutput::RewrittenVarSet *varsToReplace,
        ImmutableString toStruct,
        ImmutableString fromStruct,
        const std::vector<ShaderVariable> &shaderVars,
        const GlobalVars &globalVars,
        TCompiler &compiler,
        IOType ioType,
        std::string debugString);
};

// Given a list of `shaderVars` (as well as `compiler` and a list of global variables in the GLSL
// source, `globalVars`), computes the fields that should appear in the input/output pipeline
// structs and the annotations that should appear in the WGSL source.
//
// `ioblock` will be filled with strings that make up the resulting structs, and with the strings
// indicated by `fromStruct` and `toStruct`. `varsToReplace` will be filled with the symbols that
// should be replaced in the final WGSL source wtih struct accesses.
//
// Finally, `debugString` should describe `shaderVars` (e.g. "input varyings"), and `ioType`
// indicates whether `shaderVars` is meant to be an input or output variable, which is useful for
// debugging asserts.
[[nodiscard]] bool RewritePipelineVarOutputBuilder::GeneratePipelineStructStrings(
    RewritePipelineVarOutput::WgslIOBlock *ioblock,
    RewritePipelineVarOutput::RewrittenVarSet *varsToReplace,
    ImmutableString toStruct,
    ImmutableString fromStruct,
    const std::vector<ShaderVariable> &shaderVars,
    const GlobalVars &globalVars,
    TCompiler &compiler,
    IOType ioType,
    std::string debugString)
{
    for (const ShaderVariable &shaderVar : shaderVars)
    {
        if (shaderVar.name == "gl_FragData" || shaderVar.name == "gl_SecondaryFragColorEXT" ||
            shaderVar.name == "gl_SecondaryFragDataEXT")
        {
            // TODO(anglebug.com/42267100): declare gl_FragData as multiple variables.
            UNIMPLEMENTED();
            return false;
        }

        if (kOutputVariableUses)
        {
            std::cout << "Use of " << (shaderVar.isBuiltIn() ? "builtin " : "") << debugString
                      << ": " << shaderVar.name << std::endl;
        }

        if (shaderVar.isBuiltIn())
        {
            GlslToWgslBuiltinMapping wgslName;
            if (!GetWgslBuiltinName(shaderVar.name, compiler.getShaderType(), &wgslName))
            {
                return false;
            }

            const TVariable *varToReplace = wgslName.builtinVar;

            if (varToReplace == nullptr)
            {
                // Should be declared somewhere as a symbol.
                // TODO(anglebug.com/42267100): Not sure if this ever actually occurs. Will this
                // TVariable also have a declaration? Are there any gl_ variable that require or
                // even allow declaration?
                varToReplace = static_cast<const TVariable *>(compiler.getSymbolTable().findBuiltIn(
                    ImmutableString(wgslName.glslBuiltinName), compiler.getShaderVersion()));
                if (kOutputVariableUses)
                {
                    std::cout
                        << "Var " << shaderVar.name
                        << " did not have a BuiltIn var but does have a builtin in the symbol "
                           "table"
                        << std::endl;
                }
            }

            ASSERT(ioType == wgslName.ioType);

            varsToReplace->insert(varToReplace->uniqueId().get());

            ImmutableString builtinReplacement =
                CreateNameToReplaceBuiltin(wgslName.glslBuiltinName);

            // E.g. `gl_VertexID_ : i32`.
            ImmutableString globalType = wgslName.wgslTypeExpectedByShader.empty()
                                             ? wgslName.wgslBuiltinType
                                             : wgslName.wgslTypeExpectedByShader;
            ImmutableString globalStructVar =
                BuildConcatenatedImmutableString(builtinReplacement, " : ", globalType, ",");
            ioblock->angleGlobalMembers.push_back(globalStructVar);

            if (auto *builtinAnnotation =
                    std::get_if<BuiltinAnnotation>(&wgslName.wgslPipelineAnnotation))
            {
                // E.g. `@builtin(vertex_index) gl_VertexID_ : u32,`.
                const char *builtinAnnotationStart = "@builtin(";
                const char *builtinAnnotationEnd   = ") ";
                ImmutableString annotatedStructVar = BuildConcatenatedImmutableString(
                    builtinAnnotationStart, builtinAnnotation->wgslBuiltinName,
                    builtinAnnotationEnd, builtinReplacement, " : ", wgslName.wgslBuiltinType, ",");
                ioblock->angleAnnotatedMembers.push_back(annotatedStructVar);
            }
            else if (auto *locationAnnotation =
                         std::get_if<LocationAnnotation>(&wgslName.wgslPipelineAnnotation))
            {
                ASSERT(locationAnnotation->location == 0);
                // E.g. `@location(0) gl_FragColor_ : vec4<f32>,`.
                const char *locationAnnotationStr = "@location(0) ";
                ImmutableString annotatedStructVar =
                    BuildConcatenatedImmutableString(locationAnnotationStr, builtinReplacement,
                                                     " : ", wgslName.wgslBuiltinType, ",");
                ioblock->angleAnnotatedMembers.push_back(annotatedStructVar);
            }
            else
            {
                ASSERT(std::get_if<NoAnnotation>(&wgslName.wgslPipelineAnnotation));
            }

            if (!std::get_if<NoAnnotation>(&wgslName.wgslPipelineAnnotation))
            {
                // E.g. `ANGLE_input_global.gl_VertexID_ = u32(ANGLE_input_annotated.gl_VertexID_);`
                ImmutableString conversion(nullptr);
                if (wgslName.conversionFunc.empty())
                {
                    conversion =
                        BuildConcatenatedImmutableString(toStruct, ".", builtinReplacement, " = ",
                                                         fromStruct, ".", builtinReplacement, ";");
                }
                else
                {
                    conversion = BuildConcatenatedImmutableString(
                        toStruct, ".", builtinReplacement, " = ", wgslName.conversionFunc, "(",
                        fromStruct, ".", builtinReplacement, ");");
                }
                ioblock->angleConversionFuncs.push_back(conversion);
            }
        }
        else
        {
            if (!shaderVar.active)
            {
                // Skip any inactive attributes as they won't be assigned a location anyway.
                continue;
            }

            TIntermDeclaration *declNode = globalVars.find(shaderVar.name)->second;
            const TVariable *astVar      = &ViewDeclaration(*declNode).symbol.variable();

            const ImmutableString &userVarName = astVar->name();

            varsToReplace->insert(astVar->uniqueId().get());

            // E.g. `_uuserVar : i32,`.
            TStringStream typeStream;
            WriteWgslType(typeStream, astVar->getType(), {});
            TString type = typeStream.str();
            ImmutableString globalStructVar =
                BuildConcatenatedImmutableString(userVarName, " : ", type.c_str(), ",");
            ioblock->angleGlobalMembers.push_back(globalStructVar);

            // E.g. `@location(@@@@@@) _uuserVar : i32,`.
            const char *locationAnnotationStr = "@location(@@@@@@) ";
            ImmutableString annotatedStructVar =
                BuildConcatenatedImmutableString(locationAnnotationStr, globalStructVar);
            ioblock->angleAnnotatedMembers.push_back(annotatedStructVar);

            // E.g. `ANGLE_input_global._uuserVar = ANGLE_input_annotated._uuserVar;`
            ImmutableString conversion = BuildConcatenatedImmutableString(
                toStruct, ".", userVarName, " = ", fromStruct, ".", userVarName, ";");
            ioblock->angleConversionFuncs.push_back(conversion);
        }
    }

    return true;
}

bool RewritePipelineVarOutputBuilder::GenerateMainFunctionAndIOStructs(
    TCompiler &compiler,
    TIntermBlock &root,
    RewritePipelineVarOutput &outVarReplacements)
{
    GlobalVars globalVars = FindGlobalVars(&root);

    if (!RewritePipelineVarOutputBuilder::GeneratePipelineStructStrings(
            &outVarReplacements.mInputBlock, &outVarReplacements.mAngleInputVars,
            /*toStruct=*/ImmutableString(kBuiltinInputStructName),
            /*fromStruct=*/ImmutableString(kBuiltinInputAnnotatedStructName),
            compiler.getInputVaryings(), globalVars, compiler, IOType::Input, "input varyings") ||
        !RewritePipelineVarOutputBuilder::GeneratePipelineStructStrings(
            &outVarReplacements.mInputBlock, &outVarReplacements.mAngleInputVars,
            /*toStruct=*/ImmutableString(kBuiltinInputStructName),
            /*fromStruct=*/ImmutableString(kBuiltinInputAnnotatedStructName),
            compiler.getAttributes(), globalVars, compiler, IOType::Input, "input attributes") ||
        !RewritePipelineVarOutputBuilder::GeneratePipelineStructStrings(
            &outVarReplacements.mOutputBlock, &outVarReplacements.mAngleOutputVars,
            /*toStruct=*/ImmutableString(kBuiltinOutputAnnotatedStructName),
            /*fromStruct=*/ImmutableString(kBuiltinOutputStructName), compiler.getOutputVaryings(),
            globalVars, compiler, IOType::Output, "output varyings") ||
        !RewritePipelineVarOutputBuilder::GeneratePipelineStructStrings(
            &outVarReplacements.mOutputBlock, &outVarReplacements.mAngleOutputVars,
            /*toStruct=*/ImmutableString(kBuiltinOutputAnnotatedStructName),
            /*fromStruct=*/ImmutableString(kBuiltinOutputStructName), compiler.getOutputVariables(),
            globalVars, compiler, IOType::Output, "output variables"))
    {
        return false;
    }

    return true;
}

RewritePipelineVarOutput::RewritePipelineVarOutput(sh::GLenum shaderType) : mShaderType(shaderType)
{}

bool RewritePipelineVarOutput::IsInputVar(TSymbolUniqueId angleInputVar) const
{
    return mAngleInputVars.count(angleInputVar.get()) > 0;
}
bool RewritePipelineVarOutput::IsOutputVar(TSymbolUniqueId angleOutputVar) const
{
    return mAngleOutputVars.count(angleOutputVar.get()) > 0;
}

// static
bool RewritePipelineVarOutput::OutputIOStruct(TInfoSinkBase &output,
                                              WgslIOBlock &block,
                                              ImmutableString builtinStructType,
                                              ImmutableString builtinStructName,
                                              ImmutableString builtinAnnotatedStructType)
{

    if (!block.angleGlobalMembers.empty())
    {
        // Output global struct definition.
        output << "struct " << builtinStructType << " {\n";
        for (const ImmutableString &globalMember : block.angleGlobalMembers)
        {
            output << "  " << globalMember << "\n";
        }
        output << "};\n\n";
        // Output decl of global struct.
        output << "var<private> " << builtinStructName << " : " << builtinStructType << ";\n\n";
        // Output annotated struct definition.
        output << "struct " << builtinAnnotatedStructType << " {\n";
        for (const ImmutableString &annotatedMember : block.angleAnnotatedMembers)
        {
            output << "  " << annotatedMember << "\n";
        }
        output << "};\n\n";
    }

    return true;
}

bool RewritePipelineVarOutput::OutputStructs(TInfoSinkBase &output)
{
    if (!OutputIOStruct(output, mInputBlock, ImmutableString(kBuiltinInputStructType),
                        ImmutableString(kBuiltinInputStructName),
                        ImmutableString(kBuiltinInputAnnotatedStructType)) ||
        !OutputIOStruct(output, mOutputBlock, ImmutableString(kBuiltinOutputStructType),
                        ImmutableString(kBuiltinOutputStructName),
                        ImmutableString(kBuiltinOutputAnnotatedStructType)))
    {
        return false;
    }

    return true;
}

// Could split OutputMainFunction() into the different parts of the main function.
bool RewritePipelineVarOutput::OutputMainFunction(TInfoSinkBase &output)
{
    if (mShaderType == GL_VERTEX_SHADER)
    {
        output << "@vertex\n";
    }
    else
    {
        ASSERT(mShaderType == GL_FRAGMENT_SHADER);
        output << "@fragment\n";
    }
    output << "fn wgslMain(";
    if (!mInputBlock.angleGlobalMembers.empty())
    {
        output << kBuiltinInputAnnotatedStructName << " : " << kBuiltinInputAnnotatedStructType;
    }
    output << ")";
    if (!mOutputBlock.angleGlobalMembers.empty())
    {
        output << " -> " << kBuiltinOutputAnnotatedStructType;
    }
    output << "\n{\n";
    for (const ImmutableString &conversionFunc : mInputBlock.angleConversionFuncs)
    {
        output << "  " << conversionFunc << "\n";
    }
    output << "  " << kUserDefinedNamePrefix << "main()" << ";\n";

    if (!mOutputBlock.angleGlobalMembers.empty())
    {
        output << "  var " << kBuiltinOutputAnnotatedStructName << " : "
               << kBuiltinOutputAnnotatedStructType << ";\n";
        for (const ImmutableString &conversionFunc : mOutputBlock.angleConversionFuncs)
        {
            output << "  " << conversionFunc << "\n";
        }
        output << "  return " << kBuiltinOutputAnnotatedStructName << ";\n";
    }
    output << "}\n";
    return true;
}

bool GenerateMainFunctionAndIOStructs(TCompiler &compiler,
                                      TIntermBlock &root,
                                      RewritePipelineVarOutput &outVarReplacements)
{
    return RewritePipelineVarOutputBuilder::GenerateMainFunctionAndIOStructs(compiler, root,
                                                                             outVarReplacements);
}
}  // namespace sh
