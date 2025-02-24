// RUN: %dxc -T ps_6_0 %s  | FileCheck %s

// Test vector/matrix truncation and splats in constant expressions
// If they remain constant, it should simplify down to just storeOutputs

// CHECK: define void @main
// CHECK: call void @dx.op.storeOutput
// CHECK-NEXT: call void @dx.op.storeOutput
// CHECK-NEXT: call void @dx.op.storeOutput
// CHECK-NEXT: call void @dx.op.storeOutput
// CHECK: ret void
float4 main() : SV_Target
{
  const float val = float4(0.1F, 1, 0, 1);
  const float val2 = 0.2F;
  const float2x2 mat = float3x2(1.1,1.2,2.1,2.2,3.1,3.2);
  const float2x3 mat2 = 5.0;
  return float4(val, val2, mat[0][0], mat2[1][1]);
}
