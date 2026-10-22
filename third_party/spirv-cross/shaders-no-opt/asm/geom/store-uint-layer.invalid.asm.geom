; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 74
; Schema: 0
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %stream_pos %stream_layer %input_pos
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 3
               OpSource HLSL 500
               OpName %main "main"
               OpName %VertexOutput "VertexOutput"
               OpMemberName %VertexOutput 0 "pos"
               OpName %GeometryOutput "GeometryOutput"
               OpMemberName %GeometryOutput 0 "pos"
               OpMemberName %GeometryOutput 1 "layer"
               OpName %_main_struct_VertexOutput_vf41_3__struct_GeometryOutput_vf4_u11_ "@main(struct-VertexOutput-vf41[3];struct-GeometryOutput-vf4-u11;"
               OpName %input "input"
               OpName %stream "stream"
               OpName %output "output"
               OpName %v "v"
               OpName %stream_pos "stream.pos"
               OpName %stream_layer "stream.layer"
               OpName %input_0 "input"
               OpName %input_pos "input.pos"
               OpName %stream_0 "stream"
               OpName %param "param"
               OpName %param_0 "param"
               OpDecorate %stream_pos BuiltIn Position
               OpDecorate %stream_layer BuiltIn Layer
               OpDecorate %input_pos BuiltIn Position
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%VertexOutput = OpTypeStruct %v4float
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_VertexOutput_uint_3 = OpTypeArray %VertexOutput %uint_3
%_ptr_Function__arr_VertexOutput_uint_3 = OpTypePointer Function %_arr_VertexOutput_uint_3
%GeometryOutput = OpTypeStruct %v4float %uint
%_ptr_Function_GeometryOutput = OpTypePointer Function %GeometryOutput
         %15 = OpTypeFunction %void %_ptr_Function__arr_VertexOutput_uint_3 %_ptr_Function_GeometryOutput
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_3 = OpConstant %int 3
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
 %stream_pos = OpVariable %_ptr_Output_v4float Output
%_ptr_Output_uint = OpTypePointer Output %uint
%stream_layer = OpVariable %_ptr_Output_uint Output
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Input__arr_v4float_uint_3 = OpTypePointer Input %_arr_v4float_uint_3
  %input_pos = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
      %int_2 = OpConstant %int 2
       %main = OpFunction %void None %3
          %5 = OpLabel
    %input_0 = OpVariable %_ptr_Function__arr_VertexOutput_uint_3 Function
   %stream_0 = OpVariable %_ptr_Function_GeometryOutput Function
      %param = OpVariable %_ptr_Function__arr_VertexOutput_uint_3 Function
    %param_0 = OpVariable %_ptr_Function_GeometryOutput Function
         %58 = OpAccessChain %_ptr_Input_v4float %input_pos %int_0
         %59 = OpLoad %v4float %58
         %60 = OpAccessChain %_ptr_Function_v4float %input_0 %int_0 %int_0
               OpStore %60 %59
         %61 = OpAccessChain %_ptr_Input_v4float %input_pos %int_1
         %62 = OpLoad %v4float %61
         %63 = OpAccessChain %_ptr_Function_v4float %input_0 %int_1 %int_0
               OpStore %63 %62
         %65 = OpAccessChain %_ptr_Input_v4float %input_pos %int_2
         %66 = OpLoad %v4float %65
         %67 = OpAccessChain %_ptr_Function_v4float %input_0 %int_2 %int_0
               OpStore %67 %66
         %70 = OpLoad %_arr_VertexOutput_uint_3 %input_0
               OpStore %param %70
         %72 = OpFunctionCall %void %_main_struct_VertexOutput_vf41_3__struct_GeometryOutput_vf4_u11_ %param %param_0
         %73 = OpLoad %GeometryOutput %param_0
               OpStore %stream_0 %73
               OpReturn
               OpFunctionEnd
%_main_struct_VertexOutput_vf41_3__struct_GeometryOutput_vf4_u11_ = OpFunction %void None %15
      %input = OpFunctionParameter %_ptr_Function__arr_VertexOutput_uint_3
     %stream = OpFunctionParameter %_ptr_Function_GeometryOutput
         %19 = OpLabel
     %output = OpVariable %_ptr_Function_GeometryOutput Function
          %v = OpVariable %_ptr_Function_int Function
         %25 = OpAccessChain %_ptr_Function_uint %output %int_1
               OpStore %25 %uint_1
               OpStore %v %int_0
               OpBranch %29
         %29 = OpLabel
               OpLoopMerge %31 %32 None
               OpBranch %33
         %33 = OpLabel
         %34 = OpLoad %int %v
         %37 = OpSLessThan %bool %34 %int_3
               OpBranchConditional %37 %30 %31
         %30 = OpLabel
         %38 = OpLoad %int %v
         %40 = OpAccessChain %_ptr_Function_v4float %input %38 %int_0
         %41 = OpLoad %v4float %40
         %42 = OpAccessChain %_ptr_Function_v4float %output %int_0
               OpStore %42 %41
         %45 = OpAccessChain %_ptr_Function_v4float %output %int_0
         %46 = OpLoad %v4float %45
               OpStore %stream_pos %46
         %49 = OpAccessChain %_ptr_Function_uint %output %int_1
         %50 = OpLoad %uint %49
               OpStore %stream_layer %50
               OpEmitVertex
               OpBranch %32
         %32 = OpLabel
         %51 = OpLoad %int %v
         %52 = OpIAdd %int %51 %int_1
               OpStore %v %52
               OpBranch %29
         %31 = OpLabel
               OpEndPrimitive
               OpReturn
               OpFunctionEnd
