// RUN: %dxc -E main -T ps_6_0 %s -verify

struct M {
  float m;
};

// expected-error@+2{{recursive functions are not allowed: function 'main' calls recursive function 'test_inout'}}
// expected-note@+1{{recursive function located here:}}
void test_inout(inout M m, float a)
{
    if (a.x > 1)
      test_inout(m, a-1);
    m.m = abs(m.m+a);
}

float4 main(float a : A, float b:B) : SV_TARGET
{
  M m;
  m.m = b;
  test_inout(m, a);
  return m.m;
}

