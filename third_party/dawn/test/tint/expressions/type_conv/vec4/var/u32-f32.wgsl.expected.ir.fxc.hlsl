
static uint4 u = (1u).xxxx;
void f() {
  float4 v = float4(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

