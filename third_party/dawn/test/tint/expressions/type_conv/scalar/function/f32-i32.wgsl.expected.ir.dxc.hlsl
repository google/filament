
static float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void f() {
  int v = tint_f32_to_i32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

