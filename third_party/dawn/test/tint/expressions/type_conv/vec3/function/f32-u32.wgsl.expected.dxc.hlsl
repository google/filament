[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

uint3 tint_ftou(float3 v) {
  return ((v <= (4294967040.0f).xxx) ? ((v < (0.0f).xxx) ? (0u).xxx : uint3(v)) : (4294967295u).xxx);
}

static float t = 0.0f;

float3 m() {
  t = 1.0f;
  return float3((t).xxx);
}

void f() {
  float3 tint_symbol = m();
  uint3 v = tint_ftou(tint_symbol);
}
