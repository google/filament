// RUN: %dxc -T vs_6_0 -DFUNC=dot %s | FileCheck %s
// RUN: %dxc -T vs_6_0 -DFUNC=mul %s | FileCheck %s
// RUN: %dxc -T vs_6_0 -DFUNC=dot -fcgl %s | FileCheck %s --check-prefix=CGLDOT
// RUN: %dxc -T vs_6_0 -DFUNC=mul -fcgl %s | FileCheck %s --check-prefix=CGLMUL

// Verifies correct implementation of dot and mul with vectors for various sizes and types.

// Partially pilfered from SPIRV's intrinsic.dot.hlsl

float4 main(int1 i1[2] : IO, int2 i2[2] : IT, int3 i3[2] : IH, int4 i4[2] : IF,
            float1 f1[2] : FO, float2 f2[2] : FT, float3 f3[2] : FH, float4 f4[2] : FF,
            uint1 u1[2] : UO, uint2 u2[2] : UT, uint3 u3[2] : UH, uint4 u4[2] : UF) : SV_Position {
  int i = 0;
  // CHECK-DAG: [[I0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 1, i8 0, i32 undef)
  // CHECK: mul i32 [[I0]], [[I1]]
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <1 x i32>, <1 x i32>)"(i32 [[IDOT:[0-9]*]], <1 x i32> %{{.*}}, <1 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <1 x i32>, <1 x i32>)"(i32 [[IMUL:[0-9]*]], <1 x i32> %{{.*}}, <1 x i32> %{{.*}})
  i += FUNC(i1[0], i1[1]);

  // CHECK-DAG: [[I00:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I01:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[I10:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[I11:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 1, i8 1, i32 undef)

  // CHECK: [[MUL:%.*]] = mul i32 [[I00]], [[I10]]
  // CHECK: call i32 @dx.op.tertiary.i32(i32 48, i32 [[I01]], i32 [[I11]], i32 [[MUL]])  ; IMad(a,b,c)
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <2 x i32>, <2 x i32>)"(i32 [[IDOT]], <2 x i32> %{{.*}}, <2 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <2 x i32>, <2 x i32>)"(i32 [[IMUL]], <2 x i32> %{{.*}}, <2 x i32> %{{.*}})
  i += FUNC(i2[0], i2[1]);

  // CHECK-DAG: [[I00:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I01:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[I02:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 0, i8 2, i32 undef)
  // CHECK-DAG: [[I10:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[I11:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 1, i8 1, i32 undef)
  // CHECK-DAG: [[I12:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 2, i32 1, i8 2, i32 undef)

  // PING and PONG are just conveniences to track the result as it accumulates.
  // Since we can't capture and match the source and result in the same line with the same variable.
  // CHECK: [[PING:%.*]] = mul i32 [[I00]], [[I10]]
  // CHECK: [[PONG:%.*]] = call i32 @dx.op.tertiary.i32(i32 48, i32 [[I01]], i32 [[I11]], i32 [[PING]])  ; IMad(a,b,c)
  // CHECK: [[PING:%.*]] = call i32 @dx.op.tertiary.i32(i32 48, i32 [[I02]], i32 [[I12]], i32 [[PONG]])  ; IMad(a,b,c)
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <3 x i32>, <3 x i32>)"(i32 [[IDOT]], <3 x i32> %{{.*}}, <3 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <3 x i32>, <3 x i32>)"(i32 [[IMUL]], <3 x i32> %{{.*}}, <3 x i32> %{{.*}})
  i += FUNC(i3[0], i3[1]);

  // CHECK-DAG: [[I00:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I01:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[I02:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 2, i32 undef)
  // CHECK-DAG: [[I03:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 0, i8 3, i32 undef)
  // CHECK-DAG: [[I10:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[I11:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 1, i8 1, i32 undef)
  // CHECK-DAG: [[I12:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 1, i8 2, i32 undef)
  // CHECK-DAG: [[I13:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 3, i32 1, i8 3, i32 undef)

  // CHECK: [[PING:%.*]] = mul i32 [[I00]], [[I10]]
  // CHECK: [[PONG:%.*]] = call i32 @dx.op.tertiary.i32(i32 48, i32 [[I01]], i32 [[I11]], i32 [[PING]])  ; IMad(a,b,c)
  // CHECK: [[PING:%.*]] = call i32 @dx.op.tertiary.i32(i32 48, i32 [[I02]], i32 [[I12]], i32 [[PONG]])  ; IMad(a,b,c)
  // CHECK: [[PONG:%.*]] = call i32 @dx.op.tertiary.i32(i32 48, i32 [[I03]], i32 [[I13]], i32 [[PING]])  ; IMad(a,b,c)
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <4 x i32>, <4 x i32>)"(i32 [[IDOT]], <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <4 x i32>, <4 x i32>)"(i32 [[IMUL]], <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  i += FUNC(i4[0], i4[1]);

  float f = 0.0;

  // CHECK-DAG: [[F0:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 4, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[F1:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 4, i32 1, i8 0, i32 undef)
  // CHECK: mul fast float [[F0]], [[F1]]
  // CGLDOT: call float @"dx.hl.op.rn.float (i32, <1 x float>, <1 x float>)"(i32 [[IDOT]], <1 x float> %{{.*}}, <1 x float> %{{.*}})
  // CGLMUL: call float @"dx.hl.op.rn.float (i32, <1 x float>, <1 x float>)"(i32 [[IMUL]], <1 x float> %{{.*}}, <1 x float> %{{.*}})
  f += FUNC(f1[0], f1[1]);

  // CHECK-DAG: [[F00:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 5, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[F01:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 5, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[F10:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 5, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[F11:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 5, i32 1, i8 1, i32 undef)

  // CHECK: call float @dx.op.dot2.f32(i32 54, float [[F00]], float [[F01]], float [[F10]], float [[F11]])
  // CGLDOT: call float @"dx.hl.op.rn.float (i32, <2 x float>, <2 x float>)"(i32 [[IDOT]], <2 x float> %{{.*}}, <2 x float> %{{.*}})
  // CGLMUL: call float @"dx.hl.op.rn.float (i32, <2 x float>, <2 x float>)"(i32 [[IMUL]], <2 x float> %{{.*}}, <2 x float> %{{.*}})
  f += FUNC(f2[0], f2[1]);

  // CHECK-DAG: [[F00:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 6, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[F01:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 6, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[F02:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 6, i32 0, i8 2, i32 undef)
  // CHECK-DAG: [[F10:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 6, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[F11:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 6, i32 1, i8 1, i32 undef)
  // CHECK-DAG: [[F12:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 6, i32 1, i8 2, i32 undef)

  // CHECK: call float @dx.op.dot3.f32(i32 55, float [[F00]], float [[F01]], float [[F02]], float [[F10]], float [[F11]], float [[F12]])
  // CGLDOT: call float @"dx.hl.op.rn.float (i32, <3 x float>, <3 x float>)"(i32 [[IDOT]], <3 x float> %{{.*}}, <3 x float> %{{.*}})
  // CGLMUL: call float @"dx.hl.op.rn.float (i32, <3 x float>, <3 x float>)"(i32 [[IMUL]], <3 x float> %{{.*}}, <3 x float> %{{.*}})
  f += FUNC(f3[0], f3[1]);

  // CHECK-DAG: [[F00:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[F01:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[F02:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 0, i8 2, i32 undef)
  // CHECK-DAG: [[F03:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 0, i8 3, i32 undef)
  // CHECK-DAG: [[F10:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[F11:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 1, i8 1, i32 undef)
  // CHECK-DAG: [[F12:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 1, i8 2, i32 undef)
  // CHECK-DAG: [[F13:%.*]] = call float @dx.op.loadInput.f32(i32 4, i32 7, i32 1, i8 3, i32 undef)

  // CHECK: call float @dx.op.dot4.f32(i32 56, float [[F00]], float [[F01]], float [[F02]], float [[F03]], float [[F10]], float [[F11]], float [[F12]], float [[F13]])
  // CGLDOT: call float @"dx.hl.op.rn.float (i32, <4 x float>, <4 x float>)"(i32 [[IDOT]], <4 x float> %{{.*}}, <4 x float> %{{.*}})
  // CGLMUL: call float @"dx.hl.op.rn.float (i32, <4 x float>, <4 x float>)"(i32 [[IMUL]], <4 x float> %{{.*}}, <4 x float> %{{.*}})
  f += FUNC(f4[0], f4[1]);

  int u = 0;
  // CHECK-DAG: [[I0:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 8, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I1:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 8, i32 1, i8 0, i32 undef)
  // CHECK: mul i32 [[I0]], [[I1]]
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <1 x i32>, <1 x i32>)"(i32 [[UDOT:[0-9]*]], <1 x i32> %{{.*}}, <1 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <1 x i32>, <1 x i32>)"(i32 [[UMUL:[0-9]*]], <1 x i32> %{{.*}}, <1 x i32> %{{.*}})
  u += FUNC(u1[0], u1[1]);

  // CHECK-DAG: [[I00:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 9, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I01:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 9, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[I10:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 9, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[I11:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 9, i32 1, i8 1, i32 undef)

  // CHECK: [[MUL:%.*]] = mul i32 [[I00]], [[I10]]
  // CHECK: call i32 @dx.op.tertiary.i32(i32 49, i32 [[I01]], i32 [[I11]], i32 [[MUL]])  ; UMad(a,b,c)
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <2 x i32>, <2 x i32>)"(i32 [[UDOT]], <2 x i32> %{{.*}}, <2 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <2 x i32>, <2 x i32>)"(i32 [[UMUL]], <2 x i32> %{{.*}}, <2 x i32> %{{.*}})
  u += FUNC(u2[0], u2[1]);

  // CHECK-DAG: [[I00:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 10, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I01:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 10, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[I02:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 10, i32 0, i8 2, i32 undef)
  // CHECK-DAG: [[I10:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 10, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[I11:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 10, i32 1, i8 1, i32 undef)
  // CHECK-DAG: [[I12:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 10, i32 1, i8 2, i32 undef)

  // CHECK: [[PING:%.*]] = mul i32 [[I00]], [[I10]]
  // CHECK: [[PONG:%.*]] = call i32 @dx.op.tertiary.i32(i32 49, i32 [[I01]], i32 [[I11]], i32 [[PING]])  ; UMad(a,b,c)
  // CHECK: [[PING:%.*]] = call i32 @dx.op.tertiary.i32(i32 49, i32 [[I02]], i32 [[I12]], i32 [[PONG]])  ; UMad(a,b,c)
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <3 x i32>, <3 x i32>)"(i32 [[UDOT]], <3 x i32> %{{.*}}, <3 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <3 x i32>, <3 x i32>)"(i32 [[UMUL]], <3 x i32> %{{.*}}, <3 x i32> %{{.*}})
  u += FUNC(u3[0], u3[1]);

  // CHECK-DAG: [[I00:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 0, i8 0, i32 undef)
  // CHECK-DAG: [[I01:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 0, i8 1, i32 undef)
  // CHECK-DAG: [[I02:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 0, i8 2, i32 undef)
  // CHECK-DAG: [[I03:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 0, i8 3, i32 undef)
  // CHECK-DAG: [[I10:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 1, i8 0, i32 undef)
  // CHECK-DAG: [[I11:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 1, i8 1, i32 undef)
  // CHECK-DAG: [[I12:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 1, i8 2, i32 undef)
  // CHECK-DAG: [[I13:%.*]] = call i32 @dx.op.loadInput.i32(i32 4, i32 11, i32 1, i8 3, i32 undef)

  // CHECK: [[PING:%.*]] = mul i32 [[I00]], [[I10]]
  // CHECK: [[PONG:%.*]] = call i32 @dx.op.tertiary.i32(i32 49, i32 [[I01]], i32 [[I11]], i32 [[PING]])  ; UMad(a,b,c)
  // CHECK: [[PING:%.*]] = call i32 @dx.op.tertiary.i32(i32 49, i32 [[I02]], i32 [[I12]], i32 [[PONG]])  ; UMad(a,b,c)
  // CHECK: [[PONG:%.*]] = call i32 @dx.op.tertiary.i32(i32 49, i32 [[I03]], i32 [[I13]], i32 [[PING]])  ; UMad(a,b,c)
  // CGLDOT: call i32 @"dx.hl.op.rn.i32 (i32, <4 x i32>, <4 x i32>)"(i32 [[UDOT]], <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  // CGLMUL: call i32 @"dx.hl.op.rn.i32 (i32, <4 x i32>, <4 x i32>)"(i32 [[UMUL]], <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  u += FUNC(u4[0], u4[1]);

  return float4(i, f, u, 0);
}
