vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = asuint(src);
  float t_low = f16tof32(v & 0xffff);
  float t_high = f16tof32((v >> 16) & 0xffff);
  return vector<float16_t, 2>(t_low.x, t_high.x);
}

[numthreads(1, 1, 1)]
void f() {
  uint a = 1073757184u;
  vector<float16_t, 2> b = tint_bitcast_to_f16(a);
  return;
}
