
void f() {
  vector<float16_t, 2> v2 = (float16_t(3.0h)).xx;
  vector<float16_t, 3> v3 = (float16_t(3.0h)).xxx;
  vector<float16_t, 4> v4 = (float16_t(3.0h)).xxxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

