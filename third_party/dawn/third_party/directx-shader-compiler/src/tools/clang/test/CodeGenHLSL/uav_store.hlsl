// RUN: %dxc -E main -T ps_6_0 %s

RWByteAddressBuffer uav1;

float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  uav1.Store4(0, 2);
  return 0;
}
