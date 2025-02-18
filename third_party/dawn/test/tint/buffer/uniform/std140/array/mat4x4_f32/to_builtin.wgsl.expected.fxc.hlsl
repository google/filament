cbuffer cbuffer_u : register(b0) {
  uint4 u[16];
};
RWByteAddressBuffer s : register(u1);

float4x4 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]), asfloat(u[scalar_offset_2 / 4]), asfloat(u[scalar_offset_3 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x4 t = transpose(u_load(128u));
  float l = length(asfloat(u[1]).ywxz);
  float a = abs(asfloat(u[1]).ywxz.x);
  s.Store(0u, asuint(((t[0].x + float(l)) + float(a))));
  return;
}
