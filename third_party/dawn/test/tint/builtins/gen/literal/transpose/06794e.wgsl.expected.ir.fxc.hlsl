SKIP: INVALID

struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int transpose_06794e() {
  matrix<float16_t, 3, 3> res = matrix<float16_t, 3, 3>((float16_t(1.0h)).xxx, (float16_t(1.0h)).xxx, (float16_t(1.0h)).xxx);
  return (((res[int(0)].x == float16_t(0.0h))) ? (int(1)) : (int(0)));
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(transpose_06794e()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(transpose_06794e()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = transpose_06794e();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  VertexOutput v_2 = v_1;
  VertexOutput v_3 = v_1;
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_2.pos};
  return v_4;
}

FXC validation failure:
<scrubbed_path>(14,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(15,13-15): error X3004: undeclared identifier 'res'


tint executable returned error: exit status 1
