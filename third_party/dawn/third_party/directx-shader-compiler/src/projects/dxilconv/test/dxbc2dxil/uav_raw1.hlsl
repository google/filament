// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




RWByteAddressBuffer uav1 : register(u3);

float4 main(uint a : A, uint b : B) : SV_Target
{
  float4 r = 0;
  uint status;
  r += uav1.Load(a);
  uav1.Load(a, status); r += status;
  uav1.Load(a, status); r += status;
  uav1.Store4(b, r);
  return r;
}
