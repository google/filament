struct FragmentInputs {
  float4 position;
  bool front_facing;
  uint sample_index;
  uint sample_mask;
};

struct main_inputs {
  float4 FragmentInputs_position : SV_Position;
  bool FragmentInputs_front_facing : SV_IsFrontFace;
  uint FragmentInputs_sample_index : SV_SampleIndex;
  uint FragmentInputs_sample_mask : SV_Coverage;
};


void main_inner(FragmentInputs inputs) {
  if (inputs.front_facing) {
    float4 foo = inputs.position;
    uint bar = (inputs.sample_index + inputs.sample_mask);
  }
}

void main(main_inputs inputs) {
  FragmentInputs v = {float4(inputs.FragmentInputs_position.xyz, (1.0f / inputs.FragmentInputs_position.w)), inputs.FragmentInputs_front_facing, inputs.FragmentInputs_sample_index, inputs.FragmentInputs_sample_mask};
  main_inner(v);
}

