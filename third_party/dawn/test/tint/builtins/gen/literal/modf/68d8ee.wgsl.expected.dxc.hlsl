//
// fragment_main
//
struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};
void modf_68d8ee() {
  modf_result_vec3_f32 res = {(-0.5f).xxx, (-1.0f).xxx};
}

void fragment_main() {
  modf_68d8ee();
  return;
}
//
// compute_main
//
struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};
void modf_68d8ee() {
  modf_result_vec3_f32 res = {(-0.5f).xxx, (-1.0f).xxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_68d8ee();
  return;
}
//
// vertex_main
//
struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};
void modf_68d8ee() {
  modf_result_vec3_f32 res = {(-0.5f).xxx, (-1.0f).xxx};
}

struct VertexOutput {
  float4 pos;
};
struct tint_symbol_1 {
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_68d8ee();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
