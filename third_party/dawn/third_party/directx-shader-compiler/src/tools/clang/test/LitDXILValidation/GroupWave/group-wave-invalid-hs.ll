; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s

; CHECK: error: Opcode GetGroupWaveCount not valid in shader model hs_6_10.
; CHECK: error: Opcode GetGroupWaveIndex not valid in shader model hs_6_10.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @"\01?PatchConstantFunc@@YA?AUHSConstantOutput@@V?$InputPatch@UHSInput@@$02@@I@Z"() {
  call void @dx.op.storePatchConstant.f32(i32 106, i32 0, i32 0, i8 0, float 1.000000e+00)  ; StorePatchConstant(outputSigID,row,col,value)
  call void @dx.op.storePatchConstant.f32(i32 106, i32 0, i32 1, i8 0, float 1.000000e+00)  ; StorePatchConstant(outputSigID,row,col,value)
  call void @dx.op.storePatchConstant.f32(i32 106, i32 0, i32 2, i8 0, float 1.000000e+00)  ; StorePatchConstant(outputSigID,row,col,value)
  call void @dx.op.storePatchConstant.f32(i32 106, i32 1, i32 0, i8 0, float 1.000000e+00)  ; StorePatchConstant(outputSigID,row,col,value)
  ret void
}

define void @mainHS() {
  %1 = call i32 @dx.op.outputControlPointID.i32(i32 107)  ; OutputControlPointID()
  %2 = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
  %3 = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 %1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %5 = insertelement <4 x float> undef, float %4, i64 0
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 %1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %7 = insertelement <4 x float> %5, float %6, i64 1
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 %1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %9 = insertelement <4 x float> %7, float %8, i64 2
  %10 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 %1)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %11 = insertelement <4 x float> %9, float %10, i64 3
  %12 = uitofp i32 %2 to float
  %13 = insertelement <4 x float> undef, float %12, i32 0
  %14 = uitofp i32 %3 to float
  %15 = insertelement <4 x float> undef, float %14, i32 0
  %16 = fadd <4 x float> %13, %15
  %17 = shufflevector <4 x float> %16, <4 x float> undef, <4 x i32> zeroinitializer
  %18 = fadd fast <4 x float> %11, %17
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
declare i32 @dx.op.outputControlPointID.i32(i32) #0

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind
declare void @dx.op.storePatchConstant.f32(i32, i32, i32, i8, float) #1

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
!2 = !{!"hs", i32 6, i32 10}
!3 = !{[11 x i32] [i32 4, i32 4, i32 15, i32 15, i32 15, i32 15, i32 13, i32 0, i32 0, i32 0, i32 0]}
!4 = !{void ()* @mainHS, !"mainHS", !5, null, !15}
!5 = !{!6, !6, !10}
!6 = !{!7}
!7 = !{i32 0, !"SV_Position", i8 9, i8 3, !8, i8 4, i32 1, i8 4, i32 0, i8 0, !9}
!8 = !{i32 0}
!9 = !{i32 3, i32 15}
!10 = !{!11, !14}
!11 = !{i32 0, !"SV_TessFactor", i8 9, i8 25, !12, i8 0, i32 3, i8 1, i32 0, i8 3, !13}
!12 = !{i32 0, i32 1, i32 2}
!13 = !{i32 3, i32 1}
!14 = !{i32 1, !"SV_InsideTessFactor", i8 9, i8 26, !8, i8 0, i32 1, i8 1, i32 3, i8 0, !13}
!15 = !{i32 0, i64 524288, i32 3, !16}
!16 = !{void ()* @"\01?PatchConstantFunc@@YA?AUHSConstantOutput@@V?$InputPatch@UHSInput@@$02@@I@Z", i32 3, i32 3, i32 2, i32 3, i32 3, float 6.400000e+01}