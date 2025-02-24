//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint countTrailingZeros_21e394() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = ((((v & 65535u) == 0u)) ? (16u) : (0u));
  uint v_2 = (((((v >> v_1) & 255u) == 0u)) ? (8u) : (0u));
  uint v_3 = ((((((v >> v_1) >> v_2) & 15u) == 0u)) ? (4u) : (0u));
  uint v_4 = (((((((v >> v_1) >> v_2) >> v_3) & 3u) == 0u)) ? (2u) : (0u));
  uint v_5 = ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & 1u) == 0u)) ? (1u) : (0u));
  uint res = ((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == 0u)) ? (1u) : (0u)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, countTrailingZeros_21e394());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint countTrailingZeros_21e394() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = ((((v & 65535u) == 0u)) ? (16u) : (0u));
  uint v_2 = (((((v >> v_1) & 255u) == 0u)) ? (8u) : (0u));
  uint v_3 = ((((((v >> v_1) >> v_2) & 15u) == 0u)) ? (4u) : (0u));
  uint v_4 = (((((((v >> v_1) >> v_2) >> v_3) & 3u) == 0u)) ? (2u) : (0u));
  uint v_5 = ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & 1u) == 0u)) ? (1u) : (0u));
  uint res = ((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == 0u)) ? (1u) : (0u)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, countTrailingZeros_21e394());
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


uint countTrailingZeros_21e394() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = ((((v & 65535u) == 0u)) ? (16u) : (0u));
  uint v_2 = (((((v >> v_1) & 255u) == 0u)) ? (8u) : (0u));
  uint v_3 = ((((((v >> v_1) >> v_2) & 15u) == 0u)) ? (4u) : (0u));
  uint v_4 = (((((((v >> v_1) >> v_2) >> v_3) & 3u) == 0u)) ? (2u) : (0u));
  uint v_5 = ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & 1u) == 0u)) ? (1u) : (0u));
  uint res = ((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == 0u)) ? (1u) : (0u)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_6 = (VertexOutput)0;
  v_6.pos = (0.0f).xxxx;
  v_6.prevent_dce = countTrailingZeros_21e394();
  VertexOutput v_7 = v_6;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

