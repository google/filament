struct main_inputs {
  float4 position : SV_Position;
  bool front_facing : SV_IsFrontFace;
  uint sample_index : SV_SampleIndex;
  uint sample_mask : SV_Coverage;
};


void main_inner(float4 position, bool front_facing, uint sample_index, uint sample_mask) {
  if (front_facing) {
    float4 foo = position;
    uint bar = (sample_index + sample_mask);
  }
}

void main(main_inputs inputs) {
  main_inner(float4(inputs.position.xyz, (1.0f / inputs.position.w)), inputs.front_facing, inputs.sample_index, inputs.sample_mask);
}

