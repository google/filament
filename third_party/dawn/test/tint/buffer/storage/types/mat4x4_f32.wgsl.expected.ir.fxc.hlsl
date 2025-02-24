
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, float4x4 obj) {
  v_1.Store4((offset + 0u), asuint(obj[0u]));
  v_1.Store4((offset + 16u), asuint(obj[1u]));
  v_1.Store4((offset + 32u), asuint(obj[2u]));
  v_1.Store4((offset + 48u), asuint(obj[3u]));
}

float4x4 v_3(uint offset) {
  return float4x4(asfloat(v.Load4((offset + 0u))), asfloat(v.Load4((offset + 16u))), asfloat(v.Load4((offset + 32u))), asfloat(v.Load4((offset + 48u))));
}

[numthreads(1, 1, 1)]
void main() {
  v_2(0u, v_3(0u));
}

