// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

void main() {

  // 32-bit uint to various 64-bit types.
  uint a;
// CHECK:            [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT: [[a_ulong:%[0-9]+]] = OpUConvert %ulong [[a]]
// CHECK-NEXT:                    OpStore %b [[a_ulong]]
  uint64_t b = a;
// CHECK:            [[a_0:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT: [[a_ulong_0:%[0-9]+]] = OpUConvert %ulong [[a_0]]
// CHECK-NEXT:[[a_double:%[0-9]+]] = OpConvertUToF %double [[a_ulong_0]]
// CHECK-NEXT:                    OpStore %c [[a_double]]
  double   c = a;
// CHECK:            [[a_1:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT: [[a_ulong_1:%[0-9]+]] = OpUConvert %ulong [[a_1]]
// CHECK-NEXT:  [[a_long:%[0-9]+]] = OpBitcast %long [[a_ulong_1]]
// CHECK-NEXT:                    OpStore %d [[a_long]]
  int64_t  d = a;


  // 32-bit int to various 64-bit types.
  int aa;
// CHECK:            [[aa:%[0-9]+]] = OpLoad %int %aa
// CHECK-NEXT:  [[aa_long:%[0-9]+]] = OpSConvert %long [[aa]]
// CHECK-NEXT: [[aa_ulong:%[0-9]+]] = OpBitcast %ulong [[aa_long]]
// CHECK-NEXT:                     OpStore %bb [[aa_ulong]]
  uint64_t bb = aa;
// CHECK:             [[aa_0:%[0-9]+]] = OpLoad %int %aa
// CHECK-NEXT:   [[aa_long_0:%[0-9]+]] = OpSConvert %long [[aa_0]]
// CHECK-NEXT: [[aa_double:%[0-9]+]] = OpConvertSToF %double [[aa_long_0]]
// CHECK-NEXT:                      OpStore %cc [[aa_double]]
  double   cc = aa;
// CHECK:           [[aa_1:%[0-9]+]] = OpLoad %int %aa
// CHECK-NEXT: [[aa_long_1:%[0-9]+]] = OpSConvert %long [[aa_1]]
// CHECK-NEXT:                    OpStore %dd [[aa_long_1]]
  int64_t  dd = aa;


  // 32-bit float to various 64-bit types.
  float aaa;
// CHECK:             [[aaa:%[0-9]+]] = OpLoad %float %aaa
// CHECK-NEXT:  [[aaa_ulong:%[0-9]+]] = OpConvertFToU %ulong [[aaa]]
// CHECK-NEXT:                       OpStore %bbb [[aaa_ulong]]
  uint64_t bbb = aaa;
// CHECK:             [[aaa_0:%[0-9]+]] = OpLoad %float %aaa
// CHECK-NEXT: [[aaa_double_0:%[0-9]+]] = OpFConvert %double [[aaa_0]]
// CHECK-NEXT:                       OpStore %ccc [[aaa_double_0]]
  double   ccc = aaa;
// CHECK:             [[aaa_1:%[0-9]+]] = OpLoad %float %aaa
// CHECK-NEXT:   [[aaa_long:%[0-9]+]] = OpConvertFToS %long [[aaa_1]]
// CHECK-NEXT:                       OpStore %ddd [[aaa_long]]
  int64_t  ddd = aaa;


  // 64-bit uint to various 32-bit types.
  uint64_t e;
// CHECK:      [[e64:%[0-9]+]] = OpLoad %ulong %e
// CHECK-NEXT: [[e32:%[0-9]+]] = OpUConvert %uint [[e64]]
// CHECK-NEXT:                OpStore %f [[e32]]
  uint  f = e;
// CHECK:          [[e64_0:%[0-9]+]] = OpLoad %ulong %e
// CHECK-NEXT:     [[e32_0:%[0-9]+]] = OpUConvert %uint [[e64_0]]
// CHECK-NEXT: [[e_float:%[0-9]+]] = OpConvertUToF %float [[e32_0]]
// CHECK-NEXT:                    OpStore %g [[e_float]]
  float g = e;
// CHECK:        [[e64_1:%[0-9]+]] = OpLoad %ulong %e
// CHECK-NEXT:   [[e32_1:%[0-9]+]] = OpUConvert %uint [[e64_1]]
// CHECK-NEXT: [[e_int:%[0-9]+]] = OpBitcast %int [[e32_1]]
// CHECK-NEXT:                  OpStore %h [[e_int]]
  int   h = e;


  // 64-bit int to various 32-bit types.
  int64_t ee;
// CHECK:           [[e:%[0-9]+]] = OpLoad %long %ee
// CHECK-NEXT:  [[e_int_0:%[0-9]+]] = OpSConvert %int [[e]]
// CHECK-NEXT: [[e_uint:%[0-9]+]] = OpBitcast %uint [[e_int_0]]
// CHECK-NEXT:                   OpStore %ff [[e_uint]]
  uint  ff = ee;
// CHECK:            [[e_0:%[0-9]+]] = OpLoad %long %ee
// CHECK-NEXT:   [[e_int_1:%[0-9]+]] = OpSConvert %int [[e_0]]
// CHECK-NEXT: [[e_float_0:%[0-9]+]] = OpConvertSToF %float [[e_int_1]]
// CHECK-NEXT:                    OpStore %gg [[e_float_0]]
  float gg = ee;
// CHECK:          [[e_1:%[0-9]+]] = OpLoad %long %ee
// CHECK-NEXT: [[e_int_2:%[0-9]+]] = OpSConvert %int [[e_1]]
// CHECK-NEXT:                  OpStore %hh [[e_int_2]]
  int   hh = ee;


  // 64-bit float to various 32-bit types.
  double eee;
// CHECK:         [[e64_2:%[0-9]+]] = OpLoad %double %eee
// CHECK-NEXT: [[e_uint_0:%[0-9]+]] = OpConvertFToU %uint [[e64_2]]
// CHECK-NEXT:                   OpStore %fff [[e_uint_0]]
  uint  fff = eee;
// CHECK:              [[e_2:%[0-9]+]] = OpLoad %double %eee
// CHECK-NEXT:   [[e_float_1:%[0-9]+]] = OpFConvert %float [[e_2]]
// CHECK-NEXT:                      OpStore %ggg [[e_float_1]]
  float ggg = eee;
// CHECK:            [[e_3:%[0-9]+]] = OpLoad %double %eee
// CHECK-NEXT:   [[e_int_3:%[0-9]+]] = OpConvertFToS %int [[e_3]]
// CHECK-NEXT:                    OpStore %hhh [[e_int_3]]
  int   hhh = eee;


  // Vector case: 64-bit float to various 32-bit types.
  double2 i;
// CHECK:      [[i_double:%[0-9]+]] = OpLoad %v2double %i
// CHECK-NEXT:   [[i_uint:%[0-9]+]] = OpConvertFToU %v2uint [[i_double]]
// CHECK-NEXT:                     OpStore %j [[i_uint]]
  uint2   j = i;
// CHECK:      [[i_double_0:%[0-9]+]] = OpLoad %v2double %i
// CHECK-NEXT:    [[i_int:%[0-9]+]] = OpConvertFToS %v2int [[i_double_0]]
// CHECK-NEXT:                     OpStore %k [[i_int]]
  int2    k = i;
// CHECK:      [[i_double_1:%[0-9]+]] = OpLoad %v2double %i
// CHECK-NEXT:  [[i_float_1:%[0-9]+]] = OpFConvert %v2float [[i_double_1]]
// CHECK-NEXT:                     OpStore %l [[i_float_1]]
  float2  l = i;


  // 16-bit uint to various 32-bit types.
  uint16_t m;
// CHECK:      [[m_ushort:%[0-9]+]] = OpLoad %ushort %m
// CHECK-NEXT:   [[m_uint:%[0-9]+]] = OpUConvert %uint [[m_ushort]]
// CHECK-NEXT:                     OpStore %n [[m_uint]]
  uint  n = m;
// CHECK:      [[m_ushort_0:%[0-9]+]] = OpLoad %ushort %m
// CHECK-NEXT:   [[m_uint_0:%[0-9]+]] = OpUConvert %uint [[m_ushort_0]]
// CHECK-NEXT:  [[m_float:%[0-9]+]] = OpConvertUToF %float [[m_uint_0]]
// CHECK-NEXT:                     OpStore %o [[m_float]]
  float o = m;
// CHECK:      [[m_ushort_1:%[0-9]+]] = OpLoad %ushort %m
// CHECK-NEXT:   [[m_uint_1:%[0-9]+]] = OpUConvert %uint [[m_ushort_1]]
// CHECK-NEXT:    [[m_int:%[0-9]+]] = OpBitcast %int [[m_uint_1]]
// CHECK-NEXT:                     OpStore %p [[m_int]]
  int   p = m;


  // 16-bit int to various 32-bit types.
  int16_t mm;
// CHECK:      [[mm_short:%[0-9]+]] = OpLoad %short %mm
// CHECK-NEXT:   [[mm_int:%[0-9]+]] = OpSConvert %int [[mm_short]]
// CHECK-NEXT:  [[mm_uint:%[0-9]+]] = OpBitcast %uint [[mm_int]]
// CHECK-NEXT:                     OpStore %nn [[mm_uint]]
  uint  nn = mm;
// CHECK:      [[mm_short_0:%[0-9]+]] = OpLoad %short %mm
// CHECK-NEXT:   [[mm_int_0:%[0-9]+]] = OpSConvert %int [[mm_short_0]]
// CHECK-NEXT: [[mm_float:%[0-9]+]] = OpConvertSToF %float [[mm_int_0]]
// CHECK-NEXT:                     OpStore %oo [[mm_float]]
  float oo = mm;
// CHECK:      [[mm_short_1:%[0-9]+]] = OpLoad %short %mm
// CHECK-NEXT:   [[mm_int_1:%[0-9]+]] = OpSConvert %int [[mm_short_1]]
// CHECK-NEXT:                     OpStore %pp [[mm_int_1]]
  int   pp = mm;
}
