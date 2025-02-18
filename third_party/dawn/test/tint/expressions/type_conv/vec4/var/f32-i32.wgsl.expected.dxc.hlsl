[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

int4 tint_ftoi(float4 v) {
  return ((v <= (2147483520.0f).xxxx) ? ((v < (-2147483648.0f).xxxx) ? (-2147483648).xxxx : int4(v)) : (2147483647).xxxx);
}

static float4 u = (1.0f).xxxx;

void f() {
  int4 v = tint_ftoi(u);
}
