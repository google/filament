//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float radians_6b0ff2() {
  float res = 0.01745329238474369049f;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(radians_6b0ff2()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float radians_6b0ff2() {
  float res = 0.01745329238474369049f;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(radians_6b0ff2()));
  return;
}
//
// vertex_main
//
float radians_6b0ff2() {
  float res = 0.01745329238474369049f;
  return res;
}

struct VertexOutput {
  float4 pos;
  float prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation float prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = radians_6b0ff2();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
