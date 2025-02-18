SKIP: INVALID

struct FragmentOutputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};
struct tint_symbol {
  int loc0 : SV_Target0;
  uint loc1 : SV_Target1;
  float loc2 : SV_Target2;
  float4 loc3 : SV_Target3;
  float16_t loc4 : SV_Target4;
  vector<float16_t, 3> loc5 : SV_Target5;
};

FragmentOutputs main_inner() {
  FragmentOutputs tint_symbol_1 = {1, 1u, 1.0f, float4(1.0f, 2.0f, 3.0f, 4.0f), float16_t(2.25h), vector<float16_t, 3>(float16_t(3.0h), float16_t(5.0h), float16_t(8.0h))};
  return tint_symbol_1;
}

tint_symbol main() {
  FragmentOutputs inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.loc0 = inner_result.loc0;
  wrapper_result.loc1 = inner_result.loc1;
  wrapper_result.loc2 = inner_result.loc2;
  wrapper_result.loc3 = inner_result.loc3;
  wrapper_result.loc4 = inner_result.loc4;
  wrapper_result.loc5 = inner_result.loc5;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(6,3-11): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
