// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make the loop which not contribute to output is removed.
// CHECK: @main()
// CHECK-NOT:phi

const uint c;
static float g_static;
Buffer<uint> buffer;

struct struct_f3 {
  float3 f3;
};

void do_nothing() {
  struct_f3 rt;
  rt.f3.xyz = 0;
}

float do_branch0(uint u) {
  if (u)
    return 0;
  return 1;
}

float do_branch1(const uint u) {
  float f = 0.F;
  if (u)
    f = max(f, g_static);
  if (u)
    f = max(f, g_static);
  if (u)
    f += g_static;
  return f;
}

void do_complex(inout struct_f3 result, const uint u) {
  if (do_branch1(asuint(buffer[0])) <= 0)
    return;

  float3 d = normalize(0 - asfloat(buffer[0]).xxx);
  float e = dot(d, asfloat(buffer[0]).xxx);
  if (e >= 0) {
    float3 b = d;
    if (((u & 1) ? 0 : asfloat(0)) <= 0)
      b = d;
    float f2 = 1;
    if (e < asfloat(0)) {
      f2 *= pow(max(0, rcp(asfloat(0))), 2);
    }
    do_branch0(asuint(buffer[0]));
    result.f3 += 2 * f2;
  }
}

float4 main(const float4 pos : SV_POSITION) : SV_TARGET {
  uint u = pos.x;
  struct_f3 result;
  result.f3 = 0;
  for (uint i = 0; i < c; ++i) {
    if (1 & u) {
      do_nothing();
      result.f3 += 0.1;
    }
  }
  g_static = result.f3.x;

  for (uint i = 0; i < 1; ++i) {
    if (1 & u) {
      do_complex(result, c);
    }
  }
  for (uint i = 0; i < 1; ++i) {
    if (1 & u) {
      do_complex(result, c);
    }
  }

  return 0;
}