//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<uint4> arg_0 : register(t0, space1);

uint3 textureDimensions_268ddb() {
  uint3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint3 res = tint_tmp;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(textureDimensions_268ddb()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<uint4> arg_0 : register(t0, space1);

uint3 textureDimensions_268ddb() {
  uint3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint3 res = tint_tmp;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(textureDimensions_268ddb()));
  return;
}
//
// vertex_main
//
Texture3D<uint4> arg_0 : register(t0, space1);

uint3 textureDimensions_268ddb() {
  uint3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint3 res = tint_tmp;
  return res;
}

struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};
struct tint_symbol_1 {
  nointerpolation uint3 prevent_dce : TEXCOORD0;
  float4 pos : SV_Position;
};

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureDimensions_268ddb();
  return tint_symbol;
}

tint_symbol_1 vertex_main() {
  VertexOutput inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.prevent_dce = inner_result.prevent_dce;
  return wrapper_result;
}
