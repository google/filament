[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool3 u = (true).xxx;

void f() {
  float3 v = float3(u);
}
