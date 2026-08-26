// REQUIRES: dxil-1-9
// RUN: %dxc -T lib_6_9 -enable-16bit-types %s | FileCheck %s

// CHECK-LABEL: test_isnan_half
// CHECK:  call i1 @dx.op.isSpecialFloat.f16(i32 8, half
export bool test_isnan_half(half h) {
  return isnan(h);
}

// CHECK-LABEL: test_isnan_half2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 8, <2 x half>
export bool2 test_isnan_half2(half2 h) {
  return isnan(h);
}

// CHECK-LABEL: test_isnan_half3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 8, <3 x half>
export bool3 test_isnan_half3(half3 h) {
  return isnan(h);
}

// CHECK-LABEL: test_isnan_half4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 8, <4 x half>
export bool4 test_isnan_half4(half4 h) {
  return isnan(h);
}

// CHECK-LABEL: test_isnan_float
// CHECK:  call i1 @dx.op.isSpecialFloat.f32(i32 8, float
export bool test_isnan_float(float h) {
  return isnan(h);
}

// CHECK-LABEL: test_isnan_float2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f32(i32 8, <2 x float>
export bool2 test_isnan_float2(float2 h) {
  return isnan(h);
}

// CHECK-LABEL: test_isnan_float3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f32(i32 8, <3 x float>
export bool3 test_isnan_float3(float3 h) {
  return isnan(h);
}

// CHECK-LABEL: test_isnan_float4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f32(i32 8, <4 x float>
export bool4 test_isnan_float4(float4 h) {
  return isnan(h);
}

// CHECK-LABEL: test_isinf_half
// CHECK:  call i1 @dx.op.isSpecialFloat.f16(i32 9, half
export bool test_isinf_half(half h) {
  return isinf(h);
}

// CHECK-LABEL: test_isinf_half2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 9, <2 x half>
export bool2 test_isinf_half2(half2 h) {
  return isinf(h);
}

// CHECK-LABEL: test_isinf_half3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 9, <3 x half>
export bool3 test_isinf_half3(half3 h) {
  return isinf(h);
}

// CHECK-LABEL: test_isinf_half4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 9, <4 x half>
export bool4 test_isinf_half4(half4 h) {
  return isinf(h);
}

// CHECK-LABEL: test_isinf_float
// CHECK:  call i1 @dx.op.isSpecialFloat.f32(i32 9, float
export bool test_isinf_float(float h) {
  return isinf(h);
}

// CHECK-LABEL: test_isinf_float2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f32(i32 9, <2 x float>
export bool2 test_isinf_float2(float2 h) {
  return isinf(h);
}

// CHECK-LABEL: test_isinf_float3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f32(i32 9, <3 x float>
export bool3 test_isinf_float3(float3 h) {
  return isinf(h);
}

// CHECK-LABEL: test_isinf_float4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f32(i32 9, <4 x float>
export bool4 test_isinf_float4(float4 h) {
  return isinf(h);
}

// CHECK-LABEL: test_isfinite_half
// CHECK:  call i1 @dx.op.isSpecialFloat.f16(i32 10, half
export bool test_isfinite_half(half h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isfinite_half2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 10, <2 x half>
export bool2 test_isfinite_half2(half2 h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isfinite_half3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 10, <3 x half>
export bool3 test_isfinite_half3(half3 h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isfinite_half4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 10, <4 x half>
export bool4 test_isfinite_half4(half4 h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isfinite_float
// CHECK:  call i1 @dx.op.isSpecialFloat.f32(i32 10, float
export bool test_isfinite_float(float h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isfinite_float2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f32(i32 10, <2 x float>
export bool2 test_isfinite_float2(float2 h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isfinite_float3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f32(i32 10, <3 x float>
export bool3 test_isfinite_float3(float3 h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isfinite_float4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f32(i32 10, <4 x float>
export bool4 test_isfinite_float4(float4 h) {
  return isfinite(h);
}

// CHECK-LABEL: test_isnormal_half
// CHECK:  call i1 @dx.op.isSpecialFloat.f16(i32 11, half
export bool test_isnormal_half(half h) {
  return isnormal(h);
}

// CHECK-LABEL: test_isnormal_half2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 11, <2 x half>
export bool2 test_isnormal_half2(half2 h) {
  return isnormal(h);
}

// CHECK-LABEL: test_isnormal_half3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 11, <3 x half>
export bool3 test_isnormal_half3(half3 h) {
  return isnormal(h);
}

// CHECK-LABEL: test_isnormal_half4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 11, <4 x half>
export bool4 test_isnormal_half4(half4 h) {
  return isnormal(h);
}

// CHECK-LABEL: test_isnormal_float
// CHECK:  call i1 @dx.op.isSpecialFloat.f32(i32 11, float
export bool test_isnormal_float(float h) {
  return isnormal(h);
}

// CHECK-LABEL: test_isnormal_float2
// CHECK:  call <2 x i1> @dx.op.isSpecialFloat.v2f32(i32 11, <2 x float>
export bool2 test_isnormal_float2(float2 h) {
  return isnormal(h);
}

// CHECK-LABEL: test_isnormal_float3
// CHECK:  call <3 x i1> @dx.op.isSpecialFloat.v3f32(i32 11, <3 x float>
export bool3 test_isnormal_float3(float3 h) {
  return isnormal(h);
}

// CHECK-LABEL: test_isnormal_float4
// CHECK:  call <4 x i1> @dx.op.isSpecialFloat.v4f32(i32 11, <4 x float>
export bool4 test_isnormal_float4(float4 h) {
  return isnormal(h);
}

