// RUN: %dxc -Tlib_6_3 -HV 2021 -verify %s
// RUN: %dxc -Tcs_6_0 -HV 2021 -verify %s

RWStructuredBuffer<int3> rw;

struct FV {
  int3 f;
};

ConstantBuffer<FV> c;

[shader("compute")]
[numthreads(1,1,1)]
void main() {
// expected-error@+1 {{condition for short-circuiting ternary operator must be scalar, for non-scalar types use 'select'}}
  rw[0] = rw[0] ? rw[0] : c.f;
}
