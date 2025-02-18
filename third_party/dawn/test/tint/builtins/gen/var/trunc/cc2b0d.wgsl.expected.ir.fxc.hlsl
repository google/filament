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
float16_t trunc_cc2b0d() {
  float16_t arg_0 = float16_t(1.5h);
  float16_t v = arg_0;
  float16_t v_1 = floor(v);
  float16_t res = (((v < float16_t(0.0h))) ? (ceil(v)) : (v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, trunc_cc2b0d());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, trunc_cc2b0d());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = trunc_cc2b0d();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  VertexOutput v_4 = v_3;
  VertexOutput v_5 = v_3;
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_4.pos};
  return v_6;
}

FXC validation failure:
<scrubbed_path>(3,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
