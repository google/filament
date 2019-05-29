; SPIR-V
; Version: 1.3
; Generator: Khronos Glslang Reference Front End; 7
; Bound: 46
; Schema: 0
               OpCapability Tessellation
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %main "main" %gl_TessLevelInner %gl_TessLevelOuter
               OpExecutionMode %main OutputVertices 1
               OpExecutionMode %main Triangles
               OpSource ESSL 310
               OpSourceExtension "GL_EXT_shader_io_blocks"
               OpSourceExtension "GL_EXT_tessellation_shader"
               OpName %main "main"
               OpName %gl_TessLevelInner "gl_TessLevelInner"
               OpName %TessLevels "TessLevels"
               OpMemberName %TessLevels 0 "inner0"
               OpMemberName %TessLevels 1 "inner1"
               OpMemberName %TessLevels 2 "outer0"
               OpMemberName %TessLevels 3 "outer1"
               OpMemberName %TessLevels 4 "outer2"
               OpMemberName %TessLevels 5 "outer3"
               OpName %sb_levels "sb_levels"
               OpName %gl_TessLevelOuter "gl_TessLevelOuter"
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpMemberDecorate %TessLevels 0 Restrict
               OpMemberDecorate %TessLevels 0 NonWritable
               OpMemberDecorate %TessLevels 0 Offset 0
               OpMemberDecorate %TessLevels 1 Restrict
               OpMemberDecorate %TessLevels 1 NonWritable
               OpMemberDecorate %TessLevels 1 Offset 4
               OpMemberDecorate %TessLevels 2 Restrict
               OpMemberDecorate %TessLevels 2 NonWritable
               OpMemberDecorate %TessLevels 2 Offset 8
               OpMemberDecorate %TessLevels 3 Restrict
               OpMemberDecorate %TessLevels 3 NonWritable
               OpMemberDecorate %TessLevels 3 Offset 12
               OpMemberDecorate %TessLevels 4 Restrict
               OpMemberDecorate %TessLevels 4 NonWritable
               OpMemberDecorate %TessLevels 4 Offset 16
               OpMemberDecorate %TessLevels 5 Restrict
               OpMemberDecorate %TessLevels 5 NonWritable
               OpMemberDecorate %TessLevels 5 Offset 20
               OpDecorate %TessLevels Block
               OpDecorate %sb_levels DescriptorSet 0
               OpDecorate %sb_levels Binding 0
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
 %TessLevels = OpTypeStruct %float %float %float %float %float %float
%_ptr_StorageBuffer_TessLevels = OpTypePointer StorageBuffer %TessLevels
  %sb_levels = OpVariable %_ptr_StorageBuffer_TessLevels StorageBuffer
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
%_ptr_Output_float = OpTypePointer Output %float
      %int_1 = OpConstant %int 1
     %uint_4 = OpConstant %uint 4
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
      %int_5 = OpConstant %int 5
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpAccessChain %_ptr_StorageBuffer_float %sb_levels %int_0
         %19 = OpLoad %float %18
         %21 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_0
               OpStore %21 %19
         %23 = OpAccessChain %_ptr_StorageBuffer_float %sb_levels %int_1
         %24 = OpLoad %float %23
         %25 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %int_1
               OpStore %25 %24
         %31 = OpAccessChain %_ptr_StorageBuffer_float %sb_levels %int_2
         %32 = OpLoad %float %31
         %33 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_0
               OpStore %33 %32
         %35 = OpAccessChain %_ptr_StorageBuffer_float %sb_levels %int_3
         %36 = OpLoad %float %35
         %37 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_1
               OpStore %37 %36
         %39 = OpAccessChain %_ptr_StorageBuffer_float %sb_levels %int_4
         %40 = OpLoad %float %39
         %41 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_2
               OpStore %41 %40
         %43 = OpAccessChain %_ptr_StorageBuffer_float %sb_levels %int_5
         %44 = OpLoad %float %43
         %45 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %int_3
               OpStore %45 %44
               OpReturn
               OpFunctionEnd
