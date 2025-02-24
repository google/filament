ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, float2x2 value) {
  tint_symbol_1.Store2((offset + 0u), asuint(value[0u]));
  tint_symbol_1.Store2((offset + 8u), asuint(value[1u]));
}

float2x2 tint_symbol_load(uint offset) {
  return float2x2(asfloat(tint_symbol.Load2((offset + 0u))), asfloat(tint_symbol.Load2((offset + 8u))));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}
