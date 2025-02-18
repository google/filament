
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, float2x2 obj) {
  v_1.Store2((offset + 0u), asuint(obj[0u]));
  v_1.Store2((offset + 8u), asuint(obj[1u]));
}

float2x2 v_3(uint offset) {
  return float2x2(asfloat(v.Load2((offset + 0u))), asfloat(v.Load2((offset + 8u))));
}

[numthreads(1, 1, 1)]
void main() {
  v_2(0u, v_3(0u));
}

