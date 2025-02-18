[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int2 tint_ftoi(float2 v) {
  return ((v <= (2147483520.0f).xx) ? ((v < (-2147483648.0f).xx) ? (-2147483648).xx : int2(v)) : (2147483647).xx);
}

static float2 u = (1.0f).xx;

void f() {
  int2 v = tint_ftoi(u);
}
