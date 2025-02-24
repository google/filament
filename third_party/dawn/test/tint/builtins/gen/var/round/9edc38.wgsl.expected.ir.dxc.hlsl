//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float round_9edc38() {
  float arg_0 = 3.5f;
  float res = round(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(round_9edc38()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float round_9edc38() {
  float arg_0 = 3.5f;
  float res = round(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(round_9edc38()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float round_9edc38() {
  float arg_0 = 3.5f;
  float res = round(arg_0);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v = (VertexOutput)0;
  v.pos = (0.0f).xxxx;
  v.prevent_dce = round_9edc38();
  VertexOutput v_1 = v;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

