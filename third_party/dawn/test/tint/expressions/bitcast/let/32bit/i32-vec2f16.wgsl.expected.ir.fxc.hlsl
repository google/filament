SKIP: INVALID


vector<float16_t, 2> tint_bitcast_to_f16(int src) {
  uint v = asuint(src);
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

[numthreads(1, 1, 1)]
void f() {
  int a = int(1073757184);
  vector<float16_t, 2> b = tint_bitcast_to_f16(a);
}

FXC validation failure:
<scrubbed_path>(2,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
