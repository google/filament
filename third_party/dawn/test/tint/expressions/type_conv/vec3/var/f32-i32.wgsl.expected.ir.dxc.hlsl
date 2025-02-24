
static float3 u = (1.0f).xxx;
int3 tint_v3f32_to_v3i32(float3 value) {
  return (((value <= (2147483520.0f).xxx)) ? ((((value >= (-2147483648.0f).xxx)) ? (int3(value)) : ((int(-2147483648)).xxx))) : ((int(2147483647)).xxx));
}

void f() {
  int3 v = tint_v3f32_to_v3i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

