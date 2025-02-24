// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fptrunc
// CHECK: fptoui
// CHECK: uitofp
double c;
void test_out(out double m) 
{
    m = c+2;
}

void test_inout(inout float m) 
{
    test_out(m);
    m = m+1;
}

float4 main(uint4 a : A) : SV_TARGET
{
  uint x = a.y;
  test_inout(x);
  return x;
}

