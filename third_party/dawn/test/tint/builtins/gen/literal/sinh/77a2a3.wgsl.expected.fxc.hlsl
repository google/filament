//
// fragment_main
//
void sinh_77a2a3() {
  float3 res = (1.17520117759704589844f).xxx;
}

void fragment_main() {
  sinh_77a2a3();
  return;
}
//
// compute_main
//
void sinh_77a2a3() {
  float3 res = (1.17520117759704589844f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sinh_77a2a3();
  return;
}
//
// vertex_main
//
void sinh_77a2a3() {
  float3 res = (1.17520117759704589844f).xxx;
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
  sinh_77a2a3();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
