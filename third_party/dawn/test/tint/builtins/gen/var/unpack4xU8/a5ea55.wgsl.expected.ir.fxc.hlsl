//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = (uint4((v).xxxx) >> v_1);
  uint4 res = (v_2 & uint4((255u).xxxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, unpack4xU8_a5ea55());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = (uint4((v).xxxx) >> v_1);
  uint4 res = (v_2 & uint4((255u).xxxx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, unpack4xU8_a5ea55());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = (uint4((v).xxxx) >> v_1);
  uint4 res = (v_2 & uint4((255u).xxxx));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = unpack4xU8_a5ea55();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

