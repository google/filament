SKIP: INVALID


float16_t get_f16() {
  return float16_t(1.0h);
}

void f() {
  vector<float16_t, 2> v2 = vector<float16_t, 2>((get_f16()).xx);
  vector<float16_t, 3> v3 = vector<float16_t, 3>((get_f16()).xxx);
  vector<float16_t, 4> v4 = vector<float16_t, 4>((get_f16()).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
