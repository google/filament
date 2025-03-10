// Rewrite unchanged result:
groupshared float2 g_gro;
groupshared precise float2 g_gro_pre;
static groupshared precise float2 g_gro_pre_sta;
static groupshared float2 g_gro_sta;
extern const float2 g_ext;
extern precise const float2 g_ext_pre;
extern precise uniform const float2 g_ext_pre_uni;
extern precise uniform const float2 g_ext_pre_uni_con;
extern precise const float2 g_ext_pre_con;
extern uniform const float2 g_ext_uni;
extern uniform const float2 g_ext_uni_con;
extern const float2 g_ext_con;
precise const float2 g_pre;
static precise float2 g_pre_sta;
static precise const float2 g_pre_sta_con;
precise uniform const float2 g_pre_uni;
precise uniform const float2 g_pre_uni_con;
precise const float2 g_pre_con;
static float2 g_sta;
static const float2 g_sta_con;
uniform const float2 g_uni;
uniform const float2 g_uni_con;
const float2 g_con;
groupshared float g_gro_init = 1.F;
groupshared precise float g_gro_pre_init = 1.F;
static groupshared precise float g_gro_pre_sta_init = 1.F;
static groupshared precise const float g_gro_pre_sta_con_init = 1.F;
groupshared precise const float g_gro_pre_con_init = 1.F;
static groupshared float g_gro_sta_init = 1.F;
static groupshared const float g_gro_sta_con_init = 1.F;
groupshared const float g_gro_con_init = 1.F;
precise const float g_pre_init = 1.F;
static precise float g_pre_sta_init = 1.F;
static precise const float g_pre_sta_con_init = 1.F;
precise uniform const float g_pre_uni_init = 1.F;
static float g_sta_init = 1.F;
static const float g_sta_con_init = 1.F;
uniform const float g_uni_init = 1.F;
typedef volatile float2 t_pre_vol;
typedef const volatile float2 t_pre_vol_con;
typedef const float2 t_pre_con;
typedef volatile float2 t_vol;
typedef const volatile float2 t_vol_con;
typedef const float2 t_con;
struct s_storage_mods {
  precise float2 f_pre;
};
struct s_interp_mods {
  nointerpolation float2 f_noi;
  linear float2 f_lin;
  linear sample float2 f_lin_sam;
  linear sample noperspective float2 f_lin_sam_nop;
  linear noperspective float2 f_lin_nop;
  linear noperspective centroid float2 f_lin_nop_cen;
  linear centroid float2 f_lin_cen;
  sample float2 f_sam;
  sample noperspective float2 f_sam_nop;
  noperspective float2 f_nop;
  noperspective centroid float2 f_nop_cen;
  centroid float2 f_cen;
};
float4 foo_pre(precise float4 val) {
  return val;
}


float4 foo_pre_uni(precise uniform float4 val) {
  return val;
}


float4 foo_pre_uni_con(precise uniform const float4 val) {
  return val;
}


float4 foo_pre_con(precise const float4 val) {
  return val;
}


float4 foo_uni(uniform float4 val) {
  return val;
}


float4 foo_uni_con(uniform const float4 val) {
  return val;
}


float4 foo_con(const float4 val) {
  return val;
}


float4 foo_noi(nointerpolation float4 val) {
  return val;
}


float4 foo_lin(linear float4 val) {
  return val;
}


float4 foo_lin_sam(linear sample float4 val) {
  return val;
}


float4 foo_lin_sam_nop(linear sample noperspective float4 val) {
  return val;
}


float4 foo_lin_nop(linear noperspective float4 val) {
  return val;
}


float4 foo_lin_nop_cen(linear noperspective centroid float4 val) {
  return val;
}


float4 foo_lin_cen(linear centroid float4 val) {
  return val;
}


float4 foo_sam(sample float4 val) {
  return val;
}


float4 foo_sam_nop(sample noperspective float4 val) {
  return val;
}


float4 foo_nop(noperspective float4 val) {
  return val;
}


float4 foo_nop_cen(noperspective centroid float4 val) {
  return val;
}


float4 foo_cen(centroid float4 val) {
  return val;
}


float4 foo_in(in float4 val) {
  return val;
}


float4 foo_out(out float4 val) {
  val = float4(1.F, 2.F, 3.F, 4.F);
  return val;
}


float4 foo_inout(inout float4 val) {
  float4 result = val;
  val = val + float4(1.F, 2.F, 3.F, 4.F);
  return result;
}


float4 foo_out3(out linear float4 val) {
  val = float4(1.F, 2.F, 3.F, 4.F);
  return val;
}


float4 foo_out4(linear out float4 val) {
  val = float4(1.F, 2.F, 3.F, 4.F);
  return val;
}


float4 foo_out6(noperspective out linear float4 val) {
  val = float4(1.F, 2.F, 3.F, 4.F);
  return val;
}


float4 foo_in_missing_in_decl(float4 val);
float4 foo_in_missing_in_decl(in float4 val) {
  return val;
}


float4 foo_in_missing_in_def(in float4 val);
float4 foo_in_missing_in_def(float4 val) {
  return val;
}


float4 use_conflicting_inout(float4 val) {
  float4 out1, inout1, in1, res1;
  float4 out2, inout2, in2, res2;
  res1 = foo_in_missing_in_def(in1);
  res2 = foo_in_missing_in_decl(in2);
  return res1 + res2 + inout1 + inout2;
}


float4 foo_noi_decl(nointerpolation float4 val);
float4 foo_interpolation_different_decl(nointerpolation float4 val);
float4 foo_interpolation_different_decl(sample float4 val) {
  return val;
}


void vain() {
  precise float l_pre;
  static precise float l_pre_sta;
  static precise volatile float l_pre_sta_vol;
  static precise const volatile float l_pre_sta_vol_con;
  static precise const float l_pre_sta_con;
  precise volatile float l_pre_vol;
  static float l_sta;
  static volatile float l_sta_vol;
  static const volatile float l_sta_vol_con;
  static const float l_sta_con;
  volatile float l_vol;
  static precise const volatile float l_pre_sta_vol_con_init = 0.;
  static precise const float l_pre_sta_con_init = 0.;
  precise const volatile float l_pre_vol_con_init = 0.;
  precise const float l_pre_con_init = 0.;
  static const volatile float l_sta_vol_con_init = 0.;
  static const float l_sta_con_init = 0.;
  const volatile float l_vol_con_init = 0.;
  const float l_con_init = 0.;
}


precise float fn_pre() {
  return 1.F;
}


static float fn_sta() {
  return 1.F;
}


static const float fn_sta_con() {
  return 1.F;
}


const float fn_con() {
  return 1.F;
}


nointerpolation float fn_noi() {
  return 1.F;
}


linear float fn_lin() {
  return 1.F;
}


linear sample float fn_lin_sam() {
  return 1.F;
}


linear sample noperspective float fn_lin_sam_nop() {
  return 1.F;
}


linear noperspective float fn_lin_nop() {
  return 1.F;
}


linear noperspective centroid float fn_lin_nop_cen() {
  return 1.F;
}


linear centroid float fn_lin_cen() {
  return 1.F;
}


sample float fn_sam() {
  return 1.F;
}


sample noperspective float fn_sam_nop() {
  return 1.F;
}


noperspective float fn_nop() {
  return 1.F;
}


noperspective centroid float fn_nop_cen() {
  return 1.F;
}


centroid float fn_cen() {
  return 1.F;
}


class C {
  precise float fn_pre()   {
    return 1.F;
  }


  static precise float fn_pre_sta()   {
    return 1.F;
  }


  static float fn_sta()   {
    return 1.F;
  }


  static const float fn_sta_con()   {
    return 1.F;
  }


  const float fn_con()   {
    return 1.F;
  }


};
