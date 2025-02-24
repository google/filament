ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, float16_t value[4]) {
  float16_t array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      tint_symbol_1.Store<float16_t>((offset + (i * 2u)), array_1[i]);
    }
  }
}

typedef float16_t tint_symbol_load_ret[4];
tint_symbol_load_ret tint_symbol_load(uint offset) {
  float16_t arr[4] = (float16_t[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = tint_symbol.Load<float16_t>((offset + (i_1 * 2u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}
