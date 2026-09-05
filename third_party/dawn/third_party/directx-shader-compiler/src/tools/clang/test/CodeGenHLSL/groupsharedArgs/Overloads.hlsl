// RUN: %dxc -E main -T cs_6_0 -HV 202x -fcgl %s | FileCheck %s

// Verify we are calling the correct overloads
void fn(groupshared float4 Arr[2]);
void fn(inout float4 Arr[2]);

void fn2(groupshared int4 Shared);
void fn2(int4 Local);

// CHECK-LABEL: define void @main()
[numthreads(4,1,1)]
void main() {
  float4 Local[2] = {1.0.xxxx, 2.0.xxxx};
// CHECK: call void @"\01?fn@@YAXY01$$CAV?$vector@M$03@@@Z"([2 x <4 x float>]* %Local)
  fn(Local);

// CHECK: call void @"\01?fn2@@YAXV?$vector@H$03@@@Z"(<4 x i32>
  fn2(11.xxxx);
}

void fn(groupshared float4 Arr[2]) {
  Arr[1] = 7.0.xxxx;
}

// CHECK-LABEL: define internal void @"\01?fn@@YAXY01$$CAV?$vector@M$03@@@Z"([2 x <4 x float>]* noalias %Arr)
void fn(inout float4 Arr[2]) {
  Arr[1] = 5.0.xxxx;
}

void fn2(groupshared int4 Shared) {
  Shared.x = 10;
}

// CHECK-LABEL: define internal void @"\01?fn2@@YAXV?$vector@H$03@@@Z"(<4 x i32> %Local)
void fn2(int4 Local) {
  int X = Local.y;
}
