SKIP: INVALID

struct frexp_result_vec3_f16 {
  vector<float16_t, 3> fract;
  int3 exp;
};
void frexp_ae4a66() {
  frexp_result_vec3_f16 res = {(float16_t(0.5h)).xxx, (1).xxx};
}

void fragment_main() {
  frexp_ae4a66();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_ae4a66();
  return;
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
  frexp_ae4a66();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(2,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
