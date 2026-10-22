// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

// To test with the classic compiler, run
// fxc.exe /T vs_5_1 struct-assignments.hlsl

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

// Classes
class c_f2 {  // uses 'this.' for member access.
  float2 f2;
  void set(float2 v);
  float2 get();
  float2 get_inc();
  float2 get_inc_inline() {
    return this.f2++;
  }
};
void c_f2::set(float2 v) {
  this.f2 = v;
}
float2 c_f2::get() {
  return this.f2;
}
float2 c_f2::get_inc() {
  return this.f2++;
}

class c_f3 {
  float3 f3;
  void set(float3 v);
  float3 get();
  float3 get_inc();
  float3 get_inc_inline() {
    return f3++;
  }
};
void c_f3::set(float3 v) {
  f3 = v;
}
float3 c_f3::get() {
  return f3;
}
float3 c_f3::get_inc() {
  return f3++;
}

void main() {

  s_f2 zsf2_zero_cast = (s_f2)1;

  // No initialization.
  s_f sf_none;

  // Direct initialization fails.

  c_f2 cf2 = { float2(1,2)};

  // Initialization list with members.
  s_f  sf_all       = { 0.1f };
  s_f2 sf2_all      = { float2(1, 2) };
  //s_f2 sf2_all_flat = { 0.1f, 0.2f };
 // s_ff sff_all      = { 0.1f, 0.2f };
  s_f3_f3 sf3f3_all = { float3(1,2,3), float3(3,2,1) };
  //s_f3_f3 sf3f3_all_flat = { 1,2,3, 3,2,1 };
//  s_f3_f3 sf3f3_all_straddle = { float2(1,2), float2(3,4), float2(5,6) };
 // s_f3_f3 sf3f3_all_straddle_instances[2] = { float2(1,2), sf3f3_all, float4(1,2,3,4) };
  //s_f3_f3 sf3f3_nested = { { 1, 2, 3, 4, 5 }, { 1 } };

  // zero must be cast to struct for assignment to struct
  s_f2 sf2_zero_cast = (s_f2)1;

  // Initialization list with insufficient members fails.

  // Initialization list with too many members fails.

  // Initialization list with different element types.
  //s_f2 f2_ints = { 1, 2 };

  // Initialization list with different element types.
  double d = 0.123;
 // s_f2 f2_int_double = { 1, d };

  // Initialization list with packed element.
  //s_f2 f2_f2 = { f2_ints };

  // Initialization list with mixed packed elements.
  s_ff sff = { 1, 0.1f };


  s_f2 sf2_zero_assign;

  // Initialization list with too few mixed packed elements fails.

  // Constructor with wrong element count fails.

  // Simple parameter passing.
  pick_one(sf2_all);
  sf2_all = pick_two(sf2_all);

}