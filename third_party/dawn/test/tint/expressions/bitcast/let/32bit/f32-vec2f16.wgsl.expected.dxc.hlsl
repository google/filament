vector<float16_t, 2> tint_bitcast_to_f16(float src) {
  uint v = asuint(src);
  float t_low = f16tof32(v & 0xffff);
  float t_high = f16tof32((v >> 16) & 0xffff);
  return vector<float16_t, 2>(t_low.x, t_high.x);
}

[numthreads(1, 1, 1)]
void f() {
  float a = 2.003662109375f;
  vector<float16_t, 2> b = tint_bitcast_to_f16(a);
  return;
}
