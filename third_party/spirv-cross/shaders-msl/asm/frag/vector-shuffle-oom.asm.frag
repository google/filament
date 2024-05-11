; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 2
; Bound: 25007
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %5663 "main" %5800 %gl_FragCoord %4317
               OpExecutionMode %5663 OriginUpperLeft
               OpMemberDecorate %_struct_1116 0 Offset 0
               OpMemberDecorate %_struct_1116 1 Offset 16
               OpMemberDecorate %_struct_1116 2 Offset 32
               OpDecorate %_struct_1116 Block
               OpDecorate %22044 DescriptorSet 0
               OpDecorate %22044 Binding 0
               OpDecorate %5785 DescriptorSet 0
               OpDecorate %5785 Binding 14
               OpDecorate %5688 DescriptorSet 0
               OpDecorate %5688 Binding 6
               OpMemberDecorate %_struct_994 0 Offset 0
               OpMemberDecorate %_struct_994 1 Offset 16
               OpMemberDecorate %_struct_994 2 Offset 28
               OpMemberDecorate %_struct_994 3 Offset 32
               OpMemberDecorate %_struct_994 4 Offset 44
               OpMemberDecorate %_struct_994 5 Offset 48
               OpMemberDecorate %_struct_994 6 Offset 60
               OpMemberDecorate %_struct_994 7 Offset 64
               OpMemberDecorate %_struct_994 8 Offset 76
               OpMemberDecorate %_struct_994 9 Offset 80
               OpMemberDecorate %_struct_994 10 Offset 92
               OpMemberDecorate %_struct_994 11 Offset 96
               OpMemberDecorate %_struct_994 12 Offset 108
               OpMemberDecorate %_struct_994 13 Offset 112
               OpMemberDecorate %_struct_994 14 Offset 120
               OpMemberDecorate %_struct_994 15 Offset 128
               OpMemberDecorate %_struct_994 16 Offset 140
               OpMemberDecorate %_struct_994 17 Offset 144
               OpMemberDecorate %_struct_994 18 Offset 148
               OpMemberDecorate %_struct_994 19 Offset 152
               OpMemberDecorate %_struct_994 20 Offset 156
               OpMemberDecorate %_struct_994 21 Offset 160
               OpMemberDecorate %_struct_994 22 Offset 176
               OpMemberDecorate %_struct_994 23 RowMajor
               OpMemberDecorate %_struct_994 23 Offset 192
               OpMemberDecorate %_struct_994 23 MatrixStride 16
               OpMemberDecorate %_struct_994 24 Offset 256
               OpDecorate %_struct_994 Block
               OpDecorate %12348 DescriptorSet 0
               OpDecorate %12348 Binding 2
               OpDecorate %3312 DescriptorSet 0
               OpDecorate %3312 Binding 13
               OpDecorate %4646 DescriptorSet 0
               OpDecorate %4646 Binding 5
               OpDecorate %4862 DescriptorSet 0
               OpDecorate %4862 Binding 4
               OpDecorate %3594 DescriptorSet 0
               OpDecorate %3594 Binding 3
               OpDecorate %_arr_mat4v4float_uint_2 ArrayStride 64
               OpDecorate %_arr_v4float_uint_2 ArrayStride 16
               OpMemberDecorate %_struct_408 0 RowMajor
               OpMemberDecorate %_struct_408 0 Offset 0
               OpMemberDecorate %_struct_408 0 MatrixStride 16
               OpMemberDecorate %_struct_408 1 RowMajor
               OpMemberDecorate %_struct_408 1 Offset 64
               OpMemberDecorate %_struct_408 1 MatrixStride 16
               OpMemberDecorate %_struct_408 2 RowMajor
               OpMemberDecorate %_struct_408 2 Offset 128
               OpMemberDecorate %_struct_408 2 MatrixStride 16
               OpMemberDecorate %_struct_408 3 RowMajor
               OpMemberDecorate %_struct_408 3 Offset 192
               OpMemberDecorate %_struct_408 3 MatrixStride 16
               OpMemberDecorate %_struct_408 4 Offset 256
               OpMemberDecorate %_struct_408 5 Offset 272
               OpMemberDecorate %_struct_408 6 Offset 288
               OpMemberDecorate %_struct_408 7 Offset 292
               OpMemberDecorate %_struct_408 8 Offset 296
               OpMemberDecorate %_struct_408 9 Offset 300
               OpMemberDecorate %_struct_408 10 Offset 304
               OpMemberDecorate %_struct_408 11 Offset 316
               OpMemberDecorate %_struct_408 12 Offset 320
               OpMemberDecorate %_struct_408 13 Offset 332
               OpMemberDecorate %_struct_408 14 Offset 336
               OpMemberDecorate %_struct_408 15 Offset 348
               OpMemberDecorate %_struct_408 16 Offset 352
               OpMemberDecorate %_struct_408 17 Offset 364
               OpMemberDecorate %_struct_408 18 Offset 368
               OpMemberDecorate %_struct_408 19 Offset 372
               OpMemberDecorate %_struct_408 20 Offset 376
               OpMemberDecorate %_struct_408 21 Offset 384
               OpMemberDecorate %_struct_408 22 Offset 392
               OpMemberDecorate %_struct_408 23 Offset 400
               OpMemberDecorate %_struct_408 24 Offset 416
               OpMemberDecorate %_struct_408 25 Offset 424
               OpMemberDecorate %_struct_408 26 Offset 432
               OpMemberDecorate %_struct_408 27 Offset 448
               OpMemberDecorate %_struct_408 28 Offset 460
               OpMemberDecorate %_struct_408 29 Offset 464
               OpMemberDecorate %_struct_408 30 Offset 468
               OpMemberDecorate %_struct_408 31 Offset 472
               OpMemberDecorate %_struct_408 32 Offset 476
               OpMemberDecorate %_struct_408 33 Offset 480
               OpMemberDecorate %_struct_408 34 Offset 488
               OpMemberDecorate %_struct_408 35 Offset 492
               OpMemberDecorate %_struct_408 36 Offset 496
               OpMemberDecorate %_struct_408 37 RowMajor
               OpMemberDecorate %_struct_408 37 Offset 512
               OpMemberDecorate %_struct_408 37 MatrixStride 16
               OpMemberDecorate %_struct_408 38 Offset 640
               OpDecorate %_struct_408 Block
               OpDecorate %15259 DescriptorSet 0
               OpDecorate %15259 Binding 1
               OpDecorate %5800 Location 0
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %4317 Location 0
               OpMemberDecorate %_struct_1395 0 Offset 0
               OpMemberDecorate %_struct_1395 1 Offset 16
               OpMemberDecorate %_struct_1395 2 Offset 32
               OpMemberDecorate %_struct_1395 3 Offset 40
               OpMemberDecorate %_struct_1395 4 Offset 48
               OpMemberDecorate %_struct_1395 5 Offset 60
               OpMemberDecorate %_struct_1395 6 Offset 64
               OpMemberDecorate %_struct_1395 7 Offset 76
               OpMemberDecorate %_struct_1395 8 Offset 80
               OpMemberDecorate %_struct_1395 9 Offset 96
               OpMemberDecorate %_struct_1395 10 Offset 112
               OpMemberDecorate %_struct_1395 11 Offset 128
               OpMemberDecorate %_struct_1395 12 Offset 140
               OpMemberDecorate %_struct_1395 13 Offset 144
               OpMemberDecorate %_struct_1395 14 Offset 156
               OpMemberDecorate %_struct_1395 15 Offset 160
               OpMemberDecorate %_struct_1395 16 Offset 176
               OpMemberDecorate %_struct_1395 17 Offset 192
               OpMemberDecorate %_struct_1395 18 Offset 204
               OpMemberDecorate %_struct_1395 19 Offset 208
               OpMemberDecorate %_struct_1395 20 Offset 224
               OpDecorate %_struct_1395 Block
               OpMemberDecorate %_struct_1018 0 Offset 0
               OpDecorate %_struct_1018 Block
       %void = OpTypeVoid
       %1282 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
    %v4float = OpTypeVector %float 4
    %v3float = OpTypeVector %float 3
%_struct_1017 = OpTypeStruct %v4float
%_struct_1116 = OpTypeStruct %v4float %float %v4float
%_ptr_Uniform__struct_1116 = OpTypePointer Uniform %_struct_1116
      %22044 = OpVariable %_ptr_Uniform__struct_1116 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %150 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_150 = OpTypePointer UniformConstant %150
       %5785 = OpVariable %_ptr_UniformConstant_150 UniformConstant
        %508 = OpTypeSampler
%_ptr_UniformConstant_508 = OpTypePointer UniformConstant %508
       %5688 = OpVariable %_ptr_UniformConstant_508 UniformConstant
        %510 = OpTypeSampledImage %150
    %float_0 = OpConstant %float 0
       %uint = OpTypeInt 32 0
      %int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_1 = OpConstant %float 1
%mat4v4float = OpTypeMatrix %v4float 4
%_struct_994 = OpTypeStruct %v3float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v2float %v2float %v3float %float %float %float %float %float %v4float %v4float %mat4v4float %v4float
%_ptr_Uniform__struct_994 = OpTypePointer Uniform %_struct_994
      %12348 = OpVariable %_ptr_Uniform__struct_994 Uniform
      %int_5 = OpConstant %int 5
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
       %3312 = OpVariable %_ptr_UniformConstant_150 UniformConstant
       %4646 = OpVariable %_ptr_UniformConstant_508 UniformConstant
       %bool = OpTypeBool
       %4862 = OpVariable %_ptr_UniformConstant_150 UniformConstant
       %3594 = OpVariable %_ptr_UniformConstant_508 UniformConstant
     %uint_2 = OpConstant %uint 2
       %2938 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_arr_mat4v4float_uint_2 = OpTypeArray %mat4v4float %uint_2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_struct_408 = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %float %float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float %v2float %v2float %v2float %v4float %v2float %v2float %v2float %v3float %float %float %float %float %float %v2float %float %float %v3float %_arr_mat4v4float_uint_2 %_arr_v4float_uint_2
%_ptr_Uniform__struct_408 = OpTypePointer Uniform %_struct_408
      %15259 = OpVariable %_ptr_Uniform__struct_408 Uniform
     %int_23 = OpConstant %int 23
      %int_2 = OpConstant %int 2
   %float_n2 = OpConstant %float -2
  %float_0_5 = OpConstant %float 0.5
       %1196 = OpConstantComposite %v3float %float_0 %float_n2 %float_0_5
   %float_n1 = OpConstant %float -1
        %836 = OpConstantComposite %v3float %float_n1 %float_n1 %float_0_5
 %float_0_75 = OpConstant %float 0.75
       %1367 = OpConstantComposite %v3float %float_0 %float_n1 %float_0_75
        %141 = OpConstantComposite %v3float %float_1 %float_n1 %float_0_5
         %38 = OpConstantComposite %v3float %float_n2 %float_0 %float_0_5
         %95 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
        %626 = OpConstantComposite %v3float %float_0 %float_0 %float_1
       %2411 = OpConstantComposite %v3float %float_1 %float_0 %float_0_75
    %float_2 = OpConstant %float 2
       %2354 = OpConstantComposite %v3float %float_2 %float_0 %float_0_5
        %837 = OpConstantComposite %v3float %float_n1 %float_1 %float_0_5
       %1368 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
        %142 = OpConstantComposite %v3float %float_1 %float_1 %float_0_5
       %1197 = OpConstantComposite %v3float %float_0 %float_2 %float_0_5
%_ptr_Input_v2float = OpTypePointer Input %v2float
       %5800 = OpVariable %_ptr_Input_v2float Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %4317 = OpVariable %_ptr_Output_v4float Output
%_struct_1395 = OpTypeStruct %v4float %v4float %v2float %v2float %v3float %float %v3float %float %v4float %v4float %v4float %v3float %float %v3float %float %v3float %v4float %v3float %float %v3float %v2float
%_struct_1018 = OpTypeStruct %v4float
      %10264 = OpUndef %_struct_1017
       %5663 = OpFunction %void None %1282
      %25006 = OpLabel
      %17463 = OpLoad %v4float %gl_FragCoord
      %13863 = OpCompositeInsert %_struct_1017 %2938 %10264 0
      %22969 = OpVectorShuffle %v2float %17463 %17463 0 1
      %13206 = OpAccessChain %_ptr_Uniform_v4float %15259 %int_23
      %10343 = OpLoad %v4float %13206
       %7422 = OpVectorShuffle %v2float %10343 %10343 0 1
      %19927 = OpFMul %v2float %22969 %7422
      %18174 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_2
      %16206 = OpLoad %v4float %18174
      %20420 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %21354 = OpLoad %v4float %20420
       %7688 = OpVectorShuffle %v4float %21354 %21354 0 1 0 1
      %17581 = OpFMul %v4float %16206 %7688
      %10673 = OpVectorShuffle %v2float %1196 %1196 0 1
      %18824 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10344 = OpLoad %v4float %18824
       %8638 = OpVectorShuffle %v2float %10344 %10344 0 1
       %9197 = OpFMul %v2float %10673 %8638
      %18505 = OpFAdd %v2float %19927 %9197
       %7011 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21058 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13149 = OpExtInst %v2float %1 FClamp %18505 %7011 %21058
      %23584 = OpLoad %150 %5785
      %10339 = OpLoad %508 %5688
      %12147 = OpSampledImage %510 %23584 %10339
      %15371 = OpImageSampleExplicitLod %v4float %12147 %13149 Lod %float_0
      %15266 = OpCompositeExtract %float %15371 3
      %12116 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12972 = OpLoad %float %12116
      %15710 = OpFMul %float %15266 %12972
      %15279 = OpExtInst %float %1 FClamp %15710 %float_0 %float_1
      %22213 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11756 = OpLoad %v3float %22213
      %12103 = OpVectorTimesScalar %v3float %11756 %15279
      %15516 = OpLoad %150 %3312
      %24569 = OpLoad %508 %4646
      %12148 = OpSampledImage %510 %15516 %24569
      %17670 = OpImageSampleExplicitLod %v4float %12148 %13149 Lod %float_0
      %16938 = OpCompositeExtract %float %17670 1
      %14185 = OpFOrdGreaterThan %bool %16938 %float_0
               OpSelectionMerge %22307 DontFlatten
               OpBranchConditional %14185 %12821 %22307
      %12821 = OpLabel
      %13239 = OpLoad %150 %4862
      %19960 = OpLoad %508 %3594
      %12149 = OpSampledImage %510 %13239 %19960
      %15675 = OpImageSampleExplicitLod %v4float %12149 %13149 Lod %float_0
      %13866 = OpCompositeExtract %float %17670 1
      %12427 = OpCompositeExtract %float %17670 2
      %23300 = OpFMul %float %13866 %12427
      %17612 = OpExtInst %float %1 FClamp %23300 %float_0 %float_1
      %20291 = OpVectorShuffle %v3float %15675 %15675 0 1 2
      %11186 = OpVectorTimesScalar %v3float %20291 %17612
      %15293 = OpFAdd %v3float %12103 %11186
               OpBranch %22307
      %22307 = OpLabel
       %7719 = OpPhi %v3float %12103 %25006 %15293 %12821
      %23399 = OpVectorTimesScalar %v3float %7719 %float_0_5
       %9339 = OpFAdd %float %float_0 %float_0_5
      %16235 = OpVectorShuffle %v3float %2938 %2938 0 1 2
      %22177 = OpFAdd %v3float %16235 %23399
      %15527 = OpVectorShuffle %v4float %2938 %22177 4 5 6 3
       %6434 = OpCompositeInsert %_struct_1017 %15527 %13863 0
      %24572 = OpVectorShuffle %v2float %836 %836 0 1
      %13207 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10345 = OpLoad %v4float %13207
       %8639 = OpVectorShuffle %v2float %10345 %10345 0 1
       %9198 = OpFMul %v2float %24572 %8639
      %18506 = OpFAdd %v2float %19927 %9198
       %7012 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21059 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13150 = OpExtInst %v2float %1 FClamp %18506 %7012 %21059
      %23585 = OpLoad %150 %5785
      %10340 = OpLoad %508 %5688
      %12150 = OpSampledImage %510 %23585 %10340
      %15372 = OpImageSampleExplicitLod %v4float %12150 %13150 Lod %float_0
      %15267 = OpCompositeExtract %float %15372 3
      %12117 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12973 = OpLoad %float %12117
      %15711 = OpFMul %float %15267 %12973
      %15280 = OpExtInst %float %1 FClamp %15711 %float_0 %float_1
      %22214 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11757 = OpLoad %v3float %22214
      %12104 = OpVectorTimesScalar %v3float %11757 %15280
      %15517 = OpLoad %150 %3312
      %24570 = OpLoad %508 %4646
      %12151 = OpSampledImage %510 %15517 %24570
      %17671 = OpImageSampleExplicitLod %v4float %12151 %13150 Lod %float_0
      %16939 = OpCompositeExtract %float %17671 1
      %14186 = OpFOrdGreaterThan %bool %16939 %float_0
               OpSelectionMerge %22308 DontFlatten
               OpBranchConditional %14186 %12822 %22308
      %12822 = OpLabel
      %13240 = OpLoad %150 %4862
      %19961 = OpLoad %508 %3594
      %12152 = OpSampledImage %510 %13240 %19961
      %15676 = OpImageSampleExplicitLod %v4float %12152 %13150 Lod %float_0
      %13867 = OpCompositeExtract %float %17671 1
      %12428 = OpCompositeExtract %float %17671 2
      %23301 = OpFMul %float %13867 %12428
      %17613 = OpExtInst %float %1 FClamp %23301 %float_0 %float_1
      %20292 = OpVectorShuffle %v3float %15676 %15676 0 1 2
      %11187 = OpVectorTimesScalar %v3float %20292 %17613
      %15294 = OpFAdd %v3float %12104 %11187
               OpBranch %22308
      %22308 = OpLabel
       %7720 = OpPhi %v3float %12104 %22307 %15294 %12822
      %23400 = OpVectorTimesScalar %v3float %7720 %float_0_5
       %9340 = OpFAdd %float %9339 %float_0_5
      %16236 = OpVectorShuffle %v3float %15527 %15527 0 1 2
      %22178 = OpFAdd %v3float %16236 %23400
      %15528 = OpVectorShuffle %v4float %15527 %22178 4 5 6 3
       %6435 = OpCompositeInsert %_struct_1017 %15528 %6434 0
      %24573 = OpVectorShuffle %v2float %1367 %1367 0 1
      %13208 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10346 = OpLoad %v4float %13208
       %8640 = OpVectorShuffle %v2float %10346 %10346 0 1
       %9199 = OpFMul %v2float %24573 %8640
      %18507 = OpFAdd %v2float %19927 %9199
       %7013 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21060 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13151 = OpExtInst %v2float %1 FClamp %18507 %7013 %21060
      %23586 = OpLoad %150 %5785
      %10341 = OpLoad %508 %5688
      %12153 = OpSampledImage %510 %23586 %10341
      %15373 = OpImageSampleExplicitLod %v4float %12153 %13151 Lod %float_0
      %15268 = OpCompositeExtract %float %15373 3
      %12118 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12974 = OpLoad %float %12118
      %15712 = OpFMul %float %15268 %12974
      %15281 = OpExtInst %float %1 FClamp %15712 %float_0 %float_1
      %22215 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11758 = OpLoad %v3float %22215
      %12105 = OpVectorTimesScalar %v3float %11758 %15281
      %15518 = OpLoad %150 %3312
      %24571 = OpLoad %508 %4646
      %12154 = OpSampledImage %510 %15518 %24571
      %17672 = OpImageSampleExplicitLod %v4float %12154 %13151 Lod %float_0
      %16940 = OpCompositeExtract %float %17672 1
      %14187 = OpFOrdGreaterThan %bool %16940 %float_0
               OpSelectionMerge %22309 DontFlatten
               OpBranchConditional %14187 %12823 %22309
      %12823 = OpLabel
      %13241 = OpLoad %150 %4862
      %19962 = OpLoad %508 %3594
      %12155 = OpSampledImage %510 %13241 %19962
      %15677 = OpImageSampleExplicitLod %v4float %12155 %13151 Lod %float_0
      %13868 = OpCompositeExtract %float %17672 1
      %12429 = OpCompositeExtract %float %17672 2
      %23302 = OpFMul %float %13868 %12429
      %17614 = OpExtInst %float %1 FClamp %23302 %float_0 %float_1
      %20293 = OpVectorShuffle %v3float %15677 %15677 0 1 2
      %11188 = OpVectorTimesScalar %v3float %20293 %17614
      %15295 = OpFAdd %v3float %12105 %11188
               OpBranch %22309
      %22309 = OpLabel
       %7721 = OpPhi %v3float %12105 %22308 %15295 %12823
      %23401 = OpVectorTimesScalar %v3float %7721 %float_0_75
       %9341 = OpFAdd %float %9340 %float_0_75
      %16237 = OpVectorShuffle %v3float %15528 %15528 0 1 2
      %22179 = OpFAdd %v3float %16237 %23401
      %15529 = OpVectorShuffle %v4float %15528 %22179 4 5 6 3
       %6436 = OpCompositeInsert %_struct_1017 %15529 %6435 0
      %24574 = OpVectorShuffle %v2float %141 %141 0 1
      %13209 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10347 = OpLoad %v4float %13209
       %8641 = OpVectorShuffle %v2float %10347 %10347 0 1
       %9200 = OpFMul %v2float %24574 %8641
      %18508 = OpFAdd %v2float %19927 %9200
       %7014 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21061 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13152 = OpExtInst %v2float %1 FClamp %18508 %7014 %21061
      %23587 = OpLoad %150 %5785
      %10342 = OpLoad %508 %5688
      %12156 = OpSampledImage %510 %23587 %10342
      %15374 = OpImageSampleExplicitLod %v4float %12156 %13152 Lod %float_0
      %15269 = OpCompositeExtract %float %15374 3
      %12119 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12975 = OpLoad %float %12119
      %15713 = OpFMul %float %15269 %12975
      %15282 = OpExtInst %float %1 FClamp %15713 %float_0 %float_1
      %22216 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11759 = OpLoad %v3float %22216
      %12106 = OpVectorTimesScalar %v3float %11759 %15282
      %15519 = OpLoad %150 %3312
      %24575 = OpLoad %508 %4646
      %12157 = OpSampledImage %510 %15519 %24575
      %17673 = OpImageSampleExplicitLod %v4float %12157 %13152 Lod %float_0
      %16941 = OpCompositeExtract %float %17673 1
      %14188 = OpFOrdGreaterThan %bool %16941 %float_0
               OpSelectionMerge %22310 DontFlatten
               OpBranchConditional %14188 %12824 %22310
      %12824 = OpLabel
      %13242 = OpLoad %150 %4862
      %19963 = OpLoad %508 %3594
      %12158 = OpSampledImage %510 %13242 %19963
      %15678 = OpImageSampleExplicitLod %v4float %12158 %13152 Lod %float_0
      %13869 = OpCompositeExtract %float %17673 1
      %12430 = OpCompositeExtract %float %17673 2
      %23303 = OpFMul %float %13869 %12430
      %17615 = OpExtInst %float %1 FClamp %23303 %float_0 %float_1
      %20294 = OpVectorShuffle %v3float %15678 %15678 0 1 2
      %11189 = OpVectorTimesScalar %v3float %20294 %17615
      %15296 = OpFAdd %v3float %12106 %11189
               OpBranch %22310
      %22310 = OpLabel
       %7722 = OpPhi %v3float %12106 %22309 %15296 %12824
      %23402 = OpVectorTimesScalar %v3float %7722 %float_0_5
       %9342 = OpFAdd %float %9341 %float_0_5
      %16238 = OpVectorShuffle %v3float %15529 %15529 0 1 2
      %22180 = OpFAdd %v3float %16238 %23402
      %15530 = OpVectorShuffle %v4float %15529 %22180 4 5 6 3
       %6437 = OpCompositeInsert %_struct_1017 %15530 %6436 0
      %24576 = OpVectorShuffle %v2float %38 %38 0 1
      %13210 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10348 = OpLoad %v4float %13210
       %8642 = OpVectorShuffle %v2float %10348 %10348 0 1
       %9201 = OpFMul %v2float %24576 %8642
      %18509 = OpFAdd %v2float %19927 %9201
       %7015 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21062 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13153 = OpExtInst %v2float %1 FClamp %18509 %7015 %21062
      %23588 = OpLoad %150 %5785
      %10349 = OpLoad %508 %5688
      %12159 = OpSampledImage %510 %23588 %10349
      %15375 = OpImageSampleExplicitLod %v4float %12159 %13153 Lod %float_0
      %15270 = OpCompositeExtract %float %15375 3
      %12120 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12976 = OpLoad %float %12120
      %15714 = OpFMul %float %15270 %12976
      %15283 = OpExtInst %float %1 FClamp %15714 %float_0 %float_1
      %22217 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11760 = OpLoad %v3float %22217
      %12107 = OpVectorTimesScalar %v3float %11760 %15283
      %15520 = OpLoad %150 %3312
      %24577 = OpLoad %508 %4646
      %12160 = OpSampledImage %510 %15520 %24577
      %17674 = OpImageSampleExplicitLod %v4float %12160 %13153 Lod %float_0
      %16942 = OpCompositeExtract %float %17674 1
      %14189 = OpFOrdGreaterThan %bool %16942 %float_0
               OpSelectionMerge %22311 DontFlatten
               OpBranchConditional %14189 %12825 %22311
      %12825 = OpLabel
      %13243 = OpLoad %150 %4862
      %19964 = OpLoad %508 %3594
      %12161 = OpSampledImage %510 %13243 %19964
      %15679 = OpImageSampleExplicitLod %v4float %12161 %13153 Lod %float_0
      %13870 = OpCompositeExtract %float %17674 1
      %12431 = OpCompositeExtract %float %17674 2
      %23304 = OpFMul %float %13870 %12431
      %17616 = OpExtInst %float %1 FClamp %23304 %float_0 %float_1
      %20295 = OpVectorShuffle %v3float %15679 %15679 0 1 2
      %11190 = OpVectorTimesScalar %v3float %20295 %17616
      %15297 = OpFAdd %v3float %12107 %11190
               OpBranch %22311
      %22311 = OpLabel
       %7723 = OpPhi %v3float %12107 %22310 %15297 %12825
      %23403 = OpVectorTimesScalar %v3float %7723 %float_0_5
       %9343 = OpFAdd %float %9342 %float_0_5
      %16239 = OpVectorShuffle %v3float %15530 %15530 0 1 2
      %22181 = OpFAdd %v3float %16239 %23403
      %15531 = OpVectorShuffle %v4float %15530 %22181 4 5 6 3
       %6438 = OpCompositeInsert %_struct_1017 %15531 %6437 0
      %24578 = OpVectorShuffle %v2float %95 %95 0 1
      %13211 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10350 = OpLoad %v4float %13211
       %8643 = OpVectorShuffle %v2float %10350 %10350 0 1
       %9202 = OpFMul %v2float %24578 %8643
      %18510 = OpFAdd %v2float %19927 %9202
       %7016 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21063 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13154 = OpExtInst %v2float %1 FClamp %18510 %7016 %21063
      %23589 = OpLoad %150 %5785
      %10351 = OpLoad %508 %5688
      %12162 = OpSampledImage %510 %23589 %10351
      %15376 = OpImageSampleExplicitLod %v4float %12162 %13154 Lod %float_0
      %15271 = OpCompositeExtract %float %15376 3
      %12121 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12977 = OpLoad %float %12121
      %15715 = OpFMul %float %15271 %12977
      %15284 = OpExtInst %float %1 FClamp %15715 %float_0 %float_1
      %22218 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11761 = OpLoad %v3float %22218
      %12108 = OpVectorTimesScalar %v3float %11761 %15284
      %15521 = OpLoad %150 %3312
      %24579 = OpLoad %508 %4646
      %12163 = OpSampledImage %510 %15521 %24579
      %17675 = OpImageSampleExplicitLod %v4float %12163 %13154 Lod %float_0
      %16943 = OpCompositeExtract %float %17675 1
      %14190 = OpFOrdGreaterThan %bool %16943 %float_0
               OpSelectionMerge %22312 DontFlatten
               OpBranchConditional %14190 %12826 %22312
      %12826 = OpLabel
      %13244 = OpLoad %150 %4862
      %19965 = OpLoad %508 %3594
      %12164 = OpSampledImage %510 %13244 %19965
      %15680 = OpImageSampleExplicitLod %v4float %12164 %13154 Lod %float_0
      %13871 = OpCompositeExtract %float %17675 1
      %12432 = OpCompositeExtract %float %17675 2
      %23305 = OpFMul %float %13871 %12432
      %17617 = OpExtInst %float %1 FClamp %23305 %float_0 %float_1
      %20296 = OpVectorShuffle %v3float %15680 %15680 0 1 2
      %11191 = OpVectorTimesScalar %v3float %20296 %17617
      %15298 = OpFAdd %v3float %12108 %11191
               OpBranch %22312
      %22312 = OpLabel
       %7724 = OpPhi %v3float %12108 %22311 %15298 %12826
      %23404 = OpVectorTimesScalar %v3float %7724 %float_0_75
       %9344 = OpFAdd %float %9343 %float_0_75
      %16240 = OpVectorShuffle %v3float %15531 %15531 0 1 2
      %22182 = OpFAdd %v3float %16240 %23404
      %15532 = OpVectorShuffle %v4float %15531 %22182 4 5 6 3
       %6439 = OpCompositeInsert %_struct_1017 %15532 %6438 0
      %24580 = OpVectorShuffle %v2float %626 %626 0 1
      %13212 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10352 = OpLoad %v4float %13212
       %8644 = OpVectorShuffle %v2float %10352 %10352 0 1
       %9203 = OpFMul %v2float %24580 %8644
      %18511 = OpFAdd %v2float %19927 %9203
       %7017 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21064 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13155 = OpExtInst %v2float %1 FClamp %18511 %7017 %21064
      %23590 = OpLoad %150 %5785
      %10353 = OpLoad %508 %5688
      %12165 = OpSampledImage %510 %23590 %10353
      %15377 = OpImageSampleExplicitLod %v4float %12165 %13155 Lod %float_0
      %15272 = OpCompositeExtract %float %15377 3
      %12122 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12978 = OpLoad %float %12122
      %15716 = OpFMul %float %15272 %12978
      %15285 = OpExtInst %float %1 FClamp %15716 %float_0 %float_1
      %22219 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11762 = OpLoad %v3float %22219
      %12109 = OpVectorTimesScalar %v3float %11762 %15285
      %15522 = OpLoad %150 %3312
      %24581 = OpLoad %508 %4646
      %12166 = OpSampledImage %510 %15522 %24581
      %17676 = OpImageSampleExplicitLod %v4float %12166 %13155 Lod %float_0
      %16944 = OpCompositeExtract %float %17676 1
      %14191 = OpFOrdGreaterThan %bool %16944 %float_0
               OpSelectionMerge %22313 DontFlatten
               OpBranchConditional %14191 %12827 %22313
      %12827 = OpLabel
      %13245 = OpLoad %150 %4862
      %19966 = OpLoad %508 %3594
      %12167 = OpSampledImage %510 %13245 %19966
      %15681 = OpImageSampleExplicitLod %v4float %12167 %13155 Lod %float_0
      %13872 = OpCompositeExtract %float %17676 1
      %12433 = OpCompositeExtract %float %17676 2
      %23306 = OpFMul %float %13872 %12433
      %17618 = OpExtInst %float %1 FClamp %23306 %float_0 %float_1
      %20297 = OpVectorShuffle %v3float %15681 %15681 0 1 2
      %11192 = OpVectorTimesScalar %v3float %20297 %17618
      %15299 = OpFAdd %v3float %12109 %11192
               OpBranch %22313
      %22313 = OpLabel
       %7725 = OpPhi %v3float %12109 %22312 %15299 %12827
      %23405 = OpVectorTimesScalar %v3float %7725 %float_1
       %9345 = OpFAdd %float %9344 %float_1
      %16241 = OpVectorShuffle %v3float %15532 %15532 0 1 2
      %22183 = OpFAdd %v3float %16241 %23405
      %15533 = OpVectorShuffle %v4float %15532 %22183 4 5 6 3
       %6440 = OpCompositeInsert %_struct_1017 %15533 %6439 0
      %24582 = OpVectorShuffle %v2float %2411 %2411 0 1
      %13213 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10354 = OpLoad %v4float %13213
       %8645 = OpVectorShuffle %v2float %10354 %10354 0 1
       %9204 = OpFMul %v2float %24582 %8645
      %18512 = OpFAdd %v2float %19927 %9204
       %7018 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21065 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13156 = OpExtInst %v2float %1 FClamp %18512 %7018 %21065
      %23591 = OpLoad %150 %5785
      %10355 = OpLoad %508 %5688
      %12168 = OpSampledImage %510 %23591 %10355
      %15378 = OpImageSampleExplicitLod %v4float %12168 %13156 Lod %float_0
      %15273 = OpCompositeExtract %float %15378 3
      %12123 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12979 = OpLoad %float %12123
      %15717 = OpFMul %float %15273 %12979
      %15286 = OpExtInst %float %1 FClamp %15717 %float_0 %float_1
      %22220 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11763 = OpLoad %v3float %22220
      %12110 = OpVectorTimesScalar %v3float %11763 %15286
      %15523 = OpLoad %150 %3312
      %24583 = OpLoad %508 %4646
      %12169 = OpSampledImage %510 %15523 %24583
      %17677 = OpImageSampleExplicitLod %v4float %12169 %13156 Lod %float_0
      %16945 = OpCompositeExtract %float %17677 1
      %14192 = OpFOrdGreaterThan %bool %16945 %float_0
               OpSelectionMerge %22314 DontFlatten
               OpBranchConditional %14192 %12828 %22314
      %12828 = OpLabel
      %13246 = OpLoad %150 %4862
      %19967 = OpLoad %508 %3594
      %12170 = OpSampledImage %510 %13246 %19967
      %15682 = OpImageSampleExplicitLod %v4float %12170 %13156 Lod %float_0
      %13873 = OpCompositeExtract %float %17677 1
      %12434 = OpCompositeExtract %float %17677 2
      %23307 = OpFMul %float %13873 %12434
      %17619 = OpExtInst %float %1 FClamp %23307 %float_0 %float_1
      %20298 = OpVectorShuffle %v3float %15682 %15682 0 1 2
      %11193 = OpVectorTimesScalar %v3float %20298 %17619
      %15300 = OpFAdd %v3float %12110 %11193
               OpBranch %22314
      %22314 = OpLabel
       %7726 = OpPhi %v3float %12110 %22313 %15300 %12828
      %23406 = OpVectorTimesScalar %v3float %7726 %float_0_75
       %9346 = OpFAdd %float %9345 %float_0_75
      %16242 = OpVectorShuffle %v3float %15533 %15533 0 1 2
      %22184 = OpFAdd %v3float %16242 %23406
      %15534 = OpVectorShuffle %v4float %15533 %22184 4 5 6 3
       %6441 = OpCompositeInsert %_struct_1017 %15534 %6440 0
      %24584 = OpVectorShuffle %v2float %2354 %2354 0 1
      %13214 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10356 = OpLoad %v4float %13214
       %8646 = OpVectorShuffle %v2float %10356 %10356 0 1
       %9205 = OpFMul %v2float %24584 %8646
      %18513 = OpFAdd %v2float %19927 %9205
       %7019 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21066 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13157 = OpExtInst %v2float %1 FClamp %18513 %7019 %21066
      %23592 = OpLoad %150 %5785
      %10357 = OpLoad %508 %5688
      %12171 = OpSampledImage %510 %23592 %10357
      %15379 = OpImageSampleExplicitLod %v4float %12171 %13157 Lod %float_0
      %15274 = OpCompositeExtract %float %15379 3
      %12124 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12980 = OpLoad %float %12124
      %15718 = OpFMul %float %15274 %12980
      %15287 = OpExtInst %float %1 FClamp %15718 %float_0 %float_1
      %22221 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11764 = OpLoad %v3float %22221
      %12111 = OpVectorTimesScalar %v3float %11764 %15287
      %15524 = OpLoad %150 %3312
      %24585 = OpLoad %508 %4646
      %12172 = OpSampledImage %510 %15524 %24585
      %17678 = OpImageSampleExplicitLod %v4float %12172 %13157 Lod %float_0
      %16946 = OpCompositeExtract %float %17678 1
      %14193 = OpFOrdGreaterThan %bool %16946 %float_0
               OpSelectionMerge %22315 DontFlatten
               OpBranchConditional %14193 %12829 %22315
      %12829 = OpLabel
      %13247 = OpLoad %150 %4862
      %19968 = OpLoad %508 %3594
      %12173 = OpSampledImage %510 %13247 %19968
      %15683 = OpImageSampleExplicitLod %v4float %12173 %13157 Lod %float_0
      %13874 = OpCompositeExtract %float %17678 1
      %12435 = OpCompositeExtract %float %17678 2
      %23308 = OpFMul %float %13874 %12435
      %17620 = OpExtInst %float %1 FClamp %23308 %float_0 %float_1
      %20299 = OpVectorShuffle %v3float %15683 %15683 0 1 2
      %11194 = OpVectorTimesScalar %v3float %20299 %17620
      %15301 = OpFAdd %v3float %12111 %11194
               OpBranch %22315
      %22315 = OpLabel
       %7727 = OpPhi %v3float %12111 %22314 %15301 %12829
      %23407 = OpVectorTimesScalar %v3float %7727 %float_0_5
       %9347 = OpFAdd %float %9346 %float_0_5
      %16243 = OpVectorShuffle %v3float %15534 %15534 0 1 2
      %22185 = OpFAdd %v3float %16243 %23407
      %15535 = OpVectorShuffle %v4float %15534 %22185 4 5 6 3
       %6442 = OpCompositeInsert %_struct_1017 %15535 %6441 0
      %24586 = OpVectorShuffle %v2float %837 %837 0 1
      %13215 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10358 = OpLoad %v4float %13215
       %8647 = OpVectorShuffle %v2float %10358 %10358 0 1
       %9206 = OpFMul %v2float %24586 %8647
      %18514 = OpFAdd %v2float %19927 %9206
       %7020 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21067 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13158 = OpExtInst %v2float %1 FClamp %18514 %7020 %21067
      %23593 = OpLoad %150 %5785
      %10359 = OpLoad %508 %5688
      %12174 = OpSampledImage %510 %23593 %10359
      %15380 = OpImageSampleExplicitLod %v4float %12174 %13158 Lod %float_0
      %15275 = OpCompositeExtract %float %15380 3
      %12125 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12981 = OpLoad %float %12125
      %15719 = OpFMul %float %15275 %12981
      %15288 = OpExtInst %float %1 FClamp %15719 %float_0 %float_1
      %22222 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11765 = OpLoad %v3float %22222
      %12112 = OpVectorTimesScalar %v3float %11765 %15288
      %15525 = OpLoad %150 %3312
      %24587 = OpLoad %508 %4646
      %12175 = OpSampledImage %510 %15525 %24587
      %17679 = OpImageSampleExplicitLod %v4float %12175 %13158 Lod %float_0
      %16947 = OpCompositeExtract %float %17679 1
      %14194 = OpFOrdGreaterThan %bool %16947 %float_0
               OpSelectionMerge %22316 DontFlatten
               OpBranchConditional %14194 %12830 %22316
      %12830 = OpLabel
      %13248 = OpLoad %150 %4862
      %19969 = OpLoad %508 %3594
      %12176 = OpSampledImage %510 %13248 %19969
      %15684 = OpImageSampleExplicitLod %v4float %12176 %13158 Lod %float_0
      %13875 = OpCompositeExtract %float %17679 1
      %12436 = OpCompositeExtract %float %17679 2
      %23309 = OpFMul %float %13875 %12436
      %17621 = OpExtInst %float %1 FClamp %23309 %float_0 %float_1
      %20300 = OpVectorShuffle %v3float %15684 %15684 0 1 2
      %11195 = OpVectorTimesScalar %v3float %20300 %17621
      %15302 = OpFAdd %v3float %12112 %11195
               OpBranch %22316
      %22316 = OpLabel
       %7728 = OpPhi %v3float %12112 %22315 %15302 %12830
      %23408 = OpVectorTimesScalar %v3float %7728 %float_0_5
       %9348 = OpFAdd %float %9347 %float_0_5
      %16244 = OpVectorShuffle %v3float %15535 %15535 0 1 2
      %22186 = OpFAdd %v3float %16244 %23408
      %15536 = OpVectorShuffle %v4float %15535 %22186 4 5 6 3
       %6443 = OpCompositeInsert %_struct_1017 %15536 %6442 0
      %24588 = OpVectorShuffle %v2float %1368 %1368 0 1
      %13216 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10360 = OpLoad %v4float %13216
       %8648 = OpVectorShuffle %v2float %10360 %10360 0 1
       %9207 = OpFMul %v2float %24588 %8648
      %18515 = OpFAdd %v2float %19927 %9207
       %7021 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21068 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13159 = OpExtInst %v2float %1 FClamp %18515 %7021 %21068
      %23594 = OpLoad %150 %5785
      %10361 = OpLoad %508 %5688
      %12177 = OpSampledImage %510 %23594 %10361
      %15381 = OpImageSampleExplicitLod %v4float %12177 %13159 Lod %float_0
      %15276 = OpCompositeExtract %float %15381 3
      %12126 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12982 = OpLoad %float %12126
      %15720 = OpFMul %float %15276 %12982
      %15289 = OpExtInst %float %1 FClamp %15720 %float_0 %float_1
      %22223 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11766 = OpLoad %v3float %22223
      %12113 = OpVectorTimesScalar %v3float %11766 %15289
      %15526 = OpLoad %150 %3312
      %24589 = OpLoad %508 %4646
      %12178 = OpSampledImage %510 %15526 %24589
      %17680 = OpImageSampleExplicitLod %v4float %12178 %13159 Lod %float_0
      %16948 = OpCompositeExtract %float %17680 1
      %14195 = OpFOrdGreaterThan %bool %16948 %float_0
               OpSelectionMerge %22317 DontFlatten
               OpBranchConditional %14195 %12831 %22317
      %12831 = OpLabel
      %13249 = OpLoad %150 %4862
      %19970 = OpLoad %508 %3594
      %12179 = OpSampledImage %510 %13249 %19970
      %15685 = OpImageSampleExplicitLod %v4float %12179 %13159 Lod %float_0
      %13876 = OpCompositeExtract %float %17680 1
      %12437 = OpCompositeExtract %float %17680 2
      %23310 = OpFMul %float %13876 %12437
      %17622 = OpExtInst %float %1 FClamp %23310 %float_0 %float_1
      %20301 = OpVectorShuffle %v3float %15685 %15685 0 1 2
      %11196 = OpVectorTimesScalar %v3float %20301 %17622
      %15303 = OpFAdd %v3float %12113 %11196
               OpBranch %22317
      %22317 = OpLabel
       %7729 = OpPhi %v3float %12113 %22316 %15303 %12831
      %23409 = OpVectorTimesScalar %v3float %7729 %float_0_75
       %9349 = OpFAdd %float %9348 %float_0_75
      %16245 = OpVectorShuffle %v3float %15536 %15536 0 1 2
      %22187 = OpFAdd %v3float %16245 %23409
      %15537 = OpVectorShuffle %v4float %15536 %22187 4 5 6 3
       %6444 = OpCompositeInsert %_struct_1017 %15537 %6443 0
      %24590 = OpVectorShuffle %v2float %142 %142 0 1
      %13217 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10362 = OpLoad %v4float %13217
       %8649 = OpVectorShuffle %v2float %10362 %10362 0 1
       %9208 = OpFMul %v2float %24590 %8649
      %18516 = OpFAdd %v2float %19927 %9208
       %7022 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21069 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13160 = OpExtInst %v2float %1 FClamp %18516 %7022 %21069
      %23595 = OpLoad %150 %5785
      %10363 = OpLoad %508 %5688
      %12180 = OpSampledImage %510 %23595 %10363
      %15382 = OpImageSampleExplicitLod %v4float %12180 %13160 Lod %float_0
      %15277 = OpCompositeExtract %float %15382 3
      %12127 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12983 = OpLoad %float %12127
      %15721 = OpFMul %float %15277 %12983
      %15290 = OpExtInst %float %1 FClamp %15721 %float_0 %float_1
      %22224 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11767 = OpLoad %v3float %22224
      %12114 = OpVectorTimesScalar %v3float %11767 %15290
      %15538 = OpLoad %150 %3312
      %24591 = OpLoad %508 %4646
      %12181 = OpSampledImage %510 %15538 %24591
      %17681 = OpImageSampleExplicitLod %v4float %12181 %13160 Lod %float_0
      %16949 = OpCompositeExtract %float %17681 1
      %14196 = OpFOrdGreaterThan %bool %16949 %float_0
               OpSelectionMerge %22318 DontFlatten
               OpBranchConditional %14196 %12832 %22318
      %12832 = OpLabel
      %13250 = OpLoad %150 %4862
      %19971 = OpLoad %508 %3594
      %12182 = OpSampledImage %510 %13250 %19971
      %15686 = OpImageSampleExplicitLod %v4float %12182 %13160 Lod %float_0
      %13877 = OpCompositeExtract %float %17681 1
      %12438 = OpCompositeExtract %float %17681 2
      %23311 = OpFMul %float %13877 %12438
      %17623 = OpExtInst %float %1 FClamp %23311 %float_0 %float_1
      %20302 = OpVectorShuffle %v3float %15686 %15686 0 1 2
      %11197 = OpVectorTimesScalar %v3float %20302 %17623
      %15304 = OpFAdd %v3float %12114 %11197
               OpBranch %22318
      %22318 = OpLabel
       %7730 = OpPhi %v3float %12114 %22317 %15304 %12832
      %23410 = OpVectorTimesScalar %v3float %7730 %float_0_5
       %9350 = OpFAdd %float %9349 %float_0_5
      %16246 = OpVectorShuffle %v3float %15537 %15537 0 1 2
      %22188 = OpFAdd %v3float %16246 %23410
      %15539 = OpVectorShuffle %v4float %15537 %22188 4 5 6 3
       %6445 = OpCompositeInsert %_struct_1017 %15539 %6444 0
      %24592 = OpVectorShuffle %v2float %1197 %1197 0 1
      %13218 = OpAccessChain %_ptr_Uniform_v4float %22044 %int_0
      %10364 = OpLoad %v4float %13218
       %8650 = OpVectorShuffle %v2float %10364 %10364 0 1
       %9209 = OpFMul %v2float %24592 %8650
      %18517 = OpFAdd %v2float %19927 %9209
       %7023 = OpVectorShuffle %v2float %17581 %17581 0 1
      %21070 = OpVectorShuffle %v2float %17581 %17581 2 3
      %13161 = OpExtInst %v2float %1 FClamp %18517 %7023 %21070
      %23596 = OpLoad %150 %5785
      %10365 = OpLoad %508 %5688
      %12183 = OpSampledImage %510 %23596 %10365
      %15383 = OpImageSampleExplicitLod %v4float %12183 %13161 Lod %float_0
      %15278 = OpCompositeExtract %float %15383 3
      %12128 = OpAccessChain %_ptr_Uniform_float %22044 %int_1
      %12984 = OpLoad %float %12128
      %15722 = OpFMul %float %15278 %12984
      %15291 = OpExtInst %float %1 FClamp %15722 %float_0 %float_1
      %22225 = OpAccessChain %_ptr_Uniform_v3float %12348 %int_5
      %11768 = OpLoad %v3float %22225
      %12115 = OpVectorTimesScalar %v3float %11768 %15291
      %15540 = OpLoad %150 %3312
      %24593 = OpLoad %508 %4646
      %12184 = OpSampledImage %510 %15540 %24593
      %17682 = OpImageSampleExplicitLod %v4float %12184 %13161 Lod %float_0
      %16950 = OpCompositeExtract %float %17682 1
      %14197 = OpFOrdGreaterThan %bool %16950 %float_0
               OpSelectionMerge %22319 DontFlatten
               OpBranchConditional %14197 %12833 %22319
      %12833 = OpLabel
      %13251 = OpLoad %150 %4862
      %19972 = OpLoad %508 %3594
      %12185 = OpSampledImage %510 %13251 %19972
      %15687 = OpImageSampleExplicitLod %v4float %12185 %13161 Lod %float_0
      %13878 = OpCompositeExtract %float %17682 1
      %12439 = OpCompositeExtract %float %17682 2
      %23312 = OpFMul %float %13878 %12439
      %17624 = OpExtInst %float %1 FClamp %23312 %float_0 %float_1
      %20303 = OpVectorShuffle %v3float %15687 %15687 0 1 2
      %11198 = OpVectorTimesScalar %v3float %20303 %17624
      %15305 = OpFAdd %v3float %12115 %11198
               OpBranch %22319
      %22319 = OpLabel
       %7731 = OpPhi %v3float %12115 %22318 %15305 %12833
      %23411 = OpVectorTimesScalar %v3float %7731 %float_0_5
       %9351 = OpFAdd %float %9350 %float_0_5
      %16247 = OpVectorShuffle %v3float %15539 %15539 0 1 2
      %22189 = OpFAdd %v3float %16247 %23411
      %15541 = OpVectorShuffle %v4float %15539 %22189 4 5 6 3
       %6719 = OpCompositeInsert %_struct_1017 %15541 %6445 0
      %23412 = OpVectorShuffle %v3float %15541 %15541 0 1 2
      %10833 = OpCompositeConstruct %v3float %9351 %9351 %9351
      %13750 = OpFDiv %v3float %23412 %10833
      %24033 = OpVectorShuffle %v4float %15541 %13750 4 5 6 3
       %8636 = OpCompositeInsert %_struct_1017 %24033 %6719 0
      %16315 = OpCompositeInsert %_struct_1017 %float_1 %8636 0 3
      %11544 = OpCompositeExtract %v4float %16315 0
               OpStore %4317 %11544
               OpReturn
               OpFunctionEnd
