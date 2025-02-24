// Rewrite unchanged result:
float pick_one(float2 f2) {
  return f2.x;
}


void main() {
  float2 f2_none;
  float2 f2_all = { 0.100000001F, 0.200000003F };
  float2 f2_ints = { 1, 2 };
  double d = 0.123;
  float2 f2_int_double = { 1, d };
  float2 f2_f2 = { f2_all };
  float3 f3_f2_f = { f2_all, 0.100000001F };
  float2 f2c_f_f = float2(0.100000001F, 0.200000003F);
  float2 f2c_f2 = float2(f2c_f_f);
  float3 f3c_f2_f = float3(f2c_f_f, 1);
  pick_one(float2(0.100000001F, 0.200000003F));
}


