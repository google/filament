// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




struct Foo
{
  float2 a;
  float3 b;
  int2 c[4];
};

StructuredBuffer<Foo> buf1;
RWStructuredBuffer<Foo> buf2;

float4 main(float idx1 : Idx1, float idx2 : Idx2) : SV_Target
{
  uint status;
  float4 r = 0;
  r.xy += buf1.Load(idx1).a;
  r.xyz += buf1.Load(idx1).b;
  r.wy += buf1.Load(idx1).c[idx2];
  r.xy += buf1.Load(idx2, status).a; r += status;
  r.xyz += buf1.Load(idx2, status).b; r += status;
  r.wy += buf1.Load(idx2, status).c[idx2]; r += status;

  r.xy += buf2.Load(idx1+200).a;
  r.xyz += buf2.Load(idx1+200).b;
  r.wy += buf2.Load(idx1+200).c[idx2];
  r.xy += buf2.Load(idx2+200, status).a; r += status;
  r.xyz += buf2.Load(idx2+200, status).b; r += status;
  r.wy += buf2.Load(idx2+200, status).c[idx2]; r += status;

  buf2[idx1*3].a = r.xy;
  buf2[idx1*3].b = r.xyz;
  buf2[idx1*3].c[idx2] = r.yw;
  return r;
}
