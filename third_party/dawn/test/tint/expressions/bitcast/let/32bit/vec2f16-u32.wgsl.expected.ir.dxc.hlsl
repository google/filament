
uint tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return ((r.x & 65535u) | ((r.y & 65535u) << 16u));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  uint b = tint_bitcast_from_f16(a);
}

