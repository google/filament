//
// fragment_main
//
void length_555aba() {
  float res = 0.0f;
}

void fragment_main() {
  length_555aba();
  return;
}
//
// compute_main
//
void length_555aba() {
  float res = 0.0f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_555aba();
  return;
}
//
// vertex_main
//
void length_555aba() {
  float res = 0.0f;
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
  length_555aba();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
