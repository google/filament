[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint3 u = (1u).xxx;

void f() {
  float3 v = float3(u);
}
