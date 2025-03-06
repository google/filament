// RUN: %dxc -E main -T vs_6_0 -HV 2021 %s | FileCheck %s

// CHECK: @main
struct Vertex
{
    float4 position     : POSITION0;
    float4 color        : COLOR0;
};

struct Interpolants
{
    float4 position     : SV_POSITION0;
    float4 color        : COLOR0;
};


struct T {
  float4 t;
};

struct TA {
  T  ta[2];
};

TA test(T t[2]) {
  TA ta = { t };
  return ta;
}

Interpolants main(  Vertex In)
{
  TA ta = (TA)In;

  return (Interpolants)test(ta.ta);
}
