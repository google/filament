SKIP: INVALID

struct VertexOutput {
  float4 pos;
  float16_t prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float16_t VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
float16_t atanh_d2d8cd() {
  float16_t arg_0 = float16_t(0.5h);
  float16_t v = arg_0;
  float16_t res = (log(((float16_t(1.0h) + v) / (float16_t(1.0h) - v))) * float16_t(0.5h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, atanh_d2d8cd());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, atanh_d2d8cd());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = atanh_d2d8cd();
  VertexOutput v_1 = tint_symbol;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  VertexOutput v_3 = v_2;
  VertexOutput v_4 = v_2;
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_3.pos};
  return v_5;
}

FXC validation failure:
<scrubbed_path>(3,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
