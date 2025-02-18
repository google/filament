//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint4 v = uint4(round((clamp(arg_0, (0.0f).xxxx, (1.0f).xxxx) * 255.0f)));
  uint res = (v.x | ((v.y << 8u) | ((v.z << 16u) | (v.w << 24u))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, pack4x8unorm_95c456());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint4 v = uint4(round((clamp(arg_0, (0.0f).xxxx, (1.0f).xxxx) * 255.0f)));
  uint res = (v.x | ((v.y << 8u) | ((v.z << 16u) | (v.w << 24u))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, pack4x8unorm_95c456());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint4 v = uint4(round((clamp(arg_0, (0.0f).xxxx, (1.0f).xxxx) * 255.0f)));
  uint res = (v.x | ((v.y << 8u) | ((v.z << 16u) | (v.w << 24u))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = pack4x8unorm_95c456();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

