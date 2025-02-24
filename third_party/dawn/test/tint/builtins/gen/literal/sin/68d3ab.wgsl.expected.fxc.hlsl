//
// fragment_main
//
void sin_68d3ab() {
  float2 res = (1.0f).xx;
}

void fragment_main() {
  sin_68d3ab();
  return;
}
//
// compute_main
//
void sin_68d3ab() {
  float2 res = (1.0f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_68d3ab();
  return;
}
//
// vertex_main
//
void sin_68d3ab() {
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
  sin_68d3ab();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
