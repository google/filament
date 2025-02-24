SKIP: INVALID

vector<float16_t, 4> tint_bitcast_to_f16(int2 src) {
  uint2 v = asuint(src);
  float2 t_low = f16tof32(v & 0xffff);
  float2 t_high = f16tof32((v >> 16) & 0xffff);
  return vector<float16_t, 4>(t_low.x, t_high.x, t_low.y, t_high.y);
}

[numthreads(1, 1, 1)]
void f() {
  int2 a = int2(1073757184, -1006616064);
  vector<float16_t, 4> b = tint_bitcast_to_f16(a);
  return;
}
FXC validation failure:
<scrubbed_path>(1,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
