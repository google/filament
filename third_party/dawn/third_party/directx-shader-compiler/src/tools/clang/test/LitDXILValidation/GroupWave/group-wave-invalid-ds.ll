; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: error: Opcode GetGroupWaveCount not valid in shader model ds_6_10.
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model ds_6_10.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @mainDS() {
  %1 = call float @dx.op.domainLocation.f32(i32 105, i8 0)  ; DomainLocation(component)
  %2 = call float @dx.op.domainLocation.f32(i32 105, i8 1)  ; DomainLocation(component)
  %3 = call float @dx.op.domainLocation.f32(i32 105, i8 2)  ; DomainLocation(component)
  %4 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %5 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 0)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %7 = insertelement <4 x float> undef, float %6, i64 0
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 0)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %9 = insertelement <4 x float> %7, float %8, i64 1
  %10 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 0)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %11 = insertelement <4 x float> %9, float %10, i64 2
  %12 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 0)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %13 = insertelement <4 x float> %11, float %12, i64 3
  %14 = insertelement <4 x float> undef, float %1, i32 0
  %15 = shufflevector <4 x float> %14, <4 x float> undef, <4 x i32> zeroinitializer
  %16 = fmul fast <4 x float> %13, %15
  %17 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %18 = insertelement <4 x float> undef, float %17, i64 0
  %19 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %20 = insertelement <4 x float> %18, float %19, i64 1
  %21 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %22 = insertelement <4 x float> %20, float %21, i64 2
  %23 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %24 = insertelement <4 x float> %22, float %23, i64 3
  %25 = insertelement <4 x float> undef, float %2, i32 0
  %26 = shufflevector <4 x float> %25, <4 x float> undef, <4 x i32> zeroinitializer
  %27 = fmul fast <4 x float> %24, %26
  %28 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 2)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %29 = insertelement <4 x float> undef, float %28, i64 0
  %30 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 2)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %31 = insertelement <4 x float> %29, float %30, i64 1
  %32 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 2)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %33 = insertelement <4 x float> %31, float %32, i64 2
  %34 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 2)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %35 = insertelement <4 x float> %33, float %34, i64 3
  %36 = insertelement <4 x float> undef, float %3, i32 0
  %37 = shufflevector <4 x float> %36, <4 x float> undef, <4 x i32> zeroinitializer
  %38 = fmul fast <4 x float> %35, %37
  %39 = uitofp i32 %4 to float
  %40 = insertelement <4 x float> undef, float %39, i32 0
  %41 = uitofp i32 %5 to float
  %42 = insertelement <4 x float> undef, float %41, i32 0
  %43 = fadd <4 x float> %40, %42
  %44 = shufflevector <4 x float> %43, <4 x float> undef, <4 x i32> zeroinitializer
  %45 = fadd fast <4 x float> %16, %44
  %46 = fadd fast <4 x float> %45, %27
  %47 = fadd fast <4 x float> %46, %38
  %48 = extractelement <4 x float> %47, i64 0
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %48)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %49 = extractelement <4 x float> %47, i64 1
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %49)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %50 = extractelement <4 x float> %47, i64 2
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %50)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %51 = extractelement <4 x float> %47, i64 3
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %51)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.domainLocation.f32(i32, i8) #0

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
!2 = !{!"ds", i32 6, i32 10}
!3 = !{[20 x i32] [i32 4, i32 4, i32 15, i32 15, i32 15, i32 15, i32 13, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0]}
!4 = !{void ()* @mainDS, !"mainDS", !5, null, !14}
!5 = !{!6, !6, !10}
!6 = !{!7}
!7 = !{i32 0, !"SV_Position", i8 9, i8 3, !8, i8 4, i32 1, i8 4, i32 0, i8 0, !9}
!8 = !{i32 0}
!9 = !{i32 3, i32 15}
!10 = !{!11, !13}
!11 = !{i32 0, !"SV_TessFactor", i8 9, i8 25, !12, i8 0, i32 3, i8 1, i32 0, i8 3, null}
!12 = !{i32 0, i32 1, i32 2}
!13 = !{i32 1, !"SV_InsideTessFactor", i8 9, i8 26, !8, i8 0, i32 1, i8 1, i32 3, i8 0, null}
!14 = !{i32 0, i64 524288, i32 2, !15}
!15 = !{i32 2, i32 3}