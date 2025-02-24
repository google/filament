struct S {
  float a;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


static bool bool_var = false;
static int i32_var = int(0);
static uint u32_var = 0u;
static float f32_var = 0.0f;
static int2 v2i32_var = (int(0)).xx;
static uint3 v3u32_var = (0u).xxx;
static float4 v4f32_var = (0.0f).xxxx;
static float2x3 m2x3_var = float2x3((0.0f).xxx, (0.0f).xxx);
static float arr_var[4] = (float[4])0;
static S struct_var = (S)0;
groupshared float wg_var;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wg_var = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
  bool_var = false;
  i32_var = int(0);
  u32_var = 0u;
  f32_var = 0.0f;
  v2i32_var = (int(0)).xx;
  v3u32_var = (0u).xxx;
  v4f32_var = (0.0f).xxxx;
  m2x3_var = float2x3((0.0f).xxx, (0.0f).xxx);
  float v[4] = (float[4])0;
  arr_var = v;
  S v_1 = (S)0;
  struct_var = v_1;
  wg_var = 42.0f;
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

