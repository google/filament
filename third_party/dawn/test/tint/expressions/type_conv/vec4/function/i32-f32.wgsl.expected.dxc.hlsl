[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int t = 0;

int4 m() {
  t = 1;
  return int4((t).xxxx);
}

void f() {
  int4 tint_symbol = m();
  float4 v = float4(tint_symbol);
}
