// RUN: %dxc -T ps_6_0 -E main %s  | FileCheck %s

// Make sure a = {sin(aa) works.
// CHECK: Sin

struct A {
  float2 a;
  int2 b;
};

float2 aa;
int2 bb;
int2 cc;

static const A a = {sin(aa), bb+cc};
static const A a2 = {aa, bb+cc};

float2 main() : SV_TARGET
{
  return a.a + a2.b;
}