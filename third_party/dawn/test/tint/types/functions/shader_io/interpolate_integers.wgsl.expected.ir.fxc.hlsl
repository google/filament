//
// vert_main
//
struct Interface {
  int i;
  uint u;
  int4 vi;
  uint4 vu;
  float4 pos;
};

struct vert_main_outputs {
  nointerpolation int Interface_i : TEXCOORD0;
  nointerpolation uint Interface_u : TEXCOORD1;
  nointerpolation int4 Interface_vi : TEXCOORD2;
  nointerpolation uint4 Interface_vu : TEXCOORD3;
  float4 Interface_pos : SV_Position;
};


Interface vert_main_inner() {
  Interface v = (Interface)0;
  return v;
}

vert_main_outputs vert_main() {
  Interface v_1 = vert_main_inner();
  vert_main_outputs v_2 = {v_1.i, v_1.u, v_1.vi, v_1.vu, v_1.pos};
  return v_2;
}

//
// frag_main
//
struct Interface {
  int i;
  uint u;
  int4 vi;
  uint4 vu;
  float4 pos;
};

struct frag_main_outputs {
  int tint_symbol : SV_Target0;
};

struct frag_main_inputs {
  nointerpolation int Interface_i : TEXCOORD0;
  nointerpolation uint Interface_u : TEXCOORD1;
  nointerpolation int4 Interface_vi : TEXCOORD2;
  nointerpolation uint4 Interface_vu : TEXCOORD3;
  float4 Interface_pos : SV_Position;
};


int frag_main_inner(Interface inputs) {
  return inputs.i;
}

frag_main_outputs frag_main(frag_main_inputs inputs) {
  Interface v = {inputs.Interface_i, inputs.Interface_u, inputs.Interface_vi, inputs.Interface_vu, float4(inputs.Interface_pos.xyz, (1.0f / inputs.Interface_pos.w))};
  frag_main_outputs v_1 = {frag_main_inner(v)};
  return v_1;
}

