//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint pack4xI8_bfce01() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = ((asuint(v) & uint4((255u).xxxx)) << v_1);
  uint res = dot(v_2, uint4((1u).xxxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, pack4xI8_bfce01());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint pack4xI8_bfce01() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = ((asuint(v) & uint4((255u).xxxx)) << v_1);
  uint res = dot(v_2, uint4((1u).xxxx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, pack4xI8_bfce01());
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


uint pack4xI8_bfce01() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = ((asuint(v) & uint4((255u).xxxx)) << v_1);
  uint res = dot(v_2, uint4((1u).xxxx));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = pack4xI8_bfce01();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

