//
// fragment_main
//
void select_3a14be() {
  bool arg_2 = true;
  int2 res = (arg_2 ? (1).xx : (1).xx);
}

void fragment_main() {
  select_3a14be();
  return;
}
//
// compute_main
//
void select_3a14be() {
  bool arg_2 = true;
  int2 res = (arg_2 ? (1).xx : (1).xx);
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_3a14be();
  return;
}
//
// vertex_main
//
void select_3a14be() {
  bool arg_2 = true;
  int2 res = (arg_2 ? (1).xx : (1).xx);
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
  select_3a14be();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
