//
// fragment_main
//
void sqrt_4ac2c5() {
  float4 res = (1.0f).xxxx;
}

void fragment_main() {
  sqrt_4ac2c5();
  return;
}
//
// compute_main
//
void sqrt_4ac2c5() {
  float4 res = (1.0f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_4ac2c5();
  return;
}
//
// vertex_main
//
void sqrt_4ac2c5() {
  float4 res = (1.0f).xxxx;
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
  sqrt_4ac2c5();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
