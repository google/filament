SKIP: INVALID

RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, matrix<float16_t, 4, 2> value) {
  tint_symbol.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  tint_symbol.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  tint_symbol.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
  tint_symbol.Store<vector<float16_t, 2> >((offset + 12u), value[3u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> m = matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  tint_symbol_store(0u, matrix<float16_t, 4, 2>(m));
  return;
}
FXC validation failure:
<scrubbed_path>(3,44-52): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(4,3-19): error X3018: invalid subscript 'Store'


tint executable returned error: exit status 1
