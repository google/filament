; SPIR-V
; Version: 1.3
; Generator: Khronos SPIR-V Tools Assembler; 0
; Bound: 54
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %gl_FragCoord %o_color
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpName %main "main"
               OpName %myType "myType"
               OpMemberName %myType 0 "data"
               OpName %myData "myData"
               OpName %uv "uv"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %index "index"
               OpName %elt "elt"
               OpName %o_color "o_color"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %o_color Location 0
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
      %float = OpTypeFloat 32
     %myType = OpTypeStruct %float
       %uint = OpTypeInt 32 0
     %uint_5 = OpConstant %uint 5
%_arr_myType_uint_5 = OpTypeArray %myType %uint_5
%_ptr_Private__arr_myType_uint_5 = OpTypePointer Private %_arr_myType_uint_5
     %myData = OpVariable %_ptr_Private__arr_myType_uint_5 Private
    %float_0 = OpConstant %float 0
         %18 = OpConstantComposite %myType %float_0
    %float_1 = OpConstant %float 1
         %20 = OpConstantComposite %myType %float_1
         %21 = OpConstantComposite %_arr_myType_uint_5 %18 %20 %18 %20 %18
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
     %uint_0 = OpConstant %uint 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_4 = OpConstant %float 4
%_ptr_Function_myType = OpTypePointer Function %myType
%_ptr_Private_myType = OpTypePointer Private %myType
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %o_color = OpVariable %_ptr_Output_v4float Output
         %36 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
         %37 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
       %main = OpFunction %void None %11
         %38 = OpLabel
         %uv = OpVariable %_ptr_Function_v2float Function
      %index = OpVariable %_ptr_Function_int Function
        %elt = OpVariable %_ptr_Function_myType Function
               OpStore %myData %21
         %39 = OpLoad %v4float %gl_FragCoord
         %40 = OpVectorShuffle %v2float %39 %39 0 1
               OpStore %uv %40
         %41 = OpAccessChain %_ptr_Function_float %uv %uint_0
         %42 = OpLoad %float %41
         %43 = OpFMod %float %42 %float_4
         %44 = OpConvertFToS %int %43
               OpStore %index %44
         %45 = OpLoad %int %index
         %46 = OpAccessChain %_ptr_Private_myType %myData %45
         %47 = OpLoad %myType %46
               OpStore %elt %47
         %48 = OpAccessChain %_ptr_Function_float %elt %int_0
         %49 = OpLoad %float %48
         %50 = OpFOrdGreaterThan %bool %49 %float_0
               OpSelectionMerge %51 None
               OpBranchConditional %50 %52 %53
         %52 = OpLabel
               OpStore %o_color %36
               OpBranch %51
         %53 = OpLabel
               OpStore %o_color %37
               OpBranch %51
         %51 = OpLabel
               OpReturn
               OpFunctionEnd
