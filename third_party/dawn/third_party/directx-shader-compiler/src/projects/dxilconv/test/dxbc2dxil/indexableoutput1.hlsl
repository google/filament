// FXC command line: fxc /T vs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




struct VSOut
{
  float4 f[7] : F;
  float4 pos : SV_POSITION;
};

uint c;

VSOut main(float a : A, uint b : B)
{
  VSOut Out;
  Out.pos = a;
  Out.f[2] = a;
  return Out;
}
