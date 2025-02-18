[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  matrix<float16_t, 3, 3> v = matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
}
