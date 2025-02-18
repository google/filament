SKIP: INVALID

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
  FragmentOutputs v_2 = v_1;
  FragmentOutputs v_3 = v_1;
  FragmentOutputs v_4 = v_1;
  FragmentOutputs v_5 = v_1;
  FragmentOutputs v_6 = v_1;
  FragmentOutputs v_7 = v_1;
  FragmentOutputs v_8 = v_1;
  FragmentOutputs v_9 = v_1;
  main_outputs v_10 = {v_2.loc0, v_4.loc1, v_5.loc2, v_7.loc3, v_8.loc4, v_9.loc5, v_3.frag_depth, v_6.sample_mask};
  return v_10;
}

FXC validation failure:
<scrubbed_path>(8,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
