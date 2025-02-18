
static float3 u = (1.0f).xxx;
uint3 tint_v3f32_to_v3u32(float3 value) {
  return (((value <= (4294967040.0f).xxx)) ? ((((value >= (0.0f).xxx)) ? (uint3(value)) : ((0u).xxx))) : ((4294967295u).xxx));
}

void f() {
  uint3 v = tint_v3f32_to_v3u32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

