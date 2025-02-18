[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

uint3 tint_ftou(float3 v) {
  return ((v <= (4294967040.0f).xxx) ? ((v < (0.0f).xxx) ? (0u).xxx : uint3(v)) : (4294967295u).xxx);
}

static float3 u = (1.0f).xxx;

void f() {
  uint3 v = tint_ftou(u);
}
