
static float t = 0.0f;
float m() {
  t = 1.0f;
  return float(t);
}

uint tint_f32_to_u32(float value) {
  return (((value <= 4294967040.0f)) ? ((((value >= 0.0f)) ? (uint(value)) : (0u))) : (4294967295u));
}

void f() {
  uint v = tint_f32_to_u32(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

