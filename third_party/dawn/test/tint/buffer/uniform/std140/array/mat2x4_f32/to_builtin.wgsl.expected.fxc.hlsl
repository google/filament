cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);

float2x4 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 t = transpose(u_load(64u));
  float l = length(asfloat(u[1]).ywxz);
  float a = abs(asfloat(u[1]).ywxz.x);
  s.Store(0u, asuint(((t[0].x + float(l)) + float(a))));
  return;
}
