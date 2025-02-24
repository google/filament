//
// fragment_main
//
void sign_0799fd() {
  float2 res = (1.0f).xx;
}

void fragment_main() {
  sign_0799fd();
  return;
}
//
// compute_main
//
void sign_0799fd() {
  float2 res = (1.0f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_0799fd();
  return;
}
//
// vertex_main
//
void sign_0799fd() {
  float2 res = (1.0f).xx;
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
  sign_0799fd();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
