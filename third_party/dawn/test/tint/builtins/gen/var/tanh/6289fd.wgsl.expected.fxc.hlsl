//
// fragment_main
//
void tanh_6289fd() {
  float3 res = (0.76159417629241943359f).xxx;
}

void fragment_main() {
  tanh_6289fd();
  return;
}
//
// compute_main
//
void tanh_6289fd() {
  float3 res = (0.76159417629241943359f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_6289fd();
  return;
}
//
// vertex_main
//
void tanh_6289fd() {
  float3 res = (0.76159417629241943359f).xxx;
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
  tanh_6289fd();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
