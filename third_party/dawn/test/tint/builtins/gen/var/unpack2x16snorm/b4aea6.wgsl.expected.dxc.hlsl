//
// fragment_main
//
float2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}

RWByteAddressBuffer prevent_dce : register(u0);

float2 unpack2x16snorm_b4aea6() {
  uint arg_0 = 1u;
  float2 res = tint_unpack2x16snorm(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(unpack2x16snorm_b4aea6()));
  return;
}
//
// compute_main
//
float2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}

RWByteAddressBuffer prevent_dce : register(u0);

float2 unpack2x16snorm_b4aea6() {
  uint arg_0 = 1u;
  float2 res = tint_unpack2x16snorm(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(unpack2x16snorm_b4aea6()));
  return;
}
//
// vertex_main
//
float2 tint_unpack2x16snorm(uint param_0) {
  int j = int(param_0);
  int2 i = int2(j << 16, j) >> 16;
  return clamp(float2(i) / 32767.0, -1.0, 1.0);
}

float2 unpack2x16snorm_b4aea6() {
  uint arg_0 = 1u;
  float2 res = tint_unpack2x16snorm(arg_0);
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
  tint_symbol.prevent_dce = unpack2x16snorm_b4aea6();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
