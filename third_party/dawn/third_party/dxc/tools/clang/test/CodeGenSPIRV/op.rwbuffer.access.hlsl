// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageBuffer

RWBuffer<int> intbuf;
RWBuffer<uint> uintbuf;
RWBuffer<float> floatbuf;
RWBuffer<int3> int3buf;
RWBuffer<uint3> uint3buf;
RWBuffer<float3> float3buf;

// CHECK: [[int_1_2_3:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_2 %int_3
// CHECK: [[uint_4_5_6:%[0-9]+]] = OpConstantComposite %v3uint %uint_4 %uint_5 %uint_6
// CHECK: [[float_7_8_9:%[0-9]+]] = OpConstantComposite %v3float %float_7 %float_8 %float_9

void main() {

// CHECK:      [[img1:%[0-9]+]] = OpLoad %type_buffer_image %intbuf
// CHECK-NEXT: OpImageWrite [[img1]] %uint_1 %int_1
  intbuf[1] = int(1);

// CHECK:      [[img2:%[0-9]+]] = OpLoad %type_buffer_image_0 %uintbuf
// CHECK-NEXT: OpImageWrite [[img2]] %uint_2 %uint_2
  uintbuf[2] = uint(2);

// CHECK:      [[img3:%[0-9]+]] = OpLoad %type_buffer_image_1 %floatbuf
// CHECK-NEXT: OpImageWrite [[img3]] %uint_3 %float_3
  floatbuf[3] = float(3);

// CHECK:      [[img7:%[0-9]+]] = OpLoad %type_buffer_image_2 %int3buf
// CHECK-NEXT: OpImageWrite [[img7]] %uint_7 [[int_1_2_3]]
  int3buf[7] = int3(1,2,3);

// CHECK:      [[img8:%[0-9]+]] = OpLoad %type_buffer_image_3 %uint3buf
// CHECK-NEXT: OpImageWrite [[img8]] %uint_8 [[uint_4_5_6]]
  uint3buf[8] = uint3(4,5,6);

// CHECK:      [[img9:%[0-9]+]] = OpLoad %type_buffer_image_4 %float3buf
// CHECK-NEXT: OpImageWrite [[img9]] %uint_9 [[float_7_8_9]]
  float3buf[9] = float3(7,8,9);
}
