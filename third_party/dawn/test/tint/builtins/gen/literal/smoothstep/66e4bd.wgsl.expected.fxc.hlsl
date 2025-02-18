//
// fragment_main
//
void smoothstep_66e4bd() {
  float3 res = (0.5f).xxx;
}

void fragment_main() {
  smoothstep_66e4bd();
  return;
}
//
// compute_main
//
void smoothstep_66e4bd() {
  float3 res = (0.5f).xxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothstep_66e4bd();
  return;
}
//
// vertex_main
//
void smoothstep_66e4bd() {
  float3 res = (0.5f).xxx;
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
  smoothstep_66e4bd();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
