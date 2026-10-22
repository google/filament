// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: @main
float2 fn_float_arr(float2 arr[2][3]) {
  arr[0][1] = 123.2;
  return arr[0][1];
}

float4 f4;
float2x3 m;

[numthreads(8,8,1)]
void main() {

  float2 arr2[][3] = { 1.2, 2.2, f4, m };
  float fn = fn_float_arr(arr2);
}