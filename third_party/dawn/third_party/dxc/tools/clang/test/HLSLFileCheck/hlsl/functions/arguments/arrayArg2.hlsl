// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: @main
float fn_float_arr(float arr[6]) {
  arr[0] = 123.2;
  return arr[0];
}

float4 f4;


[numthreads(8,8,1)]
void main() {

  float arr2[] = { 1.2, 2.2, f4 };
  float m = fn_float_arr(arr2);
}