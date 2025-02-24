struct FragmentInputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};

struct main_inputs {
  nointerpolation int FragmentInputs_loc0 : TEXCOORD0;
  nointerpolation uint FragmentInputs_loc1 : TEXCOORD1;
  float FragmentInputs_loc2 : TEXCOORD2;
  float4 FragmentInputs_loc3 : TEXCOORD3;
  float16_t FragmentInputs_loc4 : TEXCOORD4;
  vector<float16_t, 3> FragmentInputs_loc5 : TEXCOORD5;
};


void main_inner(FragmentInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  float4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  vector<float16_t, 3> y = inputs.loc5;
}

void main(main_inputs inputs) {
  FragmentInputs v_1 = {inputs.FragmentInputs_loc0, inputs.FragmentInputs_loc1, inputs.FragmentInputs_loc2, inputs.FragmentInputs_loc3, inputs.FragmentInputs_loc4, inputs.FragmentInputs_loc5};
  main_inner(v_1);
}

