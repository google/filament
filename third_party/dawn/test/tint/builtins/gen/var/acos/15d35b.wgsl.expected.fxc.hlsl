//
// fragment_main
//
void acos_15d35b() {
  float2 res = (0.25f).xx;
}

void fragment_main() {
  acos_15d35b();
  return;
}
//
// compute_main
//
void acos_15d35b() {
  float2 res = (0.25f).xx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_15d35b();
  return;
}
//
// vertex_main
//
void acos_15d35b() {
  float2 res = (0.25f).xx;
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
  acos_15d35b();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
