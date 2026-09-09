; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: error: Opcode GetGroupWaveCount not valid in shader model ps_6_10.
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model ps_6_10.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @mainPS() {
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %2 = insertelement <4 x float> undef, float %1, i64 0
  %3 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %4 = insertelement <4 x float> %2, float %3, i64 1
  %5 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %6 = insertelement <4 x float> %4, float %5, i64 2
  %7 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %8 = insertelement <4 x float> %6, float %7, i64 3
  %9 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %10 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %11 = uitofp i32 %9 to float
  %12 = insertelement <4 x float> undef, float %11, i32 0
  %13 = shufflevector <4 x float> %12, <4 x float> undef, <4 x i32> zeroinitializer
  %14 = fadd fast <4 x float> %13, %8
  %15 = uitofp i32 %10 to float
  %16 = insertelement <4 x float> undef, float %15, i32 0
  %17 = shufflevector <4 x float> %16, <4 x float> undef, <4 x i32> zeroinitializer
  %18 = fadd fast <4 x float> %14, %17
  %19 = extractelement <4 x float> %18, i64 0
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %19)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %20 = extractelement <4 x float> %18, i64 1
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %20)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %21 = extractelement <4 x float> %18, i64 2
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %21)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %22 = extractelement <4 x float> %18, i64 3
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %22)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.getGroupWaveCount(i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.getGroupWaveIndex(i32) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.viewIdState = !{!3}
!dx.entryPoints = !{!4}

!0 = !{!"dxc(private) 1.9.0.5169 (Group-Wave-Intrinsics, deeac02f3-dirty)"}
!1 = !{i32 1, i32 10}
!2 = !{!"ps", i32 6, i32 10}
!3 = !{[6 x i32] [i32 4, i32 4, i32 15, i32 15, i32 15, i32 15]}
!4 = !{void ()* @mainPS, !"mainPS", !5, null, !12}
!5 = !{!6, !10, null}
!6 = !{!7}
!7 = !{i32 0, !"SV_Position", i8 9, i8 3, !8, i8 4, i32 1, i8 4, i32 0, i8 0, !9}
!8 = !{i32 0}
!9 = !{i32 3, i32 15}
!10 = !{!11}
!11 = !{i32 0, !"SV_Target", i8 9, i8 16, !8, i8 0, i32 1, i8 4, i32 0, i8 0, !9}
!12 = !{i32 0, i64 524288}