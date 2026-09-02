// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main(float4 a : A, int4 b : B, float4 c[6] : C) : SV_Target
{
  float4 r = 0;

  r += EvaluateAttributeAtSample(a, b.x);
  r += EvaluateAttributeAtSample(a, 3);
  r += EvaluateAttributeCentroid(a);
  r += EvaluateAttributeSnapped(a, int2(-2, 5));

  r += EvaluateAttributeAtSample(c[b.y].zzxy, b.x);
  r += EvaluateAttributeCentroid(c[b.z]);
  r += EvaluateAttributeSnapped(c[b.z], int2(-2, 5));

  return r;
}
