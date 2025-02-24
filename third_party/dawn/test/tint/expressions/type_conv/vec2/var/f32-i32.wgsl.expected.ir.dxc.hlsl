
static float2 u = (1.0f).xx;
int2 tint_v2f32_to_v2i32(float2 value) {
  return (((value <= (2147483520.0f).xx)) ? ((((value >= (-2147483648.0f).xx)) ? (int2(value)) : ((int(-2147483648)).xx))) : ((int(2147483647)).xx));
}

void f() {
  int2 v = tint_v2f32_to_v2i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

