[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int4 tint_ftoi(float4 v) {
  return ((v <= (2147483520.0f).xxxx) ? ((v < (-2147483648.0f).xxxx) ? (-2147483648).xxxx : int4(v)) : (2147483647).xxxx);
}

static float t = 0.0f;

float4 m() {
  t = 1.0f;
  return float4((t).xxxx);
}

void f() {
  float4 tint_symbol = m();
  int4 v = tint_ftoi(tint_symbol);
}
