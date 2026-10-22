// RUN: %dxc -E main -T cs_6_0 -HV 202x -fcgl %s | FileCheck %s

groupshared float4 SharedArr[64];

// CHECK-LABEL: define internal void @"\01?fn@@YAXAGAY0EA@$$CAV?$vector@M$03@@M@Z"([64 x <4 x float>] addrspace(3)* dereferenceable(1024) %Arr, float %F)
// CHECK: [[ArrIdx:%.*]] = getelementptr inbounds [64 x <4 x float>], [64 x <4 x float>] addrspace(3)* %Arr, i32 0, i32 5
// CHECK-NEXT: store <4 x float> {{.*}}, <4 x float> addrspace(3)* [[ArrIdx]], align 4
void fn(groupshared float4 Arr[64], float F) {
  float4 tmp = F.xxxx;
  Arr[5] = tmp;
}

[numthreads(4,1,1)]
void main() {
  fn(SharedArr, 6.0);
}
