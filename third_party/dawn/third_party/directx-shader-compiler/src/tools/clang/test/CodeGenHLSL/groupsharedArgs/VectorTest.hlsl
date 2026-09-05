// RUN: %dxc -E main -T cs_6_3 -HV 202x -fcgl %s | FileCheck %s

groupshared float4 SharedData;

// CHECK-LABEL: @"\01?fn1@@YAXAGAV?$vector@M$03@@@Z"
// CHECK: [[Tmp:%.*]] = alloca <1 x float>, align 4
// CHECK: store <1 x float> <float 5.000000e+00>, <1 x float>* [[Tmp]]
// CHECK: [[Z:%.*]] = load <1 x float>, <1 x float>* [[Tmp]]
// CHECK: [[Y:%.*]] = shufflevector <1 x float> [[Z]], <1 x float> undef, <4 x i32> zeroinitializer
// CHECK: store <4 x float> [[Y]], <4 x float> addrspace(3)* %Sh, align 4
void fn1(groupshared float4 Sh) {
  Sh = 5.0.xxxx;
}

[numthreads(4, 1, 1)]
void main(uint3 TID : SV_GroupThreadID) {
  fn1(SharedData);
}
