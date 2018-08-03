; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 279
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %IN_p %IN_uv %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %Params "Params"
               OpMemberName %Params 0 "TextureSize"
               OpMemberName %Params 1 "Params1"
               OpMemberName %Params 2 "Params2"
               OpMemberName %Params 3 "Params3"
               OpMemberName %Params 4 "Params4"
               OpMemberName %Params 5 "Bloom"
               OpName %CB1 "CB1"
               OpMemberName %CB1 0 "CB1"
               OpName %_ ""
               OpName %mapSampler "mapSampler"
               OpName %mapTexture "mapTexture"
               OpName %IN_p "IN.p"
               OpName %IN_uv "IN.uv"
               OpName %_entryPointOutput "@entryPointOutput"
               OpMemberDecorate %Params 0 Offset 0
               OpMemberDecorate %Params 1 Offset 16
               OpMemberDecorate %Params 2 Offset 32
               OpMemberDecorate %Params 3 Offset 48
               OpMemberDecorate %Params 4 Offset 64
               OpMemberDecorate %Params 5 Offset 80
               OpMemberDecorate %CB1 0 Offset 0
               OpDecorate %CB1 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 1
               OpDecorate %mapSampler DescriptorSet 1
               OpDecorate %mapSampler Binding 2
               OpDecorate %mapTexture DescriptorSet 1
               OpDecorate %mapTexture Binding 2
               OpDecorate %IN_p BuiltIn FragCoord
               OpDecorate %IN_uv Location 0
               OpDecorate %_entryPointOutput Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
          %9 = OpTypeSampler
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
    %v4float = OpTypeVector %float 4
%float_0_222222 = OpConstant %float 0.222222
         %33 = OpTypeSampledImage %11
       %uint = OpTypeInt 32 0
   %float_80 = OpConstant %float 80
%float_0_0008 = OpConstant %float 0.0008
%float_8en05 = OpConstant %float 8e-05
%float_0_008 = OpConstant %float 0.008
    %float_0 = OpConstant %float 0
        %int = OpTypeInt 32 1
     %int_n3 = OpConstant %int -3
      %int_3 = OpConstant %int 3
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
     %Params = OpTypeStruct %v4float %v4float %v4float %v4float %v4float %v4float
        %CB1 = OpTypeStruct %Params
%_ptr_Uniform_CB1 = OpTypePointer Uniform %CB1
          %_ = OpVariable %_ptr_Uniform_CB1 Uniform
      %int_0 = OpConstant %int 0
     %uint_3 = OpConstant %uint 3
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
 %mapSampler = OpVariable %_ptr_UniformConstant_9 UniformConstant
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
 %mapTexture = OpVariable %_ptr_UniformConstant_11 UniformConstant
%_ptr_Input_v4float = OpTypePointer Input %v4float
       %IN_p = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_v2float = OpTypePointer Input %v2float
      %IN_uv = OpVariable %_ptr_Input_v2float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
        %158 = OpLoad %v2float %IN_uv
        %178 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_0 %uint_3
        %179 = OpLoad %float %178
        %180 = OpCompositeConstruct %v2float %float_0 %179
        %184 = OpLoad %9 %mapSampler
        %185 = OpLoad %11 %mapTexture
        %204 = OpSampledImage %33 %185 %184
        %206 = OpImageSampleImplicitLod %v4float %204 %158
        %207 = OpCompositeExtract %float %206 1
        %209 = OpFMul %float %207 %float_80
        %210 = OpFMul %float %209 %float_0_0008
        %211 = OpExtInst %float %1 FClamp %210 %float_8en05 %float_0_008
               OpBranch %212
        %212 = OpLabel
        %276 = OpPhi %float %float_0 %5 %252 %218
        %277 = OpPhi %float %float_0 %5 %255 %218
        %278 = OpPhi %int %int_n3 %5 %257 %218
        %217 = OpSLessThanEqual %bool %278 %int_3
               OpLoopMerge %213 %218 None
               OpBranchConditional %217 %218 %213
        %218 = OpLabel
        %220 = OpConvertSToF %float %278
        %222 = OpFNegate %float %220
        %224 = OpFMul %float %222 %220
        %226 = OpFMul %float %224 %float_0_222222
        %227 = OpExtInst %float %1 Exp %226
        %230 = OpSampledImage %33 %185 %184
        %234 = OpVectorTimesScalar %v2float %180 %220
        %235 = OpFAdd %v2float %158 %234
        %236 = OpImageSampleImplicitLod %v4float %230 %235
        %273 = OpCompositeExtract %float %236 1
        %241 = OpFSub %float %273 %207
        %242 = OpExtInst %float %1 FAbs %241
        %244 = OpFOrdLessThan %bool %242 %211
        %245 = OpSelect %float %244 %float_1 %float_0
        %246 = OpFMul %float %227 %245
        %275 = OpCompositeExtract %float %236 0
        %250 = OpFMul %float %275 %246
        %252 = OpFAdd %float %276 %250
        %255 = OpFAdd %float %277 %246
        %257 = OpIAdd %int %278 %int_1
               OpBranch %212
        %213 = OpLabel
        %260 = OpFDiv %float %276 %277
        %190 = OpCompositeConstruct %v4float %260 %207 %float_0 %float_1
               OpStore %_entryPointOutput %190
               OpReturn
               OpFunctionEnd
