// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
ByteAddressBuffer buf1;
RWByteAddressBuffer buf2;

float4 main(float a : A, float b : B) : SV_Target
{
  uint status;
  float4 r = 0;
  r += (min16float)buf1.Load(a);
  r.xy += buf1.Load2(a+1);
  r.xyz += buf1.Load3(a+2);
  r += buf1.Load4(a+3);
  r += buf1.Load(a, status); r += status;
  r.xy += buf1.Load(a+1, status); r += status;
  r.xyz += buf1.Load(a+2, status); r += status;
  r += buf1.Load(a+3, status); r += status;

  r += buf2.Load(a);
  r.xy += buf2.Load2(a+1);
  r.xyz += buf2.Load3(a+2);
  r += buf2.Load4(a+3);
  r += buf2.Load(a, status); r += status;
  r.xy += buf2.Load(a+1, status); r += status;
  r.xyz += buf2.Load(a+2, status); r += status;
  r += buf2.Load(a+3, status); r += status;

  buf2.Store(b, r.w);
  buf2.Store2(b+1, r.wz);
  buf2.Store3(b+2, r.wzy);
  buf2.Store4(b+3, r.wzyx);
  return r;
}
