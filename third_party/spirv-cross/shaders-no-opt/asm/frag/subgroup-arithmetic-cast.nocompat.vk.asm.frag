; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 8
; Bound: 78
; Schema: 0
               OpCapability Shader
               OpCapability GroupNonUniform
               OpCapability GroupNonUniformArithmetic
               OpCapability GroupNonUniformClustered
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %index %FragColor
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_KHR_shader_subgroup_arithmetic"
               OpSourceExtension "GL_KHR_shader_subgroup_basic"
               OpSourceExtension "GL_KHR_shader_subgroup_clustered"
               OpName %main "main"
               OpName %index "index"
               OpName %FragColor "FragColor"
               OpDecorate %index Flat
               OpDecorate %index Location 0
               OpDecorate %FragColor Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_0 = OpConstant %uint 0
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %index = OpVariable %_ptr_Input_int Input
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
%_ptr_Output_uint = OpTypePointer Output %uint
  %FragColor = OpVariable %_ptr_Output_uint Output
       %main = OpFunction %void None %3
          %5 = OpLabel
         %i = OpLoad %int %index
         %u = OpBitcast %uint %i
         %res0 = OpGroupNonUniformSMin %uint %uint_3 Reduce %i
         %res1 = OpGroupNonUniformSMax %uint %uint_3 Reduce %u
         %res2 = OpGroupNonUniformUMin %uint %uint_3 Reduce %i
         %res3 = OpGroupNonUniformUMax %uint %uint_3 Reduce %u
         %res4 = OpGroupNonUniformSMax %uint %uint_3 InclusiveScan %i
         %res5 = OpGroupNonUniformSMin %uint %uint_3 InclusiveScan %u
         %res6 = OpGroupNonUniformUMax %uint %uint_3 ExclusiveScan %i
         %res7 = OpGroupNonUniformUMin %uint %uint_3 ExclusiveScan %u
         %res8 = OpGroupNonUniformSMin %uint %uint_3 ClusteredReduce %i %uint_4
         %res9 = OpGroupNonUniformSMax %uint %uint_3 ClusteredReduce %u %uint_4
         %res10 = OpGroupNonUniformUMin %uint %uint_3 ClusteredReduce %i %uint_4
         %res11 = OpGroupNonUniformUMax %uint %uint_3 ClusteredReduce %u %uint_4
               OpStore %FragColor %res0
               OpStore %FragColor %res1
               OpStore %FragColor %res2
               OpStore %FragColor %res3
               OpStore %FragColor %res4
               OpStore %FragColor %res5
               OpStore %FragColor %res6
               OpStore %FragColor %res7
               OpStore %FragColor %res8
               OpStore %FragColor %res9
               OpStore %FragColor %res10
               OpStore %FragColor %res11
               OpReturn
               OpFunctionEnd
