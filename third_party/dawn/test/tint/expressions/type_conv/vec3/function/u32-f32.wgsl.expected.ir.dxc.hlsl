
static uint t = 0u;
uint3 m() {
  t = 1u;
  return uint3((t).xxx);
}

void f() {
  float3 v = float3(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

