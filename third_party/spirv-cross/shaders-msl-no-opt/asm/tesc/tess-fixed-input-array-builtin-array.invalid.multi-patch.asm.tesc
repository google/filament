; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 2
; Bound: 162
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %hs_main "main" %p_pos %p_1 %i_1 %_entryPointOutput_pos %_entryPointOutput %_patchConstantOutput_EdgeTess %_patchConstantOutput_InsideTess
               OpExecutionMode %hs_main OutputVertices 3
               OpExecutionMode %hs_main Triangles
               OpExecutionMode %hs_main SpacingFractionalOdd
               OpExecutionMode %hs_main VertexOrderCw
               OpSource HLSL 500
               OpName %hs_main "hs_main"
               OpName %VertexOutput "VertexOutput"
               OpMemberName %VertexOutput 0 "pos"
               OpMemberName %VertexOutput 1 "uv"
               OpName %HSOut "HSOut"
               OpMemberName %HSOut 0 "pos"
               OpMemberName %HSOut 1 "uv"
               OpName %_hs_main_struct_VertexOutput_vf4_vf21_3__u1_ "@hs_main(struct-VertexOutput-vf4-vf21[3];u1;"
               OpName %p "p"
               OpName %i "i"
               OpName %HSConstantOut "HSConstantOut"
               OpMemberName %HSConstantOut 0 "EdgeTess"
               OpMemberName %HSConstantOut 1 "InsideTess"
               OpName %PatchHS_struct_VertexOutput_vf4_vf21_3__ "PatchHS(struct-VertexOutput-vf4-vf21[3];"
               OpName %patch "patch"
               OpName %output "output"
               OpName %p_0 "p"
               OpName %p_pos "p.pos"
               OpName %VertexOutput_0 "VertexOutput"
               OpMemberName %VertexOutput_0 0 "uv"
               OpName %p_1 "p"
               OpName %i_0 "i"
               OpName %i_1 "i"
               OpName %flattenTemp "flattenTemp"
               OpName %param "param"
               OpName %param_0 "param"
               OpName %_entryPointOutput_pos "@entryPointOutput.pos"
               OpName %HSOut_0 "HSOut"
               OpMemberName %HSOut_0 0 "uv"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %_patchConstantResult "@patchConstantResult"
               OpName %param_1 "param"
               OpName %_patchConstantOutput_EdgeTess "@patchConstantOutput.EdgeTess"
               OpName %_patchConstantOutput_InsideTess "@patchConstantOutput.InsideTess"
               OpName %output_0 "output"
               OpDecorate %p_pos BuiltIn Position
               OpDecorate %p_1 Location 0
               OpDecorate %i_1 BuiltIn InvocationId
               OpDecorate %_entryPointOutput_pos BuiltIn Position
               OpDecorate %_entryPointOutput Location 0
               OpDecorate %_patchConstantOutput_EdgeTess Patch
               OpDecorate %_patchConstantOutput_EdgeTess BuiltIn TessLevelOuter
               OpDecorate %_patchConstantOutput_InsideTess Patch
               OpDecorate %_patchConstantOutput_InsideTess BuiltIn TessLevelInner
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
%VertexOutput = OpTypeStruct %v4float %v2float
       %uint = OpTypeInt 32 0
     %uint_3 = OpConstant %uint 3
%_arr_VertexOutput_uint_3 = OpTypeArray %VertexOutput %uint_3
%_ptr_Function__arr_VertexOutput_uint_3 = OpTypePointer Function %_arr_VertexOutput_uint_3
%_ptr_Function_uint = OpTypePointer Function %uint
      %HSOut = OpTypeStruct %v4float %v2float
         %16 = OpTypeFunction %HSOut %_ptr_Function__arr_VertexOutput_uint_3 %_ptr_Function_uint
%_arr_float_uint_3 = OpTypeArray %float %uint_3
%HSConstantOut = OpTypeStruct %_arr_float_uint_3 %float
         %23 = OpTypeFunction %HSConstantOut %_ptr_Function__arr_VertexOutput_uint_3
%_ptr_Function_HSOut = OpTypePointer Function %HSOut
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Input__arr_v4float_uint_3 = OpTypePointer Input %_arr_v4float_uint_3
      %p_pos = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%VertexOutput_0 = OpTypeStruct %v2float
%_arr_VertexOutput_0_uint_3 = OpTypeArray %VertexOutput_0 %uint_3
%_ptr_Input__arr_VertexOutput_0_uint_3 = OpTypePointer Input %_arr_VertexOutput_0_uint_3
        %p_1 = OpVariable %_ptr_Input__arr_VertexOutput_0_uint_3 Input
%_ptr_Input_v2float = OpTypePointer Input %v2float
      %int_2 = OpConstant %int 2
%_ptr_Input_uint = OpTypePointer Input %uint
        %i_1 = OpVariable %_ptr_Input_uint Input
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
%_entryPointOutput_pos = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
    %HSOut_0 = OpTypeStruct %v2float
%_arr_HSOut_0_uint_3 = OpTypeArray %HSOut_0 %uint_3
%_ptr_Output__arr_HSOut_0_uint_3 = OpTypePointer Output %_arr_HSOut_0_uint_3
%_entryPointOutput = OpVariable %_ptr_Output__arr_HSOut_0_uint_3 Output
%_ptr_Output_v2float = OpTypePointer Output %v2float
     %uint_2 = OpConstant %uint 2
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
       %bool = OpTypeBool
%_ptr_Function_HSConstantOut = OpTypePointer Function %HSConstantOut
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%_patchConstantOutput_EdgeTess = OpVariable %_ptr_Output__arr_float_uint_4 Output
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%_patchConstantOutput_InsideTess = OpVariable %_ptr_Output__arr_float_uint_2 Output
    %float_1 = OpConstant %float 1
    %hs_main = OpFunction %void None %3
          %5 = OpLabel
        %p_0 = OpVariable %_ptr_Function__arr_VertexOutput_uint_3 Function
        %i_0 = OpVariable %_ptr_Function_uint Function
%flattenTemp = OpVariable %_ptr_Function_HSOut Function
      %param = OpVariable %_ptr_Function__arr_VertexOutput_uint_3 Function
    %param_0 = OpVariable %_ptr_Function_uint Function
%_patchConstantResult = OpVariable %_ptr_Function_HSConstantOut Function
    %param_1 = OpVariable %_ptr_Function__arr_VertexOutput_uint_3 Function
         %50 = OpAccessChain %_ptr_Input_v4float %p_pos %int_0
         %51 = OpLoad %v4float %50
         %52 = OpAccessChain %_ptr_Function_v4float %p_0 %int_0 %int_0
               OpStore %52 %51
         %58 = OpAccessChain %_ptr_Input_v2float %p_1 %int_0 %int_0
         %59 = OpLoad %v2float %58
         %60 = OpAccessChain %_ptr_Function_v2float %p_0 %int_0 %int_1
               OpStore %60 %59
         %61 = OpAccessChain %_ptr_Input_v4float %p_pos %int_1
         %62 = OpLoad %v4float %61
         %63 = OpAccessChain %_ptr_Function_v4float %p_0 %int_1 %int_0
               OpStore %63 %62
         %64 = OpAccessChain %_ptr_Input_v2float %p_1 %int_1 %int_0
         %65 = OpLoad %v2float %64
         %66 = OpAccessChain %_ptr_Function_v2float %p_0 %int_1 %int_1
               OpStore %66 %65
         %68 = OpAccessChain %_ptr_Input_v4float %p_pos %int_2
         %69 = OpLoad %v4float %68
         %70 = OpAccessChain %_ptr_Function_v4float %p_0 %int_2 %int_0
               OpStore %70 %69
         %71 = OpAccessChain %_ptr_Input_v2float %p_1 %int_2 %int_0
         %72 = OpLoad %v2float %71
         %73 = OpAccessChain %_ptr_Function_v2float %p_0 %int_2 %int_1
               OpStore %73 %72
         %77 = OpLoad %uint %i_1
               OpStore %i_0 %77
         %80 = OpLoad %_arr_VertexOutput_uint_3 %p_0
               OpStore %param %80
         %82 = OpLoad %uint %i_0
               OpStore %param_0 %82
         %83 = OpFunctionCall %HSOut %_hs_main_struct_VertexOutput_vf4_vf21_3__u1_ %param %param_0
               OpStore %flattenTemp %83
         %86 = OpAccessChain %_ptr_Function_v4float %flattenTemp %int_0
         %87 = OpLoad %v4float %86
         %94 = OpLoad %uint %i_1
         %89 = OpAccessChain %_ptr_Output_v4float %_entryPointOutput_pos %94
               OpStore %89 %87
         %95 = OpAccessChain %_ptr_Function_v2float %flattenTemp %int_1
         %96 = OpLoad %v2float %95
         %98 = OpAccessChain %_ptr_Output_v2float %_entryPointOutput %94 %int_0
               OpStore %98 %96
               OpControlBarrier %uint_2 %uint_1 %uint_0
        %102 = OpLoad %uint %i_1
        %104 = OpIEqual %bool %102 %int_0
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %110 = OpLoad %_arr_VertexOutput_uint_3 %p_0
               OpStore %param_1 %110
        %111 = OpFunctionCall %HSConstantOut %PatchHS_struct_VertexOutput_vf4_vf21_3__ %param_1
               OpStore %_patchConstantResult %111
        %117 = OpAccessChain %_ptr_Function_float %_patchConstantResult %int_0 %int_0
        %118 = OpLoad %float %117
        %120 = OpAccessChain %_ptr_Output_float %_patchConstantOutput_EdgeTess %int_0
               OpStore %120 %118
        %121 = OpAccessChain %_ptr_Function_float %_patchConstantResult %int_0 %int_1
        %122 = OpLoad %float %121
        %123 = OpAccessChain %_ptr_Output_float %_patchConstantOutput_EdgeTess %int_1
               OpStore %123 %122
        %124 = OpAccessChain %_ptr_Function_float %_patchConstantResult %int_0 %int_2
        %125 = OpLoad %float %124
        %126 = OpAccessChain %_ptr_Output_float %_patchConstantOutput_EdgeTess %int_2
               OpStore %126 %125
        %130 = OpAccessChain %_ptr_Function_float %_patchConstantResult %int_1
        %131 = OpLoad %float %130
        %132 = OpAccessChain %_ptr_Output_float %_patchConstantOutput_InsideTess %int_0
               OpStore %132 %131
               OpBranch %106
        %106 = OpLabel
               OpReturn
               OpFunctionEnd
%_hs_main_struct_VertexOutput_vf4_vf21_3__u1_ = OpFunction %HSOut None %16
          %p = OpFunctionParameter %_ptr_Function__arr_VertexOutput_uint_3
          %i = OpFunctionParameter %_ptr_Function_uint
         %20 = OpLabel
     %output = OpVariable %_ptr_Function_HSOut Function
         %31 = OpLoad %uint %i
         %33 = OpAccessChain %_ptr_Function_v4float %p %31 %int_0
         %34 = OpLoad %v4float %33
         %35 = OpAccessChain %_ptr_Function_v4float %output %int_0
               OpStore %35 %34
         %37 = OpLoad %uint %i
         %39 = OpAccessChain %_ptr_Function_v2float %p %37 %int_1
         %40 = OpLoad %v2float %39
         %41 = OpAccessChain %_ptr_Function_v2float %output %int_1
               OpStore %41 %40
         %42 = OpLoad %HSOut %output
               OpReturnValue %42
               OpFunctionEnd
%PatchHS_struct_VertexOutput_vf4_vf21_3__ = OpFunction %HSConstantOut None %23
      %patch = OpFunctionParameter %_ptr_Function__arr_VertexOutput_uint_3
         %26 = OpLabel
   %output_0 = OpVariable %_ptr_Function_HSConstantOut Function
        %135 = OpAccessChain %_ptr_Function_v2float %patch %int_0 %int_1
        %136 = OpLoad %v2float %135
        %137 = OpCompositeConstruct %v2float %float_1 %float_1
        %138 = OpFAdd %v2float %137 %136
        %139 = OpCompositeExtract %float %138 0
        %140 = OpAccessChain %_ptr_Function_float %output_0 %int_0 %int_0
               OpStore %140 %139
        %141 = OpAccessChain %_ptr_Function_v2float %patch %int_0 %int_1
        %142 = OpLoad %v2float %141
        %143 = OpCompositeConstruct %v2float %float_1 %float_1
        %144 = OpFAdd %v2float %143 %142
        %145 = OpCompositeExtract %float %144 0
        %146 = OpAccessChain %_ptr_Function_float %output_0 %int_0 %int_1
               OpStore %146 %145
        %147 = OpAccessChain %_ptr_Function_v2float %patch %int_0 %int_1
        %148 = OpLoad %v2float %147
        %149 = OpCompositeConstruct %v2float %float_1 %float_1
        %150 = OpFAdd %v2float %149 %148
        %151 = OpCompositeExtract %float %150 0
        %152 = OpAccessChain %_ptr_Function_float %output_0 %int_0 %int_2
               OpStore %152 %151
        %153 = OpAccessChain %_ptr_Function_v2float %patch %int_0 %int_1
        %154 = OpLoad %v2float %153
        %155 = OpCompositeConstruct %v2float %float_1 %float_1
        %156 = OpFAdd %v2float %155 %154
        %157 = OpCompositeExtract %float %156 0
        %158 = OpAccessChain %_ptr_Function_float %output_0 %int_1
               OpStore %158 %157
        %159 = OpLoad %HSConstantOut %output_0
               OpReturnValue %159
               OpFunctionEnd
