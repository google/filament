; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

;
;  Modified base on output of
;
; SamplerComparisonState samp1;
; Texture2DArray<float4> tex2d_array;
; Texture2D<float4> tex2d;
; Texture2D<int4> tex2di;
; RWTexture2D<float4> u;
; float cmpVal;
; float bias;
; float2 ddx;
; float2 ddy;
; float clamp;
; float main(float4 a
;   : A) : SV_Target {
;   uint status;
;   float r = 0;  u[uint2(0,0)] = tex2di[uint2(0,0)];
;   r += tex2d.SampleCmpBias(samp1, a.xy, cmpVal, 1.5, uint2(-5, 7), clamp);
;   r += tex2d_array.SampleCmpGrad(samp1, a.xyz, cmpVal, ddx, ddy, uint2(-4, 1), 0.5f);
;   r += tex2d.CalculateLevelOfDetail(samp1, a.xy);
;   return r;
; }

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%dx.types.CBufRet.f32 = type { float, float, float, float }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%"class.Texture2DArray<vector<float, 4> >" = type { <4 x float>, %"class.Texture2DArray<vector<float, 4> >::mips_type" }
%"class.Texture2DArray<vector<float, 4> >::mips_type" = type { i32 }
%"class.Texture2D<vector<float, 4> >" = type { <4 x float>, %"class.Texture2D<vector<float, 4> >::mips_type" }
%"class.Texture2D<vector<float, 4> >::mips_type" = type { i32 }
%"class.Texture2D<vector<int, 4> >" = type { <4 x i32>, %"class.Texture2D<vector<int, 4> >::mips_type" }
%"class.Texture2D<vector<int, 4> >::mips_type" = type { i32 }
%"class.RWTexture2D<vector<float, 4> >" = type { <4 x float> }
%"$Globals" = type { float, float, <2 x float>, <2 x float>, float }
%struct.SamplerComparisonState = type { i32 }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 2, i32 2, i32 0, i8 0 }, i32 2, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %3 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 1, i32 1, i32 0, i8 0 }, i32 1, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %4 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %5 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %6 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 2 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %7 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %6, %dx.types.ResourceProperties { i32 13, i32 28 })  ; AnnotateHandle(res,props)  resource: CBuffer
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %9 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %10 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %11 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 3, i32 1028 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xI32>
  %12 = call %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32 66, %dx.types.Handle %11, i32 0, i32 0, i32 0, i32 undef, i32 undef, i32 undef, i32 undef)  ; TextureLoad(srv,mipLevelOrSampleCount,coord0,coord1,coord2,offset0,offset1,offset2)
  %13 = extractvalue %dx.types.ResRet.i32 %12, 0
  %14 = extractvalue %dx.types.ResRet.i32 %12, 1
  %15 = extractvalue %dx.types.ResRet.i32 %12, 2
  %16 = extractvalue %dx.types.ResRet.i32 %12, 3
  %17 = sitofp i32 %13 to float
  %18 = sitofp i32 %14 to float
  %19 = sitofp i32 %15 to float
  %20 = sitofp i32 %16 to float
  %21 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4098, i32 1033 })  ; AnnotateHandle(res,props)  resource: RWTexture2D<4xF32>
  call void @dx.op.textureStore.f32(i32 67, %dx.types.Handle %21, i32 0, i32 0, i32 undef, float %17, float %18, float %19, float %20, i8 15)  ; TextureStore(srv,coord0,coord1,coord2,value0,value1,value2,value3,mask)
  %22 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %7, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
  %23 = extractvalue %dx.types.CBufRet.f32 %22, 2
  %24 = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %7, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  %25 = extractvalue %dx.types.CBufRet.f32 %24, 0
  %26 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 2, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>
  %27 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 14, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState

; CHECK: bias amount for sample_b must be in the range [-16.000000,15.990000], but 150.000000 was specified as an immediate
; CHECK: sample_c_*/gather_c instructions require sampler declared in comparison mode
; CHECK: sample, lod and gather should be on srv resource

  %28 = call %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32 255, %dx.types.Handle %21, %dx.types.Handle %27, float %8, float %9, float undef, float undef, i32 -5, i32 7, i32 undef, float %25, float 1.500000e+2, float %23)  ; SampleCmpBias(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,bias,clamp)
  %29 = extractvalue %dx.types.ResRet.f32 %28, 0
  %30 = extractvalue %dx.types.CBufRet.f32 %22, 0
  %31 = extractvalue %dx.types.CBufRet.f32 %22, 1
  %32 = extractvalue %dx.types.CBufRet.f32 %24, 2
  %33 = extractvalue %dx.types.CBufRet.f32 %24, 3
  %34 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %4, %dx.types.ResourceProperties { i32 3, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2DArray<4xF32>

; CHECK: sample_c_*/gather_c instructions require sampler declared in comparison mode
; CHECK: sample_* instructions require resource to be declared to return UNORM, SNORM or FLOAT
; CHECK: samplec requires resource declared as texture1D/2D/Cube/1DArray/2DArray/CubeArray

  %35 = call %dx.types.ResRet.f32 @dx.op.sampleCmpGrad.f32(i32 254, %dx.types.Handle %11, %dx.types.Handle %27, float %8, float %9, float undef, float undef, i32 -4, i32 1, i32 undef, float %25, float %32, float %33, float undef, float %30, float %31, float undef, float 5.000000e-01)  ; SampleCmpGrad(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,ddx0,ddx1,ddx2,ddy0,ddy1,ddy2,clamp)
  %36 = extractvalue %dx.types.ResRet.f32 %35, 0
  %37 = fadd fast float %36, %29
  %38 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %3, %dx.types.ResourceProperties { i32 3, i32 1033 })  ; AnnotateHandle(res,props)  resource: Texture2D<4xF32>
  %39 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %5, %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState

; CHECK: lod requires resource declared as texture1D/2D/3D/Cube/CubeArray/1DArray/2DArray

  %40 = call float @dx.op.calculateLOD.f32(i32 81, %dx.types.Handle %38, %dx.types.Handle %39, float %8, float %9, float undef, i1 true)  ; CalculateLOD(handle,sampler,coord0,coord1,coord2,clamped)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.i32 @dx.op.textureLoad.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i32) #2

; Function Attrs: nounwind
declare void @dx.op.textureStore.f32(i32, %dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #1

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float, float, float) #2

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sampleCmpGrad.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float, float, float, float, float, float, float, float) #2

; Function Attrs: nounwind readonly
declare float @dx.op.calculateLOD.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, i1) #2

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, %dx.types.Handle, i32) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.viewIdState = !{!16}
!dx.entryPoints = !{!17}

!0 = !{!"dxc(private) 1.7.0.4396 (test_time, 849f8b884-dirty)"}
!1 = !{i32 1, i32 8}
!2 = !{!"ps", i32 6, i32 8}
!3 = !{!4, !10, !12, !14}
!4 = !{!5, !7, !8}
!5 = !{i32 0, %"class.Texture2DArray<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 7, i32 0, !6}
!6 = !{i32 0, i32 9}
!7 = !{i32 1, %"class.Texture2D<vector<float, 4> >"* undef, !"", i32 0, i32 1, i32 1, i32 2, i32 0, !6}
!8 = !{i32 2, %"class.Texture2D<vector<int, 4> >"* undef, !"", i32 0, i32 2, i32 1, i32 2, i32 0, !9}
!9 = !{i32 0, i32 4}
!10 = !{!11}
!11 = !{i32 0, %"class.RWTexture2D<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 2, i1 false, i1 false, i1 false, !6}
!12 = !{!13}
!13 = !{i32 0, %"$Globals"* undef, !"", i32 0, i32 0, i32 1, i32 28, null}
!14 = !{!15}
!15 = !{i32 0, %struct.SamplerComparisonState* undef, !"", i32 0, i32 0, i32 1, i32 0, null}
!16 = !{[6 x i32] [i32 4, i32 1, i32 1, i32 1, i32 1, i32 0]}
!17 = !{void ()* @main, !"main", !18, !3, !26}
!18 = !{!19, !23, null}
!19 = !{!20}
!20 = !{i32 0, !"A", i8 9, i8 0, !21, i8 2, i32 1, i8 4, i32 0, i8 0, !22}
!21 = !{i32 0}
!22 = !{i32 3, i32 7}
!23 = !{!24}
!24 = !{i32 0, !"SV_Target", i8 9, i8 16, !21, i8 0, i32 1, i8 1, i32 0, i8 0, !25}
!25 = !{i32 3, i32 1}
!26 = !{i32 0, i64 8589938688}

