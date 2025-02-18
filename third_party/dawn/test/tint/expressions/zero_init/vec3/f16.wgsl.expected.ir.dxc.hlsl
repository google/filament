
void f() {
  vector<float16_t, 3> v = (float16_t(0.0h)).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

