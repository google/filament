
static bool2 u = (true).xx;
void f() {
  float2 v = float2(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

