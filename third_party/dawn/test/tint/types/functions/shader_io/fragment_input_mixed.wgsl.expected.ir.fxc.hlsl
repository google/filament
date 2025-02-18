struct FragmentInputs0 {
  float4 position;
  int loc0;
};

struct FragmentInputs1 {
  float4 loc3;
  uint sample_mask;
};

struct main_inputs {
  nointerpolation int FragmentInputs0_loc0 : TEXCOORD0;
  nointerpolation uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 FragmentInputs1_loc3 : TEXCOORD3;
  float4 FragmentInputs0_position : SV_Position;
  bool front_facing : SV_IsFrontFace;
  uint sample_index : SV_SampleIndex;
  uint FragmentInputs1_sample_mask : SV_Coverage;
};


void main_inner(FragmentInputs0 inputs0, bool front_facing, uint loc1, uint sample_index, FragmentInputs1 inputs1, float loc2) {
  if (front_facing) {
    float4 foo = inputs0.position;
    uint bar = (sample_index + inputs1.sample_mask);
    int i = inputs0.loc0;
    uint u = loc1;
    float f = loc2;
    float4 v = inputs1.loc3;
  }
}

void main(main_inputs inputs) {
  FragmentInputs0 v_1 = {float4(inputs.FragmentInputs0_position.xyz, (1.0f / inputs.FragmentInputs0_position.w)), inputs.FragmentInputs0_loc0};
  FragmentInputs1 v_2 = {inputs.FragmentInputs1_loc3, inputs.FragmentInputs1_sample_mask};
  main_inner(v_1, inputs.front_facing, inputs.loc1, inputs.sample_index, v_2, inputs.loc2);
}

