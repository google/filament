struct S {
  float f;
  uint u;
  float4 v;
};

struct frag_main_inputs {
  float S_f : TEXCOORD0;
  nointerpolation uint S_u : TEXCOORD1;
  float4 S_v : SV_Position;
};


RWByteAddressBuffer output : register(u0);
void v_1(uint offset, S obj) {
  output.Store((offset + 0u), asuint(obj.f));
  output.Store((offset + 4u), obj.u);
  output.Store4((offset + 128u), asuint(obj.v));
}

void frag_main_inner(S input) {
  float f = input.f;
  uint u = input.u;
  float4 v = input.v;
  v_1(0u, input);
}

void frag_main(frag_main_inputs inputs) {
  S v_2 = {inputs.S_f, inputs.S_u, float4(inputs.S_v.xyz, (1.0f / inputs.S_v.w))};
  frag_main_inner(v_2);
}

