struct FragmentOutputs {
  int loc0;
  float frag_depth;
  uint loc1;
  float loc2;
  uint sample_mask;
  float4 loc3;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};

struct main_outputs {
  int FragmentOutputs_loc0 : SV_Target0;
  uint FragmentOutputs_loc1 : SV_Target1;
  float FragmentOutputs_loc2 : SV_Target2;
  float4 FragmentOutputs_loc3 : SV_Target3;
  float16_t FragmentOutputs_loc4 : SV_Target4;
  vector<float16_t, 3> FragmentOutputs_loc5 : SV_Target5;
  float FragmentOutputs_frag_depth : SV_Depth;
  uint FragmentOutputs_sample_mask : SV_Coverage;
};


FragmentOutputs main_inner() {
  FragmentOutputs v = {int(1), 2.0f, 1u, 1.0f, 2u, float4(1.0f, 2.0f, 3.0f, 4.0f), float16_t(2.25h), vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h))};
  return v;
}

main_outputs main() {
  FragmentOutputs v_1 = main_inner();
  main_outputs v_2 = {v_1.loc0, v_1.loc1, v_1.loc2, v_1.loc3, v_1.loc4, v_1.loc5, v_1.frag_depth, v_1.sample_mask};
  return v_2;
}

