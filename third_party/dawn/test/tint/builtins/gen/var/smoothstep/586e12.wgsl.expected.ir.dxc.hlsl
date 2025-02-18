//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t smoothstep_586e12() {
  float16_t arg_0 = float16_t(2.0h);
  float16_t arg_1 = float16_t(4.0h);
  float16_t arg_2 = float16_t(3.0h);
  float16_t v = arg_0;
  float16_t v_1 = clamp(((arg_2 - v) / (arg_1 - v)), float16_t(0.0h), float16_t(1.0h));
  float16_t res = (v_1 * (v_1 * (float16_t(3.0h) - (float16_t(2.0h) * v_1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, smoothstep_586e12());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t smoothstep_586e12() {
  float16_t arg_0 = float16_t(2.0h);
  float16_t arg_1 = float16_t(4.0h);
  float16_t arg_2 = float16_t(3.0h);
  float16_t v = arg_0;
  float16_t v_1 = clamp(((arg_2 - v) / (arg_1 - v)), float16_t(0.0h), float16_t(1.0h));
  float16_t res = (v_1 * (v_1 * (float16_t(3.0h) - (float16_t(2.0h) * v_1))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, smoothstep_586e12());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float16_t prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float16_t VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float16_t smoothstep_586e12() {
  float16_t arg_0 = float16_t(2.0h);
  float16_t arg_1 = float16_t(4.0h);
  float16_t arg_2 = float16_t(3.0h);
  float16_t v = arg_0;
  float16_t v_1 = clamp(((arg_2 - v) / (arg_1 - v)), float16_t(0.0h), float16_t(1.0h));
  float16_t res = (v_1 * (v_1 * (float16_t(3.0h) - (float16_t(2.0h) * v_1))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = smoothstep_586e12();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

