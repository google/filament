struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint b : SV_InstanceID;
};


float4 main_inner(uint b) {
  return float4((float(b)).xxxx);
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.b)};
  return v;
}

