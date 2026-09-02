// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Note: Matrix truncation cast does not allow truncation into a mat1x1.

void main() {
  float3x4 m = { 1,  2,  3, 4,
                 5,  6,  7, 8,
                 9, 10, 11, 12 };

  float1x4 n = {1, 2, 3, 4};
  float3x1 o = {1, 2, 3};


// TEST: float3x4 --> float2x3

// CHECK:           [[m_0:%[0-9]+]] = OpLoad %mat3v4float %m
// CHECK-NEXT: [[m_0_row0:%[0-9]+]] = OpCompositeExtract %v4float [[m_0]] 0
// CHECK-NEXT:   [[a_row0:%[0-9]+]] = OpVectorShuffle %v3float [[m_0_row0]] [[m_0_row0]] 0 1 2
// CHECK-NEXT: [[m_0_row1:%[0-9]+]] = OpCompositeExtract %v4float [[m_0]] 1
// CHECK-NEXT:   [[a_row1:%[0-9]+]] = OpVectorShuffle %v3float [[m_0_row1]] [[m_0_row1]] 0 1 2
// CHECK-NEXT:          {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[a_row0]] [[a_row1]]
  float2x3 a = (float2x3)m;


// TEST: float3x4 --> float1x3

// CHECK:           [[m_1:%[0-9]+]] = OpLoad %mat3v4float %m
// CHECK-NEXT: [[m_1_row0:%[0-9]+]] = OpCompositeExtract %v4float [[m_1]] 0
// CHECK-NEXT:          {{%[0-9]+}} = OpVectorShuffle %v3float [[m_1_row0]] [[m_1_row0]] 0 1 2
  float1x3 b = m;


// TEST: float3x4 --> float1x4

// CHECK:      [[m_2:%[0-9]+]] = OpLoad %mat3v4float %m
// CHECK-NEXT:     {{%[0-9]+}} = OpCompositeExtract %v4float [[m_2]] 0
  float1x4 c = m;


// TEST: float3x4 --> float2x1

// CHECK:           [[m_3:%[0-9]+]] = OpLoad %mat3v4float %m
// CHECK-NEXT: [[m_3_row0:%[0-9]+]] = OpCompositeExtract %v4float [[m_3]] 0
// CHECK-NEXT:   [[d_row0:%[0-9]+]] = OpCompositeExtract %float [[m_3_row0]] 0
// CHECK-NEXT: [[m_3_row1:%[0-9]+]] = OpCompositeExtract %v4float [[m_3]] 1
// CHECK-NEXT:   [[d_row1:%[0-9]+]] = OpCompositeExtract %float [[m_3_row1]] 0
// CHECK-NEXT:          {{%[0-9]+}} = OpCompositeConstruct %v2float [[d_row0]] [[d_row1]]
  float2x1 d = m;


// TEST: float3x4 --> float3x1

// CHECK:           [[m_4:%[0-9]+]] = OpLoad %mat3v4float %m
// CHECK-NEXT: [[m_4_row0:%[0-9]+]] = OpCompositeExtract %v4float [[m_4]] 0
// CHECK-NEXT:   [[e_row0:%[0-9]+]] = OpCompositeExtract %float [[m_4_row0]] 0
// CHECK-NEXT: [[m_4_row1:%[0-9]+]] = OpCompositeExtract %v4float [[m_4]] 1
// CHECK-NEXT:   [[e_row1:%[0-9]+]] = OpCompositeExtract %float [[m_4_row1]] 0
// CHECK-NEXT: [[m_4_row2:%[0-9]+]] = OpCompositeExtract %v4float [[m_4]] 2
// CHECK-NEXT:   [[e_row2:%[0-9]+]] = OpCompositeExtract %float [[m_4_row2]] 0
// CHECK-NEXT:          {{%[0-9]+}} = OpCompositeConstruct %v3float [[e_row0]] [[e_row1]] [[e_row2]]
  float3x1 e = (float3x1)m;


// TEST float1x4 --> float1x3

// CHECK:      [[n:%[0-9]+]] = OpLoad %v4float %n
// CHECK-NEXT:   {{%[0-9]+}} = OpVectorShuffle %v3float [[n]] [[n]] 0 1 2
  float1x3 f = n;


// TEST float1x4 --> float1x3

// CHECK:       [[n_0:%[0-9]+]] = OpLoad %v4float %n
// CHECK-NEXT: [[n0:%[0-9]+]] = OpCompositeExtract %float [[n_0]] 0
// CHECK-NEXT:               OpStore %scalar [[n0]]
  float1x1 scalar = n;


// TEST float3x1 --> float2x1

// CHECK:      [[o:%[0-9]+]] = OpLoad %v3float %o
// CHECK-NEXT:   {{%[0-9]+}} = OpVectorShuffle %v2float [[o]] [[o]] 0 1
  float2x1 g = (float2x1)o;

  // Non-floating point matrices
  int3x4 h;
  int2x3 i;
  int3x1 j;
  int1x4 k;
// CHECK:       [[h:%[0-9]+]] = OpLoad %_arr_v4int_uint_3 %h
// CHECK-NEXT: [[h0:%[0-9]+]] = OpCompositeExtract %v4int [[h]] 0
// CHECK-NEXT: [[i0:%[0-9]+]] = OpVectorShuffle %v3int [[h0]] [[h0]] 0 1 2
// CHECK-NEXT: [[h1:%[0-9]+]] = OpCompositeExtract %v4int [[h]] 1
// CHECK-NEXT: [[i1:%[0-9]+]] = OpVectorShuffle %v3int [[h1]] [[h1]] 0 1 2
// CHECK-NEXT:  [[i:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[i0]] [[i1]]
// CHECK-NEXT:               OpStore %i [[i]]
  i = (int2x3)h;
// CHECK:         [[h_0:%[0-9]+]] = OpLoad %_arr_v4int_uint_3 %h
// CHECK-NEXT:   [[h0_0:%[0-9]+]] = OpCompositeExtract %v4int [[h_0]] 0
// CHECK-NEXT: [[h0e0:%[0-9]+]] = OpCompositeExtract %int [[h0_0]] 0
// CHECK-NEXT:   [[h1_0:%[0-9]+]] = OpCompositeExtract %v4int [[h_0]] 1
// CHECK-NEXT: [[h1e0:%[0-9]+]] = OpCompositeExtract %int [[h1_0]] 0
// CHECK-NEXT:   [[h2:%[0-9]+]] = OpCompositeExtract %v4int [[h_0]] 2
// CHECK-NEXT: [[h2e0:%[0-9]+]] = OpCompositeExtract %int [[h2]] 0
// CHECK-NEXT:    [[j:%[0-9]+]] = OpCompositeConstruct %v3int [[h0e0]] [[h1e0]] [[h2e0]]
// CHECK-NEXT:                 OpStore %j [[j]]
  j = (int3x1)h;
// CHECK:       [[h_1:%[0-9]+]] = OpLoad %_arr_v4int_uint_3 %h
// CHECK-NEXT: [[h0_1:%[0-9]+]] = OpCompositeExtract %v4int [[h_1]] 0
// CHECK-NEXT:               OpStore %k [[h0_1]]
  k = (int1x4)h;

  bool3x4 p;
  bool2x3 q;
  bool3x1 r;
  bool1x4 s;
// CHECK:       [[p:%[0-9]+]] = OpLoad %_arr_v4bool_uint_3 %p
// CHECK-NEXT: [[p0:%[0-9]+]] = OpCompositeExtract %v4bool [[p]] 0
// CHECK-NEXT: [[q0:%[0-9]+]] = OpVectorShuffle %v3bool [[p0]] [[p0]] 0 1 2
// CHECK-NEXT: [[p1:%[0-9]+]] = OpCompositeExtract %v4bool [[p]] 1
// CHECK-NEXT: [[q1:%[0-9]+]] = OpVectorShuffle %v3bool [[p1]] [[p1]] 0 1 2
// CHECK-NEXT:  [[q:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[q0]] [[q1]]
// CHECK-NEXT:               OpStore %q [[q]]
  q = (bool2x3)p;
// CHECK:         [[p_0:%[0-9]+]] = OpLoad %_arr_v4bool_uint_3 %p
// CHECK-NEXT:   [[p0_0:%[0-9]+]] = OpCompositeExtract %v4bool [[p_0]] 0
// CHECK-NEXT: [[p0e0:%[0-9]+]] = OpCompositeExtract %bool [[p0_0]] 0
// CHECK-NEXT:   [[p1_0:%[0-9]+]] = OpCompositeExtract %v4bool [[p_0]] 1
// CHECK-NEXT: [[p1e0:%[0-9]+]] = OpCompositeExtract %bool [[p1_0]] 0
// CHECK-NEXT:   [[p2:%[0-9]+]] = OpCompositeExtract %v4bool [[p_0]] 2
// CHECK-NEXT: [[p2e0:%[0-9]+]] = OpCompositeExtract %bool [[p2]] 0
// CHECK-NEXT:    [[r:%[0-9]+]] = OpCompositeConstruct %v3bool [[p0e0]] [[p1e0]] [[p2e0]]
// CHECK-NEXT:                 OpStore %r [[r]]
  r = (bool3x1)p;
// CHECK:       [[p_1:%[0-9]+]] = OpLoad %_arr_v4bool_uint_3 %p
// CHECK-NEXT: [[p0_1:%[0-9]+]] = OpCompositeExtract %v4bool [[p_1]] 0
// CHECK-NEXT:               OpStore %s [[p0_1]]
  s = (bool1x4)p;
}
