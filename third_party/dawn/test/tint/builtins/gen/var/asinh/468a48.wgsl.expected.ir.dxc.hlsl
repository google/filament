//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t asinh_468a48() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t res = log((v + sqrt(((v * v) + float16_t(1.0h)))));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, asinh_468a48());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float16_t asinh_468a48() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t res = log((v + sqrt(((v * v) + float16_t(1.0h)))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, asinh_468a48());
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


float16_t asinh_468a48() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t res = log((v + sqrt(((v * v) + float16_t(1.0h)))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = asinh_468a48();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

