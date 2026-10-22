; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 55
; Schema: 0
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %OUT_pos %positions_1
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 3
               OpSource HLSL 500
               OpName %main "main"
               OpName %SceneOut "SceneOut"
               OpMemberName %SceneOut 0 "pos"
               OpName %_main_vf4_3__struct_SceneOut_vf41_ "@main(vf4[3];struct-SceneOut-vf41;"
               OpName %positions "positions"
               OpName %OUT "OUT"
               OpName %i "i"
               OpName %o "o"
               OpName %OUT_pos "OUT.pos"
               OpName %positions_0 "positions"
               OpName %positions_1 "positions"
               OpName %OUT_0 "OUT"
               OpName %param "param"
               OpName %param_0 "param"
               OpDecorate %OUT_pos BuiltIn Position
               OpDecorate %positions_1 BuiltIn Position
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Function__arr_v4float_uint_3 = OpTypePointer Function %_arr_v4float_uint_3
   %SceneOut = OpTypeStruct %v4float
%_ptr_Function_SceneOut = OpTypePointer Function %SceneOut
         %14 = OpTypeFunction %void %_ptr_Function__arr_v4float_uint_3 %_ptr_Function_SceneOut
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %OUT_pos = OpVariable %_ptr_Output_v4float Output
      %int_1 = OpConstant %int 1
%_ptr_Input__arr_v4float_uint_3 = OpTypePointer Input %_arr_v4float_uint_3
%positions_1 = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
       %main = OpFunction %void None %3
          %5 = OpLabel
%positions_0 = OpVariable %_ptr_Function__arr_v4float_uint_3 Function
      %OUT_0 = OpVariable %_ptr_Function_SceneOut Function
      %param = OpVariable %_ptr_Function__arr_v4float_uint_3 Function
    %param_0 = OpVariable %_ptr_Function_SceneOut Function
         %48 = OpLoad %_arr_v4float_uint_3 %positions_1
               OpStore %positions_0 %48
         %51 = OpLoad %_arr_v4float_uint_3 %positions_0
               OpStore %param %51
         %53 = OpFunctionCall %void %_main_vf4_3__struct_SceneOut_vf41_ %param %param_0
         %54 = OpLoad %SceneOut %param_0
               OpStore %OUT_0 %54
               OpReturn
               OpFunctionEnd
%_main_vf4_3__struct_SceneOut_vf41_ = OpFunction %void None %14
  %positions = OpFunctionParameter %_ptr_Function__arr_v4float_uint_3
        %OUT = OpFunctionParameter %_ptr_Function_SceneOut
         %18 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
          %o = OpVariable %_ptr_Function_SceneOut Function
               OpStore %i %int_0
               OpBranch %23
         %23 = OpLabel
               OpLoopMerge %25 %26 None
               OpBranch %27
         %27 = OpLabel
         %28 = OpLoad %int %i
         %31 = OpSLessThan %bool %28 %int_3
               OpBranchConditional %31 %24 %25
         %24 = OpLabel
         %33 = OpLoad %int %i
         %35 = OpAccessChain %_ptr_Function_v4float %positions %33
         %36 = OpLoad %v4float %35
         %37 = OpAccessChain %_ptr_Function_v4float %o %int_0
               OpStore %37 %36
         %40 = OpAccessChain %_ptr_Function_v4float %o %int_0
         %41 = OpLoad %v4float %40
               OpStore %OUT_pos %41
               OpEmitVertex
               OpBranch %26
         %26 = OpLabel
         %42 = OpLoad %int %i
         %44 = OpIAdd %int %42 %int_1
               OpStore %i %44
               OpBranch %23
         %25 = OpLabel
               OpEndPrimitive
               OpReturn
               OpFunctionEnd
