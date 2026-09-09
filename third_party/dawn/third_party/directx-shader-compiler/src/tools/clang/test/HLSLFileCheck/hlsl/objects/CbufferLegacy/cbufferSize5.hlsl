// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: error: CBuffer size is 65540 bytes, exceeding maximum of 65536 bytes.

cbuffer Foo1 : register(b5)
{
  float arr[4096];
  float3 after_array;
  float overflow;
}

float4 main() : SV_TARGET
{
  return float4(arr[4095] + overflow, after_array);
}
