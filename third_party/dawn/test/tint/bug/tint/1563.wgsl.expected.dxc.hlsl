void set_vector_element(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float foo() {
  int oob = 99;
  float b = (0.0f).xxxx[min(uint(oob), 3u)];
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  set_vector_element(v, min(uint(oob), 3u), b);
  return b;
}
