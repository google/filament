ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, float4x2 value) {
  tint_symbol_1.Store2((offset + 0u), asuint(value[0u]));
  tint_symbol_1.Store2((offset + 8u), asuint(value[1u]));
  tint_symbol_1.Store2((offset + 16u), asuint(value[2u]));
  tint_symbol_1.Store2((offset + 24u), asuint(value[3u]));
}

float4x2 tint_symbol_load(uint offset) {
  return float4x2(asfloat(tint_symbol.Load2((offset + 0u))), asfloat(tint_symbol.Load2((offset + 8u))), asfloat(tint_symbol.Load2((offset + 16u))), asfloat(tint_symbol.Load2((offset + 24u))));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}
