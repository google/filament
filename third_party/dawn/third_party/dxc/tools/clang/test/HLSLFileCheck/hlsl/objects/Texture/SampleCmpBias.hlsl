// RUN: %dxc -Tps_6_8 %s | FileCheck %s

// CHECK: SampleCmp with gradient or bias
SamplerComparisonState samp1;
Texture1D<float4> tex1d;
Texture1DArray<float4> tex1d_array;
Texture2D<float4> tex2d;

TextureCube<float4> texcube;

float cmpVal;
float bias;


float clamp;

// CHECK: define void @main()

// CHECK: %[[TCube:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 0 }, i32 3, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
// CHECK: %[[T2D:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
// CHECK: %[[T1DArray:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
// CHECK: %[[T1D:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)

// CHECK: %[[Sampler:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)

float main(float4 a
  : A) : SV_Target {
  uint status;
  float r = 0;

// CHECK: %[[AnnotT1D:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T1D]], %dx.types.ResourceProperties { i32 1, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture1D<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32 255, %dx.types.Handle %[[AnnotT1D]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float undef, float undef, float undef, i32 0, i32 undef, i32 undef, float %{{.*}}, float %{{.*}}, float undef)

  r += tex1d.SampleCmpBias(samp1, a.x, cmpVal, bias);

// CHECK: %[[AnnotT1DArray:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T1DArray]], %dx.types.ResourceProperties { i32 6, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture1DArray<4xF32>
// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32 255, %dx.types.Handle %[[AnnotT1DArray]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float %{{.*}}, float undef, float undef, i32 -5, i32 undef, i32 undef, float %{{.*}}, float %{{.*}}, float undef)  ; SampleCmpBias(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,bias,clamp)
  r += tex1d_array.SampleCmpBias(samp1, a.xy, cmpVal, bias, -5);

// CHECK: %[[AnnotT2D:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T2D]], %dx.types.ResourceProperties { i32 2, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>
// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32 255, %dx.types.Handle %[[AnnotT2D]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float %{{.*}}, float undef, float undef, i32 -5, i32 7, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}})  ; SampleCmpBias(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,bias,clamp)
  
  r += tex2d.SampleCmpBias(samp1, a.xy, cmpVal, bias, uint2(-5, 7), clamp);

// CHECK: %[[AnnotTCube:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[TCube]], %dx.types.ResourceProperties { i32 5, i32 1033 })  ; AnnotateHandle(res,props)  resource: TextureCube<4xF32>
// CHECK: %[[TCubeStatus:.+]] = call %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32 255, %dx.types.Handle %[[AnnotTCube]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef, i32 undef, i32 undef, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}})  ; SampleCmpBias(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,bias,clamp)
// CHECK: extractvalue %dx.types.ResRet.f32 %[[TCubeStatus]], 4

  r += texcube.SampleCmpBias(samp1, a.xyz, cmpVal, bias, clamp, status);
  r += status;


  return r;
}

