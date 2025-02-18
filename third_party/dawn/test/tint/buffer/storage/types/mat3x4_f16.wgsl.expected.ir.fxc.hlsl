SKIP: INVALID


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, matrix<float16_t, 3, 4> obj) {
  tint_symbol_1.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  tint_symbol_1.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  tint_symbol_1.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
}

matrix<float16_t, 3, 4> v_1(uint offset) {
  vector<float16_t, 4> v_2 = tint_symbol.Load<vector<float16_t, 4> >((offset + 0u));
  vector<float16_t, 4> v_3 = tint_symbol.Load<vector<float16_t, 4> >((offset + 8u));
  return matrix<float16_t, 3, 4>(v_2, v_3, tint_symbol.Load<vector<float16_t, 4> >((offset + 16u)));
}

[numthreads(1, 1, 1)]
void main() {
  v(0u, v_1(0u));
}

FXC validation failure:
<scrubbed_path>(4,28-36): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(5,3-21): error X3018: invalid subscript 'Store'


tint executable returned error: exit status 1
