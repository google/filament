
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, matrix<float16_t, 4, 4> obj) {
  v.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  v.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  v.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  v.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 4> m = matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
  v_1(0u, matrix<float16_t, 4, 4>(m));
}

