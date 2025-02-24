//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_REWRITE_BUILTIN_VARIABLES_H_
#define COMPILER_REWRITE_BUILTIN_VARIABLES_H_

#include <variant>

#include "compiler/translator/Common.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolUniqueId.h"
#include "compiler/translator/wgsl/Utils.h"

namespace sh
{

// In WGSL, all input values are parameters to the shader's main function and all output values are
// return values of the shader's main functions (the input/output values can be embedded within
// struct types). So this rewrites all accesses of GLSL's input/output variables (including
// builtins) to be accesses of a global struct, and writes a new main function (called wgslMain())
// that populates the global input struct with parameters of the wgslMain function, and populates
// wgslMain's return value with the fields of the global output struct.
//
// TODO(anglebug.com/42267100): some of these WGSL builtins do not correspond exactly to GLSL
// builtin values and will need modification at the beginning (resp. end) of the main function for
// input values (resp. output values). E.g. normalized device coordinates in GLSL have -1.0 <= z
// <= 1.0, whereas NDC in WGSL have 0.0 <= z <= 1.0.
// See e.g. bool TranslatorMSL::appendVertexShaderDepthCorrectionToMain().
//
// Example GLSL:
//
// attribute vec2 xy_position;
// void setPosition() {
//   gl_Position = vec4(xy_position.x, xy_position.y, 0.0, 1.0);
// }
// void main()
// {
//   setPosition();
// }
//
// The resulting WGSL:
// struct ANGLE_Input_Global {
//   xy_position : vec2<f32>,
// };
//
// var<private> ANGLE_input_global : ANGLE_Input_Global;
//
// struct ANGLE_Input_Annotated {
//   @location(@@@@@@) xy_position : vec2<f32>,
// };
//
// struct ANGLE_Output_Global {
//   gl_Position_ : vec4<f32>,
// };
//
// var<private> ANGLE_output_global : ANGLE_Output_Global;
//
// struct ANGLE_Output_Annotated {
//   @builtin(position) gl_Position_ : vec4<f32>,
// };
//
// // Generated versions of _umain() and _usetPosition() go here.
//
// @vertex
// fn wgslMain(ANGLE_input_annotated : ANGLE_Input_Annotated) -> ANGLE_Output_Annotated
// {
//   ANGLE_input_global.xy_position = ANGLE_input_annotated.xy_position;
//   _umain();
//   var ANGLE_output_annotated : ANGLE_Output_Annotated;
//   ANGLE_output_annotated.gl_Position_ = ANGLE_output_global.gl_Position_;
//   return ANGLE_output_annotated;
// }
//
// Note the WGSL outputter should not output any declarations of global in/out variables, nor any
// redeclarations of builtin variables. And all accesses to global in/out variables should be
// rewritten as struct accesses of the global structs.

const char kBuiltinInputStructType[]           = "ANGLE_Input_Global";
const char kBuiltinOutputStructType[]          = "ANGLE_Output_Global";
const char kBuiltinInputAnnotatedStructType[]  = "ANGLE_Input_Annotated";
const char kBuiltinOutputAnnotatedStructType[] = "ANGLE_Output_Annotated";
const char kBuiltinInputStructName[]           = "ANGLE_input_global";
const char kBuiltinOutputStructName[]          = "ANGLE_output_global";
const char kBuiltinInputAnnotatedStructName[]  = "ANGLE_input_annotated";
const char kBuiltinOutputAnnotatedStructName[] = "ANGLE_output_annotated";

class RewritePipelineVarOutputBuilder;

struct RewritePipelineVarOutput
{
  public:
    RewritePipelineVarOutput(sh::GLenum shaderType);

    // Every time the translator goes to output a TVariable/TSymbol it checks these functions to see
    // if it should generate a struct access instead.
    bool IsInputVar(TSymbolUniqueId angleInputVar) const;
    bool IsOutputVar(TSymbolUniqueId angleOutputVar) const;

    bool OutputStructs(TInfoSinkBase &output);
    bool OutputMainFunction(TInfoSinkBase &output);

  private:
    friend RewritePipelineVarOutputBuilder;

    // The key is TSymbolUniqueId::get().
    using RewrittenVarSet = TUnorderedSet<int>;

    struct WgslIOBlock
    {
        TVector<ImmutableString> angleGlobalMembers;
        TVector<ImmutableString> angleAnnotatedMembers;
        TVector<ImmutableString> angleConversionFuncs;
    };

    static bool OutputIOStruct(TInfoSinkBase &output,
                               WgslIOBlock &block,
                               ImmutableString builtinStructType,
                               ImmutableString builtinStructName,
                               ImmutableString builtinAnnotatedStructType);

    // Represents the input and output structs for the WGSL main function.
    WgslIOBlock mInputBlock;
    WgslIOBlock mOutputBlock;

    // Tracks all symbols (attributes, output vars, builtins) that need to be rewritten into struct
    // accesses by the WGSL outputter. Used in `IsInputVar()` and `IsOutputVar()`.
    RewrittenVarSet mAngleInputVars;
    RewrittenVarSet mAngleOutputVars;

    sh::GLenum mShaderType;
};

// `outVarReplacements` is a RewritePipelineVarOutput that can output the WGSL main function and the
// input/output structs that represent the input and output variables of the GLSL shader,
[[nodiscard]] bool GenerateMainFunctionAndIOStructs(TCompiler &compiler,
                                                    TIntermBlock &root,
                                                    RewritePipelineVarOutput &outVarReplacements);

}  // namespace sh

#endif  // COMPILER_REWRITE_BUILTIN_VARIABLES_H_
