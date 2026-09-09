// Rewrite unchanged result:
struct s_f {
  float f;
};
struct s_i {
  int i;
};
struct s_i_f2 {
  float2 f2;
  int i;
};
struct s_f2 {
  float2 f2;
};
struct s_ff {
  float f0;
  float f1;
};
struct s_f3_f3 {
  float3 f0;
  float3 f1;
};
float pick_one(s_f2 sf2) {
  return sf2.f2.x;
}


s_f2 pick_two(s_f2 sf2) {
  return sf2;
}


void main() {
  s_f2 zsf2_zero_cast = (s_f2)1;
  s_f sf_none;
  s_f sf_all = { 0.100000001F };
  s_f2 sf2_all = { float2(1, 2) };
  s_f2 sf2_all_flat = { 0.100000001F, 0.200000003F };
  s_ff sff_all = { 0.100000001F, 0.200000003F };
  s_f3_f3 sf3f3_all = { float3(1, 2, 3), float3(3, 2, 1) };
  s_f3_f3 sf3f3_all_flat = { 1, 2, 3, 3, 2, 1 };
  s_f3_f3 sf3f3_all_straddle = { float2(1, 2), float2(3, 4), float2(5, 6) };
  s_f3_f3 sf3f3_all_straddle_instances[2] = { float2(1, 2), sf3f3_all, float4(1, 2, 3, 4) };
  s_f3_f3 sf3f3_nested = { { 1, 2, 3, 4, 5 }, { 1 } };
  s_f2 sf2_zero_cast = (s_f2)1;
  s_f2 f2_ints = { 1, 2 };
  double d = 0.123;
  s_f2 f2_int_double = { 1, d };
  s_f2 f2_f2 = { f2_ints };
  s_ff sff = { 1, 0.100000001F };
  s_f2 sf2_zero_assign;
  pick_one(sf2_all);
  sf2_all = pick_two(sf2_all);
}


