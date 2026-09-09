// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

Buffer<uint3> b1;
RWBuffer<float4> b2;

void main() {
  uint dim;

// CHECK:          [[b1:%[0-9]+]] = OpLoad %type_buffer_image %b1
// CHECK-NEXT: [[query1:%[0-9]+]] = OpImageQuerySize %uint [[b1]]
// CHECK-NEXT:                   OpStore %dim [[query1]]
  b1.GetDimensions(dim);

// CHECK:          [[b2:%[0-9]+]] = OpLoad %type_buffer_image_0 %b2
// CHECK-NEXT: [[query2:%[0-9]+]] = OpImageQuerySize %uint [[b2]]
// CHECK-NEXT:                   OpStore %dim [[query2]]
  b2.GetDimensions(dim);
}
