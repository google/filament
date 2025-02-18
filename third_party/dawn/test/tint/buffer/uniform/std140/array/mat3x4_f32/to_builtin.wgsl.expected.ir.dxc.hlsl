
cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1);
float3x4 v(uint start_byte_offset) {
  return float3x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x3 t = transpose(v(96u));
  float l = length(asfloat(u[1u]).ywxz);
  float a = abs(asfloat(u[1u]).ywxz.x);
  float v_1 = (t[0u].x + float(l));
  s.Store(0u, asuint((v_1 + float(a))));
}

