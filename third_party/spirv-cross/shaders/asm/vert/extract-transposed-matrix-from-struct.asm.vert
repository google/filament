; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 79
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %VS "main" %PosL_1 %instanceID_1 %_entryPointOutput_Position %_entryPointOutput_Color
               OpSource HLSL 500
               OpName %VS "VS"
               OpName %V2F "V2F"
               OpMemberName %V2F 0 "Position"
               OpMemberName %V2F 1 "Color"
               OpName %_VS_vf3_u1_ "@VS(vf3;u1;"
               OpName %PosL "PosL"
               OpName %instanceID "instanceID"
               OpName %InstanceData "InstanceData"
               OpMemberName %InstanceData 0 "MATRIX_MVP"
               OpMemberName %InstanceData 1 "Color"
               OpName %instData "instData"
               OpName %InstanceData_0 "InstanceData"
               OpMemberName %InstanceData_0 0 "MATRIX_MVP"
               OpMemberName %InstanceData_0 1 "Color"
               OpName %gInstanceData "gInstanceData"
               OpMemberName %gInstanceData 0 "@data"
               OpName %gInstanceData_0 "gInstanceData"
               OpName %v2f "v2f"
               OpName %PosL_0 "PosL"
               OpName %PosL_1 "PosL"
               OpName %instanceID_0 "instanceID"
               OpName %instanceID_1 "instanceID"
               OpName %flattenTemp "flattenTemp"
               OpName %param "param"
               OpName %param_0 "param"
               OpName %_entryPointOutput_Position "@entryPointOutput.Position"
               OpName %_entryPointOutput_Color "@entryPointOutput.Color"
               OpMemberDecorate %InstanceData_0 0 RowMajor
               OpMemberDecorate %InstanceData_0 0 Offset 0
               OpMemberDecorate %InstanceData_0 0 MatrixStride 16
               OpMemberDecorate %InstanceData_0 1 Offset 64
               OpDecorate %_runtimearr_InstanceData_0 ArrayStride 80
               OpMemberDecorate %gInstanceData 0 NonWritable
               OpMemberDecorate %gInstanceData 0 Offset 0
               OpDecorate %gInstanceData BufferBlock
               OpDecorate %gInstanceData_0 DescriptorSet 1
               OpDecorate %gInstanceData_0 Binding 0
               OpDecorate %PosL_1 Location 0
               OpDecorate %instanceID_1 BuiltIn InstanceIndex
               OpDecorate %_entryPointOutput_Position BuiltIn Position
               OpDecorate %_entryPointOutput_Color Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
    %v4float = OpTypeVector %float 4
        %V2F = OpTypeStruct %v4float %v4float
         %13 = OpTypeFunction %V2F %_ptr_Function_v3float %_ptr_Function_uint
%mat4v4float = OpTypeMatrix %v4float 4
%InstanceData = OpTypeStruct %mat4v4float %v4float
%_ptr_Function_InstanceData = OpTypePointer Function %InstanceData
%InstanceData_0 = OpTypeStruct %mat4v4float %v4float
%_runtimearr_InstanceData_0 = OpTypeRuntimeArray %InstanceData_0
%gInstanceData = OpTypeStruct %_runtimearr_InstanceData_0
%_ptr_Uniform_gInstanceData = OpTypePointer Uniform %gInstanceData
%gInstanceData_0 = OpVariable %_ptr_Uniform_gInstanceData Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_InstanceData_0 = OpTypePointer Uniform %InstanceData_0
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
      %int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_V2F = OpTypePointer Function %V2F
    %float_1 = OpConstant %float 1
%_ptr_Input_v3float = OpTypePointer Input %v3float
     %PosL_1 = OpVariable %_ptr_Input_v3float Input
%_ptr_Input_uint = OpTypePointer Input %uint
%instanceID_1 = OpVariable %_ptr_Input_uint Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput_Position = OpVariable %_ptr_Output_v4float Output
%_entryPointOutput_Color = OpVariable %_ptr_Output_v4float Output
         %VS = OpFunction %void None %3
          %5 = OpLabel
     %PosL_0 = OpVariable %_ptr_Function_v3float Function
%instanceID_0 = OpVariable %_ptr_Function_uint Function
%flattenTemp = OpVariable %_ptr_Function_V2F Function
      %param = OpVariable %_ptr_Function_v3float Function
    %param_0 = OpVariable %_ptr_Function_uint Function
         %61 = OpLoad %v3float %PosL_1
               OpStore %PosL_0 %61
         %65 = OpLoad %uint %instanceID_1
               OpStore %instanceID_0 %65
         %68 = OpLoad %v3float %PosL_0
               OpStore %param %68
         %70 = OpLoad %uint %instanceID_0
               OpStore %param_0 %70
         %71 = OpFunctionCall %V2F %_VS_vf3_u1_ %param %param_0
               OpStore %flattenTemp %71
         %74 = OpAccessChain %_ptr_Function_v4float %flattenTemp %int_0
         %75 = OpLoad %v4float %74
               OpStore %_entryPointOutput_Position %75
         %77 = OpAccessChain %_ptr_Function_v4float %flattenTemp %int_1
         %78 = OpLoad %v4float %77
               OpStore %_entryPointOutput_Color %78
               OpReturn
               OpFunctionEnd
%_VS_vf3_u1_ = OpFunction %V2F None %13
       %PosL = OpFunctionParameter %_ptr_Function_v3float
 %instanceID = OpFunctionParameter %_ptr_Function_uint
         %17 = OpLabel
   %instData = OpVariable %_ptr_Function_InstanceData Function
        %v2f = OpVariable %_ptr_Function_V2F Function
         %29 = OpLoad %uint %instanceID
         %31 = OpAccessChain %_ptr_Uniform_InstanceData_0 %gInstanceData_0 %int_0 %29
         %32 = OpLoad %InstanceData_0 %31
         %33 = OpCompositeExtract %mat4v4float %32 0
         %35 = OpAccessChain %_ptr_Function_mat4v4float %instData %int_0
               OpStore %35 %33
         %36 = OpCompositeExtract %v4float %32 1
         %39 = OpAccessChain %_ptr_Function_v4float %instData %int_1
               OpStore %39 %36
         %42 = OpAccessChain %_ptr_Function_mat4v4float %instData %int_0
         %43 = OpLoad %mat4v4float %42
         %44 = OpLoad %v3float %PosL
         %46 = OpCompositeExtract %float %44 0
         %47 = OpCompositeExtract %float %44 1
         %48 = OpCompositeExtract %float %44 2
         %49 = OpCompositeConstruct %v4float %46 %47 %48 %float_1
         %50 = OpMatrixTimesVector %v4float %43 %49
         %51 = OpAccessChain %_ptr_Function_v4float %v2f %int_0
               OpStore %51 %50
         %52 = OpAccessChain %_ptr_Function_v4float %instData %int_1
         %53 = OpLoad %v4float %52
         %54 = OpAccessChain %_ptr_Function_v4float %v2f %int_1
               OpStore %54 %53
         %55 = OpLoad %V2F %v2f
               OpReturnValue %55
               OpFunctionEnd
