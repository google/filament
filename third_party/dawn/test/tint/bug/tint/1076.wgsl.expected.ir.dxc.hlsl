struct FragIn {
  float a;
  uint mask;
};

struct main_outputs {
  float FragIn_a : SV_Target0;
  uint FragIn_mask : SV_Coverage;
};

struct main_inputs {
  float FragIn_a : TEXCOORD0;
  float b : TEXCOORD1;
  uint FragIn_mask : SV_Coverage;
};


FragIn main_inner(FragIn v, float b) {
  if ((v.mask == 0u)) {
    return v;
  }
  FragIn v_1 = {b, 1u};
  return v_1;
}

main_outputs main(main_inputs inputs) {
  FragIn v_2 = {inputs.FragIn_a, inputs.FragIn_mask};
  FragIn v_3 = main_inner(v_2, inputs.b);
  main_outputs v_4 = {v_3.a, v_3.mask};
  return v_4;
}

