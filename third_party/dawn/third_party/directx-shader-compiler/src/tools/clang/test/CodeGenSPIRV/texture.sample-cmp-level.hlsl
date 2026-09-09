// RUN: %dxc -T ps_6_7 -spirv -fcgl %s | FileCheck %s

Texture1D<float4> tex1d;
Texture2D<float4> tex2d;
TextureCube<float4> texCube;
TextureCubeArray<float4> texCubeArray;
SamplerComparisonState samplerComparisonState;
RWStructuredBuffer<uint> data;

// CHECK-DAG:                   [[v2f_0_0:%[0-9]+]] = OpConstantComposite %v2float %float_0 %float_0
// CHECK-DAG:                 [[v3f_0_0_0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK-DAG:               [[v4f_0_0_0_0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK-DAG:         [[t_1d_sampled_image:%[^ ]+]] = OpTypeSampledImage %type_1d_image
// CHECK-DAG:         [[t_2d_sampled_image:%[^ ]+]] = OpTypeSampledImage %type_2d_image
// CHECK-DAG:       [[t_cube_sampled_image:%[^ ]+]] = OpTypeSampledImage %type_cube_image
// CHECK-DAG: [[t_cube_array_sampled_image:%[^ ]+]] = OpTypeSampledImage %type_cube_image_array

float4 main() : SV_Target {
// CHECK:           [[texture:%[0-9]+]] = OpLoad %type_1d_image %tex1d
// CHECK-DAG:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG:  [[sampledImage:%[0-9]+]] = OpSampledImage [[t_1d_sampled_image]] [[texture]] [[sampler]]
// CHECK-NEXT:                            OpImageSampleDrefExplicitLod %float [[sampledImage]] %float_1 %float_2 Lod %float_0
  float4 a = tex1d.SampleCmpLevelZero(samplerComparisonState, 1, 2);

// CHECK:           [[texture:%[0-9]+]] = OpLoad %type_1d_image %tex1d
// CHECK-DAG:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG:  [[sampledImage:%[0-9]+]] = OpSampledImage [[t_1d_sampled_image]] [[texture]] [[sampler]]
// CHECK-NEXT:                            OpImageSampleDrefExplicitLod %float [[sampledImage]] %float_1 %float_2 Lod|ConstOffset %float_0 %int_3
  float4 b = tex1d.SampleCmpLevelZero(samplerComparisonState, 1, 2, 3);

// CHECK:          [[texture:%[0-9]+]] = OpLoad %type_1d_image %tex1d
// CHECK-DAG:      [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG: [[sampledImage:%[0-9]+]] = OpSampledImage [[t_1d_sampled_image]] [[texture]] [[sampler]]
// CHECK-DAG:          [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %data %int_0 %uint_0
// CHECK-DAG:          [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK-DAG:       [[offset:%[0-9]+]] = OpBitcast %int [[tmp]]
// CHECK-NEXT:                           OpImageSampleDrefExplicitLod %float [[sampledImage]] %float_1 %float_2 Lod|Offset %float_0 [[offset]]
  float4 c = tex1d.SampleCmpLevelZero(samplerComparisonState, 1, 2, data[0]);

// CHECK:          [[texture:%[0-9]+]] = OpLoad %type_1d_image %tex1d
// CHECK-DAG:      [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG: [[sampledImage:%[0-9]+]] = OpSampledImage [[t_1d_sampled_image]] [[texture]] [[sampler]]
// CHECK-DAG:          [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint %data %int_0 %uint_0
// CHECK-DAG:          [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK-DAG:       [[offset:%[0-9]+]] = OpBitcast %int [[tmp]]
// CHECK-NEXT:         [[tmp:%[0-9]+]] = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[sampledImage]] %float_1 %float_2 Lod|Offset %float_0 [[offset]]
// CHECK-NEXT:          [[res:%[0-9]+]] = OpCompositeExtract %uint [[tmp]] 0
// CHECK-NEXT:                            OpStore %status_0 [[res]]
  uint status_0;
  float4 d = tex1d.SampleCmpLevelZero(samplerComparisonState, 1, 2, data[0], status_0);

// CHECK:           [[texture:%[0-9]+]] = OpLoad %type_2d_image %tex2d
// CHECK-DAG:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG:  [[sampledImage:%[0-9]+]] = OpSampledImage [[t_2d_sampled_image]] [[texture]] [[sampler]]
// CHECK-NEXT:                            OpImageSampleDrefExplicitLod %float [[sampledImage]] [[v2f_0_0]] %float_2 Lod %float_3
  float4 e = tex2d.SampleCmpLevel(samplerComparisonState, float2(0, 0), 2, 3);

// CHECK:           [[texture:%[0-9]+]] = OpLoad %type_cube_image %texCube
// CHECK-DAG:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG:  [[sampledImage:%[0-9]+]] = OpSampledImage [[t_cube_sampled_image]] [[texture]] [[sampler]]
// CHECK-NEXT:                            OpImageSampleDrefExplicitLod %float [[sampledImage]] [[v3f_0_0_0]] %float_1 Lod %float_2
  float4 f = texCube.SampleCmpLevel(samplerComparisonState, float3(0, 0, 0), 1, 2);

// CHECK:           [[texture:%[0-9]+]] = OpLoad %type_cube_image %texCube
// CHECK-DAG:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG:  [[sampledImage:%[0-9]+]] = OpSampledImage [[t_cube_sampled_image]] [[texture]] [[sampler]]
// CHECK-NEXT:          [[tmp:%[0-9]+]] = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[sampledImage]] [[v3f_0_0_0]] %float_1 Lod %float_2
// CHECK-NEXT:          [[res:%[0-9]+]] = OpCompositeExtract %uint [[tmp]] 0
// CHECK-NEXT:                            OpStore %status_1 [[res]]
  uint status_1;
  float4 g = texCube.SampleCmpLevel(samplerComparisonState, float3(0, 0, 0), 1, 2, status_1);

// CHECK:           [[texture:%[0-9]+]] = OpLoad %type_cube_image_array %texCubeArray
// CHECK-DAG:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG:  [[sampledImage:%[0-9]+]] = OpSampledImage [[t_cube_array_sampled_image]] [[texture]] [[sampler]]
// CHECK-NEXT:                            OpImageSampleDrefExplicitLod %float [[sampledImage]] [[v4f_0_0_0_0]] %float_1 Lod %float_2
  float4 h = texCubeArray.SampleCmpLevel(samplerComparisonState, float4(0, 0, 0, 0), 1, 2);

// CHECK:           [[texture:%[0-9]+]] = OpLoad %type_cube_image_array %texCubeArray
// CHECK-DAG:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %samplerComparisonState
// CHECK-DAG:  [[sampledImage:%[0-9]+]] = OpSampledImage [[t_cube_array_sampled_image]] [[texture]] [[sampler]]
// CHECK-NEXT:          [[tmp:%[0-9]+]] = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[sampledImage]] [[v4f_0_0_0_0]] %float_1 Lod %float_2
// CHECK-NEXT:          [[res:%[0-9]+]] = OpCompositeExtract %uint [[tmp]] 0
// CHECK-NEXT:                            OpStore %status_2 [[res]]
  uint status_2;
  float4 i = texCubeArray.SampleCmpLevel(samplerComparisonState, float4(0, 0, 0, 0), 1, 2, status_2);

  return float4(0, 0, 0, 0);
}
