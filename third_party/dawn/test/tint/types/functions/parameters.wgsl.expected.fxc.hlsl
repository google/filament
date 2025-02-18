struct S {
  float a;
};

void foo(bool param_bool, int param_i32, uint param_u32, float param_f32, int2 param_v2i32, uint3 param_v3u32, float4 param_v4f32, float2x3 param_m2x3, float param_arr[4], S param_struct, inout float param_ptr_f32, inout float4 param_ptr_vec, inout float param_ptr_arr[4]) {
}

[numthreads(1, 1, 1)]
void main() {
  float a[4] = (float[4])0;
  float b = 1.0f;
  float4 c = (0.0f).xxxx;
  float d[4] = (float[4])0;
  S tint_symbol = (S)0;
  foo(true, 1, 1u, 1.0f, (3).xx, (4u).xxx, (5.0f).xxxx, float2x3((0.0f).xxx, (0.0f).xxx), a, tint_symbol, b, c, d);
  return;
}
