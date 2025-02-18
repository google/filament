static matrix<float16_t, 4, 3> m = matrix<float16_t, 4, 3>(vector<float16_t, 3>(float16_t(0.0h), float16_t(1.0h), float16_t(2.0h)), vector<float16_t, 3>(float16_t(3.0h), float16_t(4.0h), float16_t(5.0h)), vector<float16_t, 3>(float16_t(6.0h), float16_t(7.0h), float16_t(8.0h)), vector<float16_t, 3>(float16_t(9.0h), float16_t(10.0h), float16_t(11.0h)));
RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, matrix<float16_t, 4, 3> value) {
  tint_symbol.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  tint_symbol.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  tint_symbol.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
  tint_symbol.Store<vector<float16_t, 3> >((offset + 24u), value[3u]);
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol_store(0u, matrix<float16_t, 4, 3>(m));
  return;
}
