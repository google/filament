//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 extractBits_a99a8d() {
  int2 arg_0 = (int(1)).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int2 v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + arg_2)));
  int2 v_3 = (((v_2 < 32u)) ? ((v << uint2((v_2).xx))) : ((int(0)).xx));
  int2 res = ((((v_2 + v_1) < 32u)) ? ((v_3 >> uint2(((v_2 + v_1)).xx))) : (((v_3 >> (31u).xx) >> (1u).xx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(extractBits_a99a8d()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 extractBits_a99a8d() {
  int2 arg_0 = (int(1)).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int2 v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + arg_2)));
  int2 v_3 = (((v_2 < 32u)) ? ((v << uint2((v_2).xx))) : ((int(0)).xx));
  int2 res = ((((v_2 + v_1) < 32u)) ? ((v_3 >> uint2(((v_2 + v_1)).xx))) : (((v_3 >> (31u).xx) >> (1u).xx)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(extractBits_a99a8d()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int2 extractBits_a99a8d() {
  int2 arg_0 = (int(1)).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int2 v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + arg_2)));
  int2 v_3 = (((v_2 < 32u)) ? ((v << uint2((v_2).xx))) : ((int(0)).xx));
  int2 res = ((((v_2 + v_1) < 32u)) ? ((v_3 >> uint2(((v_2 + v_1)).xx))) : (((v_3 >> (31u).xx) >> (1u).xx)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  v_4.prevent_dce = extractBits_a99a8d();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

