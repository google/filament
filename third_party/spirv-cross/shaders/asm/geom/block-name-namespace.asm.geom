; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 6
; Bound: 48
; Schema: 0
               OpCapability Geometry
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %_ %VertexOutput %vin
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 4
               OpSource GLSL 450
               OpName %main "main"
               OpName %VertexInput3 "VertexInput"
               OpName %gl_PerVertex "gl_PerVertex"
               OpMemberName %gl_PerVertex 0 "gl_Position"
               OpMemberName %gl_PerVertex 1 "gl_PointSize"
               OpMemberName %gl_PerVertex 2 "gl_ClipDistance"
               OpMemberName %gl_PerVertex 3 "gl_CullDistance"
               OpName %_ ""
               OpName %VertexInput "VertexInput"
               OpMemberName %VertexInput 0 "a"
               OpName %VertexInput4 "VertexInput"
               OpName %VertexInput_0 "VertexInput"
               OpMemberName %VertexInput_0 0 "b"
               OpName %VertexInput2 "VertexInput"
               OpName %VertexInput_1 "VertexInput"
               OpMemberName %VertexInput_1 0 "vColor"
               OpName %VertexOutput "VertexInput"
               OpName %VertexInput_2 "VertexInput"
               OpMemberName %VertexInput_2 0 "vColor"
               OpName %vin "vin"
               OpMemberDecorate %gl_PerVertex 0 BuiltIn Position
               OpMemberDecorate %gl_PerVertex 1 BuiltIn PointSize
               OpMemberDecorate %gl_PerVertex 2 BuiltIn ClipDistance
               OpMemberDecorate %gl_PerVertex 3 BuiltIn CullDistance
               OpDecorate %gl_PerVertex Block
               OpMemberDecorate %VertexInput 0 Offset 0
               OpDecorate %VertexInput Block
               OpDecorate %VertexInput4 DescriptorSet 0
               OpDecorate %VertexInput4 Binding 0
               OpMemberDecorate %VertexInput_0 0 Offset 0
               OpDecorate %VertexInput_0 BufferBlock
               OpDecorate %VertexInput2 DescriptorSet 0
               OpDecorate %VertexInput2 Binding 0
               OpDecorate %VertexInput_1 Block
               OpDecorate %VertexOutput Location 0
               OpDecorate %VertexInput_2 Block
               OpDecorate %vin Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %11 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%gl_PerVertex = OpTypeStruct %v4float %float %_arr_float_uint_1 %_arr_float_uint_1
%_ptr_Output_gl_PerVertex = OpTypePointer Output %gl_PerVertex
          %_ = OpVariable %_ptr_Output_gl_PerVertex Output
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%VertexInput = OpTypeStruct %v4float
%_ptr_Uniform_VertexInput = OpTypePointer Uniform %VertexInput
%VertexInput4 = OpVariable %_ptr_Uniform_VertexInput Uniform
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%VertexInput_0 = OpTypeStruct %v4float
%_ptr_Uniform_VertexInput_0 = OpTypePointer Uniform %VertexInput_0
%VertexInput2 = OpVariable %_ptr_Uniform_VertexInput_0 Uniform
%_ptr_Output_v4float = OpTypePointer Output %v4float
%VertexInput_1 = OpTypeStruct %v4float
%_ptr_Output_VertexInput_1 = OpTypePointer Output %VertexInput_1
%VertexOutput = OpVariable %_ptr_Output_VertexInput_1 Output
%VertexInput_2 = OpTypeStruct %v4float
     %uint_3 = OpConstant %uint 3
%_arr_VertexInput_2_uint_3 = OpTypeArray %VertexInput_2 %uint_3
%_ptr_Input__arr_VertexInput_2_uint_3 = OpTypePointer Input %_arr_VertexInput_2_uint_3
        %vin = OpVariable %_ptr_Input__arr_VertexInput_2_uint_3 Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
       %main = OpFunction %void None %3
          %5 = OpLabel
%VertexInput3 = OpVariable %_ptr_Function_v4float Function
               OpStore %VertexInput3 %11
         %20 = OpLoad %v4float %VertexInput3
         %25 = OpAccessChain %_ptr_Uniform_v4float %VertexInput4 %int_0
         %26 = OpLoad %v4float %25
         %27 = OpFAdd %v4float %20 %26
         %31 = OpAccessChain %_ptr_Uniform_v4float %VertexInput2 %int_0
         %32 = OpLoad %v4float %31
         %33 = OpFAdd %v4float %27 %32
         %35 = OpAccessChain %_ptr_Output_v4float %_ %int_0
               OpStore %35 %33
         %45 = OpAccessChain %_ptr_Input_v4float %vin %int_0 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpAccessChain %_ptr_Output_v4float %VertexOutput %int_0
               OpStore %47 %46
               OpEmitVertex
               OpReturn
               OpFunctionEnd
