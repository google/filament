//
// fragment_main
//
struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};
void frexp_eb2421() {
  frexp_result_vec2_f32 res = {(0.5f).xx, (1).xx};
}

void fragment_main() {
  frexp_eb2421();
  return;
}
//
// compute_main
//
struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};
void frexp_eb2421() {
  frexp_result_vec2_f32 res = {(0.5f).xx, (1).xx};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_eb2421();
  return;
}
//
// vertex_main
//
struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};
void frexp_eb2421() {
  frexp_result_vec2_f32 res = {(0.5f).xx, (1).xx};
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
  frexp_eb2421();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
