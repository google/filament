
void f() {
  matrix<float16_t, 2, 2> v = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

