SKIP: INVALID

struct S {
  float f;
  uint u;
  float4 v;
  float16_t x;
  vector<float16_t, 3> y;
};

struct frag_main_inputs {
  float S_f : TEXCOORD0;
  nointerpolation uint S_u : TEXCOORD1;
  float16_t S_x : TEXCOORD2;
  vector<float16_t, 3> S_y : TEXCOORD3;
  float4 S_v : SV_Position;
};


RWByteAddressBuffer output : register(u0);
void v_1(uint offset, S obj) {
  output.Store((offset + 0u), asuint(obj.f));
  output.Store((offset + 4u), obj.u);
  output.Store4((offset + 128u), asuint(obj.v));
  output.Store<float16_t>((offset + 160u), obj.x);
  output.Store<vector<float16_t, 3> >((offset + 192u), obj.y);
}

void frag_main_inner(S input) {
  float f = input.f;
  uint u = input.u;
  float4 v = input.v;
  float16_t x = input.x;
  vector<float16_t, 3> y = input.y;
  v_1(0u, input);
}

void frag_main(frag_main_inputs inputs) {
  S v_2 = {inputs.S_f, inputs.S_u, float4(inputs.S_v.xyz, (1.0f / inputs.S_v[3u])), inputs.S_x, inputs.S_y};
  frag_main_inner(v_2);
}

FXC validation failure:
<scrubbed_path>(5,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
