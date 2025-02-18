SKIP: INVALID


static float16_t t = float16_t(0.0h);
vector<float16_t, 3> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 3>((t).xxx);
}

uint3 tint_v3f16_to_v3u32(vector<float16_t, 3> value) {
  return (((value <= (float16_t(65504.0h)).xxx)) ? ((((value >= (float16_t(0.0h)).xxx)) ? (uint3(value)) : ((0u).xxx))) : ((4294967295u).xxx));
}

void f() {
  uint3 v = tint_v3f16_to_v3u32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,8-16): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
