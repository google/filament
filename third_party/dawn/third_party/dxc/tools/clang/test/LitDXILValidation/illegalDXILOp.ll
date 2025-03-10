; REQUIRES: dxil-1-8
; RUN: not %dxv %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%"class.Texture2D<float>" = type { float, %"class.Texture2D<float>::mips_type" }
%"class.Texture2D<float>::mips_type" = type { i32 }
%struct.SamplerComparisonState = type { i32 }


define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind zeroinitializer, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 3 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)


; CHECK: error: 'dx.op.loadInput.f32' is not a DXILOpFuncition for DXILOpcode 'LoadInput'.
; CHECK: note: at '%3 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0)' in block '#0' of function 'main'.

  %3 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)

; CHECK: error: 'dx.op.loadInput.f32' is not a DXILOpFuncition for DXILOpcode 'LoadInput'.
; CHECK: note: at '%4 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1)' in block '#0' of function 'main'.

  %4 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)


; CHECK: error: 'dx.op.annotateHandle' is not a DXILOpFuncition for DXILOpcode 'MinPrecXRegStore'.
; CHECK: note: at '%5 = call %dx.types.Handle @dx.op.annotateHandle(i32 3, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 2, i32 265 })' in block '#0' of function 'main'.

  %5 = call %dx.types.Handle @dx.op.annotateHandle(i32 3, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 2, i32 265 })  ; AnnotateHandle(res,props)  resource: Texture2D<F32>

; CHECK: error: 'dx.op.annotateHandle2' is not a DXILOpFuncition for DXILOpcode 'AnnotateHandle'.
; CHECK: note: at '%6 = call %dx.types.Handle @dx.op.annotateHandle2(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 32782, i32 0 })' in block '#0' of function 'main'.

  %6 = call %dx.types.Handle @dx.op.annotateHandle2(i32 216, %dx.types.Handle %2, %dx.types.ResourceProperties { i32 32782, i32 0 })  ; AnnotateHandle(res,props)  resource: SamplerComparisonState

; CHECK: error: DXILOpCode must be [0..{{[0-9]+}}].  1999981 specified.
; CHECK: note: at '%7 = call float @dx.op.calculateLOD.f32(i32 1999981, %dx.types.Handle %5, %dx.types.Handle %6, float %3, float %4, float undef, i1 true)' in block '#0' of function 'main'.

  %7 = call float @dx.op.calculateLOD.f32(i32 1999981, %dx.types.Handle %5, %dx.types.Handle %6, float %3, float %4, float undef, i1 true)  ; CalculateLOD(handle,sampler,coord0,coord1,coord2,clamped)

  %I = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)

; CHECK: error: Opcode of DXIL operation must be an immediate constant.
; CHECK: note: at 'call void @dx.op.storeOutput.f32(i32 %I, i32 0, i32 0, i8 0, float %7)' in block '#0' of function 'main'.
  call void @dx.op.storeOutput.f32(i32 %I, i32 0, i32 0, i8 0, float %7)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)


; CHECK-DAG: error: Opcode SampleCmpBias not valid in shader model ps_6_7.
  %CmpBias = call %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32 255, %dx.types.Handle %5, %dx.types.Handle %6, float %3, float %4, float undef, float undef, i32 0, i32 0, i32 undef, float 5.000000e-01, float 5.000000e-01, float undef)  ; SampleCmpBias(srv,sampler,coord0,coord1,coord2,coord3,offset0,offset1,offset2,compareValue,bias,clamp)

  ret void
}


; CHECK-DAG: error: DXIL intrinsic overload must be valid.
; CHECK-DAG: note: at '%4 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1)' in block '#0' of function 'main'.
; CHECK-DAG: error: DXIL intrinsic overload must be valid.
; CHECK-DAG: note: at '%3 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0)' in block '#0' of function 'main'.
; CHECK-DAG: error: DXIL intrinsic overload must be valid.
; CHECK-DAG: note: at '%5 = call %dx.types.Handle @dx.op.annotateHandle(i32 3, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 2, i32 265 })' in block '#0' of function 'main'.

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readonly
declare float @dx.op.calculateLOD.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, i1) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

declare %dx.types.Handle @dx.op.annotateHandle2(i32, %dx.types.Handle, %dx.types.ResourceProperties) #0

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #0

declare i32 @dx.op.loadInput.i32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.sampleCmpBias.f32(i32, %dx.types.Handle, %dx.types.Handle, float, float, float, float, i32, i32, i32, float, float, float) #2

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.viewIdState = !{!9}
!dx.entryPoints = !{!10}

!0 = !{!"dxc(private) 1.7.0.4396 (test_time, 849f8b884-dirty)"}
!1 = !{i32 1, i32 7}
!2 = !{!"ps", i32 6, i32 7}
!3 = !{!4, null, null, !7}
!4 = !{!5}
!5 = !{i32 0, %"class.Texture2D<float>"* undef, !"", i32 0, i32 0, i32 1, i32 2, i32 0, !6}
!6 = !{i32 0, i32 9}
!7 = !{!8}
!8 = !{i32 0, %struct.SamplerComparisonState* undef, !"", i32 0, i32 0, i32 1, i32 1, null}
!9 = !{[4 x i32] [i32 2, i32 1, i32 1, i32 1]}
!10 = !{void ()* @main, !"main", !11, !3, null}
!11 = !{!12, !16, null}
!12 = !{!13}
!13 = !{i32 0, !"A", i8 9, i8 0, !14, i8 2, i32 1, i8 2, i32 0, i8 0, !15}
!14 = !{i32 0}
!15 = !{i32 3, i32 3}
!16 = !{!17}
!17 = !{i32 0, !"SV_Target", i8 9, i8 16, !14, i8 0, i32 1, i8 1, i32 0, i8 0, !18}
!18 = !{i32 3, i32 1}
