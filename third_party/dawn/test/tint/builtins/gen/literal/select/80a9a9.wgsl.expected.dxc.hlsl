//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int select_80a9a9() {
  bool3 res = (true).xxx;
  return (all((res == (false).xxx)) ? 1 : 0);
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(select_80a9a9()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int select_80a9a9() {
  bool3 res = (true).xxx;
  return (all((res == (false).xxx)) ? 1 : 0);
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(select_80a9a9()));
  return;
}
//
// vertex_main
//
int select_80a9a9() {
  bool3 res = (true).xxx;
  return (all((res == (false).xxx)) ? 1 : 0);
}

struct VertexOutput {
  float4 pos;
  int prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation int prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = select_80a9a9();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
