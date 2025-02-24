// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fptosi float

ByteAddressBuffer A : register(t5);

float main(float3 a : A, int b : B, int N : C) : SV_Target
{
  float r = a.z;

  switch (a.x) {
  case 0:
    r += A.Load(a.y);
    break;

  case 1:
    r += A.Load(a.z);
    break;

  default:
    r += 23;
    break;
  }

  return r*3+2;
}
