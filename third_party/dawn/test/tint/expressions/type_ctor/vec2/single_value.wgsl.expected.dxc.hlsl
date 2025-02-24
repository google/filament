[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float2 v() {
  return (0.0f).xx;
}

void f() {
  float2 a = (1.0f).xx;
  float2 b = float2(a);
  float2 tint_symbol = v();
  float2 c = float2(tint_symbol);
  float2 d = float2((a * 2.0f));
}
