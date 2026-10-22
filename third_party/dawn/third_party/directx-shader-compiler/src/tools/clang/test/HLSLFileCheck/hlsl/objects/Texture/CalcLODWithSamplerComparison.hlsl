// RUN: %dxc -Tps_6_8 %s | FileCheck %s

SamplerComparisonState samp1;
Texture1D<float4> tex1d;
Texture1DArray<float4> tex1d_array;
Texture2D<float4> tex2d;
Texture2DArray<float4> tex2d_array;
TextureCube<float4> texcube;
TextureCubeArray<float4> texcube_array;

// CHECK: define void @main()

// CHECK: %[[TCubeArray:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 5, i32 5, i32 0, i8 0 }, i32 5, i1 false)
// CHECK: %[[TCube:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 4, i32 4, i32 0, i8 0 }, i32 4, i1 false)
// CHECK: %[[T2DArray:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 0 }, i32 3, i1 false)
// CHECK: %[[T2D:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)
// CHECK: %[[T1DArray:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)
// CHECK: %[[T1D:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)
// CHECK: %[[Sampler:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)

float main(float4 a
  : A) : SV_Target {
float r = 0;
// CHECK: %[[AnnotT1D:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T1D]], %dx.types.ResourceProperties { i32 1, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture1D<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call float @dx.op.calculateLOD.f32(i32 81, %dx.types.Handle %[[AnnotT1D]], %dx.types.Handle %[[AnnotSampler]], float %{{.+}}, float undef, float undef, i1 true)

  r += tex1d.CalculateLevelOfDetail(samp1, a.x);

// CHECK: %[[AnnotT1DArray:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T1DArray]], %dx.types.ResourceProperties { i32 6, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture1DArray<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call float @dx.op.calculateLOD.f32(i32 81, %dx.types.Handle %[[AnnotT1DArray]], %dx.types.Handle %[[AnnotSampler]], float %{{.+}}, float undef, float undef, i1 false)
  r += tex1d_array.CalculateLevelOfDetailUnclamped(samp1, a.x);

// CHECK: %[[AnnotT2D:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T2D]], %dx.types.ResourceProperties { i32 2, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call float @dx.op.calculateLOD.f32(i32 81, %dx.types.Handle %[[AnnotT2D]], %dx.types.Handle %[[AnnotSampler]], float %{{.+}}, float %{{.+}}, float undef, i1 true)

  r += tex2d.CalculateLevelOfDetail(samp1, a.xy);

// CHECK: %[[AnnotT2DArray:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T2DArray]], %dx.types.ResourceProperties { i32 7, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2DArray<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call float @dx.op.calculateLOD.f32(i32 81, %dx.types.Handle %[[AnnotT2DArray]], %dx.types.Handle %[[AnnotSampler]], float %{{.+}}, float %{{.+}}, float undef, i1 false)

  r += tex2d_array.CalculateLevelOfDetailUnclamped(samp1, a.xy);

// CHECK: %[[AnnotTCube:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[TCube]], %dx.types.ResourceProperties { i32 5, i32 1033 })  ; AnnotateHandle(res,props)  resource: TextureCube<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call float @dx.op.calculateLOD.f32(i32 81, %dx.types.Handle %[[AnnotTCube]], %dx.types.Handle %[[AnnotSampler]], float %{{.+}}, float %{{.+}}, float %{{.+}}, i1 true)

  r += texcube.CalculateLevelOfDetail(samp1, a.xyz);

// CHECK: %[[AnnotTCubeArray:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[TCubeArray]], %dx.types.ResourceProperties { i32 9, i32 1033 })  ; AnnotateHandle(res,props)  resource: TextureCubeArray<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call float @dx.op.calculateLOD.f32(i32 81, %dx.types.Handle %[[AnnotTCubeArray]], %dx.types.Handle %[[AnnotSampler]], float %{{.+}}, float %{{.+}}, float %{{.+}}, i1 false)

  r += texcube_array.CalculateLevelOfDetailUnclamped(samp1, a.xyz);

  return r;
}

