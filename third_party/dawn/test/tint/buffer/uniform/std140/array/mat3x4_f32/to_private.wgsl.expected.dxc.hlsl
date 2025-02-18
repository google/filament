cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1);
static float3x4 p[4] = (float3x4[4])0;

float3x4 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]), asfloat(u[scalar_offset_2 / 4]));
}

typedef float3x4 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float3x4 arr[4] = (float3x4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 48u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  p = u_load(0u);
  p[1] = u_load_1(96u);
  p[1][0] = asfloat(u[1]).ywxz;
  p[1][0].x = asfloat(u[1].x);
  s.Store(0u, asuint(p[1][0].x));
  return;
}
