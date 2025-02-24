
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, matrix<float16_t, 3, 2> obj) {
  v.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  v.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  v.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 3, 2> m = matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  v_1(0u, matrix<float16_t, 3, 2>(m));
}

