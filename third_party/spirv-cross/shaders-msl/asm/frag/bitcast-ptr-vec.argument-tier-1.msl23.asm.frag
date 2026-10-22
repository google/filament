; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 83
; Schema: 0
               OpCapability Shader
               OpCapability Int64
               OpCapability RuntimeDescriptorArray
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_EXT_descriptor_indexing"
               OpExtension "SPV_KHR_physical_storage_buffer"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint Fragment %2 "main" %3 %4
               OpExecutionMode %2 OriginUpperLeft
               OpSource HLSL 600
               OpName %5 "type.ConstantBuffer.PushConstants"
               OpMemberName %5 0 "VertexShaderConstants"
               OpMemberName %5 1 "PixelShaderConstants"
               OpMemberName %5 2 "SharedConstants"
               OpName %6 "g_PushConstants"
               OpName %7 "type.2d.image"
               OpName %8 "g_Texture2DDescriptorHeap"
               OpName %4 "out.var.SV_Target"
               OpName %2 "main"
               OpDecorate %3 BuiltIn FragCoord
               OpDecorate %4 Location 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 0
               OpMemberDecorate %5 0 Offset 0
               OpMemberDecorate %5 1 Offset 8
               OpMemberDecorate %5 2 Offset 16
               OpDecorate %5 Block
          %9 = OpTypeInt 32 0
         %10 = OpTypeInt 32 1
         %11 = OpConstant %10 2
         %12 = OpTypeInt 64 0
         %13 = OpConstant %12 12
         %14 = OpConstant %12 16
         %15 = OpConstant %10 0
         %16 = OpTypeVector %10 2
         %17 = OpConstantComposite %16 %15 %15
         %18 = OpTypeBool
         %19 = OpConstantTrue %18
         %20 = OpConstant %12 24
         %21 = OpTypeFloat 32
         %22 = OpConstant %21 0
         %23 = OpTypeVector %21 4
         %24 = OpConstantComposite %23 %22 %22 %22 %22
          %5 = OpTypeStruct %12 %12 %12
         %25 = OpTypePointer PushConstant %5
          %7 = OpTypeImage %21 2D 2 0 0 1 Unknown
         %26 = OpTypeRuntimeArray %7
         %27 = OpTypePointer UniformConstant %26
         %28 = OpTypePointer Input %23
         %29 = OpTypePointer Output %23
         %30 = OpTypeVoid
         %31 = OpTypeFunction %30
         %32 = OpTypePointer PushConstant %12
         %33 = OpTypePointer PhysicalStorageBuffer %9
         %34 = OpTypePointer UniformConstant %7
         %35 = OpTypeVector %21 2
         %36 = OpTypePointer PhysicalStorageBuffer %16
         %37 = OpTypeVector %18 2
         %38 = OpTypeVector %10 3
         %39 = OpTypeVector %21 3
         %40 = OpTypePointer PhysicalStorageBuffer %39
          %6 = OpVariable %25 PushConstant
          %8 = OpVariable %27 UniformConstant
          %3 = OpVariable %28 Input
          %4 = OpVariable %29 Output
          %2 = OpFunction %30 None %31
         %41 = OpLabel
         %42 = OpLoad %23 %3
         %43 = OpAccessChain %32 %6 %11
         %44 = OpLoad %12 %43
         %45 = OpIAdd %12 %44 %13
         %46 = OpBitcast %33 %45
         %47 = OpLoad %9 %46 Aligned 4
         %48 = OpAccessChain %34 %8 %47
         %49 = OpLoad %7 %48
         %50 = OpVectorShuffle %35 %42 %42 0 1
         %51 = OpConvertFToS %16 %50
         %52 = OpIAdd %12 %44 %14
         %53 = OpBitcast %36 %52
         %54 = OpLoad %16 %53 Aligned 4
         %55 = OpISub %16 %51 %54
         %56 = OpSLessThan %37 %55 %17
         %57 = OpAny %18 %56
         %58 = OpLogicalNot %18 %57
               OpSelectionMerge %59 None
               OpBranchConditional %58 %60 %59
         %60 = OpLabel
         %61 = OpIAdd %12 %44 %20
         %62 = OpBitcast %36 %61
         %63 = OpLoad %16 %62 Aligned 4
         %64 = OpSGreaterThanEqual %37 %55 %63
         %65 = OpAny %18 %64
               OpBranch %59
         %59 = OpLabel
         %66 = OpPhi %18 %19 %41 %65 %60
         %67 = OpCompositeConstruct %37 %66 %66
         %68 = OpSelect %16 %67 %17 %55
               OpSelectionMerge %69 None
               OpBranchConditional %66 %70 %71
         %70 = OpLabel
               OpBranch %69
         %71 = OpLabel
         %72 = OpCompositeExtract %10 %68 0
         %73 = OpCompositeExtract %10 %68 1
         %74 = OpCompositeConstruct %38 %72 %73 %15
         %75 = OpVectorShuffle %16 %74 %74 0 1
         %76 = OpImageFetch %23 %49 %75 Lod %15
               OpBranch %69
         %69 = OpLabel
         %77 = OpPhi %23 %24 %70 %76 %71
         %78 = OpVectorShuffle %39 %77 %77 0 1 2
         %79 = OpBitcast %40 %44
         %80 = OpLoad %39 %79 Aligned 4
         %81 = OpExtInst %39 %1 Pow %78 %80
         %82 = OpVectorShuffle %23 %77 %81 4 5 6 3
               OpStore %4 %82
               OpReturn
               OpFunctionEnd