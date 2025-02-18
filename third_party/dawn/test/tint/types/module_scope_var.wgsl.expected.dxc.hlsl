groupshared float wg_var;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wg_var = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
}

struct S {
  float a;
};

static bool bool_var = false;
static int i32_var = 0;
static uint u32_var = 0u;
static float f32_var = 0.0f;
static int2 v2i32_var = int2(0, 0);
static uint3 v3u32_var = uint3(0u, 0u, 0u);
static float4 v4f32_var = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float2x3 m2x3_var = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
static float arr_var[4] = (float[4])0;
static S struct_var = (S)0;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  bool_var = false;
  i32_var = 0;
  u32_var = 0u;
  f32_var = 0.0f;
  v2i32_var = (0).xx;
  v3u32_var = (0u).xxx;
  v4f32_var = (0.0f).xxxx;
  m2x3_var = float2x3((0.0f).xxx, (0.0f).xxx);
  float tint_symbol_2[4] = (float[4])0;
  arr_var = tint_symbol_2;
  S tint_symbol_3 = (S)0;
  struct_var = tint_symbol_3;
  wg_var = 42.0f;
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
