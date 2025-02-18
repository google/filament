SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float16_t dot_8e40f1() {
  float16_t res = float16_t(3.0h);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, dot_8e40f1());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, dot_8e40f1());
  return;
}

struct VertexOutput {
  float4 pos;
  float16_t prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation float16_t prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = dot_8e40f1();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(3,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
