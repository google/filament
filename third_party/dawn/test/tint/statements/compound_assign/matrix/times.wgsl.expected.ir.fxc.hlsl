
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float4x4 obj) {
  v.Store4((offset + 0u), asuint(obj[0u]));
  v.Store4((offset + 16u), asuint(obj[1u]));
  v.Store4((offset + 32u), asuint(obj[2u]));
  v.Store4((offset + 48u), asuint(obj[3u]));
}

float4x4 v_2(uint offset) {
  return float4x4(asfloat(v.Load4((offset + 0u))), asfloat(v.Load4((offset + 16u))), asfloat(v.Load4((offset + 32u))), asfloat(v.Load4((offset + 48u))));
}

void foo() {
  v_1(0u, mul(float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx), v_2(0u)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

