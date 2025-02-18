cbuffer cbuffer_u : register(b0) {
  uint4 u[16];
};
RWByteAddressBuffer s : register(u1);
static float4x3 p[4] = (float4x3[4])0;

float4x3 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz), asfloat(u[scalar_offset_2 / 4].xyz), asfloat(u[scalar_offset_3 / 4].xyz));
}

typedef float4x3 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float4x3 arr[4] = (float4x3[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 64u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  p = u_load(0u);
  p[1] = u_load_1(128u);
  p[1][0] = asfloat(u[1].xyz).zxy;
  p[1][0].x = asfloat(u[1].x);
  s.Store(0u, asuint(p[1][0].x));
  return;
}
