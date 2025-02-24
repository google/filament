SKIP: INVALID

float tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asfloat(uint((r.x & 0xffff) | ((r.y & 0xffff) << 16)));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  float b = tint_bitcast_from_f16(a);
  return;
}
FXC validation failure:
<scrubbed_path>(1,36-44): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(2,29-31): error X3004: undeclared identifier 'src'
<scrubbed_path>(2,22-32): error X3014: incorrect number of arguments to numeric-type constructor


tint executable returned error: exit status 1
