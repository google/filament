; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: error: Opcode GetGroupWaveCount not valid in shader model gs_6_10.
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model gs_6_10.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @mainGS() {
  %1 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %2 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  br label %4

; <label>:3                                       ; preds = %4
  call void @dx.op.cutStream(i32 98, i8 0)  ; CutStream(streamId)
  ret void

; <label>:4                                       ; preds = %4, %0
  %5 = phi i32 [ 0, %0 ], [ %25, %4 ]
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 %5)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %7 = insertelement <4 x float> undef, float %6, i64 0
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 %5)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %9 = insertelement <4 x float> %7, float %8, i64 1
  %10 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 %5)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %11 = insertelement <4 x float> %9, float %10, i64 2
  %12 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 %5)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %13 = insertelement <4 x float> %11, float %12, i64 3
  %14 = uitofp i32 %1 to float
  %15 = insertelement <4 x float> undef, float %14, i32 0
  %16 = uitofp i32 %2 to float
  %17 = insertelement <4 x float> undef, float %16, i32 0
  %18 = fadd <4 x float> %15, %17
  %19 = shufflevector <4 x float> %18, <4 x float> undef, <4 x i32> zeroinitializer
  %20 = fadd fast <4 x float> %13, %19
  %21 = extractelement <4 x float> %20, i64 0
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %21)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %22 = extractelement <4 x float> %20, i64 1
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %22)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %23 = extractelement <4 x float> %20, i64 2
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %23)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %24 = extractelement <4 x float> %20, i64 3
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %24)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.emitStream(i32 97, i8 0)  ; EmitStream(streamId)
  %25 = add nuw nsw i32 %5, 1
  %26 = icmp eq i32 %25, 3
  br i1 %26, label %3, label %4
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind
declare void @dx.op.cutStream(i32, i8) #1

; Function Attrs: nounwind
declare void @dx.op.emitStream(i32, i8) #1

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
!2 = !{!"gs", i32 6, i32 10}
!3 = !{[9 x i32] [i32 4, i32 4, i32 15, i32 15, i32 15, i32 15, i32 0, i32 0, i32 0]}
!4 = !{void ()* @mainGS, !"mainGS", !5, null, !10}
!5 = !{!6, !6, null}
!6 = !{!7}
!7 = !{i32 0, !"SV_Position", i8 9, i8 3, !8, i8 4, i32 1, i8 4, i32 0, i8 0, !9}
!8 = !{i32 0}
!9 = !{i32 3, i32 15}
!10 = !{i32 0, i64 524288, i32 1, !11}
!11 = !{i32 3, i32 3, i32 1, i32 5, i32 1}