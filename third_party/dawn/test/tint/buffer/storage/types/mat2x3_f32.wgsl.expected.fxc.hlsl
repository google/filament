ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, float2x3 value) {
  tint_symbol_1.Store3((offset + 0u), asuint(value[0u]));
  tint_symbol_1.Store3((offset + 16u), asuint(value[1u]));
}

float2x3 tint_symbol_load(uint offset) {
  return float2x3(asfloat(tint_symbol.Load3((offset + 0u))), asfloat(tint_symbol.Load3((offset + 16u))));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}
