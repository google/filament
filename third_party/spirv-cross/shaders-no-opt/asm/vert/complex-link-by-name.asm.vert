; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 10
; Bound: 59
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %_ %output_location_0 %output_location_2 %output_location_3
               OpSource GLSL 450
               OpName %main "main"
               OpName %Foo "Struct_vec4"
               OpMemberName %Foo 0 "m0"
               OpName %c "c"
               OpName %Foo_0 "Struct_vec4"
               OpMemberName %Foo_0 0 "m0"
               OpName %Bar "Struct_vec4"
               OpMemberName %Bar 0 "m0"
               OpName %UBO "UBO"
               OpMemberName %UBO 0 "m0"
               OpMemberName %UBO 1 "m1"
               OpName %ubo_binding_0 "ubo_binding_0"
               OpName %Bar_0 "Struct_vec4"
               OpMemberName %Bar_0 0 "m0"
               OpName %b "b"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpName %VertexOut "VertexOut"
               OpMemberName %VertexOut 0 "m0"
               OpMemberName %VertexOut 1 "m1"
               OpName %output_location_0 "output_location_0"
               OpName %output_location_2 "output_location_2"
               OpName %output_location_3 "output_location_3"
               OpMemberDecorate %Foo_0 0 Offset 0
               OpMemberDecorate %Bar 0 Offset 0
               OpMemberDecorate %UBO 0 Offset 0
               OpMemberDecorate %UBO 1 Offset 16
               OpDecorate %UBO Block
               OpDecorate %ubo_binding_0 DescriptorSet 0
               OpDecorate %ubo_binding_0 Binding 0
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpDecorate %VertexOut Block
               OpDecorate %output_location_0 Location 0
               OpDecorate %output_location_2 Location 2
               OpDecorate %output_location_3 Location 3
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
        %Foo = OpTypeStruct %v4float
%_ptr_Function_Foo = OpTypePointer Function %Foo
      %Foo_0 = OpTypeStruct %v4float
        %Bar = OpTypeStruct %v4float
        %UBO = OpTypeStruct %Foo_0 %Bar
%_ptr_Uniform_UBO = OpTypePointer Uniform %UBO
%ubo_binding_0 = OpVariable %_ptr_Uniform_UBO Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_Foo_0 = OpTypePointer Uniform %Foo_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %Bar_0 = OpTypeStruct %v4float
%_ptr_Function_Bar_0 = OpTypePointer Function %Bar_0
      %int_1 = OpConstant %int 1
%_ptr_Uniform_Bar = OpTypePointer Uniform %Bar
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %VertexOut = OpTypeStruct %Foo %Bar_0
%_ptr_Output_VertexOut = OpTypePointer Output %VertexOut
%output_location_0 = OpVariable %_ptr_Output_VertexOut Output
%_ptr_Output_Foo = OpTypePointer Output %Foo
%_ptr_Output_Bar_0 = OpTypePointer Output %Bar_0
%output_location_2 = OpVariable %_ptr_Output_Foo Output
%output_location_3 = OpVariable %_ptr_Output_Bar_0 Output
       %main = OpFunction %void None %3
          %5 = OpLabel
          %c = OpVariable %_ptr_Function_Foo Function
          %b = OpVariable %_ptr_Function_Bar_0 Function
         %19 = OpAccessChain %_ptr_Uniform_Foo_0 %ubo_binding_0 %int_0
         %20 = OpLoad %Foo_0 %19
         %21 = OpCompositeExtract %v4float %20 0
         %23 = OpAccessChain %_ptr_Function_v4float %c %int_0
               OpStore %23 %21
         %29 = OpAccessChain %_ptr_Uniform_Bar %ubo_binding_0 %int_1
         %30 = OpLoad %Bar %29
         %31 = OpCompositeExtract %v4float %30 0
         %32 = OpAccessChain %_ptr_Function_v4float %b %int_0
               OpStore %32 %31
         %39 = OpAccessChain %_ptr_Function_v4float %c %int_0
         %40 = OpLoad %v4float %39
         %41 = OpAccessChain %_ptr_Function_v4float %b %int_0
         %42 = OpLoad %v4float %41
         %43 = OpFAdd %v4float %40 %42
         %45 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %45 %43
         %49 = OpLoad %Foo %c
         %51 = OpAccessChain %_ptr_Output_Foo %output_location_0 %int_0
               OpStore %51 %49
         %52 = OpLoad %Bar_0 %b
         %54 = OpAccessChain %_ptr_Output_Bar_0 %output_location_0 %int_1
               OpStore %54 %52
         %56 = OpLoad %Foo %c
               OpStore %output_location_2 %56
         %58 = OpLoad %Bar_0 %b
               OpStore %output_location_3 %58
               OpReturn
               OpFunctionEnd
