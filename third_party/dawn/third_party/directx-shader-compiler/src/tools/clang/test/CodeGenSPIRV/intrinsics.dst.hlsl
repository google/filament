// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to the HLSL reference:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb219790(v=vs.85).aspx
//
// dest dst(float4 src0, float4 src1)
//
// dest.x = 1;
// dest.y = src0.y * src1.y;
// dest.z = src0.z;
// dest.w = src1.w;

void main() {
  float4 src0, src1;
  
// CHECK:         [[src0:%[0-9]+]] = OpLoad %v4float %src0
// CHECK-NEXT:    [[src1:%[0-9]+]] = OpLoad %v4float %src1
// CHECK-NEXT:   [[src0y:%[0-9]+]] = OpCompositeExtract %float [[src0]] 1
// CHECK-NEXT:   [[src1y:%[0-9]+]] = OpCompositeExtract %float [[src1]] 1
// CHECK-NEXT:   [[src0z:%[0-9]+]] = OpCompositeExtract %float [[src0]] 2
// CHECK-NEXT:   [[src1w:%[0-9]+]] = OpCompositeExtract %float [[src1]] 3
// CHECK-NEXT: [[resultY:%[0-9]+]] = OpFMul %float [[src0y]] [[src1y]]
// CHECK-NEXT:         {{%[0-9]+}} = OpCompositeConstruct %v4float %float_1 [[resultY]] [[src0z]] [[src1w]]
  float4 result = dst(src0, src1);
}
