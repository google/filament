
; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 11
; Bound: 177
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %92 %96
               OpExecutionMode %4 OriginUpperLeft
               OpSource HLSL 500
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %4 "main"
               OpName %19 "ui"
               OpName %23 "ui_sampler"
               OpName %46 "scene"
               OpName %48 "scene_sampler"
               OpName %55 "world_info"
               OpMemberName %55 0 "ambient_light"
               OpMemberName %55 1 "world_resolution"
               OpMemberName %55 2 "level_size"
               OpName %57 "world_info"
               OpName %92 "input.uv"
               OpName %96 "@entryPointOutput"
               OpDecorate %19 Binding 1
               OpDecorate %19 DescriptorSet 2
               OpDecorate %23 Binding 1
               OpDecorate %23 DescriptorSet 2
               OpDecorate %46 Binding 0
               OpDecorate %46 DescriptorSet 2
               OpDecorate %48 Binding 0
               OpDecorate %48 DescriptorSet 2
               OpDecorate %55 Block
               OpMemberDecorate %55 0 Offset 0
               OpMemberDecorate %55 1 Offset 16
               OpMemberDecorate %55 2 Offset 24
               OpDecorate %57 Binding 2
               OpDecorate %57 DescriptorSet 1
               OpDecorate %92 Location 0
               OpDecorate %96 Location 0
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 4
          %8 = OpTypeVector %6 2
         %15 = OpTypePointer Function %7
         %17 = OpTypeImage %6 2D 0 0 0 1 Unknown
         %18 = OpTypePointer UniformConstant %17
         %19 = OpVariable %18 UniformConstant
         %21 = OpTypeSampler
         %22 = OpTypePointer UniformConstant %21
         %23 = OpVariable %22 UniformConstant
         %25 = OpTypeSampledImage %17
         %27 = OpTypeInt 32 1
         %33 = OpTypeInt 32 0
         %34 = OpConstant %33 3
         %35 = OpTypePointer Function %6
         %38 = OpConstant %6 1
         %39 = OpTypeBool
         %46 = OpVariable %18 UniformConstant
         %48 = OpVariable %22 UniformConstant
         %55 = OpTypeStruct %7 %8 %8
         %56 = OpTypePointer Uniform %55
         %57 = OpVariable %56 Uniform
         %58 = OpConstant %27 0
         %59 = OpTypePointer Uniform %7
         %91 = OpTypePointer Input %8
         %92 = OpVariable %91 Input
         %95 = OpTypePointer Output %7
         %96 = OpVariable %95 Output
        %115 = OpConstant %33 0
          %4 = OpFunction %2 None %3
          %5 = OpLabel
        %122 = OpVariable %15 Function
        %124 = OpVariable %15 Function
         %93 = OpLoad %8 %92
               OpSelectionMerge %166 None
               OpSwitch %115 %128
        %128 = OpLabel
        %129 = OpLoad %17 %19
        %130 = OpLoad %21 %23
        %131 = OpSampledImage %25 %129 %130
        %134 = OpImageSampleImplicitLod %7 %131 %93
               OpStore %122 %134
        %135 = OpAccessChain %35 %122 %34
        %136 = OpLoad %6 %135
        %137 = OpFOrdEqual %39 %136 %38
               OpSelectionMerge %140 None
               OpBranchConditional %137 %138 %140
        %138 = OpLabel
               OpBranch %166
        %140 = OpLabel
        %141 = OpLoad %17 %46
        %142 = OpLoad %21 %48
        %143 = OpSampledImage %25 %141 %142
        %146 = OpImageSampleImplicitLod %7 %143 %93
        %147 = OpAccessChain %59 %57 %58
        %148 = OpLoad %7 %147
               OpStore %124 %148
        %149 = OpAccessChain %35 %124 %34
        %150 = OpLoad %6 %149
        %152 = OpVectorTimesScalar %7 %148 %150
               OpStore %124 %152
        %164 = OpFMul %7 %146 %152
        %165 = OpFAdd %7 %134 %164
               OpBranch %166
        %166 = OpLabel
        %176 = OpPhi %7 %134 %138 %165 %140
               OpStore %96 %176
               OpReturn
               OpFunctionEnd
