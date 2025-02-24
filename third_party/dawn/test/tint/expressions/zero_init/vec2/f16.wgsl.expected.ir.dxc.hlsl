
void f() {
  vector<float16_t, 2> v = (float16_t(0.0h)).xx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

