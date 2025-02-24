// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float2 a[3] : A, float2 b : B, float2 c[6] : C, int d : D, uint SI : SV_SampleIndex) : SV_TARGET
{
  return a[2].y + a[SI].x + c[d].x;// + b.x;
}
