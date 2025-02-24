SKIP: INVALID

struct FragmentInputs0 {
  float4 position;
  int loc0;
};

struct FragmentInputs1 {
  float4 loc3;
  vector<float16_t, 3> loc5;
  uint sample_mask;
};

struct main_inputs {
  nointerpolation int FragmentInputs0_loc0 : TEXCOORD0;
  nointerpolation uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 FragmentInputs1_loc3 : TEXCOORD3;
  float16_t loc4 : TEXCOORD4;
  vector<float16_t, 3> FragmentInputs1_loc5 : TEXCOORD5;
  float4 FragmentInputs0_position : SV_Position;
  bool front_facing : SV_IsFrontFace;
  uint sample_index : SV_SampleIndex;
  uint FragmentInputs1_sample_mask : SV_Coverage;
};


void main_inner(FragmentInputs0 inputs0, bool front_facing, uint loc1, uint sample_index, FragmentInputs1 inputs1, float loc2, float16_t loc4) {
  if (front_facing) {
    float4 foo = inputs0.position;
    uint bar = (sample_index + inputs1.sample_mask);
    int i = inputs0.loc0;
    uint u = loc1;
    float f = loc2;
    float4 v = inputs1.loc3;
    float16_t x = loc4;
    vector<float16_t, 3> y = inputs1.loc5;
  }
}

void main(main_inputs inputs) {
  FragmentInputs0 v_1 = {float4(inputs.FragmentInputs0_position.xyz, (1.0f / inputs.FragmentInputs0_position[3u])), inputs.FragmentInputs0_loc0};
  FragmentInputs0 v_2 = v_1;
  FragmentInputs1 v_3 = {inputs.FragmentInputs1_loc3, inputs.FragmentInputs1_loc5, inputs.FragmentInputs1_sample_mask};
  main_inner(v_2, inputs.front_facing, inputs.loc1, inputs.sample_index, v_3, inputs.loc2, inputs.loc4);
}

FXC validation failure:
<scrubbed_path>(8,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
