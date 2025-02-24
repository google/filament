[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int3 tint_ftoi(float3 v) {
  return ((v <= (2147483520.0f).xxx) ? ((v < (-2147483648.0f).xxx) ? (-2147483648).xxx : int3(v)) : (2147483647).xxx);
}

static float t = 0.0f;

float3 m() {
  t = 1.0f;
  return float3((t).xxx);
}

void f() {
  float3 tint_symbol = m();
  int3 v = tint_ftoi(tint_symbol);
}
