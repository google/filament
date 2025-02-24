//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float atanh_7997d8() {
  float arg_0 = 0.5f;
  float v = arg_0;
  float res = (log(((1.0f + v) / (1.0f - v))) * 0.5f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atanh_7997d8()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float atanh_7997d8() {
  float arg_0 = 0.5f;
  float v = arg_0;
  float res = (log(((1.0f + v) / (1.0f - v))) * 0.5f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atanh_7997d8()));
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


float atanh_7997d8() {
  float arg_0 = 0.5f;
  float v = arg_0;
  float res = (log(((1.0f + v) / (1.0f - v))) * 0.5f);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = atanh_7997d8();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

