// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWBuffer<uint>     MyRWBuffer;
RWTexture2D<int>   MyRWTexture;
RWBuffer<uint4>    MyRWBuffer4;
RWTexture3D<uint3> MyRWTexture3;

// CHECK:      [[c25:%[0-9]+]] = OpConstantComposite %v3uint %uint_25 %uint_25 %uint_25
// CHECK:      [[c26:%[0-9]+]] = OpConstantComposite %v4uint %uint_26 %uint_26 %uint_26 %uint_26
// CHECK:      [[c28:%[0-9]+]] = OpConstantComposite %v2uint %uint_28 %uint_28

void main() {
    // <scalar-value>.x
// CHECK:      [[buf:%[0-9]+]] = OpLoad %type_buffer_image %MyRWBuffer
// CHECK-NEXT:                OpImageWrite [[buf]] %uint_1 %uint_15
    MyRWBuffer[1].x = 15;

    // <scalar-value>.x
// CHECK:      [[tex:%[0-9]+]] = OpLoad %type_2d_image %MyRWTexture
// CHECK-NEXT:                OpImageWrite [[tex]] {{%[0-9]+}} %int_16
    MyRWTexture[uint2(2, 3)].x = 16;

    // Out-of-order swizzling
// CHECK:      [[buf_0:%[0-9]+]] = OpLoad %type_buffer_image_0 %MyRWBuffer4
// CHECK-NEXT: [[old:%[0-9]+]] = OpImageRead %v4uint [[buf_0]] %uint_4 None
// CHECK-NEXT: [[new:%[0-9]+]] = OpVectorShuffle %v4uint [[old]] [[c25]] 6 1 4 5
// CHECK-NEXT: [[buf_1:%[0-9]+]] = OpLoad %type_buffer_image_0 %MyRWBuffer4
// CHECK-NEXT:                OpImageWrite [[buf_1]] %uint_4 [[new]]
    MyRWBuffer4[4].zwx = 25;

    // Swizzling resulting in the original vector
// CHECK:      [[buf_2:%[0-9]+]] = OpLoad %type_buffer_image_0 %MyRWBuffer4
// CHECK-NEXT:                OpImageWrite [[buf_2]] %uint_4 [[c26]]
    MyRWBuffer4[4].xyzw = 26;

    // Selecting one element
// CHECK:      [[tex_0:%[0-9]+]] = OpLoad %type_3d_image %MyRWTexture3
// CHECK-NEXT: [[old_0:%[0-9]+]] = OpImageRead %v4uint [[tex_0]] {{%[0-9]+}} None
// CHECK-NEXT:  [[v3:%[0-9]+]] = OpVectorShuffle %v3uint [[old_0]] [[old_0]] 0 1 2
// CHECK-NEXT: [[new_0:%[0-9]+]] = OpCompositeInsert %v3uint %uint_27 [[v3]] 1
// CHECK-NEXT: [[tex_1:%[0-9]+]] = OpLoad %type_3d_image %MyRWTexture3
// CHECK-NEXT:                OpImageWrite [[tex_1]] {{%[0-9]+}} [[new_0]]
    MyRWTexture3[uint3(5, 6, 7)].y = 27;

    // In-order swizzling
// CHECK:      [[tex_2:%[0-9]+]] = OpLoad %type_3d_image %MyRWTexture3
// CHECK-NEXT: [[old_1:%[0-9]+]] = OpImageRead %v4uint [[tex_2]] {{%[0-9]+}} None
// CHECK-NEXT:  [[v3_0:%[0-9]+]] = OpVectorShuffle %v3uint [[old_1]] [[old_1]] 0 1 2
// CHECK-NEXT: [[new_1:%[0-9]+]] = OpVectorShuffle %v3uint [[v3_0]] [[c28]] 3 4 2
// CHECK-NEXT: [[tex_3:%[0-9]+]] = OpLoad %type_3d_image %MyRWTexture3
// CHECK-NEXT:                OpImageWrite [[tex_3]] {{%[0-9]+}} [[new_1]]
    MyRWTexture3[uint3(8, 9, 10)].xy = 28;

// CHECK:      [[buf_3:%[0-9]+]] = OpLoad %type_buffer_image %MyRWBuffer
// CHECK-NEXT: [[old_2:%[0-9]+]] = OpImageRead %v4uint [[buf_3]] %uint_11 None
// CHECK-NEXT: [[val:%[0-9]+]] = OpCompositeExtract %uint [[old_2]] 0
// CHECK-NEXT: [[add:%[0-9]+]] = OpIAdd %uint [[val]] %uint_30
// CHECK-NEXT: [[buf_4:%[0-9]+]] = OpLoad %type_buffer_image %MyRWBuffer
// CHECK-NEXT:                OpImageWrite [[buf_4]] %uint_11 [[add]]
    MyRWBuffer[11] += 30;

// CHECK:      [[tex_4:%[0-9]+]] = OpLoad %type_2d_image %MyRWTexture
// CHECK-NEXT: [[old_3:%[0-9]+]] = OpImageRead %v4int [[tex_4]] {{%[0-9]+}} None
// CHECK-NEXT: [[val_0:%[0-9]+]] = OpCompositeExtract %int [[old_3]] 0
// CHECK-NEXT: [[mul:%[0-9]+]] = OpIMul %int [[val_0]] %int_31
// CHECK-NEXT: [[tex_5:%[0-9]+]] = OpLoad %type_2d_image %MyRWTexture
// CHECK-NEXT:                OpImageWrite [[tex_5]] {{%[0-9]+}} [[mul]]
    MyRWTexture[uint2(12, 13)] *= 31;
}
