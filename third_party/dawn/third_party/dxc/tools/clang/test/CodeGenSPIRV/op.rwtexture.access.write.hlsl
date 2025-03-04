// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWTexture1D        <float>  t1;
RWTexture2D        <int2>   t2;
RWTexture3D        <uint3>  t3;
RWTexture1DArray   <float4> t4;
RWTexture2DArray   <int3>   t5;

// CHECK:   [[ci12:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2
// CHECK:   [[cu01:%[0-9]+]] = OpConstantComposite %v2uint %uint_0 %uint_1
// CHECK:  [[cu123:%[0-9]+]] = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
// CHECK:  [[cu012:%[0-9]+]] = OpConstantComposite %v3uint %uint_0 %uint_1 %uint_2
// CHECK: [[cf1234:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
// CHECK:  [[ci123:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_2 %int_3

void main() {

// CHECK:      [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT: OpImageWrite [[t1]] %uint_0 %float_1
  t1[0] = 1.0;

// CHECK-NEXT: [[t2:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT: OpImageWrite [[t2]] [[cu01]] [[ci12]]
  t2[uint2(0,1)] = int2(1,2);

// CHECK-NEXT: [[t3:%[0-9]+]] = OpLoad %type_3d_image %t3
// CHECK-NEXT: OpImageWrite [[t3]] [[cu012]] [[cu123]]
  t3[uint3(0,1,2)] = uint3(1,2,3);

// CHECK-NEXT: [[t4:%[0-9]+]] = OpLoad %type_1d_image_array %t4
// CHECK-NEXT: OpImageWrite [[t4]] [[cu01]] [[cf1234]]
  t4[uint2(0,1)] = float4(1,2,3,4);

// CHECK-NEXT: [[t5:%[0-9]+]] = OpLoad %type_2d_image_array %t5
// CHECK-NEXT: OpImageWrite [[t5]] [[cu012]] [[ci123]]
  t5[uint3(0,1,2)] = int3(1,2,3);

}
