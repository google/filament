//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 unpack2x16unorm_7699c0() {
  float2 res = float2(0.00001525902189314365f, 0.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(unpack2x16unorm_7699c0()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float2 unpack2x16unorm_7699c0() {
  float2 res = float2(0.00001525902189314365f, 0.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(unpack2x16unorm_7699c0()));
  return;
}
//
// vertex_main
//
float2 unpack2x16unorm_7699c0() {
  float2 res = float2(0.00001525902189314365f, 0.0f);
  return res;
}

struct VertexOutput {
  float4 pos;
  float2 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation float2 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = unpack2x16unorm_7699c0();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
