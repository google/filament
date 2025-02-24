//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int firstLeadingBit_57a1a3() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = (((v < 2147483648u)) ? (v) : (~(v)));
  uint v_2 = ((((v_1 & 4294901760u) == 0u)) ? (0u) : (16u));
  uint v_3 = (((((v_1 >> v_2) & 65280u) == 0u)) ? (0u) : (8u));
  uint v_4 = ((((((v_1 >> v_2) >> v_3) & 240u) == 0u)) ? (0u) : (4u));
  uint v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & 12u) == 0u)) ? (0u) : (2u));
  int res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == 0u)) ? (4294967295u) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & 2u) == 0u)) ? (0u) : (1u)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(firstLeadingBit_57a1a3()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int firstLeadingBit_57a1a3() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = (((v < 2147483648u)) ? (v) : (~(v)));
  uint v_2 = ((((v_1 & 4294901760u) == 0u)) ? (0u) : (16u));
  uint v_3 = (((((v_1 >> v_2) & 65280u) == 0u)) ? (0u) : (8u));
  uint v_4 = ((((((v_1 >> v_2) >> v_3) & 240u) == 0u)) ? (0u) : (4u));
  uint v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & 12u) == 0u)) ? (0u) : (2u));
  int res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == 0u)) ? (4294967295u) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & 2u) == 0u)) ? (0u) : (1u)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(firstLeadingBit_57a1a3()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int firstLeadingBit_57a1a3() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = (((v < 2147483648u)) ? (v) : (~(v)));
  uint v_2 = ((((v_1 & 4294901760u) == 0u)) ? (0u) : (16u));
  uint v_3 = (((((v_1 >> v_2) & 65280u) == 0u)) ? (0u) : (8u));
  uint v_4 = ((((((v_1 >> v_2) >> v_3) & 240u) == 0u)) ? (0u) : (4u));
  uint v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & 12u) == 0u)) ? (0u) : (2u));
  int res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == 0u)) ? (4294967295u) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & 2u) == 0u)) ? (0u) : (1u)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_6 = (VertexOutput)0;
  v_6.pos = (0.0f).xxxx;
  v_6.prevent_dce = firstLeadingBit_57a1a3();
  VertexOutput v_7 = v_6;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

