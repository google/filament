// RUN: %dxc -Tps_6_8 %s | FileCheck %s

// CHECK: SampleCmp with gradient or bias
SamplerComparisonState samp1;

Texture2D<float4> tex2d;
Texture2DArray<float4> tex2d_array;
TextureCube<float4> texcube;
TextureCubeArray<float4> texcube_array;

float cmpVal;

float2 ddx;
float2 ddy;

float clamp;

// CHECK: define void @main()

// CHECK: %[[CubeArray:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 3, i32 3, i32 0, i8 0 }, i32 3, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
// CHECK: %[[Cube:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
// CHECK: %[[T2DArray:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
// CHECK: %[[T2D:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)


// CHECK: %[[Sampler:.+]] = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)

float main(float4 a
  : A) : SV_Target {
  uint status;
  float r = 0;

// CHECK: %[[AnnotCube:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Cube]], %dx.types.ResourceProperties { i32 5, i32 1033 })  ; AnnotateHandle(res,props)  resource: TextureCube<4xF32>
// CHECK: %[[AnnotSampler:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[Sampler]], %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState
// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpGrad.f32(i32 254, %dx.types.Handle %[[AnnotCube]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef, i32 undef, i32 undef, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef)  ; SampleCmpGrad(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,ddx0,ddx1,ddx2,ddy0,ddy1,ddy2,clamp)

  r += texcube.SampleCmpGrad(samp1, a.xyz, cmpVal, ddx.xxy, ddy.yyx);

// CHECK: %[[AnnotCubeArray:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[CubeArray]], %dx.types.ResourceProperties { i32 9, i32 1033 })  ; AnnotateHandle(res,props)  resource: TextureCubeArray<4xF32>
// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpGrad.f32(i32 254, %dx.types.Handle %[[AnnotCubeArray]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, i32 undef, i32 undef, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}}, float %{{.*}})  ; SampleCmpGrad(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,ddx0,ddx1,ddx2,ddy0,ddy1,ddy2,clamp)

  r += texcube_array.SampleCmpGrad(samp1, a.xyzw, cmpVal, ddx.xxy, ddy.yyx, clamp);

// CHECK: %[[AnnotT2DArray:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T2DArray]], %dx.types.ResourceProperties { i32 7, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2DArray<4xF32>
// CHECK: call %dx.types.ResRet.f32 @dx.op.sampleCmpGrad.f32(i32 254, %dx.types.Handle %[[AnnotT2DArray]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef, i32 -4, i32 1, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef, float %{{.*}}, float %{{.*}}, float undef, float 5.000000e-01)  ; SampleCmpGrad(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,ddx0,ddx1,ddx2,ddy0,ddy1,ddy2,clamp)

  r += tex2d_array.SampleCmpGrad(samp1, a.xyz, cmpVal, ddx, ddy, uint2(-4, 1), 0.5f);

// CHECK: %[[AnnotT2D:.+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %[[T2D]], %dx.types.ResourceProperties { i32 2, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>
// CHECK: %[[T2DStatus:.+]] = call %dx.types.ResRet.f32 @dx.op.sampleCmpGrad.f32(i32 254, %dx.types.Handle %[[AnnotT2D]], %dx.types.Handle %[[AnnotSampler]], float %{{.*}}, float %{{.*}}, float undef, float undef, i32 -3, i32 2, i32 undef, float %{{.*}}, float %{{.*}}, float %{{.*}}, float undef, float %{{.*}}, float %{{.*}}, float undef, float 0.000000e+00)  ; SampleCmpGrad(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,ddx0,ddx1,ddx2,ddy0,ddy1,ddy2,clamp)
// CHECK: extractvalue %dx.types.ResRet.f32 %[[T2DStatus]], 4

  r += tex2d.SampleCmpGrad(samp1, a.xy, cmpVal, ddx, ddy, uint2(-3, 2), 0.f, status);
  r += status;

  return r;
}

