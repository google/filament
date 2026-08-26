// RUN: %dxc -E vecret -T ps_6_0 %s | FileCheck %s
// RUN: %dxc -T lib_6_3 %s | FileCheck %s -check-prefix=CHK_MAT

// verify that the comma operator works with matrix types

// CHECK: dx.op.loadInput
// CHECK-NEXT: dx.op.loadInput
// CHECK-NEXT: dx.op.loadInput
// CHECK-NEXT: dx.op.loadInput

// CHECK-NEXT: dx.op.storeOutput
// CHECK-NEXT: dx.op.storeOutput
// CHECK-NEXT: dx.op.storeOutput
// CHECK-NEXT: dx.op.storeOutput

// CHK_MAT: store %class.matrix.float.2.2
// CHK_MAT: load %class.matrix.float.2.2

// Check where the result IS NOT a matrix type
float4 vecret(float2x2 pmat : M, float4 pvec : V) : SV_Target {
  return pmat , pvec;
}

// Check where the result IS a matrix type
export
float2x2 matret(float2x2 pmat : M, float4 pvec : V) {
  return pmat;
}

