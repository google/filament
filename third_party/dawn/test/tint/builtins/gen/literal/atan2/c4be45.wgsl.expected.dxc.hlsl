//
// fragment_main
//
void atan2_c4be45() {
  float4 res = (0.78539818525314331055f).xxxx;
}

void fragment_main() {
  atan2_c4be45();
  return;
}
//
// compute_main
//
void atan2_c4be45() {
  float4 res = (0.78539818525314331055f).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_c4be45();
  return;
}
//
// vertex_main
//
void atan2_c4be45() {
  float4 res = (0.78539818525314331055f).xxxx;
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
  atan2_c4be45();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
