
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float2x3 obj) {
  s.Store3((offset + 0u), asuint(obj[0u]));
  s.Store3((offset + 16u), asuint(obj[1u]));
}

float2x3 v_1(uint start_byte_offset) {
  return float2x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, v_1(0u));
  s.Store3(16u, asuint(asfloat(u[0u].xyz)));
  s.Store3(16u, asuint(asfloat(u[0u].xyz).zxy));
  s.Store(4u, asuint(asfloat(u[1u].x)));
}

