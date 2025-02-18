
static float4 u = (1.0f).xxxx;
void f() {
  bool4 v = bool4(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

