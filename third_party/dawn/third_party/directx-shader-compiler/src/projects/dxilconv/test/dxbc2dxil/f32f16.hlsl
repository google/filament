// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main(float4 a : A, uint4 b : B) : SV_TARGET
{
  uint3 q = f32tof16(a.xyw);
  float4 r = f16tof32(b.zyxx);
  return q.xxyz + r;
}
