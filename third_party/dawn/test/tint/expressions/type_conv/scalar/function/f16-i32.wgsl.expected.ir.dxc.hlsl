
static float16_t t = float16_t(0.0h);
float16_t m() {
  t = float16_t(1.0h);
  return float16_t(t);
}

int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void f() {
  int v = tint_f16_to_i32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

