
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, float2x3 obj) {
  v_1.Store3((offset + 0u), asuint(obj[0u]));
  v_1.Store3((offset + 16u), asuint(obj[1u]));
}

float2x3 v_3(uint offset) {
  return float2x3(asfloat(v.Load3((offset + 0u))), asfloat(v.Load3((offset + 16u))));
}

[numthreads(1, 1, 1)]
void main() {
  v_2(0u, v_3(0u));
}

