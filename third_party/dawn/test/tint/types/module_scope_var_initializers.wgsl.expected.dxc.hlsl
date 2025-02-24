struct S {
  float a;
};

static bool bool_var = false;
static int i32_var = 0;
static uint u32_var = 0u;
static float f32_var = 0.0f;
static int2 v2i32_var = (0).xx;
static uint3 v3u32_var = (0u).xxx;
static float4 v4f32_var = (0.0f).xxxx;
static float2x3 m2x3_var = float2x3((0.0f).xxx, (0.0f).xxx);
static float arr_var[4] = (float[4])0;
static S struct_var = (S)0;

[numthreads(1, 1, 1)]
void main() {
  bool_var = false;
  i32_var = 0;
  u32_var = 0u;
  f32_var = 0.0f;
  v2i32_var = (0).xx;
  v3u32_var = (0u).xxx;
  v4f32_var = (0.0f).xxxx;
  m2x3_var = float2x3((0.0f).xxx, (0.0f).xxx);
  float tint_symbol[4] = (float[4])0;
  arr_var = tint_symbol;
  S tint_symbol_1 = (S)0;
  struct_var = tint_symbol_1;
  return;
}
