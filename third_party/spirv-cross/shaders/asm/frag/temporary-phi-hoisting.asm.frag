; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 1
; Bound: 87
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %MyStruct "MyStruct"
               OpMemberName %MyStruct 0 "color"
               OpName %MyStruct_CB "MyStruct_CB"
               OpMemberName %MyStruct_CB 0 "g_MyStruct"
               OpName %_ ""
               OpName %_entryPointOutput "@entryPointOutput"
               OpMemberDecorate %MyStruct 0 Offset 0
               OpDecorate %_arr_MyStruct_uint_4 ArrayStride 16
               OpMemberDecorate %MyStruct_CB 0 Offset 0
               OpDecorate %MyStruct_CB Block
               OpDecorate %_ DescriptorSet 0
			   OpDecorate %_ Binding 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
    %float_0 = OpConstant %float 0
         %15 = OpConstantComposite %v3float %float_0 %float_0 %float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
   %MyStruct = OpTypeStruct %v4float
       %uint = OpTypeInt 32 0
     %uint_4 = OpConstant %uint 4
%_arr_MyStruct_uint_4 = OpTypeArray %MyStruct %uint_4
%MyStruct_CB = OpTypeStruct %_arr_MyStruct_uint_4
%_ptr_Uniform_MyStruct_CB = OpTypePointer Uniform %MyStruct_CB
          %_ = OpVariable %_ptr_Uniform_MyStruct_CB Uniform
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpBranch %64
         %64 = OpLabel
         %85 = OpPhi %v3float %15 %5 %77 %66
         %86 = OpPhi %int %int_0 %5 %79 %66
               OpLoopMerge %65 %66 None
               OpBranch %67
         %67 = OpLabel
         %69 = OpSLessThan %bool %86 %int_4
               OpBranchConditional %69 %70 %65
         %70 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %86 %int_0
         %73 = OpLoad %v4float %72
         %74 = OpVectorShuffle %v3float %73 %73 0 1 2
         %77 = OpFAdd %v3float %85 %74
               OpBranch %66
         %66 = OpLabel
         %79 = OpIAdd %int %86 %int_1
               OpBranch %64
         %65 = OpLabel
         %81 = OpCompositeExtract %float %85 0
         %82 = OpCompositeExtract %float %85 1
         %83 = OpCompositeExtract %float %85 2
         %84 = OpCompositeConstruct %v4float %81 %82 %83 %float_1
               OpStore %_entryPointOutput %84
               OpReturn
               OpFunctionEnd
