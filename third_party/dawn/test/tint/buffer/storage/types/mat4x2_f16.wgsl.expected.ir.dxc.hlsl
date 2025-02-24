
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, matrix<float16_t, 4, 2> obj) {
  v_1.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  v_1.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  v_1.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  v_1.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

matrix<float16_t, 4, 2> v_3(uint offset) {
  return matrix<float16_t, 4, 2>(v.Load<vector<float16_t, 2> >((offset + 0u)), v.Load<vector<float16_t, 2> >((offset + 4u)), v.Load<vector<float16_t, 2> >((offset + 8u)), v.Load<vector<float16_t, 2> >((offset + 12u)));
}

[numthreads(1, 1, 1)]
void main() {
  v_2(0u, v_3(0u));
}

