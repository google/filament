// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

SamplerState s;
Texture2D<float4> t;

float4 g1, g2, g3;

float4 main(float4 coord : COORD) : SV_Target
{
  float4 a = t.Sample(s, coord.xy);
  precise float4 b = a * g1;
  b = mad(b, g2, g3);
  return -b;
}
