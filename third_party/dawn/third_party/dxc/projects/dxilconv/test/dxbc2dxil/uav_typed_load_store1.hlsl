// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




RWTexture2D<float4> uav1 : register(u3);

float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  float4 r = 0;
  uint status;
  r += uav1[b];
  r += uav1.Load(a);
  uav1.Load(a, status); r += status;
  uav1.Load(a, status); r += status;
  uav1[b] = r;
  return r;
}
