// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fptrunc
// CHECK: fptoui
// CHECK: uitofp

struct E {
};

struct M : E {
};

double c;
void test_out(out double m, E e)
{
    m = c+2;
}

void test_inout(inout float m, M mm)
{
    test_out(m, (E)mm);
    m = m+1;
}

float4 main(uint4 a : A, M m) : SV_TARGET
{
  E e = (E) m;
  uint x = a.y;
  test_inout(x, m);
  return x;
}

