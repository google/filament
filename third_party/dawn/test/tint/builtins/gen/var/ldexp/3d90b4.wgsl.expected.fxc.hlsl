SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> ldexp_3d90b4() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  int2 arg_1 = (1).xx;
  vector<float16_t, 2> res = ldexp(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, ldexp_3d90b4());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, ldexp_3d90b4());
  return;
}

struct VertexOutput {
  float4 pos;
  vector<float16_t, 2> prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation vector<float16_t, 2> prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = ldexp_3d90b4();
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
