//
// fragment_main
//
float2 tint_trunc(float2 param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

RWByteAddressBuffer prevent_dce : register(u0);

float2 trunc_f370d3() {
  float2 arg_0 = (1.5f).xx;
  float2 res = tint_trunc(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(trunc_f370d3()));
  return;
}
//
// compute_main
//
float2 tint_trunc(float2 param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

RWByteAddressBuffer prevent_dce : register(u0);

float2 trunc_f370d3() {
  float2 arg_0 = (1.5f).xx;
  float2 res = tint_trunc(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(trunc_f370d3()));
  return;
}
//
// vertex_main
//
float2 tint_trunc(float2 param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float2 trunc_f370d3() {
  float2 arg_0 = (1.5f).xx;
  float2 res = tint_trunc(arg_0);
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
  tint_symbol.prevent_dce = trunc_f370d3();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
