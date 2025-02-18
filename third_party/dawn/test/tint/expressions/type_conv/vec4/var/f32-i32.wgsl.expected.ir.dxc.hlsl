
static float4 u = (1.0f).xxxx;
int4 tint_v4f32_to_v4i32(float4 value) {
  return (((value <= (2147483520.0f).xxxx)) ? ((((value >= (-2147483648.0f).xxxx)) ? (int4(value)) : ((int(-2147483648)).xxxx))) : ((int(2147483647)).xxxx));
}

void f() {
  int4 v = tint_v4f32_to_v4i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

