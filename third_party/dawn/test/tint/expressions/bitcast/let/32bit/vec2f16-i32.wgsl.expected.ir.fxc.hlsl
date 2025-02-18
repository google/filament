SKIP: INVALID


int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asint(((r.x & 65535u) | ((r.y & 65535u) << 16u)));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  int b = tint_bitcast_from_f16(a);
}

FXC validation failure:
<scrubbed_path>(2,34-42): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(3,29-31): error X3004: undeclared identifier 'src'
<scrubbed_path>(3,22-32): error X3014: incorrect number of arguments to numeric-type constructor


tint executable returned error: exit status 1
