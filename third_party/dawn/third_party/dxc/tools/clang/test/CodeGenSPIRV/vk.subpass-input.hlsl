// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability InputAttachment

// CHECK:  [[v2i00:%[0-9]+]] = OpConstantComposite %v2int %int_0 %int_0

// CHECK: %type_subpass_image = OpTypeImage %float SubpassData 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_subpass_image = OpTypePointer UniformConstant %type_subpass_image

// CHECK: %type_subpass_image_0 = OpTypeImage %int SubpassData 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_subpass_image_0 = OpTypePointer UniformConstant %type_subpass_image_0

// CHECK: %type_subpass_image_1 = OpTypeImage %uint SubpassData 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_subpass_image_1 = OpTypePointer UniformConstant %type_subpass_image_1

// CHECK: %type_subpass_image_2 = OpTypeImage %uint SubpassData 2 0 1 2 Unknown
// CHECK: %_ptr_UniformConstant_type_subpass_image_2 = OpTypePointer UniformConstant %type_subpass_image_2

// CHECK: %type_subpass_image_3 = OpTypeImage %float SubpassData 2 0 1 2 Unknown
// CHECK: %_ptr_UniformConstant_type_subpass_image_3 = OpTypePointer UniformConstant %type_subpass_image_3

// CHECK: %type_subpass_image_4 = OpTypeImage %int SubpassData 2 0 1 2 Unknown
// CHECK: %_ptr_UniformConstant_type_subpass_image_4 = OpTypePointer UniformConstant %type_subpass_image_4

// CHECK:   %SI_f4 = OpVariable %_ptr_UniformConstant_type_subpass_image UniformConstant
[[vk::input_attachment_index(0)]]  SubpassInput           SI_f4;
// CHECK:   %SI_i3 = OpVariable %_ptr_UniformConstant_type_subpass_image_0 UniformConstant
[[vk::input_attachment_index(1)]]  SubpassInput<int3>     SI_i3;
// CHECK:   %SI_u2 = OpVariable %_ptr_UniformConstant_type_subpass_image_1 UniformConstant
[[vk::input_attachment_index(2)]]  SubpassInput<uint2>    SI_u2;
// CHECK:   %SI_f1 = OpVariable %_ptr_UniformConstant_type_subpass_image UniformConstant
[[vk::input_attachment_index(3)]]  SubpassInput<float>    SI_f1;

// CHECK: %SIMS_u4 = OpVariable %_ptr_UniformConstant_type_subpass_image_2 UniformConstant
[[vk::input_attachment_index(10)]] SubpassInputMS<uint4>  SIMS_u4;
// CHECK: %SIMS_f3 = OpVariable %_ptr_UniformConstant_type_subpass_image_3 UniformConstant
[[vk::input_attachment_index(11)]] SubpassInputMS<float3> SIMS_f3;
// CHECK: %SIMS_i2 = OpVariable %_ptr_UniformConstant_type_subpass_image_4 UniformConstant
[[vk::input_attachment_index(12)]] SubpassInputMS<int2>   SIMS_i2;
// CHECK: %SIMS_u1 = OpVariable %_ptr_UniformConstant_type_subpass_image_2 UniformConstant
[[vk::input_attachment_index(13)]] SubpassInputMS<uint>   SIMS_u1;

float4 ReadSourceFromTile(SubpassInput spi) {
  return spi.SubpassLoad() ;
}

float4 main() : SV_Target {
// CHECK:        [[img:%[0-9]+]] = OpLoad %type_subpass_image %SI_f4
// CHECK-NEXT: [[texel:%[0-9]+]] = OpImageRead %v4float [[img]] [[v2i00]] None
// CHECK-NEXT:                  OpStore %v0 [[texel]]
    float4 v0 = SI_f4.SubpassLoad();
// CHECK:        [[img_0:%[0-9]+]] = OpLoad %type_subpass_image_0 %SI_i3
// CHECK-NEXT: [[texel_0:%[0-9]+]] = OpImageRead %v4int [[img_0]] [[v2i00]] None
// CHECK-NEXT:   [[val:%[0-9]+]] = OpVectorShuffle %v3int [[texel_0]] [[texel_0]] 0 1 2
// CHECK-NEXT:                  OpStore %v1 [[val]]
    int3   v1 = SI_i3.SubpassLoad();
// CHECK:        [[img_1:%[0-9]+]] = OpLoad %type_subpass_image_1 %SI_u2
// CHECK-NEXT: [[texel_1:%[0-9]+]] = OpImageRead %v4uint [[img_1]] [[v2i00]] None
// CHECK-NEXT:   [[val_0:%[0-9]+]] = OpVectorShuffle %v2uint [[texel_1]] [[texel_1]] 0 1
// CHECK-NEXT:                  OpStore %v2 [[val_0]]
    uint2  v2 = SI_u2.SubpassLoad();
// CHECK:        [[img_2:%[0-9]+]] = OpLoad %type_subpass_image %SI_f1
// CHECK-NEXT: [[texel_2:%[0-9]+]] = OpImageRead %v4float [[img_2]] [[v2i00]] None
// CHECK-NEXT:   [[val_1:%[0-9]+]] = OpCompositeExtract %float [[texel_2]] 0
// CHECK-NEXT:                  OpStore %v3 [[val_1]]
    float  v3 = SI_f1.SubpassLoad();
// CHECK:        [[val_2:%[0-9]+]] = OpFunctionCall %v4float %ReadSourceFromTile %param_var_spi
// CHECK-NEXT:                    OpStore %v4 [[val_2]]
    SubpassInput si = SI_f4;
    float4 v4 = ReadSourceFromTile(si);

// CHECK:        [[img_3:%[0-9]+]] = OpLoad %type_subpass_image_2 %SIMS_u4
// CHECK-NEXT: [[texel_3:%[0-9]+]] = OpImageRead %v4uint [[img_3]] [[v2i00]] Sample %int_1
// CHECK-NEXT:                  OpStore %v10 [[texel_3]]
    uint4  v10 = SIMS_u4.SubpassLoad(1);
// CHECK:        [[img_4:%[0-9]+]] = OpLoad %type_subpass_image_3 %SIMS_f3
// CHECK-NEXT: [[texel_4:%[0-9]+]] = OpImageRead %v4float [[img_4]] [[v2i00]] Sample %int_2
// CHECK-NEXT:   [[val_2:%[0-9]+]] = OpVectorShuffle %v3float [[texel_4]] [[texel_4]] 0 1 2
// CHECK-NEXT:                  OpStore %v11 [[val_2]]
    float3 v11 = SIMS_f3.SubpassLoad(2);
// CHECK:        [[img_5:%[0-9]+]] = OpLoad %type_subpass_image_4 %SIMS_i2
// CHECK-NEXT: [[texel_5:%[0-9]+]] = OpImageRead %v4int [[img_5]] [[v2i00]] Sample %int_3
// CHECK-NEXT:   [[val_3:%[0-9]+]] = OpVectorShuffle %v2int [[texel_5]] [[texel_5]] 0 1
// CHECK-NEXT:                  OpStore %v12 [[val_3]]
    int2   v12 = SIMS_i2.SubpassLoad(3);
// CHECK:        [[img_6:%[0-9]+]] = OpLoad %type_subpass_image_2 %SIMS_u1
// CHECK-NEXT: [[texel_6:%[0-9]+]] = OpImageRead %v4uint [[img_6]] [[v2i00]] Sample %int_4
// CHECK-NEXT:   [[val_4:%[0-9]+]] = OpCompositeExtract %uint [[texel_6]] 0
// CHECK-NEXT:                  OpStore %v13 [[val_4]]
    uint   v13 = SIMS_u1.SubpassLoad(4);

    return v0.x + v1.y + v2.x + v3 + v4.x + v10.x + v11.y + v12.x + v13;
}
