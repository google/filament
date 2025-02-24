//
// fragment_main
//
uint tint_pack4x8unorm(float4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

RWByteAddressBuffer prevent_dce : register(u0);

uint pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint res = tint_pack4x8unorm(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(pack4x8unorm_95c456()));
  return;
}
//
// compute_main
//
uint tint_pack4x8unorm(float4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

RWByteAddressBuffer prevent_dce : register(u0);

uint pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint res = tint_pack4x8unorm(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(pack4x8unorm_95c456()));
  return;
}
//
// vertex_main
//
uint tint_pack4x8unorm(float4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

uint pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint res = tint_pack4x8unorm(arg_0);
  return res;
}

struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = pack4x8unorm_95c456();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
