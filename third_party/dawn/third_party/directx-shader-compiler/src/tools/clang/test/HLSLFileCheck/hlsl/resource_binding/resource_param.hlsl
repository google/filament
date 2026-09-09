// RUN: %dxc -E main -T ps_6_0 -O0 %s | FileCheck %s

// TODO: check pointer of handle later.
// CHECK: %dx.types.Handle

RWBuffer<float4> uav1[2];
RWBuffer<float4> uav2[2];

RWBuffer<float4>  getUav(float4 a)
{
    if (a.x > 1)
       return uav1[1];
    else
       return uav1[0];
}


void getUav(float4 a, out RWBuffer<float4> uav)
{
    if (a.x > 1)
       uav = uav2[0];
    else
       uav = uav2[1];
}

void test_inout(RWBuffer<float4> uav, float4 a)
{
    uav[a.y] = a-1;
}

float4 main(float4 a : A, float4 b:B) : SV_TARGET
{
  RWBuffer<float4> u1 = getUav(a);
  test_inout(u1, a);

  RWBuffer<float4> u2;
  getUav(b, u2);
  test_inout(u2, b);
  return b;
}

