// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: error: Length is only allowed for HLSL 2016 and lower
// CHECK: error: member reference base type 'float [2]' is not a structure or union
// CHECK-NOT: error:
int fn_int_arr(float arr[2]) {
  return arr.Length + arr.x;
}

[numthreads(8,8,1)]
void main() {

  float arr2[2] = { 1, 2 };
  int m = fn_int_arr(arr2);
}
