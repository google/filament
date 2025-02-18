SKIP: INVALID


vector<float16_t, 4> tint_bitcast_to_f16(int2 src) {
  uint2 v = asuint(src);
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

[numthreads(1, 1, 1)]
void f() {
  int2 a = int2(int(1073757184), int(-1006616064));
  vector<float16_t, 4> b = tint_bitcast_to_f16(a);
}

FXC validation failure:
<scrubbed_path>(2,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
