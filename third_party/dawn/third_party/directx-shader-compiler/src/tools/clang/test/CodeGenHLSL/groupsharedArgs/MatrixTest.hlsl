// RUN: %dxc -E main -T cs_6_3 -HV 202x -fcgl %s | FileCheck %s

groupshared float4x4 SharedData;

// CHECK-LABEL: @"\01?fn1@@YAXAGAV?$matrix@M$03$03@@@Z"
// CHECK: [[M1:%.*]] = addrspacecast %class.matrix.float.4.4 addrspace(3)* %Sh to %class.matrix.float.4.4*
// CHECK: [[A:%.*]] = call <4 x float>* @"dx.hl.subscript.colMajor[].rn.<4 x float>* (i32, %class.matrix.float.4.4*, i32, i32, i32, i32)"(i32 1, %class.matrix.float.4.4* [[M1]], i32 0, i32 4, i32 8, i32 12)
// CHECK: [[B:%.*]] = getelementptr <4 x float>, <4 x float>* [[A]], i32 0, i32 1
// CHECK: store float 5.000000e+00, float* [[B]]
// CHECK: ret void
void fn1(groupshared float4x4 Sh) {
  Sh[0][1] = 5.0;
}

[numthreads(4,1,1)]
void main(uint3 TID : SV_GroupThreadID) {
  fn1(SharedData);
}
