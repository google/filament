// RUN: %dxc -T ps_6_0 -E main -O3  %s -spirv | FileCheck %s

// Check the names
//
// CHECK: OpName %secondGlobal_t "secondGlobal.t"
// CHECK: OpName %firstGlobal_0__0__t "firstGlobal[0][0].t"
// CHECK: OpName %firstGlobal_0__1__t "firstGlobal[0][1].t"
// CHECK: OpName %firstGlobal_1__0__t "firstGlobal[1][0].t"
// CHECK: OpName %firstGlobal_1__1__t "firstGlobal[1][1].t"
// CHECK: OpName %secondGlobal_tt_0__s "secondGlobal.tt[0].s"
// CHECK: OpName %secondGlobal_tt_1__s "secondGlobal.tt[1].s"
// CHECK: OpName %firstGlobal_0__0__tt_0__s "firstGlobal[0][0].tt[0].s"
// CHECK: OpName %firstGlobal_0__0__tt_1__s "firstGlobal[0][0].tt[1].s"
// CHECK: OpName %firstGlobal_0__1__tt_0__s "firstGlobal[0][1].tt[0].s"
// CHECK: OpName %firstGlobal_0__1__tt_1__s "firstGlobal[0][1].tt[1].s"
// CHECK: OpName %firstGlobal_1__0__tt_0__s "firstGlobal[1][0].tt[0].s"
// CHECK: OpName %firstGlobal_1__0__tt_1__s "firstGlobal[1][0].tt[1].s"
// CHECK: OpName %firstGlobal_1__1__tt_0__s "firstGlobal[1][1].tt[0].s"
// CHECK: OpName %firstGlobal_1__1__tt_1__s "firstGlobal[1][1].tt[1].s"

// Check flattening of bindings
// Explanation: Only the resources that are used will have a binding assignment
// Unused resources will result in gaps in the bindings. This is consistent with
// the behavior of FXC.
// Notes: because resource arrays are not flattened, if one binding is used,
// then the whole arrays remains.
//
// In this example:
//
// firstGlobal[0][0].t[0]        binding: 0  (used)
// firstGlobal[0][0].t[1]        binding: 1  (used)
// firstGlobal[0][0].tt[0].s[0]  binding: 2
// firstGlobal[0][0].tt[0].s[1]  binding: 3  (used)
// firstGlobal[0][0].tt[0].s[2]  binding: 4
// firstGlobal[0][0].tt[1].s[0]  binding: 5
// firstGlobal[0][0].tt[1].s[1]  binding: 6
// firstGlobal[0][0].tt[1].s[2]  binding: 7  (used)
// firstGlobal[0][1].t[0]        binding: 8  (used)
// firstGlobal[0][1].t[1]        binding: 9  (used)
// firstGlobal[0][1].tt[0].s[0]  binding: 10
// firstGlobal[0][1].tt[0].s[1]  binding: 11 (used)
// firstGlobal[0][1].tt[0].s[2]  binding: 12
// firstGlobal[0][1].tt[1].s[0]  binding: 13
// firstGlobal[0][1].tt[1].s[1]  binding: 14
// firstGlobal[0][1].tt[1].s[2]  binding: 15 (used)
// firstGlobal[1][0].t[0]        binding: 16 (used)
// firstGlobal[1][0].t[1]        binding: 17 (used)
// firstGlobal[1][0].tt[0].s[0]  binding: 18
// firstGlobal[1][0].tt[0].s[1]  binding: 19 (used)
// firstGlobal[1][0].tt[0].s[2]  binding: 20
// firstGlobal[1][0].tt[1].s[0]  binding: 21
// firstGlobal[1][0].tt[1].s[1]  binding: 22
// firstGlobal[1][0].tt[1].s[2]  binding: 23 (used)
// firstGlobal[1][1].t[0]        binding: 24 (used)
// firstGlobal[1][1].t[1]        binding: 25 (used)
// firstGlobal[1][1].tt[0].s[0]  binding: 26
// firstGlobal[1][1].tt[0].s[1]  binding: 27 (used)
// firstGlobal[1][1].tt[0].s[2]  binding: 28
// firstGlobal[1][1].tt[1].s[0]  binding: 29
// firstGlobal[1][1].tt[1].s[1]  binding: 30
// firstGlobal[1][1].tt[1].s[2]  binding: 31 (used)
// secondGlobal.t[0]             binding: 32 (used)
// secondGlobal.t[1]             binding: 33 (used)
// secondGlobal.tt[0].s[0]       binding: 34
// secondGlobal.tt[0].s[1]       binding: 35 (used)
// secondGlobal.tt[0].s[2]       binding: 36
// secondGlobal.tt[1].s[0]       binding: 37
// secondGlobal.tt[1].s[1]       binding: 38
// secondGlobal.tt[1].s[2]       binding: 39 (used)
//
// CHECK: OpDecorate %secondGlobal_t DescriptorSet 0
// CHECK: OpDecorate %secondGlobal_t Binding 32
// CHECK: OpDecorate %firstGlobal_0__0__t DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_0__0__t Binding 0
// CHECK: OpDecorate %firstGlobal_0__1__t DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_0__1__t Binding 8
// CHECK: OpDecorate %firstGlobal_1__0__t DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_1__0__t Binding 16
// CHECK: OpDecorate %firstGlobal_1__1__t DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_1__1__t Binding 24
// CHECK: OpDecorate %secondGlobal_tt_0__s DescriptorSet 0
// CHECK: OpDecorate %secondGlobal_tt_0__s Binding 34
// CHECK: OpDecorate %secondGlobal_tt_1__s DescriptorSet 0
// CHECK: OpDecorate %secondGlobal_tt_1__s Binding 37
// CHECK: OpDecorate %firstGlobal_0__0__tt_0__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_0__0__tt_0__s Binding 2
// CHECK: OpDecorate %firstGlobal_0__0__tt_1__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_0__0__tt_1__s Binding 5
// CHECK: OpDecorate %firstGlobal_0__1__tt_0__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_0__1__tt_0__s Binding 10
// CHECK: OpDecorate %firstGlobal_0__1__tt_1__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_0__1__tt_1__s Binding 13
// CHECK: OpDecorate %firstGlobal_1__0__tt_0__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_1__0__tt_0__s Binding 18
// CHECK: OpDecorate %firstGlobal_1__0__tt_1__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_1__0__tt_1__s Binding 21
// CHECK: OpDecorate %firstGlobal_1__1__tt_0__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_1__1__tt_0__s Binding 26
// CHECK: OpDecorate %firstGlobal_1__1__tt_1__s DescriptorSet 0
// CHECK: OpDecorate %firstGlobal_1__1__tt_1__s Binding 29

// Check existence of replacement variables
//
// CHECK:            %secondGlobal_t = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_2 UniformConstant
// CHECK:       %firstGlobal_0__0__t = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_2 UniformConstant
// CHECK:       %firstGlobal_0__1__t = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_2 UniformConstant
// CHECK:       %firstGlobal_1__0__t = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_2 UniformConstant
// CHECK:       %firstGlobal_1__1__t = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_2 UniformConstant
// CHECK:      %secondGlobal_tt_0__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK:      %secondGlobal_tt_1__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_0__0__tt_0__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_0__0__tt_1__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_0__1__tt_0__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_0__1__tt_1__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_1__0__tt_0__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_1__0__tt_1__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_1__1__tt_0__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant
// CHECK: %firstGlobal_1__1__tt_1__s = OpVariable %_ptr_UniformConstant__arr_type_sampler_uint_3 UniformConstant

struct T {
  SamplerState s[3];
};

struct S {
  Texture2D t[2];
  T tt[2];
};

float4 tex2D(S x, float2 v) {
  return x.t[0].Sample(x.tt[0].s[1], v) + x.t[1].Sample(x.tt[1].s[2], v);
}

S firstGlobal[2][2];
S secondGlobal;

float4 main() : SV_Target {
  return
// CHECK:      [[fg_0_0_t:%[0-9]+]] = OpLoad %_arr_type_2d_image_uint_2 %firstGlobal_0__0__t
// CHECK:      [[fg_1_t_0:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_0_0_t]] 0
// CHECK:      [[fg_1_t_1:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_0_0_t]] 1
// CHECK:           [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_0__0__tt_0__s %uint_1
// CHECK: [[fg_1_tt_0_s_1:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK:           [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_0__0__tt_1__s %uint_2
// CHECK: [[fg_1_tt_1_s_2:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK: [[sampled_img_1:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_1_t_0]] [[fg_1_tt_0_s_1]]
// CHECK:               {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_1]]
// CHECK: [[sampled_img_2:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_1_t_1]] [[fg_1_tt_1_s_2]]
// CHECK:               {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_2]]
// CHECK:                             OpFAdd
    tex2D(firstGlobal[0][0], float2(0,0)) +

// CHECK:        [[fg_0_1_t:%[0-9]+]] = OpLoad %_arr_type_2d_image_uint_2 %firstGlobal_0__1__t
// CHECK:      [[fg_0_1_t_0:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_0_1_t]] 0
// CHECK:      [[fg_0_1_t_1:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_0_1_t]] 1
// CHECK:             [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_0__1__tt_0__s %uint_1
// CHECK: [[fg_0_1_tt_0_s_1:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK:             [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_0__1__tt_1__s %uint_2
// CHECK: [[fg_0_1_tt_1_s_2:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK:   [[sampled_img_3:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_0_1_t_0]] [[fg_0_1_tt_0_s_1]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_3]]
// CHECK:   [[sampled_img_4:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_0_1_t_1]] [[fg_0_1_tt_1_s_2]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_4]]
// CHECK:                            OpFAdd
    tex2D(firstGlobal[0][1], float2(0,0)) +
// CHECK:        [[fg_1_0_t:%[0-9]+]] = OpLoad %_arr_type_2d_image_uint_2 %firstGlobal_1__0__t
// CHECK:      [[fg_1_0_t_0:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_1_0_t]] 0
// CHECK:      [[fg_1_0_t_1:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_1_0_t]] 1
// CHECK:             [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_1__0__tt_0__s %uint_1
// CHECK: [[fg_1_0_tt_0_s_1:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK:             [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_1__0__tt_1__s %uint_2
// CHECK: [[fg_1_0_tt_1_s_2:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK:   [[sampled_img_5:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_1_0_t_0]] [[fg_1_0_tt_0_s_1]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_5]]
// CHECK:   [[sampled_img_6:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_1_0_t_1]] [[fg_1_0_tt_1_s_2]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_6]]
// CHECK:                            OpFAdd
    tex2D(firstGlobal[1][0], float2(0,0)) +
// CHECK:        [[fg_1_1_t:%[0-9]+]] = OpLoad %_arr_type_2d_image_uint_2 %firstGlobal_1__1__t
// CHECK:      [[fg_1_1_t_0:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_1_1_t]] 0
// CHECK:      [[fg_1_1_t_1:%[0-9]+]] = OpCompositeExtract %type_2d_image [[fg_1_1_t]] 1
// CHECK:             [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_1__1__tt_0__s %uint_1
// CHECK: [[fg_1_1_tt_0_s_1:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK:             [[tmp:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %firstGlobal_1__1__tt_1__s %uint_2
// CHECK: [[fg_1_1_tt_1_s_2:%[0-9]+]] = OpLoad %type_sampler [[tmp]]
// CHECK:   [[sampled_img_7:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_1_1_t_0]] [[fg_1_1_tt_0_s_1]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_7]]
// CHECK:   [[sampled_img_8:%[0-9]+]] = OpSampledImage %type_sampled_image [[fg_1_1_t_1]] [[fg_1_1_tt_1_s_2]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_8]]
// CHECK:                            OpFAdd
    tex2D(firstGlobal[1][1], float2(0,0)) +
// CHECK:            [[sg_t:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %secondGlobal_t %int_0
// CHECK:          [[sg_t_0:%[0-9]+]] = OpLoad %type_2d_image [[sg_t]]
// CHECK:            [[sg_s:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %secondGlobal_tt_0__s %int_1
// CHECK:     [[sg_tt_0_s_1:%[0-9]+]] = OpLoad %type_sampler [[sg_s]]
// CHECK:   [[sampled_img_9:%[0-9]+]] = OpSampledImage %type_sampled_image [[sg_t_0]] [[sg_tt_0_s_1]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_9]]
// CHECK:                            OpFAdd
    secondGlobal.t[0].Sample(secondGlobal.tt[0].s[1], float2(0,0)) +
// CHECK:            [[sg_t:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %secondGlobal_t %int_1
// CHECK:          [[sg_t_1:%[0-9]+]] = OpLoad %type_2d_image [[sg_t]]
// CHECK:            [[sg_s:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %secondGlobal_tt_1__s %int_2
// CHECK:     [[sg_tt_1_s_2:%[0-9]+]] = OpLoad %type_sampler [[sg_s]]
// CHECK:  [[sampled_img_10:%[0-9]+]] = OpSampledImage %type_sampled_image [[sg_t_1]] [[sg_tt_1_s_2]]
// CHECK:                 {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampled_img_10]]
    secondGlobal.t[1].Sample(secondGlobal.tt[1].s[2], float2(0,0));
}

