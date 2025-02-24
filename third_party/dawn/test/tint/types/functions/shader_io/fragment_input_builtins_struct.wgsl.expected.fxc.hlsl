struct FragmentInputs {
  float4 position;
  bool front_facing;
  uint sample_index;
  uint sample_mask;
};
struct tint_symbol_1 {
  float4 position : SV_Position;
  bool front_facing : SV_IsFrontFace;
  uint sample_index : SV_SampleIndex;
  uint sample_mask : SV_Coverage;
};

void main_inner(FragmentInputs inputs) {
  if (inputs.front_facing) {
    float4 foo = inputs.position;
    uint bar = (inputs.sample_index + inputs.sample_mask);
  }
}

void main(tint_symbol_1 tint_symbol) {
  FragmentInputs tint_symbol_2 = {float4(tint_symbol.position.xyz, (1.0f / tint_symbol.position.w)), tint_symbol.front_facing, tint_symbol.sample_index, tint_symbol.sample_mask};
  main_inner(tint_symbol_2);
  return;
}
