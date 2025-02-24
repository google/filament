//
// fragment_main
//
float2 tint_acosh(float2 x) {
  return log((x + sqrt(((x * x) - 1.0f))));
}

RWByteAddressBuffer prevent_dce : register(u0);

float2 acosh_640883() {
  float2 arg_0 = (1.54308068752288818359f).xx;
  float2 res = tint_acosh(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(acosh_640883()));
  return;
}
//
// compute_main
//
float2 tint_acosh(float2 x) {
  return log((x + sqrt(((x * x) - 1.0f))));
}

RWByteAddressBuffer prevent_dce : register(u0);

float2 acosh_640883() {
  float2 arg_0 = (1.54308068752288818359f).xx;
  float2 res = tint_acosh(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(acosh_640883()));
  return;
}
//
// vertex_main
//
float2 tint_acosh(float2 x) {
  return log((x + sqrt(((x * x) - 1.0f))));
}

float2 acosh_640883() {
  float2 arg_0 = (1.54308068752288818359f).xx;
  float2 res = tint_acosh(arg_0);
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
  tint_symbol.prevent_dce = acosh_640883();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
