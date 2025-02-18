SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> smoothstep_c43ebd() {
  vector<float16_t, 4> res = (float16_t(0.5h)).xxxx;
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, smoothstep_c43ebd());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, smoothstep_c43ebd());
  return;
}

struct VertexOutput {
  float4 pos;
  vector<float16_t, 4> prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation vector<float16_t, 4> prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = smoothstep_c43ebd();
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
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
