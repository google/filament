struct S {
  float a;
};

[numthreads(1, 1, 1)]
void main() {
  bool bool_var = false;
  bool bool_let = false;
  int i32_var = 0;
  int i32_let = 0;
  uint u32_var = 0u;
  uint u32_let = 0u;
  float f32_var = 0.0f;
  float f32_let = 0.0f;
  int2 v2i32_var = (0).xx;
  int2 v2i32_let = (0).xx;
  uint3 v3u32_var = (0u).xxx;
  uint3 v3u32_let = (0u).xxx;
  float4 v4f32_var = (0.0f).xxxx;
  float4 v4f32_let = (0.0f).xxxx;
  float2x3 m2x3_var = float2x3((0.0f).xxx, (0.0f).xxx);
  float3x4 m3x4_let = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float arr_var[4] = (float[4])0;
  float arr_let[4] = (float[4])0;
  S struct_var = (S)0;
  S struct_let = (S)0;
  return;
}
