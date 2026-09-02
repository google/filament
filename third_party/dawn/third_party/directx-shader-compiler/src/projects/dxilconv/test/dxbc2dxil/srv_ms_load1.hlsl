// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




Texture2DMS<float3> srv1 : register(t3);

float3 main(int2 a : A, int c : C, int2 b : B) : SV_Target
{
  uint status;
  uint2 offset = uint2(-5, 7);
  float3 r = 0;
  r += srv1.Load(a, c);
  r += srv1[b];
  r += srv1.Load(a, c, offset);
  r += srv1.Load(a, c, offset, status); r += status;
  r += srv1.Load(a, c, uint2(0,0), status); r += status;
  r += srv1.sample[13][a];
  return r;
}
