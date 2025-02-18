//
// fragment_main
//
void cosh_f67ff1() {
  float3 res = (1.0f).xxx;
}

void fragment_main() {
  cosh_f67ff1();
  return;
}
//
// compute_main
//
void cosh_f67ff1() {
  float3 res = (1.0f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cosh_f67ff1();
  return;
}
//
// vertex_main
//
void cosh_f67ff1() {
  float3 res = (1.0f).xxx;
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
  cosh_f67ff1();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
