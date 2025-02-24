ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, matrix<float16_t, 2, 2> value) {
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
}

matrix<float16_t, 2, 2> tint_symbol_load(uint offset) {
  return matrix<float16_t, 2, 2>(tint_symbol.Load<vector<float16_t, 2> >((offset + 0u)), tint_symbol.Load<vector<float16_t, 2> >((offset + 4u)));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}
