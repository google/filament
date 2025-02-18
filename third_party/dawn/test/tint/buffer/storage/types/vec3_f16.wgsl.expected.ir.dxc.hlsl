
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  v_1.Store<vector<float16_t, 3> >(0u, v.Load<vector<float16_t, 3> >(0u));
}

