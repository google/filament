// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




Buffer<float4> buf1 : register(t3);

float4 main(int a : A) : SV_Target
{
  uint status;
  float4 r = 0;
  r += buf1.Load(a);
  r += buf1[a];
  r += buf1.Load(a, status); r += status;
  return r;
}
