
void f() {
  vector<float16_t, 4> v = (float16_t(0.0h)).xxxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

