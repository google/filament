SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float16_t v = float16_t(3.0h);
  vector<float16_t, 2> v2 = vector<float16_t, 2>((v).xx);
  vector<float16_t, 3> v3 = vector<float16_t, 3>((v).xxx);
  vector<float16_t, 4> v4 = vector<float16_t, 4>((v).xxxx);
}
FXC validation failure:
<scrubbed_path>(7,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(7,13): error X3000: unrecognized identifier 'v'


tint executable returned error: exit status 1
