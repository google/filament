// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerState samplerState;
vk::SampledTexture2DArray<float4> tex2darray;

void main() {
    uint mipLevel = 1;

    uint width;
    uint height;
    uint elements;
    uint numLevels;
    uint numSamples;
    float f_width;
    float f_height;
    float f_elements;
    float f_numLevels;
    int i_width;
    int i_height;
    int i_elements;
    int i_numLevels;

// CHECK:      [[img1:%[0-9]+]] = OpLoad %type_sampled_image %tex2darray
// CHECK-NEXT: [[img11:%[0-9]+]] = OpImage %type_2d_image_array [[img1]]
// CHECK-NEXT: [[size1:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[img11]] %int_0
// CHECK-NEXT: [[w1:%[0-9]+]] = OpCompositeExtract %uint [[size1]] 0
// CHECK-NEXT: OpStore %width [[w1]]
// CHECK-NEXT: [[h1:%[0-9]+]] = OpCompositeExtract %uint [[size1]] 1
// CHECK-NEXT: OpStore %height [[h1]]
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeExtract %uint [[size1]] 2
// CHECK-NEXT: OpStore %elements [[e1]]
    tex2darray.GetDimensions(width, height, elements);

// CHECK:      [[img2:%[0-9]+]] = OpLoad %type_sampled_image %tex2darray
// CHECK-NEXT: [[img22:%[0-9]+]] = OpImage %type_2d_image_array [[img2]]
// CHECK-NEXT: [[mip1:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT: [[size2:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[img22]] [[mip1]]
// CHECK-NEXT: [[w2:%[0-9]+]] = OpCompositeExtract %uint [[size2]] 0
// CHECK-NEXT: OpStore %width [[w2]]
// CHECK-NEXT: [[h2:%[0-9]+]] = OpCompositeExtract %uint [[size2]] 1
// CHECK-NEXT: OpStore %height [[h2]]
// CHECK-NEXT: [[e2:%[0-9]+]] = OpCompositeExtract %uint [[size2]] 2
// CHECK-NEXT: OpStore %elements [[e2]]
// CHECK-NEXT: [[lvl2:%[0-9]+]] = OpImageQueryLevels %uint [[img22]]
// CHECK-NEXT: OpStore %numLevels [[lvl2]]
    tex2darray.GetDimensions(mipLevel, width, height, elements, numLevels);

// CHECK:      [[img3:%[0-9]+]] = OpLoad %type_sampled_image %tex2darray
// CHECK-NEXT: [[img33:%[0-9]+]] = OpImage %type_2d_image_array [[img3]]
// CHECK-NEXT: [[size3:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[img33]] %int_0
// CHECK-NEXT: [[w3:%[0-9]+]] = OpCompositeExtract %uint [[size3]] 0
// CHECK-NEXT: [[w3_f:%[0-9]+]] = OpConvertUToF %float [[w3]]
// CHECK-NEXT: OpStore %f_width [[w3_f]]
// CHECK-NEXT: [[h3:%[0-9]+]] = OpCompositeExtract %uint [[size3]] 1
// CHECK-NEXT: [[h3_f:%[0-9]+]] = OpConvertUToF %float [[h3]]
// CHECK-NEXT: OpStore %f_height [[h3_f]]
// CHECK-NEXT: [[e3:%[0-9]+]] = OpCompositeExtract %uint [[size3]] 2
// CHECK-NEXT: [[e3_f:%[0-9]+]] = OpConvertUToF %float [[e3]]
// CHECK-NEXT: OpStore %f_elements [[e3_f]]
    tex2darray.GetDimensions(f_width, f_height, f_elements);

// CHECK:      [[img4:%[0-9]+]] = OpLoad %type_sampled_image %tex2darray
// CHECK-NEXT: [[img44:%[0-9]+]] = OpImage %type_2d_image_array [[img4]]
// CHECK-NEXT: [[mip2:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT: [[size4:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[img44]] [[mip2]]
// CHECK-NEXT: [[w4:%[0-9]+]] = OpCompositeExtract %uint [[size4]] 0
// CHECK-NEXT: [[w4_f:%[0-9]+]] = OpConvertUToF %float [[w4]]
// CHECK-NEXT: OpStore %f_width [[w4_f]]
// CHECK-NEXT: [[h4:%[0-9]+]] = OpCompositeExtract %uint [[size4]] 1
// CHECK-NEXT: [[h4_f:%[0-9]+]] = OpConvertUToF %float [[h4]]
// CHECK-NEXT: OpStore %f_height [[h4_f]]
// CHECK-NEXT: [[e4:%[0-9]+]] = OpCompositeExtract %uint [[size4]] 2
// CHECK-NEXT: OpStore %elements [[e4]]
// CHECK-NEXT: [[lvl4:%[0-9]+]] = OpImageQueryLevels %uint [[img44]]
// CHECK-NEXT: [[lvl4_f:%[0-9]+]] = OpConvertUToF %float [[lvl4]]
// CHECK-NEXT: OpStore %f_numLevels [[lvl4_f]]
    tex2darray.GetDimensions(mipLevel, f_width, f_height, elements, f_numLevels);

// CHECK:      [[img5:%[0-9]+]] = OpLoad %type_sampled_image %tex2darray
// CHECK-NEXT: [[img55:%[0-9]+]] = OpImage %type_2d_image_array [[img5]]
// CHECK-NEXT: [[size5:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[img55]] %int_0
// CHECK-NEXT: [[w5:%[0-9]+]] = OpCompositeExtract %uint [[size5]] 0
// CHECK-NEXT: [[w5_i:%[0-9]+]] = OpBitcast %int [[w5]]
// CHECK-NEXT: OpStore %i_width [[w5_i]]
// CHECK-NEXT: [[h5:%[0-9]+]] = OpCompositeExtract %uint [[size5]] 1
// CHECK-NEXT: [[h5_i:%[0-9]+]] = OpBitcast %int [[h5]]
// CHECK-NEXT: OpStore %i_height [[h5_i]]
// CHECK-NEXT: [[e5:%[0-9]+]] = OpCompositeExtract %uint [[size5]] 2
// CHECK-NEXT: [[e5_i:%[0-9]+]] = OpBitcast %int [[e5]]
// CHECK-NEXT: OpStore %i_elements [[e5_i]]
    tex2darray.GetDimensions(i_width, i_height, i_elements);

// CHECK:      [[img6:%[0-9]+]] = OpLoad %type_sampled_image %tex2darray
// CHECK-NEXT: [[img66:%[0-9]+]] = OpImage %type_2d_image_array [[img6]]
// CHECK-NEXT: [[mip3:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT: [[size6:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[img66]] [[mip3]]
// CHECK-NEXT: [[w6:%[0-9]+]] = OpCompositeExtract %uint [[size6]] 0
// CHECK-NEXT: [[w6_i:%[0-9]+]] = OpBitcast %int [[w6]]
// CHECK-NEXT: OpStore %i_width [[w6_i]]
// CHECK-NEXT: [[h6:%[0-9]+]] = OpCompositeExtract %uint [[size6]] 1
// CHECK-NEXT: [[h6_i:%[0-9]+]] = OpBitcast %int [[h6]]
// CHECK-NEXT: OpStore %i_height [[h6_i]]
// CHECK-NEXT: [[e6:%[0-9]+]] = OpCompositeExtract %uint [[size6]] 2
// CHECK-NEXT: OpStore %elements [[e6]]
// CHECK-NEXT: [[lvl6:%[0-9]+]] = OpImageQueryLevels %uint [[img66]]
// CHECK-NEXT: [[lvl6_i:%[0-9]+]] = OpBitcast %int [[lvl6]]
// CHECK-NEXT: OpStore %i_numLevels [[lvl6_i]]
    tex2darray.GetDimensions(mipLevel, i_width, i_height, elements, i_numLevels);
}
