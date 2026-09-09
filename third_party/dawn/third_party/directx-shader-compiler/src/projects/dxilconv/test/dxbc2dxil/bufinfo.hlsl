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

ByteAddressBuffer buf3;
RWByteAddressBuffer buf4;

Buffer<unorm float2> buf5;
RWBuffer<int3> buf6;

uint main() : SV_Target
{
  uint r = 0, d1, d2;
  buf1.GetDimensions(d1, d2); r += d1 + d2;
  buf2.GetDimensions(d1, d2); r += d1 + d2;
  buf3.GetDimensions(d1); r += d1;
  buf4.GetDimensions(d1); r += d1;
  buf5.GetDimensions(d1); r += d1;
  buf6.GetDimensions(d1); r += d1;
  return r;
}
