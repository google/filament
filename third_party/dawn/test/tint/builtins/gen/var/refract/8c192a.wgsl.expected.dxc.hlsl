//
// fragment_main
//
void refract_8c192a() {
  float4 res = (-7.0f).xxxx;
}

void fragment_main() {
  refract_8c192a();
  return;
}
//
// compute_main
//
void refract_8c192a() {
  float4 res = (-7.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  refract_8c192a();
  return;
}
//
// vertex_main
//
void refract_8c192a() {
  float4 res = (-7.0f).xxxx;
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
  refract_8c192a();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
