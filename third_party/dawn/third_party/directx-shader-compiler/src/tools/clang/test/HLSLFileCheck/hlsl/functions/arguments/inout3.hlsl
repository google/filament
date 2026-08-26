// RUN: %dxc -E main -T ps_6_0 -O0 %s | FileCheck %s

// CHECK: float 3.000000e+00
// CHECK: fptoui
// CHECK: fptoui
// CHECK: uitofp
// CHECK: uitofp
// CHECK: fptoui
// CHECK: fptoui
// CHECK: uitofp
// CHECK: uitofp
// CHECK: float 6.000000e+00
// CHECK: float 6.000000e+00
// CHECK: fptoui
// CHECK: fptoui
// CHECK: uitofp
// CHECK: uitofp

void test_inout_more(inout float2 m) {
  m = 6+m;
}
void test_more(inout uint2 m) {
   m = m+2;
}
float x;

void test_out(out float2 m) {
  m = 3+x;
  test_more(m.yx);
}

void test_inout(inout uint2 m) 
{
    test_out(m.yx);
    m = m+1;

    test_inout_more(m);
}

float4 main(float4 a : A) : SV_TARGET
{
  float3 x = a.ywx;
  test_inout(x.yx);
  return x.xyzy;
}

