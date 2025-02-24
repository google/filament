struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};

struct main_outputs {
  float FragmentOutputs_frag_depth : SV_Depth;
  uint FragmentOutputs_sample_mask : SV_Coverage;
};


FragmentOutputs main_inner() {
  FragmentOutputs v = {1.0f, 1u};
  return v;
}

main_outputs main() {
  FragmentOutputs v_1 = main_inner();
  main_outputs v_2 = {v_1.frag_depth, v_1.sample_mask};
  return v_2;
}

