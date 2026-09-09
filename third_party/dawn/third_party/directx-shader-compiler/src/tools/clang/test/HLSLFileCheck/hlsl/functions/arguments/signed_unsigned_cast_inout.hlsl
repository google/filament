// RUN: %dxc -E main -T ps_6_0 %s

float foo(inout int c) {

  return c;
}

float main(uint i:I) : SV_Target {
  float f = foo(i);
  return f + i;
}
