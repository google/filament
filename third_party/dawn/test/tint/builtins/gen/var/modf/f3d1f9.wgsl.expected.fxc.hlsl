//
// fragment_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
void modf_f3d1f9() {
  modf_result_vec4_f32 res = {(-0.5f).xxxx, (-1.0f).xxxx};
}

void fragment_main() {
  modf_f3d1f9();
  return;
}
//
// compute_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
void modf_f3d1f9() {
  modf_result_vec4_f32 res = {(-0.5f).xxxx, (-1.0f).xxxx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_f3d1f9();
  return;
}
//
// vertex_main
//
struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};
void modf_f3d1f9() {
  modf_result_vec4_f32 res = {(-0.5f).xxxx, (-1.0f).xxxx};
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
  modf_f3d1f9();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
