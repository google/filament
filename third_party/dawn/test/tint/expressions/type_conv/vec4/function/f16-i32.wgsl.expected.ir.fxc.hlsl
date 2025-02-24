SKIP: INVALID


static float16_t t = float16_t(0.0h);
vector<float16_t, 4> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 4>((t).xxxx);
}

int4 tint_v4f16_to_v4i32(vector<float16_t, 4> value) {
  return (((value <= (float16_t(65504.0h)).xxxx)) ? ((((value >= (float16_t(-65504.0h)).xxxx)) ? (int4(value)) : ((int(-2147483648)).xxxx))) : ((int(2147483647)).xxxx));
}

void f() {
  int4 v = tint_v4f16_to_v4i32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,8-16): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
