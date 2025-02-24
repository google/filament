//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 extractBits_12b197() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint3 v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + arg_2)));
  uint3 v_3 = (((v_2 < 32u)) ? ((v << uint3((v_2).xxx))) : ((0u).xxx));
  uint3 res = ((((v_2 + v_1) < 32u)) ? ((v_3 >> uint3(((v_2 + v_1)).xxx))) : (((v_3 >> (31u).xxx) >> (1u).xxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, extractBits_12b197());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 extractBits_12b197() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint3 v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + arg_2)));
  uint3 v_3 = (((v_2 < 32u)) ? ((v << uint3((v_2).xxx))) : ((0u).xxx));
  uint3 res = ((((v_2 + v_1) < 32u)) ? ((v_3 >> uint3(((v_2 + v_1)).xxx))) : (((v_3 >> (31u).xxx) >> (1u).xxx)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, extractBits_12b197());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint3 extractBits_12b197() {
  uint3 arg_0 = (1u).xxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint3 v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + arg_2)));
  uint3 v_3 = (((v_2 < 32u)) ? ((v << uint3((v_2).xxx))) : ((0u).xxx));
  uint3 res = ((((v_2 + v_1) < 32u)) ? ((v_3 >> uint3(((v_2 + v_1)).xxx))) : (((v_3 >> (31u).xxx) >> (1u).xxx)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  v_4.prevent_dce = extractBits_12b197();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

