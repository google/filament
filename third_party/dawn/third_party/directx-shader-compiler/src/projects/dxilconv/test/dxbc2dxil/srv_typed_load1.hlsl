// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




Texture2D<float4> srv1 : register(t3);

float4 main(float3 a : A, float2 b : B) : SV_Target
{
  uint status;
  uint2 offset = uint2(-5, 7);
  float4 r = 0;
  r += srv1.Load(a);
  r += srv1[b];
  r += srv1.Load(a, offset);
  r += srv1.Load(a, offset, status); r += status;
  r += srv1.Load(a, uint2(0,0), status); r += status;
  return r;
}
