// RUN: %dxc -T ps_6_0 -E main -fspv-flatten-resource-arrays -O3  %s -spirv | FileCheck %s

// CHECK: OpName %x_0_ "x[0]"
// CHECK: OpName %x_1_ "x[1]"
// CHECK: OpName %y_0_ "y[0]"
// CHECK: OpName %y_1_ "y[1]"
// CHECK: OpName %y_2_ "y[2]"
// CHECK: OpDecorate %x_0_ DescriptorSet 0
// CHECK: OpDecorate %x_0_ Binding 1
// CHECK: OpDecorate %x_1_ DescriptorSet 0
// CHECK: OpDecorate %x_1_ Binding 2
// CHECK: OpDecorate %y_0_ DescriptorSet 0
// CHECK: OpDecorate %y_0_ Binding 2
// CHECK: OpDecorate %y_1_ DescriptorSet 0
// CHECK: OpDecorate %y_1_ Binding 3
// CHECK: OpDecorate %y_2_ DescriptorSet 0
// CHECK: OpDecorate %y_2_ Binding 4

SamplerState x[2]  : register(s1);
Texture2D    y[3]  : register(t2);

float4 main(uint instanceID : INSTANCEID, float2 texCoord : TEXCOORD) : SV_TARGET
{
// CHECK: [[instanceID:%[0-9]+]] = OpLoad %uint %in_var_INSTANCEID
// CHECK: [[texCoord:%[0-9]+]] = OpLoad %v2float %in_var_TEXCOORD
// CHECK: [[instanceID_idx:%[0-9]+]] = OpUMod %uint [[instanceID]] %uint_2
// CHECK:       OpSelectionMerge [[merge0:%[0-9]+]] None
// CHECK:       OpSwitch [[instanceID_idx]] [[default0:%[0-9]+]] 0 [[sw0_bb0:%[0-9]+]] 1 [[sw0_bb1:%[0-9]+]]
// CHECK: [[sw0_bb0]] = OpLabel
// CHECK:       OpSelectionMerge [[merge1:%[0-9]+]] None
// CHECK:       OpSwitch [[instanceID]] {{%[0-9]+}} 0 {{%[0-9]+}} 1 {{%[0-9]+}} 2
// CHECK:       OpLabel
// CHECK: [[x_0:%[0-9]+]] = OpLoad %type_sampler %x_0_
// CHECK: [[y_0:%[0-9]+]] = OpLoad %type_2d_image %y_0_
// CHECK: [[xy_00:%[0-9]+]] = OpSampledImage %type_sampled_image [[y_0]] [[x_0]]
// CHECK: [[sample0:%[0-9]+]] = OpImageSampleImplicitLod %v4float [[xy_00]] [[texCoord]] None
// CHECK:       OpBranch [[merge1]]
// CHECK:       OpLabel
// CHECK:       OpLoad %type_sampler %x_0_
// CHECK:       OpLoad %type_2d_image %y_1_
// CHECK:       OpSampledImage %type_sampled_image
// CHECK:       OpImageSampleImplicitLod %v4float
// CHECK:       OpBranch [[merge1]]
// CHECK:       OpLabel
// CHECK:       OpLoad %type_sampler %x_0_
// CHECK:       OpLoad %type_2d_image %y_2_
// CHECK:       OpSampledImage %type_sampled_image
// CHECK:       OpImageSampleImplicitLod %v4float
// CHECK:       OpBranch [[merge1]]
// CHECK: [[default1:%[0-9]+]] = OpLabel
// CHECK:       OpBranch [[merge1]]
// CHECK: [[merge1]] = OpLabel
// CHECK:       OpPhi %v4float [[sample0]] {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} [[default1]]
// CHECK:       OpBranch [[merge0]]

// CHECK: [[sw0_bb1]] = OpLabel
// CHECK:       OpSwitch [[instanceID]]

// CHECK: OpLoad %type_sampler %x_1_
// CHECK: OpLoad %type_2d_image %y_0_
// CHECK: OpImageSampleImplicitLod %v4float %59 [[texCoord]] None

// CHECK: OpLoad %type_sampler %x_1_
// CHECK: OpLoad %type_2d_image %y_1_
// CHECK: OpImageSampleImplicitLod %v4float %63 [[texCoord]] None

// CHECK: OpLoad %type_sampler %x_1_
// CHECK: OpLoad %type_2d_image %y_2_
// CHECK: OpImageSampleImplicitLod %v4float %67 [[texCoord]] None

// CHECK: OpPhi %v4float

// CHECK:       OpBranch [[merge0]]
// CHECK: [[default0]] = OpLabel
// CHECK:       OpBranch [[merge0]]
// CHECK: [[merge0]] = OpLabel
// CHECK: [[value:%[0-9]+]] = OpPhi %v4float {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} {{%[0-9]+}} [[default0]]
// CHECK:       OpStore %out_var_SV_TARGET [[value]]

  return y[instanceID].Sample(x[instanceID % 2], texCoord);
}
