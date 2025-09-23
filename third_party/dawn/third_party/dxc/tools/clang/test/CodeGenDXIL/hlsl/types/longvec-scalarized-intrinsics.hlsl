// RUN: %dxc -T lib_6_9 %s | FileCheck %s

// Long vector tests for vec ops that scalarize to something more complex
//  than a simple repetition of the same dx.op calls.

// CHECK-LABEL: test_atan2
// CHECK: fdiv fast <8 x float>
// CHECK: call <8 x float> @dx.op.unary.v8f32(i32 17, <8 x float> %{{.*}}) ; Atan(value)
// CHECK: fadd fast <8 x float> %{{.*}}, <float 0x
// CHECK: fadd fast <8 x float> %{{.*}}, <float 0x
// CHECK: fcmp fast olt <8 x float>
// CHECK: fcmp fast oeq <8 x float>
// CHECK: fcmp fast oge <8 x float>
// CHECK: fcmp fast olt <8 x float>
// CHECK: and <8 x i1>
// CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float>
// CHECK: and <8 x i1>
// CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float>
// CHECK: and <8 x i1>
// CHECK: select <8 x i1> %{{.*}}, <8 x float> <float 0x
// CHECK: and <8 x i1>
// CHECK: select <8 x i1> %{{.*}}, <8 x float> <float 0x
export void test_atan2(inout vector<float, 8> vec1, vector<float, 8> vec2) {
  vec1 = atan2(vec1, vec2);
}

// CHECK-LABEL: test_fmod
// CHECK: fdiv fast <8 x float>
// CHECK: fsub fast <8 x float> <float
// CHECK: fcmp fast oge <8 x float>
// CHECK: call <8 x float> @dx.op.unary.v8f32(i32 6, <8 x float> %{{.*}}) ; FAbs(value)
// CHECK: call <8 x float> @dx.op.unary.v8f32(i32 22, <8 x float> %{{.*}}) ; Frc(value)

// CHECK: fsub fast <8 x float> <float
// CHECK: select <8 x i1> %{{.*}}, <8 x float> %{{.*}}, <8 x float>
// CHECK: fmul fast <8 x float>
export void test_fmod(inout vector<float, 8> vec1, vector<float, 8> vec2) {
  vec1 = fmod(vec1, vec2);
}

// CHECK-LABEL: test_ldexp
// CHECK: call <8 x float> @dx.op.unary.v8f32(i32 21, <8 x float> %{{.*}}) ; Exp(value)
// CHECK: fmul fast <8 x float>

export void test_ldexp(inout vector<float, 8> vec1, vector<float, 8> vec2) {
  vec1 = ldexp(vec1, vec2);
}


// CHECK-LABEL: test_pow
// CHECK: call <8 x float> @dx.op.unary.v8f32(i32 23, <8 x float> %{{.*}}) ; Log(value)
// CHECK: fmul fast <8 x float>
// CHECK: call <8 x float> @dx.op.unary.v8f32(i32 21, <8 x float> %{{.*}}) ; Exp(value)
export void test_pow(inout vector<float, 8> vec1, vector<float, 8> vec2) {
  vec1 = pow(vec1, vec2);
}

// CHECK-LABEL: test_modf
// CHECK: call <8 x float>  @dx.op.unary.v8f32(i32 29, <8 x float>  %{{.*}}) ; Round_z(value)
// CHECK: fsub fast <8 x float>
export void test_modf(inout vector<float, 8> vec1, vector<float, 8> vec2) {
  vec1 = modf(vec1, vec2);
}

// CHECK-LABEL: test_dot
// CHECK: [[el:%.*]] = extractelement <8 x float>
// CHECK: [[mul:%.*]] = fmul fast float [[el]]
// CHECK: [[ping:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[mul]]) ; FMad(a,b,c)
// CHECK: [[pong:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[ping]]) ; FMad(a,b,c)
// CHECK: [[ping:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[pong]]) ; FMad(a,b,c)
// CHECK: [[pong:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[ping]]) ; FMad(a,b,c)
// CHECK: [[ping:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[pong]]) ; FMad(a,b,c)
// CHECK: [[pong:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[ping]]) ; FMad(a,b,c)
// CHECK: [[ping:%.*]] = call float @dx.op.tertiary.f32(i32 46, float %{{.*}}, float %{{.*}}, float [[pong]]) ; FMad(a,b,c)
export void test_dot(inout vector<float, 8> vec1, vector<float, 8> vec2) {
  vec1 = dot(vec1, vec2);
}

// CHECK-LABEL: test_any
// CHECK: or i1
// CHECK: or i1
// CHECK: or i1
// CHECK: or i1
// CHECK: or i1
// CHECK: or i1
// CHECK: or i1
export void test_any(vector<float, 8> vec1, inout vector<bool, 8> bvec) {
  bvec &= any(vec1);
}

// CHECK-LABEL: test_all
// CHECK: and i1
// CHECK: and i1
// CHECK: and i1
// CHECK: and i1
// CHECK: and i1
// CHECK: and i1
// CHECK: and i1
export void test_all(vector<float, 8> vec1, inout vector<bool, 8> bvec) {
  bvec &= all(vec1);
}

// CHECK-LABEL: test_WaveMatch
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
// call {{.*}} @dx.op.wave
export uint4 test_WaveMatch(vector<bool, 8> bvec) {
  return WaveMatch(bvec);
}
